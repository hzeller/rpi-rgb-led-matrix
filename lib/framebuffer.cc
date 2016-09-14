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

// Leave this in here for a while.
#if defined(ADAFRUIT_RGBMATRIX_HAT) && !FYI_ADAFRUIT_HAT_PIN_MAPPING_INCLUDED_
# error "You are using an old way to select the Adafruit HAT."
# error "Set HARDWARE_DESC=adafruit-hat instead of ADAFRUIT_RGBMATRIX_HAT"
# error "Check out https://github.com/hzeller/rpi-rgb-led-matrix#switch-the-pinout"
#endif

// Remind forgetful Makefile option editors
#if defined(ADAFRUIT_RGBMATRIX_HAT_PWM) && !FYI_ADAFRUIT_HAT_PIN_MAPPING_INCLUDED_
#  error "You have defined ADAFRUIT_RGBMATRIX_HAT_PWM which is for the Adafruit HAT. So you also need to set HARDWARE_DESC=adafruit-hat"
#endif

#ifndef RGB_PARALLEL_CHAINS
# error "Your pin-mapping.h file should contain an RGB_PARALLEL_CHAINS macro"
#endif

// We need one global instance of a timing correct pulser. There are different
// implementations depending on the context.
static PinPulser *sOutputEnablePulser = NULL;

#ifdef ONLY_SINGLE_SUB_PANEL
#  define SUB_PANELS_ 1
#else
#  define SUB_PANELS_ 2
#endif

PixelDesignator *PixelMapper::get(int x, int y) {
  if (x < 0 || y < 0 || x >= width_ || y >= height_)
    return NULL;
  return buffer_ + (y*width_) + x;
}

PixelMapper::PixelMapper(int width, int height)
  : width_(width), height_(height),
    buffer_(new PixelDesignator[width * height]) {
}

PixelMapper::~PixelMapper() {
  delete [] buffer_;
}

Framebuffer::Framebuffer(int rows, int columns, int parallel,
                         int scan_mode,
                         bool swap_green_blue, bool inverse_color,
                         PixelMapper **mapper)
  : rows_(rows),
    parallel_(parallel),
    height_(rows * parallel),
    columns_(columns),
    scan_mode_(scan_mode),
    swap_green_blue_(swap_green_blue), inverse_color_(inverse_color),
    pwm_bits_(kBitPlanes), do_luminance_correct_(true), brightness_(100),
    double_rows_(rows / SUB_PANELS_), row_mask_(double_rows_ - 1),
    shared_mapper_(mapper) {
  assert(shared_mapper_ != NULL);  // Storage should be provided by RGBMatrix.
  assert(rows_ == 8 || rows_ == 16 || rows_ == 32 || rows_ == 64);
  assert(parallel >= 1 && parallel <= 3);
  if (parallel > RGB_PARALLEL_CHAINS) {
    fprintf(stderr, "Parallel of %d is higher than the supported "
            "RGB_PARALLEL_CHAINS of %d\n", parallel, RGB_PARALLEL_CHAINS);
    assert(parallel == 1);
  }

  bitplane_buffer_ = new IoBits [double_rows_ * columns_ * kBitPlanes];

  // If we're the first Framebuffer created, the shared PixelMapper is
  // still NULL, so create one.
  // The first PixelMapper represents the physical layout of a standard matrix
  // with the specific knowledge of the framebuffer, setting up PixelDesignators
  // in a way that they are useful for this Framebuffer.
  //
  // Newly created PixelMappers then can just copy around PixelDesignators
  // from the parent PixelMapper opaquely without having to know the details.
  if (*shared_mapper_ == NULL) {
    *shared_mapper_ = new PixelMapper(columns_, height_);
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < columns_; ++x) {
        InitDefaultDesignator(x, y, (*shared_mapper_)->get(x, y));
      }
    }
  }

  Clear();
}

Framebuffer::~Framebuffer() {
  delete [] bitplane_buffer_;
}

