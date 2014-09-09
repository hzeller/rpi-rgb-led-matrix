// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Some experimental code.
// (c) H. Zeller <h.zeller@acm.org>. License: do whatever you want with it :)
//
// Using GPIO to control a 32x32 rgb LED panel (typically you find them with the
// suffix such as P4 or P5: that is the pitch in mm.
// So "32x32 rgb led p5" should find you something on 'the internets'.

#include "led-matrix.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SHOW_REFRESH_RATE 0

#if SHOW_REFRESH_RATE
# include <stdio.h>
# include <sys/time.h>
#endif

#include "gpio.h"
#include "thread.h"

// Clocking in a row takes about 3.4usec (TODO: this is actually per board)
// Because clocking the data in is part of the 'wait time', we need to
// substract that from the row sleep time.
static const int kRowClockTime = 3400;
static const int kBaseTime = kRowClockTime;  // smallest possible value.

enum {
  kBitPlanes = 7  // maximum usable bitplanes.
};

const long row_sleep_nanos[8] = {
  (1 * kBaseTime) - kRowClockTime,
  (2 * kBaseTime) - kRowClockTime,
  (4 * kBaseTime) - kRowClockTime,
  (8 * kBaseTime) - kRowClockTime,
  (16 * kBaseTime) - kRowClockTime,
  (32 * kBaseTime) - kRowClockTime,
  (64 * kBaseTime) - kRowClockTime,
  // Too much flicker with 8 bits. We should have a separate screen pass
  // with this bit plane. Or interlace. Or trick with -OE switch on in the
  // middle of row-clocking, thus have kRowClockTime / 2
  (128 * kBaseTime) - kRowClockTime, // too much flicker.
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
      printf("\b\b\b\b\b\b\b%5.1fHz", 1e6 / usec);
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
    pwm_bits_(kBitPlanes), do_gamma_(false),
    double_rows_(rows / 2), row_mask_(double_rows_ - 1),
  io_(io), updater_(NULL) {
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
  bitplane_framebuffer_ = new IoBits [double_rows_ * columns_ * kBitPlanes];
  Clear();
  updater_ = new UpdateThread(this);
  updater_->Start(49);  // Highest priority below kernel tasks.
}

RGBMatrix::~RGBMatrix() {
  delete updater_;
  delete [] bitplane_framebuffer_;
}

bool RGBMatrix::SetPWMBits(uint8_t value) {
  if (value > kBitPlanes)
    return false;
  pwm_bits_ = value;
  return true;
}

void RGBMatrix::Clear() {
  memset(bitplane_framebuffer_, 0,
         sizeof(*bitplane_framebuffer_) * double_rows_ * columns_ * kBitPlanes);
}

void RGBMatrix::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  for (int x = 0; x < width(); ++x) {
    for (int y = 0; y < height(); ++y) {
      SetPixel(x, y, red, green, blue);
    }
  }
}

static uint16_t simple_gamma_correct(uint8_t c) {
    // Simplified gamma ~= 2.2
    if (c < 64) return c;
    if (c < 128) return ((c - 63) * 3 + 63);
    return ((c - 127) * 6 + 255);
}

inline RGBMatrix::IoBits *RGBMatrix::ValueAt(int double_row, int column,
                                             int bit) {
  return &bitplane_framebuffer_[ double_row * (columns_ * kBitPlanes)
                                 + bit * columns_
                                 + column ];
}

void RGBMatrix::SetPixel(int x, int y,
                         uint8_t red, uint8_t green, uint8_t blue) {
  if (x < 0 || y < 0 || x >= width() || y >= height()) return;

  // Ideally, we had like 10PWM bits for this, but we're too slow for that :/
  if (do_gamma_) {
    red = simple_gamma_correct(red) >> 4;
    green = simple_gamma_correct(green) >> 4;
    blue = simple_gamma_correct(blue) >> 4;
  }

  // We only maximum use kBitPlanes. So make sure our MSBit is aligned with that.
  red >>= (8 - kBitPlanes);
  green >>= (8 - kBitPlanes);
  blue >>= (8 - kBitPlanes);

  for (int b = 0; b < kBitPlanes; ++b) {
    uint8_t mask = 1 << b;
    IoBits *bits = ValueAt(y & row_mask_, x, b);
    if (y < double_rows_) {   // Upper sub-panel.
      bits->bits.r1 = (red & mask) == mask;
      bits->bits.g1 = (green & mask) == mask;
      bits->bits.b1 = (blue & mask) == mask;
    } else {        // Lower sub-panel.
      bits->bits.r2 = (red & mask) == mask;
      bits->bits.g2 = (green & mask) == mask;
      bits->bits.b2 = (blue & mask) == mask;
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
      // Clock in the row. The time this takes is the smalles time we can
      // leave the LEDs on, thus the smallest time-constant we can use for
      // PWM (doubling the sleep time with each bit).
      // So this is the critical path (tested with DMA: actually _slower_)
      // (With this code, one row roughly takes 3.0 - 3.4usec to clock in).
      for (uint8_t col = 0; col < columns_; ++col) {
        const IoBits &out = *row_data++;
        io_->WriteMaskedBits(out.raw, color_clk_mask.raw);  // col + reset clock
        io_->SetBits(clock.raw);               // Rising edge: clock color in.
      }

      io_->ClearBits(color_clk_mask.raw);    // clock back to normal.

      io_->SetBits(output_enable.raw);  // switch output off while strobing row.

      io_->SetBits(strobe.raw);   // Strobe in the previously clocked in row.
      io_->ClearBits(strobe.raw);

      // Now switch on for the sleep time necessary for that bit-plane.
      io_->ClearBits(output_enable.raw);
      sleep_nanos(row_sleep_nanos[b]);
    }
  }
  io_->SetBits(output_enable.raw);   // Switch off output.
}
}  // namespace rgb_matrix
