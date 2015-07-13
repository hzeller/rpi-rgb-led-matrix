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

Framebuffer::Framebuffer(GPIO *io, int rows, int columns, int parallel)
  : io_(io), rows_(rows),   // TODO: io not needed anymore.
#ifdef SUPPORT_MULTI_PARALLEL
    parallel_(parallel),
#endif
    height_(rows * parallel),
    columns_(columns),
    pwm_bits_(kBitPlanes), do_luminance_correct_(true),  brightness_(100),
    double_rows_(rows / 2), row_mask_(double_rows_ - 1),
    script_(NULL) {
  assert(io_);
  assert(rows_ <= 32);
  assert(parallel >= 1 && parallel <= 3);
  membuffer_ = new MemBlock(double_rows_ * columns_ * kBitPlanes
                            * sizeof(GPIO::Data));
  bitplane_buffer_ = (GPIO::Data*) membuffer_->mem();

  // When we clock in colors, we always want to clear the clock bit as well.
  // Let's prepare that already here; that bit will never be touched later.
  for (int i = 0; i < double_rows_ * columns_ * kBitPlanes; ++i) {
    bitplane_buffer_[i].set_bits = 0;
    bitplane_buffer_[i].clear_bits = (1<<CLOCK);
  }
  color_mask_ = 0;
  color_mask_ |= (1<<P0_R1)|(1<<P0_G1)|(1<<P0_B1)|(1<<P0_R2)|(1<<P0_G2)|(1<<P0_B2);
  if (parallel > 1) {
    color_mask_ |= ((1<<P1_R1)|(1<<P1_G1)|(1<<P1_B1)|
                    (1<<P1_R2)|(1<<P1_G2)|(1<<P1_B2));
  }
  if (parallel > 2) {
    color_mask_ |= ((1<<P2_R1)|(1<<P2_G1)|(1<<P2_B1)|
                    (1<<P2_R2)|(1<<P2_G2)|(1<<P2_B2));
  }
#ifndef SUPPORT_MULTI_PARALLEL
  if (parallel > 1) {
    fprintf(stderr, "In order for parallel > 1 to work, you need to "
            "define SUPPORT_MULTI_PARALLEL in lib/Makefile.\n");
    assert(parallel == 1);
  }
#endif

  // Pre-calculate some GPIO operations.
  clock_in_.set_bits = (1<<CLOCK);
  clock_in_.clear_bits = 0;

  clock_reset_.set_bits = 0;
  clock_reset_.clear_bits = (1<<CLOCK);

  oe_start_.set_bits = 0;
  oe_start_.clear_bits = (1<<OUTPUT_ENABLE);

  oe_end_.set_bits = (1<<OUTPUT_ENABLE);
  oe_end_.clear_bits = 0;

  // We use the fact that set/reset happen in that sequence
  strobe_.set_bits = (1<<STROBE);
  strobe_.clear_bits = (1<<STROBE);

  memset(row_address_, 0, sizeof(row_address_));
  const uint32_t row_mask = (1<<ROW_A)|(1<<ROW_B)|(1<<ROW_C)|(1<<ROW_D);
  for (uint8_t d_row = 0; d_row < 16; ++d_row) {
    const uint32_t row_address =
      ((d_row & 0x1) ? (1<<ROW_A) : 0) |
      ((d_row & 0x2) ? (1<<ROW_B) : 0) |
      ((d_row & 0x4) ? (1<<ROW_C) : 0) |
      ((d_row & 0x8) ? (1<<ROW_D) : 0);
    row_address_[d_row].SetMasked(row_address, row_mask);
  }

  Clear();
}

Framebuffer::~Framebuffer() {
  delete membuffer_;
}

/* static */ void Framebuffer::InitGPIO(GPIO *io, int parallel) {
  if (sOutputEnablePulser != NULL)
    return;  // already initialized.

  // Tell GPIO about all bits we intend to use.
  uint32_t b = 0;
  b |= (1<<OUTPUT_ENABLE) | (1<<CLOCK) | (1<<STROBE);

  b |= (1<<P0_R1)|(1<<P0_G1)|(1<<P0_B1)|(1<<P0_R2)|(1<<P0_G2)|(1<<P0_B2);
  if (parallel > 1) {
    b |= (1<<P1_R1)|(1<<P1_G1)|(1<<P1_B1)|(1<<P1_R2)|(1<<P1_G2)|(1<<P1_B2);
  }
  if (parallel > 2) {
    b |= (1<<P2_R1)|(1<<P2_G1)|(1<<P2_B1)|(1<<P2_R2)|(1<<P2_G2)|(1<<P2_B2);
  }

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
  // TODO(hzeller): if different, create new hardware script.
  return true;
}

