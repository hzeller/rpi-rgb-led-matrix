// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "led-matrix.h"

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>

#include "gpio.h"
#include "thread.h"
#include "framebuffer-internal.h"

namespace rgb_matrix {

namespace {
class NullTransformer : public CanvasTransformer {
public:
  virtual Canvas *Transform(Canvas *output) { return output; }
};
}  // anonymous namespace

// Pump pixels to screen. Needs to be high priority real-time because jitter
class RGBMatrix::UpdateThread : public Thread {
public:
  UpdateThread(GPIO *io, FrameCanvas *initial_frame, bool show_refresh)
    : io_(io), show_refresh_(show_refresh), running_(true),
      current_frame_(initial_frame), next_frame_(NULL),
      requested_frame_multiple_(1) {
    pthread_cond_init(&frame_done_, NULL);
  }

  void Stop() {
    MutexLock l(&running_mutex_);
    running_ = false;
  }

  virtual void Run() {
    unsigned frame_count = 0;
    while (running()) {
      struct timeval start, end;
      if (show_refresh_) {
        gettimeofday(&start, NULL);
      }

      current_frame_->framebuffer()->DumpToMatrix(io_);

      {
        MutexLock l(&frame_sync_);
        // Do fast equality test first (likely due to frame_count reset).
        if (frame_count == requested_frame_multiple_
            || frame_count % requested_frame_multiple_ == 0) {
          // We reset to avoid frame hick-up every couple of weeks
          // run-time iff requested_frame_multiple_ is not a factor of 2^32.
          frame_count = 0;
          if (next_frame_ != NULL) {
            current_frame_ = next_frame_;
            next_frame_ = NULL;
          }
          pthread_cond_signal(&frame_done_);
        }
      }

      ++frame_count;

      if (show_refresh_) {
        gettimeofday(&end, NULL);
        int64_t usec = ((uint64_t)end.tv_sec * 1000000 + end.tv_usec)
          - ((int64_t)start.tv_sec * 1000000 + start.tv_usec);
        printf("\b\b\b\b\b\b\b\b%6.1fHz", 1e6 / usec);
      }
    }
  }

  FrameCanvas *SwapOnVSync(FrameCanvas *other, unsigned frame_fraction) {
    MutexLock l(&frame_sync_);
    FrameCanvas *previous = current_frame_;
    next_frame_ = other;
    requested_frame_multiple_ = frame_fraction;
    frame_sync_.WaitOn(&frame_done_);
    return previous;
  }

private:
  inline bool running() {
    MutexLock l(&running_mutex_);
    return running_;
  }

  GPIO *const io_;
  const bool show_refresh_;
  Mutex running_mutex_;
  bool running_;

  Mutex frame_sync_;
  pthread_cond_t frame_done_;
  FrameCanvas *current_frame_;
  FrameCanvas *next_frame_;
  unsigned requested_frame_multiple_;
};

// Some defaults. See options-initialize.cc for the command line parsing.
RGBMatrix::Options::Options()
  : rows(32), chain_length(1), parallel(1), pwm_bits(11),
    // Historically, we provided these options only as #defines. Make sure that
    // things still behave as before if someone has set these.
    // At some point: remove them from the Makefile. Later: remove them here.
#ifdef LSB_PWM_NANOSECONDS
    pwm_lsb_nanoseconds(LSB_PWM_NANOSECONDS),
#else
    pwm_lsb_nanoseconds(130),
#endif

#ifdef DISABLE_HARDWARE_PULSES
    allow_hardware_pulsing(false),
#else
    allow_hardware_pulsing(true),
#endif

