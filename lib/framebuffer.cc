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

// The framebuffer is the workhorse: it represents the frame in some internal
// format that is friendly to be dumped to the matrix quickly. Provides methods
// to manipulate the content.

#include "framebuffer-internal.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "gpio.h"

namespace rgb_matrix {
namespace internal {
enum {
  kBitPlanes = 11  // maximum usable bitplanes.
};

static const long kBaseTimeNanos = 200;

// We need one global instance of a timing correct pulser. There are different
// implementations depending on the context.
static PinPulser *sOutputEnablePulser = NULL;

// The Adafruit HAT only supports one chain.
#if defined(ADAFRUIT_RGBMATRIX_HAT) && defined(SUPPORT_MULTI_PARALLEL)
#  warning "Adafruit HAT doesn't map parallel chains. Disabling parallel chains."
#  undef SUPPORT_MULTI_PARALLEL
#endif

// Only if SUPPORT_MULTI_PARALLEL is not defined, we allow classic wiring.
// Also, the Adafruit HAT does not do classic wiring either.
#if defined(SUPPORT_MULTI_PARALLEL) || defined(ADAFRUIT_RGBMATRIX_HAT) \
  || defined(EXPERIMENTAL_NEW_PINOUT)
#  undef SUPPORT_CLASSIC_LED_GPIO_WIRING_
#else
#  define SUPPORT_CLASSIC_LED_GPIO_WIRING_
#endif

Framebuffer::Framebuffer(int rows, int columns, int parallel)
  : rows_(rows), parallel_(parallel), height_(rows * parallel),
    columns_(columns),
    pwm_bits_(kBitPlanes), do_luminance_correct_(true),
    double_rows_(rows / 2), row_mask_(double_rows_ - 1) {
  bitplane_buffer_ = new IoBits [double_rows_ * columns_ * kBitPlanes];
  Clear();
  assert(rows_ <= 32);
  assert(parallel >= 1 && parallel <= 3);
#ifndef SUPPORT_MULTI_PARALLEL
  if (parallel > 1) {
    fprintf(stderr, "In order for parallel > 1 to work, you need to "
            "define SUPPORT_MULTI_PARALLEL in lib/Makefile.\n");
    assert(parallel == 1);
  }
#endif
}

Framebuffer::~Framebuffer() {
  delete [] bitplane_buffer_;
}

/* static */ void Framebuffer::InitGPIO(GPIO *io, int parallel) {
  if (sOutputEnablePulser != NULL)
    return;  // already initialized.

  // Tell GPIO about all bits we intend to use.
  IoBits b;
  b.raw = 0;

#ifdef SUPPORT_CLASSIC_LED_GPIO_WIRING_
  b.bits.output_enable_rev1 = b.bits.output_enable_rev2 = 1;
  b.bits.clock_rev1 = b.bits.clock_rev2 = 1;
#endif

  b.bits.output_enable = 1;
  b.bits.clock = 1;
  b.bits.strobe = 1;

  b.bits.p0_r1 = b.bits.p0_g1 = b.bits.p0_b1 = 1;
  b.bits.p0_r2 = b.bits.p0_g2 = b.bits.p0_b2 = 1;

#ifdef SUPPORT_MULTI_PARALLEL
  if (parallel >= 2) {
    b.bits.p1_r1 = b.bits.p1_g1 = b.bits.p1_b1 = 1;
    b.bits.p1_r2 = b.bits.p1_g2 = b.bits.p1_b2 = 1;
  }

  if (parallel >= 3) {
    b.bits.p2_r1 = b.bits.p2_g1 = b.bits.p2_b1 = 1;
    b.bits.p2_r2 = b.bits.p2_g2 = b.bits.p2_b2 = 1;
  }
#endif

  b.bits.a = b.bits.b = b.bits.c = b.bits.d = 1;

  // Initialize outputs, make sure that all of these are supported bits.
  const uint32_t result = io->InitOutputs(b.raw);
  assert(result == b.raw);

  // Now, set up the PinPulser for output enable.
  IoBits output_enable_bits;
#ifdef SUPPORT_CLASSIC_LED_GPIO_WIRING_
  output_enable_bits.bits.output_enable_rev1
    = output_enable_bits.bits.output_enable_rev2 = 1;
#endif
  output_enable_bits.bits.output_enable = 1;

  std::vector<int> bitplane_timings;
  for (int b = 0; b < kBitPlanes; ++b) {
    bitplane_timings.push_back(kBaseTimeNanos << b);
  }
  sOutputEnablePulser = PinPulser::Create(io, output_enable_bits.raw,
                                          bitplane_timings);
}

bool Framebuffer::SetPWMBits(uint8_t value) {
  if (value < 1 || value > kBitPlanes)
    return false;
  pwm_bits_ = value;
  return true;
}

inline Framebuffer::IoBits *Framebuffer::ValueAt(int double_row,
                                                 int column, int bit) {
  return &bitplane_buffer_[ double_row * (columns_ * kBitPlanes)
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

inline uint16_t Framebuffer::MapColor(uint8_t c) {
#ifdef INVERSE_RGB_DISPLAY_COLORS
#  define COLOR_OUT_BITS(x) (x) ^ 0xffff
#else
#  define COLOR_OUT_BITS(x) (x)
#endif

  if (do_luminance_correct_) {
    // We're leaking this table. So be it :)
    static uint16_t *luminance_lookup = CreateLuminanceCIE1931LookupTable();
    return COLOR_OUT_BITS(luminance_lookup[c]);
  } else {
    enum {shift = kBitPlanes - 8};  //constexpr; shift to be left aligned.
    return COLOR_OUT_BITS((shift > 0) ? (c << shift) : (c >> -shift));
  }

#undef COLOR_OUT_BITS
}

void Framebuffer::Clear() {
#ifdef INVERSE_RGB_DISPLAY_COLORS
  Fill(0, 0, 0);
#else
  memset(bitplane_buffer_, 0,
         sizeof(*bitplane_buffer_) * double_rows_ * columns_ * kBitPlanes);
#endif
}

void Framebuffer::Fill(uint8_t r, uint8_t g, uint8_t b) {
  const uint16_t red   = MapColor(r);
  const uint16_t green = MapColor(g);
  const uint16_t blue  = MapColor(b);

  for (int b = kBitPlanes - pwm_bits_; b < kBitPlanes; ++b) {
    uint16_t mask = 1 << b;
    IoBits plane_bits;
    plane_bits.raw = 0;
    plane_bits.bits.p0_r1 = plane_bits.bits.p0_r2 = (red & mask) == mask;
    plane_bits.bits.p0_g1 = plane_bits.bits.p0_g2 = (green & mask) == mask;
    plane_bits.bits.p0_b1 = plane_bits.bits.p0_b2 = (blue & mask) == mask;

#ifdef SUPPORT_MULTI_PARALLEL
    plane_bits.bits.p1_r1 = plane_bits.bits.p1_r2 =
      plane_bits.bits.p2_r1 = plane_bits.bits.p2_r2 = (red & mask) == mask;
    plane_bits.bits.p1_g1 = plane_bits.bits.p1_g2 =
      plane_bits.bits.p2_g1 = plane_bits.bits.p2_g2 = (green & mask) == mask;
    plane_bits.bits.p1_b1 = plane_bits.bits.p1_b2 =
      plane_bits.bits.p2_b1 = plane_bits.bits.p2_b2 = (blue & mask) == mask;
#endif
    for (int row = 0; row < double_rows_; ++row) {
      IoBits *row_data = ValueAt(row, 0, b);
      for (int col = 0; col < columns_; ++col) {
        (row_data++)->raw = plane_bits.raw;
      }
    }
  }
}

void Framebuffer::SetPixel(int x, int y,
                                      uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || x >= columns_ || y < 0 || y >= height_) return;

  const uint16_t red   = MapColor(r);
  const uint16_t green = MapColor(g);
  const uint16_t blue  = MapColor(b);

  const int min_bit_plane = kBitPlanes - pwm_bits_;
  IoBits *bits = ValueAt(y & row_mask_, x, min_bit_plane);

  // Manually expand the three cases for better performance.
  // TODO(hzeller): This is a bit repetetive. Test if it pays off to just
  // pre-calc rgb mask and apply.
  if (y < rows_) {
    // Parallel chain #1
    if (y < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t mask = 1 << b;
        bits->bits.p0_r1 = (red & mask) == mask;
        bits->bits.p0_g1 = (green & mask) == mask;
        bits->bits.p0_b1 = (blue & mask) == mask;
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t mask = 1 << b;
        bits->bits.p0_r2 = (red & mask) == mask;
        bits->bits.p0_g2 = (green & mask) == mask;
        bits->bits.p0_b2 = (blue & mask) == mask;
        bits += columns_;
      }
    }
#ifdef SUPPORT_MULTI_PARALLEL
  } else if (y >= rows_ && y < 2 * rows_) {
    // Parallel chain #2
    if (y - rows_ < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t mask = 1 << b;
        bits->bits.p1_r1 = (red & mask) == mask;
        bits->bits.p1_g1 = (green & mask) == mask;
        bits->bits.p1_b1 = (blue & mask) == mask;
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t mask = 1 << b;
        bits->bits.p1_r2 = (red & mask) == mask;
        bits->bits.p1_g2 = (green & mask) == mask;
        bits->bits.p1_b2 = (blue & mask) == mask;
        bits += columns_;
      }
    }
  } else {
    // Parallel chain #3
    if (y - 2*rows_ < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t mask = 1 << b;
        bits->bits.p2_r1 = (red & mask) == mask;
        bits->bits.p2_g1 = (green & mask) == mask;
        bits->bits.p2_b1 = (blue & mask) == mask;
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t mask = 1 << b;
        bits->bits.p2_r2 = (red & mask) == mask;
        bits->bits.p2_g2 = (green & mask) == mask;
        bits->bits.p2_b2 = (blue & mask) == mask;
        bits += columns_;
      }
    }
#endif
  }
}