inline GPIO::Data *Framebuffer::ValueAt(int double_row, int column,
                                        int bit_plane) {
  return &bitplane_buffer_[ double_row * (columns_ * kBitPlanes)
                            + bit_plane * columns_
                            + column ];
}

// Do CIE1931 luminance correction and scale to output bitplanes
static uint16_t luminance_cie1931(uint8_t c, uint8_t brightness) {
  float out_factor = ((1 << kBitPlanes) - 1);
  float v = (float) c * brightness / 255.0;
  return out_factor * ((v <= 8) ? v / 902.3 : pow((v + 16) / 116.0, 3));
}

static uint16_t *CreateLuminanceCIE1931LookupTable() {
  uint16_t *result = new uint16_t[256 * 100];
  for (int i = 0; i < 256; ++i)
    for (int j = 0; j < 100; ++j)
      result[i * 100 + j] = luminance_cie1931(i, j + 1);

  return result;
}

inline uint16_t Framebuffer::MapColor(uint8_t c) {
#ifdef INVERSE_RGB_DISPLAY_COLORS
#  define COLOR_OUT_BITS(x) (x) ^ 0xffff
#else
#  define COLOR_OUT_BITS(x) (x)
#endif

  if (do_luminance_correct_) {
    static uint16_t *luminance_lookup = CreateLuminanceCIE1931LookupTable();
    return COLOR_OUT_BITS(luminance_lookup[c * 100 + brightness_ - 1]);
  } else {
    // simple scale down the color value
    c = c * brightness_ / 100;

    enum {shift = kBitPlanes - 8};  //constexpr; shift to be left aligned.
    return COLOR_OUT_BITS((shift > 0) ? (c << shift) : (c >> -shift));
  }

#undef COLOR_OUT_BITS
}

void Framebuffer::Clear() {
  Fill(0, 0, 0);
}

