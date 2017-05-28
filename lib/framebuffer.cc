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

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpio.h"

namespace rgb_matrix {
namespace internal {
enum {
  kBitPlanes = 11  // maximum usable bitplanes.
};

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

const struct HardwareMapping *Framebuffer::hardware_mapping_ = NULL;

Framebuffer::Framebuffer(int rows, int columns, int parallel,
                         int scan_mode,
                         const char *led_sequence, bool inverse_color,
                         PixelMapper **mapper)
  : rows_(rows),
    parallel_(parallel),
    height_(rows * parallel),
    columns_(columns),
    scan_mode_(scan_mode),
    led_sequence_(led_sequence), inverse_color_(inverse_color),
    pwm_bits_(kBitPlanes), do_luminance_correct_(true), brightness_(100),
    double_rows_(rows / SUB_PANELS_), row_mask_(double_rows_ - 1),
    buffer_size_(double_rows_ * columns_ * kBitPlanes * sizeof(gpio_bits_t)),
    shared_mapper_(mapper) {
  assert(hardware_mapping_ != NULL);   // Called InitHardwareMapping() ?
  assert(shared_mapper_ != NULL);  // Storage should be provided by RGBMatrix.
  assert(rows_ == 8 || rows_ == 16 || rows_ == 32 || rows_ == 64);
  if (parallel > hardware_mapping_->max_parallel_chains) {
    fprintf(stderr, "The %s GPIO mapping only supports %d parallel chain%s, "
            "but %d was requested.\n", hardware_mapping_->name,
            hardware_mapping_->max_parallel_chains,
            hardware_mapping_->max_parallel_chains > 1 ? "s" : "", parallel);
    abort();
  }
  assert(parallel >= 1 && parallel <= 3);

  bitplane_buffer_ = new gpio_bits_t[double_rows_ * columns_ * kBitPlanes];

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

// TODO: this should also be parsed from some special formatted string, e.g.
// {addr={22,23,24,25,15},oe=18,clk=17,strobe=4, p0={11,27,7,8,9,10},...}
/* static */ void Framebuffer::InitHardwareMapping(const char *named_hardware) {
  if (named_hardware == NULL || *named_hardware == '\0') {
    named_hardware = "regular";
  }

  struct HardwareMapping *mapping = NULL;
  for (HardwareMapping *it = matrix_hardware_mappings; it->name; ++it) {
    if (strcasecmp(it->name, named_hardware) == 0) {
      mapping = it;
      break;
    }
  }

  if (!mapping) {
    fprintf(stderr, "There is no hardware mapping named '%s'.\nAvailable: ",
            named_hardware);
    for (HardwareMapping *it = matrix_hardware_mappings; it->name; ++it) {
      if (it != matrix_hardware_mappings) fprintf(stderr, ", ");
      fprintf(stderr, "'%s'", it->name);
    }
    fprintf(stderr, "\n");
    abort();
  }

  if (mapping->max_parallel_chains == 0) {
    // Auto determine.
    struct HardwareMapping *h = mapping;
    if ((h->p0_r1 | h->p0_g1 | h->p0_g1 | h->p0_r2 | h->p0_g2 | h->p0_g2) > 0)
      ++mapping->max_parallel_chains;
    if ((h->p1_r1 | h->p1_g1 | h->p1_g1 | h->p1_r2 | h->p1_g2 | h->p1_g2) > 0)
      ++mapping->max_parallel_chains;
    if ((h->p2_r1 | h->p2_g1 | h->p2_g1 | h->p2_r2 | h->p2_g2 | h->p2_g2) > 0)
      ++mapping->max_parallel_chains;
  }
  hardware_mapping_ = mapping;
}

/* static */ void Framebuffer::InitGPIO(GPIO *io, int rows, int parallel,
                                        bool allow_hardware_pulsing,
                                        int pwm_lsb_nanoseconds) {
  if (sOutputEnablePulser != NULL)
    return;  // already initialized.

  const struct HardwareMapping &h = *hardware_mapping_;
  // Tell GPIO about all bits we intend to use.
  gpio_bits_t all_used_bits = 0;

  all_used_bits |= h.output_enable | h.clock | h.strobe;

  all_used_bits |= h.p0_r1 | h.p0_g1 | h.p0_b1 | h.p0_r2 | h.p0_g2 | h.p0_b2;
  if (parallel >= 2) {
    all_used_bits |= h.p1_r1 | h.p1_g1 | h.p1_b1 | h.p1_r2 | h.p1_g2 | h.p1_b2;
  }
  if (parallel >= 3) {
    all_used_bits |= h.p2_r1 | h.p2_g1 | h.p2_b1 | h.p2_r2 | h.p2_g2 | h.p2_b2;
  }

  const int double_rows = rows / 2;
  if (double_rows >= 32) all_used_bits |= h.e;
  if (double_rows >= 16) all_used_bits |= h.d;
  if (double_rows >=  8) all_used_bits |= h.c;
  if (double_rows >=  4) all_used_bits |= h.b;
  all_used_bits |= h.a;

  // Initialize outputs, make sure that all of these are supported bits.
  const uint32_t result = io->InitOutputs(all_used_bits);
  assert(result == all_used_bits);  // Impl: all bits declared in gpio.cc ?

  std::vector<int> bitplane_timings;
  for (int b = 0; b < kBitPlanes; ++b) {
    bitplane_timings.push_back(pwm_lsb_nanoseconds << b);
  }
  sOutputEnablePulser = PinPulser::Create(io, h.output_enable,
                                          allow_hardware_pulsing,
                                          bitplane_timings);
}

bool Framebuffer::SetPWMBits(uint8_t value) {
  if (value < 1 || value > kBitPlanes)
    return false;
  pwm_bits_ = value;
  return true;
}

inline gpio_bits_t *Framebuffer::ValueAt(int double_row, int column, int bit) {
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
  MapColors(r, g, b, &red, &green, &blue);

  const struct HardwareMapping &h = *hardware_mapping_;
  gpio_bits_t all_r = h.p0_r1 | h.p0_r2 | h.p1_r1 | h.p1_r2 | h.p2_r1 | h.p2_r2;
  gpio_bits_t all_g = h.p0_g1 | h.p0_g2 | h.p1_g1 | h.p1_g2 | h.p2_g1 | h.p2_g2;
  gpio_bits_t all_b = h.p0_b1 | h.p0_b2 | h.p1_b1 | h.p1_b2 | h.p2_b1 | h.p2_b2;

  for (int b = kBitPlanes - pwm_bits_; b < kBitPlanes; ++b) {
    uint16_t mask = 1 << b;
    gpio_bits_t plane_bits = 0;
    plane_bits |= ((red & mask) == mask)   ? all_r : 0;
    plane_bits |= ((green & mask) == mask) ? all_g : 0;
    plane_bits |= ((blue & mask) == mask)  ? all_b : 0;

    for (int row = 0; row < double_rows_; ++row) {
      uint32_t *row_data = ValueAt(row, 0, b);
      for (int col = 0; col < columns_; ++col) {
        *row_data++ = plane_bits;
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
  MapColors(r, g, b, &red, &green, &blue);

  uint32_t *bits = bitplane_buffer_ + pos;
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
    *bits = (*bits & designator_mask) | color_bits;
    bits += columns_;
  }
}

// Strange LED-mappings such as RBG or so are handled here.
gpio_bits_t Framebuffer::GetGpioFromLedSequence(char col,
                                                gpio_bits_t default_r,
                                                gpio_bits_t default_g,
                                                gpio_bits_t default_b) {
  const char *pos = strchr(led_sequence_, col);
  if (pos == NULL) pos = strchr(led_sequence_, tolower(col));
  if (pos == NULL) {
    fprintf(stderr, "LED sequence '%s' does not contain any '%c'.\n",
            led_sequence_, col);
    abort();
  }
  switch (pos - led_sequence_) {
  case 0: return default_r;
  case 1: return default_g;
  case 2: return default_b;
  }
  return default_r;  // String too long, should've been caught earlier.
}

void Framebuffer::InitDefaultDesignator(int x, int y, PixelDesignator *d) {
  const struct HardwareMapping &h = *hardware_mapping_;
  uint32_t *bits = ValueAt(y & row_mask_, x, 0);
  d->gpio_word = bits - bitplane_buffer_;
  d->r_bit = d->g_bit = d->b_bit = 0;
  if (y < rows_) {
    if (y < double_rows_) {
      d->r_bit = GetGpioFromLedSequence('R', h.p0_r1, h.p0_g1, h.p0_b1);
      d->g_bit = GetGpioFromLedSequence('G', h.p0_r1, h.p0_g1, h.p0_b1);
      d->b_bit = GetGpioFromLedSequence('B', h.p0_r1, h.p0_g1, h.p0_b1);
    } else {
      d->r_bit = GetGpioFromLedSequence('R', h.p0_r2, h.p0_g2, h.p0_b2);
      d->g_bit = GetGpioFromLedSequence('G', h.p0_r2, h.p0_g2, h.p0_b2);
      d->b_bit = GetGpioFromLedSequence('B', h.p0_r2, h.p0_g2, h.p0_b2);
    }
  }
  else if (y >= rows_ && y < 2 * rows_) {
    if (y - rows_ < double_rows_) {
      d->r_bit = GetGpioFromLedSequence('R', h.p1_r1, h.p1_g1, h.p1_b1);
      d->g_bit = GetGpioFromLedSequence('G', h.p1_r1, h.p1_g1, h.p1_b1);
      d->b_bit = GetGpioFromLedSequence('B', h.p1_r1, h.p1_g1, h.p1_b1);
    } else {
      d->r_bit = GetGpioFromLedSequence('R', h.p1_r2, h.p1_g2, h.p1_b2);
      d->g_bit = GetGpioFromLedSequence('G', h.p1_r2, h.p1_g2, h.p1_b2);
      d->b_bit = GetGpioFromLedSequence('B', h.p1_r2, h.p1_g2, h.p1_b2);
    }
  }
  else {
    if (y - 2*rows_ < double_rows_) {
      d->r_bit = GetGpioFromLedSequence('R', h.p2_r1, h.p2_g1, h.p2_b1);
      d->g_bit = GetGpioFromLedSequence('G', h.p2_r1, h.p2_g1, h.p2_b1);
      d->b_bit = GetGpioFromLedSequence('B', h.p2_r1, h.p2_g1, h.p2_b1);
    } else {
      d->r_bit = GetGpioFromLedSequence('R', h.p2_r2, h.p2_g2, h.p2_b2);
      d->g_bit = GetGpioFromLedSequence('G', h.p2_r2, h.p2_g2, h.p2_b2);
      d->b_bit = GetGpioFromLedSequence('B', h.p2_r2, h.p2_g2, h.p2_b2);
    }
  }

  d->mask = ~(d->r_bit | d->g_bit | d->b_bit);
}

void Framebuffer::Serialize(const char **data, size_t *len) const {
  *data = reinterpret_cast<const char*>(bitplane_buffer_);
  *len = buffer_size_;
}

bool Framebuffer::Deserialize(const char *data, size_t len) {
  if (len != buffer_size_) return false;
  memcpy(bitplane_buffer_, data, len);
  return true;
}

void Framebuffer::DumpToMatrix(GPIO *io) {
  const struct HardwareMapping &h = *hardware_mapping_;
  gpio_bits_t color_clk_mask = 0;  // Mask of bits while clocking in.
  color_clk_mask |= h.p0_r1 | h.p0_g1 | h.p0_b1 | h.p0_r2 | h.p0_g2 | h.p0_b2;
  if (parallel_ >= 2) {
    color_clk_mask |= h.p1_r1 | h.p1_g1 | h.p1_b1 | h.p1_r2 | h.p1_g2 | h.p1_b2;
  }
  if (parallel_ >= 3) {
    color_clk_mask |= h.p2_r1 | h.p2_g1 | h.p2_b1 | h.p2_r2 | h.p2_g2 | h.p2_b2;
  }

  color_clk_mask |= h.clock;

  const gpio_bits_t row_mask = h.a | h.b | h.c | h.d | h.e;

  gpio_bits_t row_address;

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

    row_address =  (d_row & 0x01) ? h.a : 0;
    row_address |= (d_row & 0x02) ? h.b : 0;
    row_address |= (d_row & 0x04) ? h.c : 0;
    row_address |= (d_row & 0x08) ? h.d : 0;
    row_address |= (d_row & 0x10) ? h.e : 0;

    // Rows can't be switched very quickly without ghosting, so we do the
    // full PWM of one row before switching rows.
    for (int b = kBitPlanes - pwm_to_show; b < kBitPlanes; ++b) {
      gpio_bits_t *row_data = ValueAt(d_row, 0, b);
      // While the output enable is still on, we can already clock in the next
      // data.
      for (int col = 0; col < columns_; ++col) {
        const gpio_bits_t &out = *row_data++;
        io->WriteMaskedBits(out, color_clk_mask);  // col + reset clock
        io->SetBits(h.clock);               // Rising edge: clock color in.
      }
      io->ClearBits(color_clk_mask);    // clock back to normal.

      // OE of the previous row-data must be finished before strobe.
      sOutputEnablePulser->WaitPulseFinished();

      // Setting address and strobing needs to happen in dark time.
      io->WriteMaskedBits(row_address, row_mask);  // Set row address
      io->SetBits(h.strobe);   // Strobe in the previously clocked in row.
      io->ClearBits(h.strobe);

      // Now switch on for the sleep time necessary for that bit-plane.
      sOutputEnablePulser->SendPulse(b);
    }
  }
}
}  // namespace internal
}  // namespace rgb_matrix