    brightness(100),

#ifdef RGB_SCAN_INTERLACED
    scan_mode(1),
#else
    scan_mode(0),
#endif

#ifdef SHOW_REFRESH_RATE
    show_refresh_rate(true),
#else
    show_refresh_rate(false),
#endif

#ifdef RGB_SWAP_GREEN_BLUE
    swap_green_blue(true),
#else
    swap_green_blue(false),
#endif

#ifdef INVERSE_RGB_DISPLAY_COLORS
    inverse_colors(true)
#else
    inverse_colors(false)
#endif
{
  // Nothing to see here.
}

RGBMatrix::RGBMatrix(GPIO *io, const Options &options)
  : params_(options), io_(NULL), updater_(NULL) {
  assert(params_.Validate(NULL));
  SetTransformer(NULL);
  active_ = CreateFrameCanvas();
  Clear();
  SetGPIO(io, true);
}

RGBMatrix::RGBMatrix(GPIO *io, int rows, int chained_displays,
                     int parallel_displays)
  : params_(Options()), io_(NULL), updater_(NULL) {
  assert(params_.Validate(NULL));
  SetTransformer(NULL);
  active_ = CreateFrameCanvas();
  Clear();
  SetGPIO(io, true);
}

RGBMatrix::~RGBMatrix() {
  updater_->Stop();
  updater_->WaitStopped();
  delete updater_;

  // Make sure LEDs are off.
  active_->Clear();
  active_->framebuffer()->DumpToMatrix(io_);

  for (size_t i = 0; i < created_frames_.size(); ++i) {
    delete created_frames_[i];
  }
}

void RGBMatrix::SetGPIO(GPIO *io, bool start_thread) {
  if (io != NULL && io_ == NULL) {
    io_ = io;
    internal::Framebuffer::InitGPIO(io_, params_.rows, params_.parallel,
                                    params_.allow_hardware_pulsing,
                                    params_.pwm_lsb_nanoseconds);
  }
  if (start_thread) {
    StartRefresh();
  }
}

bool RGBMatrix::StartRefresh() {
  if (updater_ == NULL && io_ != NULL) {
    updater_ = new UpdateThread(io_, active_, params_.show_refresh_rate);
    // If we have multiple processors, the kernel
    // jumps around between these, creating some global flicker.
    // So let's tie it to the last CPU available.
    // The Raspberry Pi2 has 4 cores, our attempt to bind it to
    //   core #3 will succeed.
    // The Raspberry Pi1 only has one core, so this affinity
    //   call will simply fail and we keep using the only core.
    updater_->Start(99, (1<<3));  // Prio: high. Also: put on last CPU.
  }
  return updater_ != NULL;
}

FrameCanvas *RGBMatrix::CreateFrameCanvas() {
  FrameCanvas *result =
    new FrameCanvas(new internal::Framebuffer(params_.rows,
                                              32 * params_.chain_length,
                                              params_.parallel,
                                              params_.scan_mode,
                                              params_.swap_green_blue,
                                              params_.inverse_colors));
  if (created_frames_.empty()) {
    // First time. Get defaults from initial Framebuffer.
    do_luminance_correct_ = result->framebuffer()->luminance_correct();
  }

  result->framebuffer()->SetPWMBits(params_.pwm_bits);
  result->framebuffer()->set_luminance_correct(do_luminance_correct_);
  result->framebuffer()->SetBrightness(params_.brightness);

  created_frames_.push_back(result);
  return result;
}

FrameCanvas *RGBMatrix::SwapOnVSync(FrameCanvas *other,
                                    unsigned frame_fraction) {
  if (frame_fraction == 0) frame_fraction = 1; // correct user error.
  FrameCanvas *const previous = updater_->SwapOnVSync(other, frame_fraction);
  if (other) active_ = other;
  return previous;
}

void RGBMatrix::SetTransformer(CanvasTransformer *transformer) {
  if (transformer == NULL) {
    static NullTransformer null_transformer;   // global instance sufficient.
    transformer_ = &null_transformer;
  } else {
    transformer_ = transformer;
  }
}

bool RGBMatrix::SetPWMBits(uint8_t value) {
  const bool success = active_->framebuffer()->SetPWMBits(value);
  if (success) {
    params_.pwm_bits = value;
  }
  return success;
}
uint8_t RGBMatrix::pwmbits() { return params_.pwm_bits; }

// Map brightness of output linearly to input with CIE1931 profile.
void RGBMatrix::set_luminance_correct(bool on) {
  active_->framebuffer()->set_luminance_correct(on);
  do_luminance_correct_ = on;
}
bool RGBMatrix::luminance_correct() const {
  return do_luminance_correct_;
}

void RGBMatrix::SetBrightness(uint8_t brightness) {
  active_->framebuffer()->SetBrightness(brightness);
  params_.brightness = brightness;
}

uint8_t RGBMatrix::brightness() {
  return params_.brightness;
}

// -- Implementation of RGBMatrix Canvas: delegation to ContentBuffer
int RGBMatrix::width() const {
  return transformer_->Transform(active_)->width();
}

int RGBMatrix::height() const {
  return transformer_->Transform(active_)->height();
}

void RGBMatrix::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  transformer_->Transform(active_)->SetPixel(x, y, red, green, blue);
}

void RGBMatrix::Clear() {
  transformer_->Transform(active_)->Clear();
}

void RGBMatrix::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  transformer_->Transform(active_)->Fill(red, green, blue);
}

// FrameCanvas implementation of Canvas
FrameCanvas::~FrameCanvas() { delete frame_; }
int FrameCanvas::width() const { return frame_->width(); }
int FrameCanvas::height() const { return frame_->height(); }
void FrameCanvas::SetPixel(int x, int y,
                         uint8_t red, uint8_t green, uint8_t blue) {
  frame_->SetPixel(x, y, red, green, blue);
}
void FrameCanvas::Clear() { return frame_->Clear(); }
void FrameCanvas::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  frame_->Fill(red, green, blue);
}
bool FrameCanvas::SetPWMBits(uint8_t value) { return frame_->SetPWMBits(value); }
uint8_t FrameCanvas::pwmbits() { return frame_->pwmbits(); }

// Map brightness of output linearly to input with CIE1931 profile.
void FrameCanvas::set_luminance_correct(bool on) { frame_->set_luminance_correct(on); }
bool FrameCanvas::luminance_correct() const { return frame_->luminance_correct(); }

void FrameCanvas::SetBrightness(uint8_t brightness) { frame_->SetBrightness(brightness); }
uint8_t FrameCanvas::brightness() { return frame_->brightness(); }

}  // end namespace rgb_matrix
