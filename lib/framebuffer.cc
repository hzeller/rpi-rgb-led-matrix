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

static const long kBaseTimeNanos = 100;

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
  : rows_(rows),
#ifdef SUPPORT_MULTI_PARALLEL
    parallel_(parallel),
#endif
    height_(rows * parallel),
    columns_(columns),
    pwm_bits_(kBitPlanes), do_luminance_correct_(true),
    double_rows_(rows / 2), row_mask_(double_rows_ - 1) {
  bitplane_buffer_ = new uint32_t [double_rows_ * columns_ * kBitPlanes];
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
  uint32_t b = 0;
  b |= (1<<OUTPUT_ENABLE) | (1<<CLOCK) | (1<<STROBE);

  b |= (1<<P0_R1)|(1<<P0_G1)|(1<<P0_B1)|(1<<P0_R2)|(1<<P0_G2)|(1<<P0_B2);
  // TODO(hzeller): make individual switch.
  b |= (1<<P1_R1)|(1<<P1_G1)|(1<<P1_B1)|(1<<P1_R2)|(1<<P1_G2)|(1<<P1_B2);
  b |= (1<<P2_R1)|(1<<P2_G1)|(1<<P2_B1)|(1<<P2_R2)|(1<<P2_G2)|(1<<P2_B2);

  b |= (1<<ROW_A)|(1<<ROW_B)|(1<<ROW_C)|(1<<ROW_D);

  // Initialize outputs, make sure that all of these are supported bits.
  const uint32_t result = io->InitOutputs(b);
  assert(result == b);

  // Now, set up the PinPulser for output enable.
  uint32_t output_enable_bits = (1<<OUTPUT_ENABLE);
  std::vector<int> bitplane_timings;
  for (int b = 0; b < kBitPlanes; ++b) {
    bitplane_timings.push_back(kBaseTimeNanos << b);
  }
  sOutputEnablePulser = PinPulser::Create(io, output_enable_bits,
                                          bitplane_timings);
}

bool Framebuffer::SetPWMBits(uint8_t value) {
  if (value < 1 || value > kBitPlanes)
    return false;
  pwm_bits_ = value;
  return true;
}

inline uint32_t *Framebuffer::ValueAt(int double_row, int column, int bit) {
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
    uint32_t plane_bits = 0;
    if ((red & mask) == mask) {
      plane_bits |= (1<<P0_R1) | (1<<P0_R2);
      plane_bits |= (1<<P1_R1) | (1<<P1_R2);
      plane_bits |= (1<<P2_R1) | (1<<P2_R2);
    }
    if ((green & mask) == mask) {
      plane_bits |= (1<<P0_G1) | (1<<P0_G2);
      plane_bits |= (1<<P1_G1) | (1<<P1_G2);
      plane_bits |= (1<<P2_G1) | (1<<P2_G2);
    }
    if ((blue & mask) == mask) {
      plane_bits |= (1<<P0_B1) | (1<<P0_B2);
      plane_bits |= (1<<P1_B1) | (1<<P1_B2);
      plane_bits |= (1<<P2_B1) | (1<<P2_B2);
    }
    for (int row = 0; row < double_rows_; ++row) {
      uint32_t *row_data = ValueAt(row, 0, b);
      for (int col = 0; col < columns_; ++col) {
        *(row_data++) = plane_bits;
      }
    }
  }
}