/* static */ void Framebuffer::InitGPIO(GPIO *io, int rows, int parallel,
                                        bool allow_hardware_pulsing,
                                        int pwm_lsb_nanoseconds) {
  if (sOutputEnablePulser != NULL)
    return;  // already initialized.

  // Tell GPIO about all bits we intend to use.
  IoBits b;
  b.raw = 0;

#ifdef PI_REV1_RGB_PINOUT
  // This is only to be enabled with classic pinout.
  b.bits.output_enable_rev1 = b.bits.output_enable_rev2 = 1;
  b.bits.clock_rev1 = b.bits.clock_rev2 = 1;
#endif

  b.bits.output_enable = 1;
  b.bits.clock = 1;
  b.bits.strobe = 1;

  b.bits.p0_r1 = b.bits.p0_g1 = b.bits.p0_b1 = 1;
  b.bits.p0_r2 = b.bits.p0_g2 = b.bits.p0_b2 = 1;

#if RGB_PARALLEL_CHAINS >= 2
  if (parallel >= 2) {
    b.bits.p1_r1 = b.bits.p1_g1 = b.bits.p1_b1 = 1;
    b.bits.p1_r2 = b.bits.p1_g2 = b.bits.p1_b2 = 1;
  }
#endif

#if RGB_PARALLEL_CHAINS >= 3
  if (parallel >= 3) {
    b.bits.p2_r1 = b.bits.p2_g1 = b.bits.p2_b1 = 1;
    b.bits.p2_r2 = b.bits.p2_g2 = b.bits.p2_b2 = 1;
  }
#endif

  const int double_rows = rows / 2;
  if (double_rows >= 32) b.bits.e = 1;
  if (double_rows >= 16) b.bits.d = 1;
  if (double_rows >=  8) b.bits.c = 1;
  if (double_rows >=  4) b.bits.b = 1;
  b.bits.a = 1;

  // Initialize outputs, make sure that all of these are supported bits.
  const uint32_t result = io->InitOutputs(b.raw);
  assert(result == b.raw);

  // Now, set up the PinPulser for output enable.
  IoBits output_enable_bits;
#ifdef PI_REV1_RGB_PINOUT
  output_enable_bits.bits.output_enable_rev1
    = output_enable_bits.bits.output_enable_rev2 = 1;
#endif
  output_enable_bits.bits.output_enable = 1;

  std::vector<int> bitplane_timings;
  for (int b = 0; b < kBitPlanes; ++b) {
    bitplane_timings.push_back(pwm_lsb_nanoseconds << b);
  }
  sOutputEnablePulser = PinPulser::Create(io, output_enable_bits.raw,
                                          allow_hardware_pulsing,
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

void Framebuffer::Clear() {
  if (inverse_color_) {
    Fill(0, 0, 0);
  } else  {
    // Cheaper.
    memset(bitplane_buffer_, 0,
           sizeof(*bitplane_buffer_) * double_rows_ * columns_ * kBitPlanes);
  }
}

// Do CIE1931 luminance correction and scale to output bitplanes
static uint16_t luminance_cie1931(uint8_t c, uint8_t brightness) {
  float out_factor = ((1 << kBitPlanes) - 1);
  float v = (float) c * brightness / 255.0;
  return out_factor * ((v <= 8) ? v / 902.3 : pow((v + 16) / 116.0, 3));
}

struct ColorLookup {
  uint16_t color[256];
};
static ColorLookup *CreateLuminanceCIE1931LookupTable() {
  ColorLookup *for_brightness = new ColorLookup[100];
  for (int c = 0; c < 256; ++c)
    for (int b = 0; b < 100; ++b)
      for_brightness[b].color[c] = luminance_cie1931(c, b + 1);

  return for_brightness;
}

static inline uint16_t CIEMapColor(uint8_t brightness, uint8_t c) {
  static ColorLookup *luminance_lookup = CreateLuminanceCIE1931LookupTable();
  return luminance_lookup[brightness - 1].color[c];
}

// Non luminance correction. TODO: consider getting rid of this.
static inline uint16_t DirectMapColor(uint8_t brightness, uint8_t c) {
  // simple scale down the color value
  c = c * brightness / 100;

  enum {shift = kBitPlanes - 8};  //constexpr; shift to be left aligned.
  return (shift > 0) ? (c << shift) : (c >> -shift);
}

inline void Framebuffer::MapColors(
  uint8_t r, uint8_t g, uint8_t b,
  uint16_t *red, uint16_t *green, uint16_t *blue) {

  if (do_luminance_correct_) {
    *red   = CIEMapColor(brightness_, r);
    *green = CIEMapColor(brightness_, g);
    *blue  = CIEMapColor(brightness_, b);
  } else {
    *red   = DirectMapColor(brightness_, r);
    *green = DirectMapColor(brightness_, g);
    *blue  = DirectMapColor(brightness_, b);
  }

  if (inverse_color_) {
    *red = ~(*red);
    *green = ~(*green);
    *blue = ~(*blue);
  }
}

void Framebuffer::Fill(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t red, green, blue;
  if (!swap_green_blue_) {
    MapColors(r, g, b, &red, &green, &blue);
  } else {
    MapColors(r, g, b, &red, &blue, &green);
  }

  for (int b = kBitPlanes - pwm_bits_; b < kBitPlanes; ++b) {
    uint16_t mask = 1 << b;
    IoBits plane_bits;
    plane_bits.raw = 0;
    plane_bits.bits.p0_r1 = plane_bits.bits.p0_r2 = (red & mask) == mask;
    plane_bits.bits.p0_g1 = plane_bits.bits.p0_g2 = (green & mask) == mask;
    plane_bits.bits.p0_b1 = plane_bits.bits.p0_b2 = (blue & mask) == mask;

#if RGB_PARALLEL_CHAINS > 1
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

int Framebuffer::width() const { return (*shared_mapper_)->width(); }
int Framebuffer::height() const { return (*shared_mapper_)->height(); }

void Framebuffer::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  const PixelDesignator *designator = (*shared_mapper_)->get(x, y);
  if (designator == NULL) return;
  const int pos = designator->gpio_word;
  if (pos < 0) return;  // non-used pixel marker.

  uint16_t red, green, blue;
  if (!swap_green_blue_) {
    MapColors(r, g, b, &red, &green, &blue);
  } else {
    MapColors(r, g, b, &red, &blue, &green);
  }

  IoBits *bits = bitplane_buffer_ + pos;
  const int min_bit_plane = kBitPlanes - pwm_bits_;
  bits += (columns_ * min_bit_plane);
  const uint32_t r_bits = designator->r_bit;
  const uint32_t g_bits = designator->g_bit;
  const uint32_t b_bits = designator->b_bit;
  const uint32_t designator_mask = designator->mask;
  for (int b = min_bit_plane; b < kBitPlanes; ++b) {
    const uint16_t mask = 1 << b;
    uint32_t color_bits = 0;
    if (red & mask)   color_bits |= r_bits;
    if (green & mask) color_bits |= g_bits;
    if (blue & mask)  color_bits |= b_bits;
    bits->raw = (bits->raw & designator_mask) | color_bits;
    bits += columns_;
  }
}

void Framebuffer::InitDefaultDesignator(int x, int y, PixelDesignator *d) {
  IoBits *bits = ValueAt(y & row_mask_, x, 0);
  d->gpio_word = bits - bitplane_buffer_;
  d->r_bit = d->g_bit = d->b_bit = 0;
  if (y < rows_) {
    if (y < double_rows_) {
      color_bits(&d->r_bit).bits.p0_r1 = 1;
      color_bits(&d->g_bit).bits.p0_g1 = 1;
      color_bits(&d->b_bit).bits.p0_b1 = 1;
    } else {
      color_bits(&d->r_bit).bits.p0_r2 = 1;
      color_bits(&d->g_bit).bits.p0_g2 = 1;
      color_bits(&d->b_bit).bits.p0_b2 = 1;
    }
  }
#if RGB_PARALLEL_CHAINS >= 2
  else if (y >= rows_ && y < 2 * rows_) {
    if (y - rows_ < double_rows_) {
      color_bits(&d->r_bit).bits.p1_r1 = 1;
      color_bits(&d->g_bit).bits.p1_g1 = 1;
      color_bits(&d->b_bit).bits.p1_b1 = 1;
    } else {
      color_bits(&d->r_bit).bits.p1_r2 = 1;
      color_bits(&d->g_bit).bits.p1_g2 = 1;
      color_bits(&d->b_bit).bits.p1_b2 = 1;
    }
  }
#endif
#if RGB_PARALLEL_CHAINS >= 3
  else {
    if (y - 2*rows_ < double_rows_) {
      color_bits(&d->r_bit).bits.p2_r1 = 1;
      color_bits(&d->g_bit).bits.p2_g1 = 1;
      color_bits(&d->b_bit).bits.p2_b1 = 1;
    } else {
      color_bits(&d->r_bit).bits.p2_r2 = 1;
      color_bits(&d->g_bit).bits.p2_g2 = 1;
      color_bits(&d->b_bit).bits.p2_b2 = 1;
    }
  }
#endif
  d->mask = ~(d->r_bit | d->g_bit | d->b_bit);
}

void Framebuffer::DumpToMatrix(GPIO *io) {
  IoBits color_clk_mask;   // Mask of bits we need to set while clocking in.
  color_clk_mask.bits.p0_r1
    = color_clk_mask.bits.p0_g1
    = color_clk_mask.bits.p0_b1
    = color_clk_mask.bits.p0_r2
    = color_clk_mask.bits.p0_g2
    = color_clk_mask.bits.p0_b2 = 1;

#if RGB_PARALLEL_CHAINS >= 2
  if (parallel_ >= 2) {
    color_clk_mask.bits.p1_r1
      = color_clk_mask.bits.p1_g1
      = color_clk_mask.bits.p1_b1
      = color_clk_mask.bits.p1_r2
      = color_clk_mask.bits.p1_g2
      = color_clk_mask.bits.p1_b2 = 1;
  }
#endif

#if RGB_PARALLEL_CHAINS >= 3
  if (parallel_ >= 3) {
    color_clk_mask.bits.p2_r1
      = color_clk_mask.bits.p2_g1
      = color_clk_mask.bits.p2_b1
      = color_clk_mask.bits.p2_r2
      = color_clk_mask.bits.p2_g2
      = color_clk_mask.bits.p2_b2 = 1;
  }
#endif

#ifdef PI_REV1_RGB_PINOUT
  color_clk_mask.bits.clock_rev1 = color_clk_mask.bits.clock_rev2 = 1;
#endif
  color_clk_mask.bits.clock = 1;

  IoBits row_mask;
  row_mask.bits.a = row_mask.bits.b = row_mask.bits.c
    = row_mask.bits.d = row_mask.bits.e = 1;

  IoBits clock, strobe, row_address;
#ifdef PI_REV1_RGB_PINOUT
  clock.bits.clock_rev1 = clock.bits.clock_rev2 = 1;
#endif
  clock.bits.clock = 1;
  strobe.bits.strobe = 1;

  // info needed for interlace mode.
  uint8_t rot_bits = 0;
  switch (double_rows_) {
  case  4: rot_bits = 1; break;
  case  8: rot_bits = 2; break;
  case 16: rot_bits = 3; break;
  case 32: rot_bits = 4; break;
  }

  const int pwm_to_show = pwm_bits_;  // Local copy, might change in process.
  for (uint8_t row_loop = 0; row_loop < double_rows_; ++row_loop) {
    uint8_t d_row;
    switch (scan_mode_) {
    case 0:  // progressive
    default:
      d_row = row_loop;
      break;

    case 1:  // interlaced
      d_row = ((row_loop << 1) | (row_loop >> rot_bits)) & row_mask_;
    }

    row_address.bits.a = d_row;
    row_address.bits.b = d_row >> 1;
    row_address.bits.c = d_row >> 2;
    row_address.bits.d = d_row >> 3;
    row_address.bits.e = d_row >> 4;

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

      // Setting address and strobing needs to happen in dark time.
      io->WriteMaskedBits(row_address.raw, row_mask.raw);  // Set row address
      io->SetBits(strobe.raw);   // Strobe in the previously clocked in row.
      io->ClearBits(strobe.raw);

      // Now switch on for the sleep time necessary for that bit-plane.
      sOutputEnablePulser->SendPulse(b);
    }
  }
}
}  // namespace internal
}  // namespace rgb_matrix
