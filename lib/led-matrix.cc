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
#include "multiplex-mappers-internal.h"

// Leave this in here for a while. Setting things from old defines.
#if defined(ADAFRUIT_RGBMATRIX_HAT)
# warning "You are using an old way to select the Adafruit HAT by defining -DADAFRUIT_RGBMATRIX_HAT"
#  warning "The new way to do this is to set HARDWARE_DESC=adafruit-hat"
# warning "Check out https://github.com/hzeller/rpi-rgb-led-matrix#switch-the-pinout"
# undef DEFAULT_HARDWARE
# define DEFAULT_HARDWARE "adafruit-hat"
#endif

#if defined(ADAFRUIT_RGBMATRIX_HAT_PWM)
#  warning "You are using an old way to select the Adafruit HAT with flicker mod by defining -DADAFRUIT_RGBMATRIX_HAT_PWM"
#  warning "The new way to do this is to set HARDWARE_DESC=adafruit-hat-pwm"
# undef DEFAULT_HARDWARE
# define DEFAULT_HARDWARE "adafruit-hat-pwm"
#endif

namespace rgb_matrix {
using namespace internal;

// Pump pixels to screen. Needs to be high priority real-time because jitter
class RGBMatrix::UpdateThread : public Thread {
public:
  UpdateThread(GPIO *io, FrameCanvas *initial_frame,
               int pwm_dither_bits, bool show_refresh)
    : io_(io), show_refresh_(show_refresh), running_(true),
      current_frame_(initial_frame), next_frame_(NULL),
      requested_frame_multiple_(1) {
    pthread_cond_init(&frame_done_, NULL);
    pthread_cond_init(&input_change_, NULL);
    switch (pwm_dither_bits) {
    case 0:
      start_bit_[0] = 0; start_bit_[1] = 0;
      start_bit_[2] = 0; start_bit_[3] = 0;
      break;
    case 1:
      start_bit_[0] = 0; start_bit_[1] = 1;
      start_bit_[2] = 0; start_bit_[3] = 1;
      break;
    case 2:
      start_bit_[0] = 0; start_bit_[1] = 1;
      start_bit_[2] = 2; start_bit_[3] = 2;
      break;
    }
  }

  void Stop() {
    MutexLock l(&running_mutex_);
    running_ = false;
  }