void Framebuffer::Fill(uint8_t r, uint8_t g, uint8_t b) {
  const uint16_t red   = MapColor(r);
  const uint16_t green = MapColor(g);
  const uint16_t blue  = MapColor(b);

  for (int b = kBitPlanes - pwm_bits_; b < kBitPlanes; ++b) {
    uint16_t mask = 1 << b;
    uint32_t bits = 0;
    if ((red & mask) == mask) {
      bits |= (1<<P0_R1)|(1<<P0_R2)|(1<<P1_R1)|(1<<P1_R2)|(1<<P2_R1)|(1<<P2_R2);
    }
    if ((green & mask) == mask) {
      bits |= (1<<P0_G1)|(1<<P0_G2)|(1<<P1_G1)|(1<<P1_G2)|(1<<P2_G1)|(1<<P2_G2);
    }
    if ((blue & mask) == mask) {
      bits |= (1<<P0_B1)|(1<<P0_B2)|(1<<P1_B1)|(1<<P1_B2)|(1<<P2_B1)|(1<<P2_B2);
    }
    for (int row = 0; row < double_rows_; ++row) {
      GPIO::Data *row_data = ValueAt(row, 0, b);
      for (int col = 0; col < columns_; ++col) {
        (row_data++)->SetMasked(bits, color_mask_);
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
  GPIO::Data *bits = ValueAt(y & row_mask_, x, min_bit_plane);
  uint32_t col = 0;

  // Manually expand the three cases for better performance.
  // TODO(hzeller): This is a bit repetetive. Test if it pays off to just
  // pre-calc rgb mask and apply.
  if (y < rows_) {
    // Parallel chain #1
    if (y < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        col = 0;
        if ((red & m) == m)   col |= (1<<P0_R1);
        if ((green & m) == m) col |= (1<<P0_G1);
        if ((blue & m) == m)  col |= (1<<P0_B1);
        bits->SetMasked(col, (1<<P0_R1)|(1<<P0_G1)|(1<<P0_B1));
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        col = 0;
        if ((red & m) == m)   col |= (1<<P0_R2);
        if ((green & m) == m) col |= (1<<P0_G2);
        if ((blue & m) == m)  col |= (1<<P0_B2);
        bits->SetMasked(col, (1<<P0_R2)|(1<<P0_G2)|(1<<P0_B2));
        bits += columns_;
      }
    }
  } else if (y >= rows_ && y < 2 * rows_) {
    // Parallel chain #2
    if (y - rows_ < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        col = 0;
        if ((red & m) == m)   col |= (1<<P1_R1);
        if ((green & m) == m) col |= (1<<P1_G1);
        if ((blue & m) == m)  col |= (1<<P1_B1);
        bits->SetMasked(col, (1<<P1_R1)|(1<<P1_G1)|(1<<P1_B1));
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        col = 0;
        if ((red & m) == m)   col |= (1<<P1_R2);
        if ((green & m) == m) col |= (1<<P1_G2);
        if ((blue & m) == m)  col |= (1<<P1_B2);
        bits->SetMasked(col, (1<<P1_R2)|(1<<P1_G2)|(1<<P1_B2));
        bits += columns_;
      }
    }
  } else {
    // Parallel chain #3
    if (y - 2*rows_ < double_rows_) {   // Upper sub-panel.
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        col = 0;
        if ((red & m) == m)   col |= (1<<P2_R1);
        if ((green & m) == m) col |= (1<<P2_G1);
        if ((blue & m) == m)  col |= (1<<P2_B1);
        bits->SetMasked(col, (1<<P2_R1)|(1<<P2_G1)|(1<<P2_B1));
        bits += columns_;
      }
    } else {
      for (int b = min_bit_plane; b < kBitPlanes; ++b) {
        const uint16_t m = 1 << b;
        col = 0;
        if ((red & m) == m)   col |= (1<<P2_R2);
        if ((green & m) == m) col |= (1<<P2_G2);
        if ((blue & m) == m)  col |= (1<<P2_B2);
        bits->SetMasked(col, (1<<P2_R2)|(1<<P2_G2)|(1<<P2_B2));
        bits += columns_;
      }
    }
  }
}

void Framebuffer::DumpToMatrix(GPIO *io) {
  if (!script_) InitializeScript(io);
  script_->RunOnce();
}

void Framebuffer::InitializeScript(GPIO *io) {
  if (!io) return;  // can't do that yet.
  assert(!script_);
  script_ = new HardwareScript(io, sOutputEnablePulser);

#define USE_PWM 1

  const int pwm_to_show = pwm_bits_;  // Local copy, might change in process.
  for (uint8_t d_row = 0; d_row < double_rows_; ++d_row) {
    script_->AppendGPIO(&row_address_[d_row]);

    // Rows can't be switched very quickly without ghosting, so we do the
    // full PWM of one row before switching rows.
    for (int b = kBitPlanes - pwm_to_show; b < kBitPlanes; ++b) {
      const GPIO::Data *row_data = ValueAt(d_row, 0, b);
      // While the output enable is still on, we can already clock in the next
      // data.
      for (int col = 0; col < columns_; ++col) {
        const GPIO::Data &out = *row_data++;
        script_->AppendGPIO(&out);         // col + reset clock
        script_->AppendGPIO(&clock_in_);   // Rising edge: clock color in.
      }
      script_->AppendGPIO(&clock_reset_);  // clock falling edge.

      // OE of the previous row-data must be finished before strobe.
#if USE_PWM
      //sOutputEnablePulser->WaitPulseFinished();
#else
      script_->AppendGPIO(&oe_end_);
#endif

      script_->AppendGPIO(&strobe_);     // set/reset in one go.

      // Now switch on for the sleep time necessary for that bit-plane.
#if USE_PWM
      script_->AppendPinPulse(b);
#else
      script_->AppendGPIO(&oe_start_);
#endif
    }
    //sOutputEnablePulser->WaitPulseFinished();
  }
  script_->FinishScript();
}
}  // namespace internal
}  // namespace rgb_matrix
