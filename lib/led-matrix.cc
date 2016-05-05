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

#ifdef SHOW_REFRESH_RATE
# include <stdio.h>
# include <sys/time.h>
#endif

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
  UpdateThread(GPIO *io, FrameCanvas *initial_frame)
    : io_(io), running_(true),
      current_frame_(initial_frame), next_frame_(NULL) {
    pthread_cond_init(&frame_done_, NULL);
  }

  void Stop() {
    MutexLock l(&running_mutex_);
    running_ = false;
  }

  virtual void Run() {
    while (running()) {
#ifdef SHOW_REFRESH_RATE
      struct timeval start, end;
      gettimeofday(&start, NULL);
#endif

      current_frame_->framebuffer()->DumpToMatrix(io_);

      {
        MutexLock l(&frame_sync_);
        if (next_frame_ != NULL) {
          current_frame_ = next_frame_;
          next_frame_ = NULL;
        }
        pthread_cond_signal(&frame_done_);
      }

#ifdef SHOW_REFRESH_RATE
      gettimeofday(&end, NULL);
      int64_t usec = ((uint64_t)end.tv_sec * 1000000 + end.tv_usec)
        - ((int64_t)start.tv_sec * 1000000 + start.tv_usec);
      printf("\b\b\b\b\b\b\b\b%6.1fHz", 1e6 / usec);
#endif
    }
  }

  FrameCanvas *SwapOnVSync(FrameCanvas *other) {
    MutexLock l(&frame_sync_);
    FrameCanvas *previous = current_frame_;
    next_frame_ = other;
    frame_sync_.WaitOn(&frame_done_);
    return previous;
  }

private:
  inline bool running() {
    MutexLock l(&running_mutex_);
    return running_;
  }

  GPIO *const io_;
  Mutex running_mutex_;
  bool running_;

  Mutex frame_sync_;
  pthread_cond_t frame_done_;
  FrameCanvas *current_frame_;
  FrameCanvas *next_frame_;
};

RGBMatrix::RGBMatrix(GPIO *io, int rows, int chained_displays,
                     int parallel_displays)
  : rows_(rows), chained_displays_(chained_displays),
    parallel_displays_(parallel_displays),
    io_(NULL), updater_(NULL) {
  SetTransformer(NULL);
  active_ = CreateFrameCanvas();
  Clear();
  SetGPIO(io);
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
    internal::Framebuffer::InitGPIO(io_, rows_, parallel_displays_);
  }
  if (start_thread && updater_ == NULL) {
    assert(io_ != NULL);  // Starting thread: need GPIO
    updater_ = new UpdateThread(io_, active_);
    // If we have multiple processors, the kernel
    // jumps around between these, creating some global flicker.
    // So let's tie it to the last CPU available.
    // The Raspberry Pi2 has 4 cores, our attempt to bind it to
    //   core #3 will succeed.
    // The Raspberry Pi1 only has one core, so this affinity
    //   call will simply fail and we keep using the only core.
    updater_->Start(99, (1<<3));  // Prio: high. Also: put on last CPU.
  }
}

FrameCanvas *RGBMatrix::CreateFrameCanvas() {
  FrameCanvas *result =
    new FrameCanvas(new internal::Framebuffer(rows_, 32 * chained_displays_,
                                              parallel_displays_));
  if (created_frames_.empty()) {
    // First time. Get defaults from initial Framebuffer.
    pwm_bits_ = result->framebuffer()->pwmbits();
    do_luminance_correct_ = result->framebuffer()->luminance_correct();
    brightness_ = result->framebuffer()->brightness();
  } else {
    result->framebuffer()->SetPWMBits(pwm_bits_);
    result->framebuffer()->set_luminance_correct(do_luminance_correct_);
    result->framebuffer()->SetBrightness(brightness_);
  }
  created_frames_.push_back(result);
  return result;
}

FrameCanvas *RGBMatrix::SwapOnVSync(FrameCanvas *other) {
  FrameCanvas *const previous = updater_->SwapOnVSync(other);
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
    pwm_bits_ = value;
  }
  return success;
}
uint8_t RGBMatrix::pwmbits() { return pwm_bits_; }

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
  brightness_ = brightness;
}

uint8_t RGBMatrix::brightness() {
  return brightness_;
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
