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

#include "timing-internal.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

namespace rgb_matrix {
enum {
  kBitPlanes = 11  // maximum usable bitplanes.
};

static const long kBaseTimeNanos = 200;

RGBMatrix::Framebuffer::Framebuffer(int rows, int columns)
  : rows_(rows), columns_(columns),
    pwm_bits_(kBitPlanes), do_luminance_correct_(true),
    double_rows_(rows / 2), row_mask_(double_rows_ - 1) {
  bitplane_buffer_ = new IoBits [double_rows_ * columns_ * kBitPlanes];
  Clear();
}

RGBMatrix::Framebuffer::~Framebuffer() {
  delete [] bitplane_buffer_;
}

/* statuc */ void RGBMatrix::Framebuffer::InitGPIO(GPIO *io) {
  // Tell GPIO about all bits we intend to use.
  IoBits b;
  b.raw = 0;
  b.bits.output_enable_rev1 = b.bits.output_enable_rev2 = 1;
  b.bits.clock_rev1 = b.bits.clock_rev2 = 1;
  b.bits.strobe = 1;
  b.bits.r1 = b.bits.g1 = b.bits.b1 = 1;
  b.bits.r2 = b.bits.g2 = b.bits.b2 = 1;
  b.bits.row = 0x0f;
  // Initialize outputs, make sure that all of these are supported bits.
  const uint32_t result = io->InitOutputs(b.raw);
  assert(result == b.raw);
}

bool RGBMatrix::Framebuffer::SetPWMBits(uint8_t value) {
  if (value < 1 || value > kBitPlanes)
    return false;
  pwm_bits_ = value;
  return true;
}

inline RGBMatrix::Framebuffer::IoBits *
RGBMatrix::Framebuffer::ValueAt(int double_row, int column, int bit) {
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

inline uint16_t RGBMatrix::Framebuffer::MapColor(uint8_t c) {
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

void RGBMatrix::Framebuffer::Clear() {
#ifdef INVERSE_RGB_DISPLAY_COLORS
  Fill(0, 0, 0);
#else
  memset(bitplane_buffer_, 0,
         sizeof(*bitplane_buffer_) * double_rows_ * columns_ * kBitPlanes);
#endif
}

void RGBMatrix::Framebuffer::Fill(uint8_t r, uint8_t g, uint8_t b) {
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

void RGBMatrix::Framebuffer::SetPixel(int x, int y,
                                      uint8_t r, uint8_t g, uint8_t b) {
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

void RGBMatrix::Framebuffer::DumpToMatrix(GPIO *io) {
  IoBits color_clk_mask;   // Mask of bits we need to set while clocking in.
  color_clk_mask.bits.r1 = color_clk_mask.bits.g1 = color_clk_mask.bits.b1 = 1;
  color_clk_mask.bits.r2 = color_clk_mask.bits.g2 = color_clk_mask.bits.b2 = 1;
  color_clk_mask.bits.clock_rev1 = color_clk_mask.bits.clock_rev2 = 1;

  IoBits row_mask;
  row_mask.bits.row = 0x0f;

  IoBits clock, output_enable, strobe, row_address;
  clock.bits.clock_rev1 = clock.bits.clock_rev2 = 1;
  output_enable.bits.output_enable_rev1 = 1;
  output_enable.bits.output_enable_rev2 = 1;
  strobe.bits.strobe = 1;

  const int pwm_to_show = pwm_bits_;  // Local copy, might change in process.
  for (uint8_t d_row = 0; d_row < double_rows_; ++d_row) {
    row_address.bits.row = d_row;
    io->WriteMaskedBits(row_address.raw, row_mask.raw);  // Set row address

    // Rows can't be switched very quickly without ghosting, so we do the
    // full PWM of one row before switching rows.
    for (int b = kBitPlanes - pwm_to_show; b < kBitPlanes; ++b) {
      IoBits *row_data = ValueAt(d_row, 0, b);
      // We clock these in while we are dark. This actually increases the
      // dark time, but we ignore that a bit.
      for (int col = 0; col < columns_; ++col) {
        const IoBits &out = *row_data++;
        io->WriteMaskedBits(out.raw, color_clk_mask.raw);  // col + reset clock
        io->SetBits(clock.raw);               // Rising edge: clock color in.
      }

      io->ClearBits(color_clk_mask.raw);    // clock back to normal.

      io->SetBits(strobe.raw);   // Strobe in the previously clocked in row.
      io->ClearBits(strobe.raw);

      // Now switch on for the sleep time necessary for that bit-plane.
      io->ClearBits(output_enable.raw);
      rgb_matrix::sleep_nanos(kBaseTimeNanos << b);
      io->SetBits(output_enable.raw);
    }
  }
}
}  // namespace rgb_matrix
