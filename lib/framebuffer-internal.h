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

namespace rgb_matrix {
class GPIO;
class PinPulser;
namespace internal {
// Internal representation of the frame-buffer that as well can
// write itself to GPIO.
// Our internal memory layout mimicks as much as possible what needs to be
// written out.
class Framebuffer {
public:
  Framebuffer(int rows, int columns, int parallel,
              int scan_mode,
              bool swap_green_blue, bool inverse_color);
  ~Framebuffer();

  // Initialize GPIO bits for output. Only call once.
  static void InitGPIO(GPIO *io, int rows, int parallel,
                       int pwm_lsb_nanoseconds);

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

  // Canvas-inspired methods, but we're not implementing this interface to not
  // have an unnecessary vtable.
  inline int width() const { return columns_; }
  inline int height() const { return height_; }
  void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);
  void Clear();
  void Fill(uint8_t red, uint8_t green, uint8_t blue);

private:
  // Define the type to do the pin-mapping. These are include fils
  // found in include directory hardware/$(name-of-mapping)
  #include "pin-mapping.h"

  // Map color
  inline uint16_t MapColor(uint8_t c);

  const int rows_;     // Number of rows. 16 or 32.
  const int parallel_; // Parallel rows of chains. 1 or 2.
  const int height_;   // rows * parallel
  const int columns_;  // Number of columns. Number of chained boards * 32.

  const int scan_mode_;
  const bool swap_green_blue_;
  const bool inverse_color_;

  uint8_t pwm_bits_;   // PWM bits to display.
  bool do_luminance_correct_;
  uint8_t brightness_;

  const int double_rows_;
  const uint8_t row_mask_;

  // The frame-buffer is organized in bitplanes.
  // Highest level (slowest to cycle through) are double rows.
  // For each double-row, we store pwm-bits columns of a bitplane.
  // Each bitplane-column is pre-filled IoBits, of which the colors are set.
  // Of course, that means that we store unrelated bits in the frame-buffer,
  // but it allows easy access in the critical section.
  IoBits *bitplane_buffer_;
  inline IoBits *ValueAt(int double_row, int column, int bit);
};
}  // namespace internal
}  // namespace rgb_matrix
#endif // RPI_RGBMATRIX_FRAMEBUFFER_INTERNAL_H
