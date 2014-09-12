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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define SHOW_REFRESH_RATE 0

#if SHOW_REFRESH_RATE
# include <stdio.h>
# include <sys/time.h>
#endif

#include "gpio.h"
#include "thread.h"

enum {
  kBitPlanes = 11  // maximum usable bitplanes.
};

static const long kBaseTimeNanos = 200;
const long row_sleep_nanos[11] = {
  (1 * kBaseTimeNanos),
  (2 * kBaseTimeNanos),
  (4 * kBaseTimeNanos),
  (8 * kBaseTimeNanos),
  (16 * kBaseTimeNanos),
  (32 * kBaseTimeNanos),
  (64 * kBaseTimeNanos),
  (128 * kBaseTimeNanos),
  (256 * kBaseTimeNanos),
  (512 * kBaseTimeNanos),
  (1024 * kBaseTimeNanos),
};

static void sleep_nanos(long nanos) {
  // For sleep times above 20usec, nanosleep seems to be fine, but it has
  // an offset of about 20usec (on the RPi distribution I was testing it on).
  // That means, we need to give it 80us to get 100us.
  // For values lower than roughly 30us, this is not accurate anymore and we
  // need to switch to busy wait.
  // TODO: compile Linux kernel realtime extensions and watch if the offset-time
  // changes and hope for less jitter.
  if (nanos > 28000) {
    struct timespec sleep_time = { 0, nanos - 20000 };
    nanosleep(&sleep_time, NULL);
  } else {
    // The following loop is determined empirically on a 700Mhz RPi
    for (int i = nanos >> 2; i != 0; --i) {
      asm("");   // force GCC not to optimize this away.
    }
  }
}

namespace rgb_matrix {
// Pump pixels to screen. Needs to be high priority real-time because jitter
class RGBMatrix::UpdateThread : public Thread {
public:
  UpdateThread(RGBMatrix *matrix) : running_(true), matrix_(matrix) {}
  virtual ~UpdateThread() {
    Stop();
  }

  virtual void Run() {
    while (running()) {
#if SHOW_REFRESH_RATE
      struct timeval start, end;
      gettimeofday(&start, NULL);
#endif
      matrix_->UpdateScreen();
#if SHOW_REFRESH_RATE
      gettimeofday(&end, NULL);
      int64_t usec = ((uint64_t)end.tv_sec * 1000000 + end.tv_usec)
        - ((int64_t)start.tv_sec * 1000000 + start.tv_usec);
      printf("\b\b\b\b\b\b\b\b%6.1fHz", 1e6 / usec);
#endif
    }

    // Make sure the screen is clean and no glaring pixels in the end.
    matrix_->Clear();
    matrix_->UpdateScreen();
  }

private:
  inline bool running() {
    MutexLock l(&mutex_);
    return running_;
  }