void Framebuffer::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  if (x < 0 || x >= columns_ || y < 0 || y >= height_) return;

  const uint16_t red   = MapColor(r);
  const uint16_t green = MapColor(g);
  const uint16_t blue  = MapColor(b);

  const int min_bit_plane = kBitPlanes - pwm_bits_;
  uint32_t *bits = ValueAt(y & row_mask_, x, min_bit_plane);

  // Manually expand the three cases for better performance.
  // TODO(hzeller): This is a bit repetetive. Test if it pays off to just
  // pre-calc rgb mask and apply.
  if (y < rows_) {
    // Parallel chain #1
    if (y < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        if ((red & m) == m)   *bits |= (1<<P0_R1); else *bits &= ~(1<<P0_R1);
        if ((green & m) == m) *bits |= (1<<P0_G1); else *bits &= ~(1<<P0_G1);
        if ((blue & m) == m)  *bits |= (1<<P0_B1); else *bits &= ~(1<<P0_B1);
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        if ((red & m) == m)   *bits |= (1<<P0_R2); else *bits &= ~(1<<P0_R2);
        if ((green & m) == m) *bits |= (1<<P0_G2); else *bits &= ~(1<<P0_G2);
        if ((blue & m) == m)  *bits |= (1<<P0_B2); else *bits &= ~(1<<P0_B2);
        bits += columns_;
      }
    }
  } else if (y >= rows_ && y < 2 * rows_) {
    // Parallel chain #2
    if (y - rows_ < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        if ((red & m) == m)   *bits |= (1<<P1_R1); else *bits &= ~(1<<P1_R1);
        if ((green & m) == m) *bits |= (1<<P1_G1); else *bits &= ~(1<<P1_G1);
        if ((blue & m) == m)  *bits |= (1<<P1_B1); else *bits &= ~(1<<P1_B1);
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        if ((red & m) == m)   *bits |= (1<<P1_R2); else *bits &= ~(1<<P1_R2);
        if ((green & m) == m) *bits |= (1<<P1_G2); else *bits &= ~(1<<P1_G2);
        if ((blue & m) == m)  *bits |= (1<<P1_B2); else *bits &= ~(1<<P1_B2);
        bits += columns_;
      }
    }
  } else {
    // Parallel chain #3
    if (y - 2*rows_ < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        if ((red & m) == m)   *bits |= (1<<P2_R1); else *bits &= ~(1<<P2_R1);
        if ((green & m) == m) *bits |= (1<<P2_G1); else *bits &= ~(1<<P2_G1);
        if ((blue & m) == m)  *bits |= (1<<P2_B1); else *bits &= ~(1<<P2_B1);
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        if ((red & m) == m)   *bits |= (1<<P2_R2); else *bits &= ~(1<<P2_R2);
        if ((green & m) == m) *bits |= (1<<P2_G2); else *bits &= ~(1<<P2_G2);
        if ((blue & m) == m)  *bits |= (1<<P2_B2); else *bits &= ~(1<<P2_B2);
        bits += columns_;
      }
    }
  }
}

void Framebuffer::DumpToMatrix(GPIO *io) {
  // Mask of bits we need to set while clocking in.
  const uint32_t color_clk_mask =
    (1<<P0_R1)|(1<<P0_G1)|(1<<P0_B1)|(1<<P0_R2)|(1<<P0_G2)|(1<<P0_B2)|
    (1<<P1_R1)|(1<<P1_G1)|(1<<P1_B1)|(1<<P1_R2)|(1<<P1_G2)|(1<<P1_B2)|
    (1<<P2_R1)|(1<<P2_G1)|(1<<P2_B1)|(1<<P2_R2)|(1<<P2_G2)|(1<<P2_B2)|
    (1<<CLOCK);

  const uint32_t row_mask =
    (1<<ROW_A)|(1<<ROW_B)|(1<<ROW_C)|(1<<ROW_D);

  const uint32_t clock = (1<<CLOCK);
  const uint32_t strobe = (1<<STROBE);

  const int pwm_to_show = pwm_bits_;  // Local copy, might change in process.
  for (uint8_t d_row = 0; d_row < double_rows_; ++d_row) {
    const uint32_t row_address =
      ((d_row & 0x1) ? (1<<ROW_A) : 0) |
      ((d_row & 0x2) ? (1<<ROW_B) : 0) |
      ((d_row & 0x4) ? (1<<ROW_C) : 0) |
      ((d_row & 0x8) ? (1<<ROW_D) : 0);

    io->WriteMaskedBits(row_address, row_mask);  // Set row address

    // Rows can't be switched very quickly without ghosting, so we do the
    // full PWM of one row before switching rows.
    for (int b = kBitPlanes - pwm_to_show; b < kBitPlanes; ++b) {
      const uint32_t *row_data = ValueAt(d_row, 0, b);
      // While the output enable is still on, we can already clock in the next
      // data.
      for (int col = 0; col < columns_; ++col) {
        const uint32_t &out = *row_data++;
        io->WriteMaskedBits(out, color_clk_mask);  // col + reset clock
        io->SetBits(clock);               // Rising edge: clock color in.
      }
      io->ClearBits(color_clk_mask);    // clock back to normal.

      // OE of the previous row-data must be finished before strobe.
      sOutputEnablePulser->WaitPulseFinished();

      io->SetBits(strobe);   // Strobe in the previously clocked in row.
      io->ClearBits(strobe);

      // Now switch on for the sleep time necessary for that bit-plane.
      sOutputEnablePulser->SendPulse(b);
    }
    sOutputEnablePulser->WaitPulseFinished();
  }
}
}  // namespace internal
}  // namespace rgb_matrix
