// Some experimental code.
// (c) H. Zeller <h.zeller@acm.org>. License: do whatever you want with it :)
// 2013-12 - Modified for a 16x32 matrix (half the original panel)
//
// Using GPIO to control a 16x32 rgb LED panel (typically you find them with the
// suffix such as P4 or P5: that is the pitch in mm.
// So "32x32 rgb led p5" should find you something on 'the internets'.

#include "led-matrix.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gpio.h"

// Clocking in a row takes about 3.4usec (TODO: this is actually per board)
// Because clocking the data in is part of the 'wait time', we need to
// substract that from the row sleep time.
static const int kRowClockTime = 3400;
static const int kBaseTime = kRowClockTime;  // smallest possible value.

const long row_sleep_nanos[8] = {   // Only using the first kPWMBits elements.
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

RGBMatrix::RGBMatrix(GPIO *io) : io_(io) {
  // Tell GPIO about all bits we intend to use.
  IoBits b;
  b.raw = 0;
  b.bits.output_enable = b.bits.clock = b.bits.strobe = 1;
  b.bits.r1 = b.bits.g1 = b.bits.b1 = 1;
  b.bits.r2 = b.bits.g2 = b.bits.b2 = 1;
  b.bits.row = 0xf;
  // Initialize outputs, make sure that all of these are supported bits.
  const uint32_t result = io_->InitOutputs(b.raw);
  assert(result == b.raw);
  assert(kPWMBits < 8);    // only up to 7 makes sense.
  ClearScreen();
}

void RGBMatrix::ClearScreen() {
  memset(&bitplane_, 0, sizeof(bitplane_));
}

void RGBMatrix::FillScreen(uint8_t red, uint8_t green, uint8_t blue) {
  for (int x = 0; x < kColumns; ++x) {
    for (int y = 0; y < width(); ++y) {
      SetPixel(x, y, red, green, blue);
    }
  }
}

void RGBMatrix::SetPixel(uint8_t x, uint8_t y,
                         uint8_t red, uint8_t green, uint8_t blue) {
  if (x >= width() || y >= height()) return;

  // My setup: A single panel connected  [>] 16 rows & 32 columns.
  
  // TODO: re-map values to be luminance corrected (sometimes called 'gamma').
  // Ideally, we had like 10PWM bits for this, but we're too slow for that :/
  
  // Scale to the number of bit planes we actually have, so that MSB matches
  // MSB of PWM.
  red   >>= 8 - kPWMBits;
  green >>= 8 - kPWMBits;
  blue  >>= 8 - kPWMBits;

  for (int b = 0; b < kPWMBits; ++b) {
    uint8_t mask = 1 << b;
    IoBits *bits = &bitplane_[b].row[y & 0x7].column[x]; // Half the mask here
    if (y < 8) {    // Upper sub-panel. Half the height check here
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
  IoBits serial_mask;   // Mask of bits we need to set while clocking in.
  serial_mask.bits.r1 = serial_mask.bits.g1 = serial_mask.bits.b1 = 1;
  serial_mask.bits.r2 = serial_mask.bits.g2 = serial_mask.bits.b2 = 1;
  serial_mask.bits.clock = 1;

  IoBits row_mask;
  row_mask.bits.row = 0xf; // Half the mask here?

  IoBits clock, output_enable, strobe;    
  clock.bits.clock = 1;
  output_enable.bits.output_enable = 1;
  strobe.bits.strobe = 1;

  IoBits row_bits;
  for (uint8_t row = 0; row < kDoubleRows; ++row) {
    // Rows can't be switched very quickly without ghosting, so we do the
    // full PWM of one row before switching rows.
    for (int b = 0; b < kPWMBits; ++b) {
      const DoubleRow &rowdata = bitplane_[b].row[row];

      // Clock in the row. The time this takes is the smalles time we can
      // leave the LEDs on, thus the smallest time-constant we can use for
      // PWM (doubling the sleep time with each bit).
      // So this is the critical path; I'd love to know if we can employ some
      // DMA techniques to speed this up.
      // (With this code, one row roughly takes 3.0 - 3.4usec to clock in).
      //
      // However, in particular for longer chaining, it seems we need some more
      // wait time to settle.
      const long kIOStabilizeWaitNanos = 256;
      for (uint8_t col = 0; col < kColumns; ++col) {
        const IoBits &out = rowdata.column[col];
        io_->ClearBits(~out.raw & serial_mask.raw);  // also: resets clock.
        sleep_nanos(kIOStabilizeWaitNanos);
        io_->SetBits(out.raw & serial_mask.raw);
        sleep_nanos(kIOStabilizeWaitNanos);
        io_->SetBits(clock.raw);
        sleep_nanos(kIOStabilizeWaitNanos);
      }

      io_->SetBits(output_enable.raw);  // switch off while strobe.

      row_bits.bits.row = row;
      io_->SetBits(row_bits.raw & row_mask.raw);
      io_->ClearBits(~row_bits.raw & row_mask.raw);

      io_->SetBits(strobe.raw);   // Strobe
      io_->ClearBits(strobe.raw);

      // Now switch on for the given sleep time.
      io_->ClearBits(output_enable.raw);
      // If we use less bits, then use the upper areas which leaves us more
      // CPU time to do other stuff.
      sleep_nanos(row_sleep_nanos[b + (7 - kPWMBits)]);
    }
  }
}