  void Stop() {
    MutexLock l(&mutex_);
    running_ = false;
  }
  Mutex mutex_;
  bool running_;
  RGBMatrix *const matrix_;
};

RGBMatrix::RGBMatrix(GPIO *io, int rows, int chained_displays)
  : rows_(rows), columns_(32 * chained_displays),
    pwm_bits_(kBitPlanes), do_luminance_correct_(true),
    double_rows_(rows / 2), row_mask_(double_rows_ - 1),
  io_(NULL), updater_(NULL) {
  bitplane_framebuffer_ = new IoBits [double_rows_ * columns_ * kBitPlanes];
  Clear();
  SetGPIO(io);
}

RGBMatrix::~RGBMatrix() {
  delete updater_;
  delete [] bitplane_framebuffer_;
}

void RGBMatrix::SetGPIO(GPIO *io) {
  if (io == NULL) return;  // nothing to set.
  if (io_ != NULL) return;  // already set.
  io_ = io;
  // Tell GPIO about all bits we intend to use.
  IoBits b;
  b.raw = 0;
  b.bits.output_enable = b.bits.clock = b.bits.strobe = 1;
  b.bits.r1 = b.bits.g1 = b.bits.b1 = 1;
  b.bits.r2 = b.bits.g2 = b.bits.b2 = 1;
  b.bits.row = 0x0f;
  // Initialize outputs, make sure that all of these are supported bits.
  const uint32_t result = io_->InitOutputs(b.raw);
  assert(result == b.raw);
  updater_ = new UpdateThread(this);
  updater_->Start(49);  // Highest priority below kernel tasks.
}

bool RGBMatrix::SetPWMBits(uint8_t value) {
  if (value > kBitPlanes)
    return false;
  pwm_bits_ = value;
  return true;
}

inline RGBMatrix::IoBits *RGBMatrix::ValueAt(int double_row, int column,
                                             int bit) {
  return &bitplane_framebuffer_[ double_row * (columns_ * kBitPlanes)
                                 + bit * columns_
                                 + column ];
}

// Do CIE1931 luminance correction and scale to output bitplanes
static uint16_t luminance_cie1931(uint8_t c) {
  float out_factor = ((1 << kBitPlanes) - 1);
  float v = c * 100.0 / 255.0;
  return out_factor * ((v <= 8) ? v / 902.3 : pow((v + 16) / 116.0, 3));
}

static uint16_t *CreateLuminanceCIE1931LookupTable() {
  uint16_t *result = new uint16_t [ 256 ];
  for (int i = 0; i < 256; ++i)
    result[i] = luminance_cie1931(i);
  return result;
}

inline uint16_t RGBMatrix::MapColor(uint8_t c) {
  if (do_luminance_correct_) {
    // We're leaking this table. So be it :)
    static uint16_t *luminance_lookup = CreateLuminanceCIE1931LookupTable();
    return luminance_lookup[c];
  } else {
    enum {shift = kBitPlanes - 8};  //constexpr; shift to be left aligned.
    return (shift > 0) ? (c << shift) : (c >> -shift);
  }
}

void RGBMatrix::Clear() {
  memset(bitplane_framebuffer_, 0,
         sizeof(*bitplane_framebuffer_) * double_rows_ * columns_ * kBitPlanes);
}

void RGBMatrix::Fill(uint8_t r, uint8_t g, uint8_t b) {
  const uint16_t red   = MapColor(r);
  const uint16_t green = MapColor(g);
  const uint16_t blue  = MapColor(b);

  for (int b = kBitPlanes - pwm_bits_; b < kBitPlanes; ++b) {
    uint16_t mask = 1 << b;
    IoBits plane_bits;
    plane_bits.raw = 0;
    plane_bits.bits.r1 = plane_bits.bits.r2 = (red & mask) == mask;
    plane_bits.bits.g1 = plane_bits.bits.g2 = (green & mask) == mask;
    plane_bits.bits.b1 = plane_bits.bits.b2 = (blue & mask) == mask;
    for (int row = 0; row < double_rows_; ++row) {
      IoBits *row_data = ValueAt(row, 0, b);
      for (int col = 0; col < columns_; ++col) {
        (row_data++)->raw = plane_bits.raw;
      }
    }
  }
}

void RGBMatrix::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || x >= columns_ || y < 0 || y >= rows_) return;

  const uint16_t red   = MapColor(r);
  const uint16_t green = MapColor(g);
  const uint16_t blue  = MapColor(b);

  const int min_bit_plane = kBitPlanes - pwm_bits_;
  IoBits *bits = ValueAt(y & row_mask_, x, min_bit_plane);
  if (y < double_rows_) {   // Upper sub-panel.
    for (int b = min_bit_plane; b < kBitPlanes; ++b) {
      const uint16_t mask = 1 << b;
      bits->bits.r1 = (red & mask) == mask;
      bits->bits.g1 = (green & mask) == mask;
      bits->bits.b1 = (blue & mask) == mask;
      bits += columns_;
    }
  } else {
    for (int b = min_bit_plane; b < kBitPlanes; ++b) {
      const uint16_t mask = 1 << b;
      bits->bits.r2 = (red & mask) == mask;
      bits->bits.g2 = (green & mask) == mask;
      bits->bits.b2 = (blue & mask) == mask;
      bits += columns_;
    }
  }
}

void RGBMatrix::UpdateScreen() {
  IoBits color_clk_mask;   // Mask of bits we need to set while clocking in.
  color_clk_mask.bits.r1 = color_clk_mask.bits.g1 = color_clk_mask.bits.b1 = 1;
  color_clk_mask.bits.r2 = color_clk_mask.bits.g2 = color_clk_mask.bits.b2 = 1;
  color_clk_mask.bits.clock = 1;

  IoBits row_mask;
  row_mask.bits.row = 0x0f;

  IoBits clock, output_enable, strobe, row_address;
  clock.bits.clock = 1;
  output_enable.bits.output_enable = 1;
  strobe.bits.strobe = 1;

  const int pwm_to_show = pwm_bits_;  // Local copy, might change in process.
  for (uint8_t d_row = 0; d_row < double_rows_; ++d_row) {
    row_address.bits.row = d_row;
    io_->WriteMaskedBits(row_address.raw, row_mask.raw);  // Set row address

    // Rows can't be switched very quickly without ghosting, so we do the
    // full PWM of one row before switching rows.
    for (int b = kBitPlanes - pwm_to_show; b < kBitPlanes; ++b) {
      IoBits *row_data = ValueAt(d_row, 0, b);
      // We clock these in while we are dark. This actually increases the
      // dark time, but we ignore that a bit.
      for (int col = 0; col < columns_; ++col) {
        const IoBits &out = *row_data++;
        io_->WriteMaskedBits(out.raw, color_clk_mask.raw);  // col + reset clock
        io_->SetBits(clock.raw);               // Rising edge: clock color in.
      }

      io_->ClearBits(color_clk_mask.raw);    // clock back to normal.

      io_->SetBits(strobe.raw);   // Strobe in the previously clocked in row.
      io_->ClearBits(strobe.raw);

      // Now switch on for the sleep time necessary for that bit-plane.
      io_->ClearBits(output_enable.raw);
      sleep_nanos(row_sleep_nanos[b]);
      io_->SetBits(output_enable.raw);
    }
  }
}
}  // namespace rgb_matrix
