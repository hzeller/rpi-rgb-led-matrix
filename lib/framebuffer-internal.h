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
#ifndef RPI_RGBMATRIX_FRAMEBUFFER_INTERNAL_H
#define RPI_RGBMATRIX_FRAMEBUFFER_INTERNAL_H

#include <stdint.h>
#include <stdlib.h>

#include "hardware-mapping.h"

namespace rgb_matrix {
class GPIO;
class PinPulser;
namespace internal {
class RowAddressSetter;

// An opaque type used within the framebuffer that can be used
// to copy between PixelMappers.
struct PixelDesignator {
  PixelDesignator() : gpio_word(-1), r_bit(0), g_bit(0), b_bit(0), mask(~0){}
  int gpio_word;
  uint32_t r_bit;
  uint32_t g_bit;
  uint32_t b_bit;
  uint32_t mask;
};

class PixelMapper {
public:
  PixelMapper(int width, int height);
  ~PixelMapper();

  // Get a writable version of the PixelDesignator. Outside Framebuffer used
  // by the RGBMatrix to re-assign mappings to new PixelMappers.
  PixelDesignator *get(int x, int y);

  inline int width() const { return width_; }
  inline int height() const { return height_; }

private:
  const int width_;
  const int height_;
  PixelDesignator *const buffer_;
};

// Internal representation of the frame-buffer that as well can
// write itself to GPIO.
// Our internal memory layout mimicks as much as possible what needs to be
// written out.
class Framebuffer {
public:
  Framebuffer(int rows, int columns, int parallel,
              int scan_mode,
              const char* led_sequence, bool inverse_color,
              PixelMapper **mapper);
  ~Framebuffer();

  // Initialize GPIO bits for output. Only call once.
  static void InitHardwareMapping(const char *named_hardware);
  static void InitGPIO(GPIO *io, int rows, int parallel,
                       bool allow_hardware_pulsing,
                       int pwm_lsb_nanoseconds,
                       int row_address_type);

  // Set PWM bits used for output. Default is 11, but if you only deal with
  // simple comic-colors, 1 might be sufficient. Lower require less CPU.
  // Returns boolean to signify if value was within range.
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits() { return pwm_bits_; }

  // Map brightness of output linearly to input with CIE1931 profile.
  void set_luminance_correct(bool on) { do_luminance_correct_ = on; }
  bool luminance_correct() const { return do_luminance_correct_; }

  // Set brightness in percent; range=1..100
  // This will only affect newly set pixels.
  void SetBrightness(uint8_t b) {
    brightness_ = (b <= 100 ? (b != 0 ? b : 1) : 100);
  }
  uint8_t brightness() { return brightness_; }

  void DumpToMatrix(GPIO *io);

  void Serialize(const char **data, size_t *len) const;
  bool Deserialize(const char *data, size_t len);
  void CopyFrom(const Framebuffer *other);

  // Canvas-inspired methods, but we're not implementing this interface to not
  // have an unnecessary vtable.
  int width() const;
  int height() const;
  void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);
  void Clear();
  void Fill(uint8_t red, uint8_t green, uint8_t blue);

private:
  static const struct HardwareMapping *hardware_mapping_;
  static RowAddressSetter *row_setter_;

  // This returns the gpio-bit for given color (one of 'R', 'G', 'B'). This is
  // returning the right value in case led_sequence_ is _not_ "RGB"
  gpio_bits_t GetGpioFromLedSequence(char col,
                                     gpio_bits_t default_r,
                                     gpio_bits_t default_g,
                                     gpio_bits_t default_b);

  void InitDefaultDesignator(int x, int y, PixelDesignator *designator);
  inline void  MapColors(uint8_t r, uint8_t g, uint8_t b,
                         uint16_t *red, uint16_t *green, uint16_t *blue);
  const int rows_;     // Number of rows. 16 or 32.
  const int parallel_; // Parallel rows of chains. 1 or 2.
  const int height_;   // rows * parallel
  const int columns_;  // Number of columns. Number of chained boards * 32.

  const int scan_mode_;
  const char *const led_sequence_;  // Some LEDs are mapped differently.
  const bool inverse_color_;

  uint8_t pwm_bits_;   // PWM bits to display.
  bool do_luminance_correct_;
  uint8_t brightness_;

  const int double_rows_;
  const size_t buffer_size_;

  // The frame-buffer is organized in bitplanes.
  // Highest level (slowest to cycle through) are double rows.
  // For each double-row, we store pwm-bits columns of a bitplane.
  // Each bitplane-column is pre-filled IoBits, of which the colors are set.
  // Of course, that means that we store unrelated bits in the frame-buffer,
  // but it allows easy access in the critical section.
  gpio_bits_t *bitplane_buffer_;
  inline gpio_bits_t *ValueAt(int double_row, int column, int bit);

  PixelMapper **shared_mapper_;  // Storage in RGBMatrix.
};
}  // namespace internal
}  // namespace rgb_matrix
#endif // RPI_RGBMATRIX_FRAMEBUFFER_INTERNAL_H