  virtual void Run() {
    unsigned frame_count = 0;
    unsigned low_bit_sequence = 0;
    uint32_t largest_time = 0;
    uint32_t last_gpio_bits = 0;

    // Let's start measure max time only after a we were running for a few
    // seconds to not pick up start-up glitches.
    static const int kHoldffTimeUs = 2000 * 1000;
    uint32_t initial_holdoff_start = GetMicrosecondCounter();
    bool max_measure_enabled = false;

    while (running()) {
      const uint32_t start_time_us = GetMicrosecondCounter();

      current_frame_->framebuffer()
        ->DumpToMatrix(io_, start_bit_[low_bit_sequence % 4]);

      // SwapOnVSync() exchange.
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

      // Read input bits.
      const uint32_t inputs = io_->Read();
      if (inputs != last_gpio_bits) {
        last_gpio_bits = inputs;
        MutexLock l(&input_sync_);
        gpio_inputs_ = inputs;
        pthread_cond_signal(&input_change_);
      }

      ++frame_count;
      ++low_bit_sequence;

#ifdef FIXED_FRAME_MICROSECONDS
      while ((GetMicrosecondCounter() - start_time_us) < (uint32_t)FIXED_FRAME_MICROSECONDS) {
        // busy wait.
      }
#endif
      const uint32_t end_time_us = GetMicrosecondCounter();
      if (show_refresh_) {
        uint32_t usec = end_time_us - start_time_us;
        printf("\b\b\b\b\b\b\b\b%6.1fHz", 1e6 / usec);
        if (usec > largest_time && max_measure_enabled) {
          largest_time = usec;
          printf(" max: %uusec\b\b\b\b\b\b\b\b\b\b\b\b\b\b", largest_time);
        } else {
          max_measure_enabled = (end_time_us - initial_holdoff_start) > kHoldffTimeUs;
        }
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

  uint32_t AwaitInputChange(int timeout_ms) {
    MutexLock l(&input_sync_);
    input_sync_.WaitOn(&input_change_, timeout_ms);
    return gpio_inputs_;
  }

private:
  inline bool running() {
    MutexLock l(&running_mutex_);
    return running_;
  }

  GPIO *const io_;
  const bool show_refresh_;
  uint32_t start_bit_[4];

  Mutex running_mutex_;
  bool running_;

  Mutex input_sync_;
  pthread_cond_t input_change_;
  uint32_t gpio_inputs_;

  Mutex frame_sync_;
  pthread_cond_t frame_done_;
  FrameCanvas *current_frame_;
  FrameCanvas *next_frame_;
  unsigned requested_frame_multiple_;
};

// Some defaults. See options-initialize.cc for the command line parsing.
RGBMatrix::Options::Options() :
  // Historically, we provided these options only as #defines. Make sure that
  // things still behave as before if someone has set these.
  // At some point: remove them from the Makefile. Later: remove them here.
#ifdef DEFAULT_HARDWARE
  hardware_mapping(DEFAULT_HARDWARE),
#else
  hardware_mapping("regular"),
#endif

  rows(32), cols(32), chain_length(1), parallel(1), pwm_bits(11),

#ifdef LSB_PWM_NANOSECONDS
    pwm_lsb_nanoseconds(LSB_PWM_NANOSECONDS),
#else
    pwm_lsb_nanoseconds(130),
#endif

  pwm_dither_bits(0),
  brightness(100),

#ifdef RGB_SCAN_INTERLACED
    scan_mode(1),
#else
    scan_mode(0),
#endif

  row_address_type(0),
  multiplexing(0),

#ifdef DISABLE_HARDWARE_PULSES
    disable_hardware_pulsing(true),
#else
    disable_hardware_pulsing(false),
#endif

#ifdef SHOW_REFRESH_RATE
    show_refresh_rate(true),
#else
    show_refresh_rate(false),
#endif

#ifdef INVERSE_RGB_DISPLAY_COLORS
    inverse_colors(true),
#else
    inverse_colors(false),
#endif
  led_rgb_sequence("RGB"),
  pixel_mapper_config(NULL),
  panel_type(NULL)
{
  // Nothing to see here.
}

RGBMatrix::RGBMatrix(GPIO *io, const Options &options)
  : params_(options), io_(NULL), updater_(NULL), shared_pixel_mapper_(NULL) {
  assert(params_.Validate(NULL));
  const MultiplexMapper *multiplex_mapper = NULL;
  if (params_.multiplexing > 0) {
    const MuxMapperList &multiplexers = GetRegisteredMultiplexMappers();
    if (params_.multiplexing <= (int) multiplexers.size()) {
      // TODO: we could also do a find-by-name here, but not sure if worthwhile
      multiplex_mapper = multiplexers[params_.multiplexing - 1];
    }
  }

  if (multiplex_mapper) {
    // The multiplexers might choose to have a different physical layout.
    // We need to configure that first before setting up the hardware.
    multiplex_mapper->EditColsRows(&params_.cols, &params_.rows);
  }

  Framebuffer::InitHardwareMapping(params_.hardware_mapping);

  active_ = CreateFrameCanvas();
  Clear();
  SetGPIO(io, true);

  // We need to apply the mapping for the panels first.
  ApplyPixelMapper(multiplex_mapper);

  // .. followed by higher level mappers that might arrange panels.
  ApplyNamedPixelMappers(options.pixel_mapper_config,
                         params_.chain_length, params_.parallel);
}

RGBMatrix::RGBMatrix(GPIO *io, int rows, int chained_displays,
                     int parallel_displays)
  : params_(Options()), io_(NULL), updater_(NULL), shared_pixel_mapper_(NULL) {
  params_.rows = rows;
  params_.chain_length = chained_displays;
  params_.parallel = parallel_displays;
  assert(params_.Validate(NULL));
  Framebuffer::InitHardwareMapping(params_.hardware_mapping);
  active_ = CreateFrameCanvas();
  Clear();
  SetGPIO(io, true);
}

RGBMatrix::~RGBMatrix() {
  if (updater_) {
    updater_->Stop();
    updater_->WaitStopped();
  }
  delete updater_;

  // Make sure LEDs are off.
  active_->Clear();
  if (io_) active_->framebuffer()->DumpToMatrix(io_, 0);

  for (size_t i = 0; i < created_frames_.size(); ++i) {
    delete created_frames_[i];
  }
  delete shared_pixel_mapper_;
}

void RGBMatrix::ApplyNamedPixelMappers(const char *pixel_mapper_config,
                                       int chain, int parallel) {
  if (pixel_mapper_config == NULL || strlen(pixel_mapper_config) == 0)
    return;
  char *const writeable_copy = strdup(pixel_mapper_config);
  const char *const end = writeable_copy + strlen(writeable_copy);
  char *s = writeable_copy;
  while (s < end) {
    char *const semicolon = strchrnul(s, ';');
    *semicolon = '\0';
    char *optional_param_start = strchr(s, ':');
    if (optional_param_start) {
      *optional_param_start++ = '\0';
    }
    if (*s == '\0' && optional_param_start && *optional_param_start != '\0') {
      fprintf(stderr, "Stray parameter ':%s' without mapper name ?\n", optional_param_start);
    }
    if (*s) {
      ApplyPixelMapper(FindPixelMapper(s, chain, parallel, optional_param_start));
    }
    s = semicolon + 1;
  }
  free(writeable_copy);
}

void RGBMatrix::SetGPIO(GPIO *io, bool start_thread) {
  if (io != NULL && io_ == NULL) {
    io_ = io;
    Framebuffer::InitGPIO(io_, params_.rows, params_.parallel,
                          !params_.disable_hardware_pulsing,
                          params_.pwm_lsb_nanoseconds, params_.pwm_dither_bits,
                          params_.row_address_type);
    Framebuffer::InitializePanels(io_, params_.panel_type,
                                  params_.cols * params_.chain_length);
  }
  if (start_thread) {
    StartRefresh();
  }
}

bool RGBMatrix::StartRefresh() {
  if (updater_ == NULL && io_ != NULL) {
    updater_ = new UpdateThread(io_, active_, params_.pwm_dither_bits,
                                params_.show_refresh_rate);
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
    new FrameCanvas(new Framebuffer(params_.rows,
                                    params_.cols * params_.chain_length,
                                    params_.parallel,
                                    params_.scan_mode,
                                    params_.led_rgb_sequence,
                                    params_.inverse_colors,
                                    &shared_pixel_mapper_));
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

uint32_t RGBMatrix::AwaitInputChange(int timeout_ms) {
  if (!updater_) return 0;
  return updater_->AwaitInputChange(timeout_ms);
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
  for (size_t i = 0; i < created_frames_.size(); ++i) {
    created_frames_[i]->framebuffer()->SetBrightness(brightness);
  }
  params_.brightness = brightness;
}

uint8_t RGBMatrix::brightness() {
  return params_.brightness;
}

// -- Implementation of RGBMatrix Canvas: delegation to ContentBuffer
int RGBMatrix::width() const {
  return active_->width();
}

int RGBMatrix::height() const {
  return active_->height();
}

void RGBMatrix::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  active_->SetPixel(x, y, red, green, blue);
}

void RGBMatrix::Clear() {
  active_->Clear();
}

void RGBMatrix::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  active_->Fill(red, green, blue);
}

bool RGBMatrix::ApplyPixelMapper(const PixelMapper *mapper) {
  if (mapper == NULL) return true;
  using internal::PixelDesignatorMap;
  const int old_width = shared_pixel_mapper_->width();
  const int old_height = shared_pixel_mapper_->height();
  int new_width, new_height;
  if (!mapper->GetSizeMapping(old_width, old_height, &new_width, &new_height)) {
    return false;
  }
  PixelDesignatorMap *new_mapper = new PixelDesignatorMap(
    new_width, new_height, shared_pixel_mapper_->GetFillColorBits());
  for (int y = 0; y < new_height; ++y) {
    for (int x = 0; x < new_width; ++x) {
      int orig_x = -1, orig_y = -1;
      mapper->MapVisibleToMatrix(old_width, old_height,
                                 x, y, &orig_x, &orig_y);
      if (orig_x < 0 || orig_y < 0 ||
          orig_x >= old_width || orig_y >= old_height) {
        fprintf(stderr, "Error in PixelMapper: (%d, %d) -> (%d, %d) [range: "
                "%dx%d]\n", x, y, orig_x, orig_y, old_width, old_height);
        continue;
      }
      const internal::PixelDesignator *orig_designator;
      orig_designator = shared_pixel_mapper_->get(orig_x, orig_y);
      *new_mapper->get(x, y) = *orig_designator;
    }
  }
  delete shared_pixel_mapper_;
  shared_pixel_mapper_ = new_mapper;
  return true;
}

#ifndef REMOVE_DEPRECATED_TRANSFORMERS
namespace {
// A pixel mapper
class PixelMapExtractionCanvas : public Canvas {
public:
  PixelMapExtractionCanvas(internal::PixelDesignatorMap *old_mapper)
    : old_mapper_(old_mapper), new_mapper_(NULL) {}

  virtual int width() const { return old_mapper_->width(); }
  virtual int height() const { return old_mapper_->height(); }

  void SetNewMapper(internal::PixelDesignatorMap *new_mapper) {
    new_mapper_ = new_mapper;
  }
  void SetNewLocation(int x, int y) {
    x_new = x;
    y_new = y;
  }
  virtual void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    const internal::PixelDesignator *orig_designator = old_mapper_->get(x, y);
    if (orig_designator && new_mapper_) {
      // Tell the new mapper at the new location what after the mapping was
      // at the old location.
      *new_mapper_->get(x_new, y_new) = *orig_designator;
    }
  }

  virtual void Clear() {}
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue) {}

private:
  internal::PixelDesignatorMap *const old_mapper_;
  internal::PixelDesignatorMap *new_mapper_;
  int x_new, y_new;
};
}  // anonymous namespace

void RGBMatrix::ApplyStaticTransformerDeprecated(
  const CanvasTransformer &transformer) {
  using internal::PixelDesignatorMap;
  assert(shared_pixel_mapper_);  // Not initialized yet ?
  PixelMapExtractionCanvas extractor_canvas(shared_pixel_mapper_);

  // These transformers traditionally only a non-const Transform()
  // method, so that they can modify an instance variable keeping the delegate.
  //
  // We can't really change that now as we want to be backwards compatible.
  //
  // Having a const-reference as parameter to ApplyStaticTransformer() however
  // is somewhat neat, so that it is possible to pass ad-hoc instances to
  // ApplyStaticTransformer() (which are discarded after that all anyway).
  //
  // So we're slightly naughty here and cast the const away.
  CanvasTransformer *non_const_transformer
    = const_cast<CanvasTransformer*>(&transformer);
  Canvas *mapped_canvas = non_const_transformer->Transform(&extractor_canvas);

  const int new_width = mapped_canvas->width();
  const int new_height = mapped_canvas->height();
  PixelDesignatorMap *new_mapper = new PixelDesignatorMap(
    new_width, new_height, shared_pixel_mapper_->GetFillColorBits());
  extractor_canvas.SetNewMapper(new_mapper);
  // Learn about the pixel mapping by going through all transformed pixels and
  // build new PixelDesignator map.
  for (int y = 0; y < new_height; ++y) {
    for (int x = 0; x < new_width; ++x) {
      extractor_canvas.SetNewLocation(x, y);
      mapped_canvas->SetPixel(x, y, 0, 0, 0); // force copy of designator.
    }
  }
  delete shared_pixel_mapper_;
  shared_pixel_mapper_ = new_mapper;
}
#endif  // REMOVE_DEPRECATED_TRANSFORMERS

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

void FrameCanvas::Serialize(const char **data, size_t *len) const {
  frame_->Serialize(data, len);
}
bool FrameCanvas::Deserialize(const char *data, size_t len) {
  return frame_->Deserialize(data, len);
}
void FrameCanvas::CopyFrom(const FrameCanvas &other) {
  frame_->CopyFrom(other.frame_);
}
}  // end namespace rgb_matrix