void Framebuffer::DumpToMatrix(GPIO *io) {
  IoBits color_clk_mask;   // Mask of bits we need to set while clocking in.
  color_clk_mask.bits.p0_r1
    = color_clk_mask.bits.p0_g1
    = color_clk_mask.bits.p0_b1
    = color_clk_mask.bits.p0_r2
    = color_clk_mask.bits.p0_g2
    = color_clk_mask.bits.p0_b2 = 1;

#ifdef SUPPORT_MULTI_PARALLEL
  if (parallel_ >= 2) {
    color_clk_mask.bits.p1_r1
      = color_clk_mask.bits.p1_g1
      = color_clk_mask.bits.p1_b1
      = color_clk_mask.bits.p1_r2
      = color_clk_mask.bits.p1_g2
      = color_clk_mask.bits.p1_b2 = 1;
  }

  if (parallel_ >= 3) {
    color_clk_mask.bits.p2_r1
      = color_clk_mask.bits.p2_g1
      = color_clk_mask.bits.p2_b1
      = color_clk_mask.bits.p2_r2
      = color_clk_mask.bits.p2_g2
      = color_clk_mask.bits.p2_b2 = 1;
  }
#endif

#ifdef SUPPORT_CLASSIC_LED_GPIO_WIRING_
  color_clk_mask.bits.clock_rev1 = color_clk_mask.bits.clock_rev2 = 1;
#endif
  color_clk_mask.bits.clock = 1;

  IoBits row_mask;
  row_mask.bits.a = row_mask.bits.b = row_mask.bits.c = row_mask.bits.d = 1;

  IoBits clock, strobe, row_address;
#ifdef SUPPORT_CLASSIC_LED_GPIO_WIRING_
  clock.bits.clock_rev1 = clock.bits.clock_rev2 = 1;
#endif
  clock.bits.clock = 1;
  strobe.bits.strobe = 1;

  const int pwm_to_show = pwm_bits_;  // Local copy, might change in process.
  for (uint8_t d_row = 0; d_row < double_rows_; ++d_row) {
    row_address.bits.a = d_row;
    row_address.bits.b = d_row >> 1;
    row_address.bits.c = d_row >> 2;
    row_address.bits.d = d_row >> 3;

    io->WriteMaskedBits(row_address.raw, row_mask.raw);  // Set row address

    // Rows can't be switched very quickly without ghosting, so we do the
    // full PWM of one row before switching rows.
    for (int b = kBitPlanes - pwm_to_show; b < kBitPlanes; ++b) {
      IoBits *row_data = ValueAt(d_row, 0, b);
      // While the output enable is still on, we can already clock in the next
      // data.
      for (int col = 0; col < columns_; ++col) {
        const IoBits &out = *row_data++;
        io->WriteMaskedBits(out.raw, color_clk_mask.raw);  // col + reset clock
        io->SetBits(clock.raw);               // Rising edge: clock color in.
      }
      io->ClearBits(color_clk_mask.raw);    // clock back to normal.

      // OE of the previous row-data must be finished before strobe.
      sOutputEnablePulser->WaitPulseFinished();

      io->SetBits(strobe.raw);   // Strobe in the previously clocked in row.
      io->ClearBits(strobe.raw);

      // Now switch on for the sleep time necessary for that bit-plane.
      sOutputEnablePulser->SendPulse(b);
    }
    sOutputEnablePulser->WaitPulseFinished();
  }
}
}  // namespace internal
}  // namespace rgb_matrix
