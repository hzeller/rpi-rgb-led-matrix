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
#include "gpio.h"

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
  Framebuffer(int rows, int columns, int parallel);
  ~Framebuffer();

  // Initialize GPIO bits for output. Only call once.
  static void InitGPIO(GPIO *io, int parallel);

  // Set PWM bits used for output. Default is 11, but if you only deal with
  // simple comic-colors, 1 might be sufficient. Lower require less CPU.
  // Returns boolean to signify if value was within range.
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits() { return pwm_bits_; }

  // Map brightness of output linearly to input with CIE1931 profile.
  void set_luminance_correct(bool on) { do_luminance_correct_ = on; }
  bool luminance_correct() const { return do_luminance_correct_; }

  void SetBrightness(uint8_t brightness) {  brightness_ = brightness; }
  uint8_t brightness() { return brightness_; }

  void DumpToMatrix(GPIO *io);
  void InitializeScript(GPIO *io);

  // Canvas-inspired methods, but we're not implementing this interface to not
  // have an unnecessary vtable.
  inline int width() const { return columns_; }
  inline int height() const { return height_; }
  void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);
  void Clear();
  void Fill(uint8_t red, uint8_t green, uint8_t blue);

private:
  // Map color
  inline uint16_t MapColor(uint8_t c);

  const int rows_;     // Number of rows. 16 or 32.
#ifdef SUPPORT_MULTI_PARALLEL
  const int parallel_; // Parallel rows of chains. 1 or 2.
#endif
  const int height_;   // rows * parallel
  const int columns_;  // Number of columns. Number of chained boards * 32.

  uint8_t pwm_bits_;   // PWM bits to display.
  bool do_luminance_correct_;
  uint8_t brightness_;

  const int double_rows_;
  const uint8_t row_mask_;
  uint32_t color_mask_;

  // Currently experimental new layout. Reshuffling completely to better meet
  // constraints:
  //   - Make work with all RPis (and leave a chance to make work with Rev1)
  //   - put output_enable bit on pin 18, which is the PWM pin.
  //   - have the ability to leave out certain critical pins
  //      o If not chain=2 is used, then SDA/SCL and UART (TxD/RxD) are free
  //      o Only on 40Pin: if chain=0 is left out, SPI bus can be re-used.
  //   - current problem: can't use Rev1, as we use 27 for a parallel0 thing.
  enum IoBits {
    unused_0,   //  0  (only on RPi 1, Revision 1)
    unused_1,   //  1  (only on RPi 1, Revision 1)
    P2_G1,      //  2 P1-03 (masks SDA when parallel=3)
    P2_B1,      //  3 P1-05 (masks SCL when parallel=3)
    STROBE,     //  4 P1-07
    P1_G1,      //  5 P1-29 (only on A+/B+/Pi2)
    P1_B1,      //  6 P1-31 (only on A+/B+/Pi2)
    // TODO: be able to disable chain 0 for higher-pin RPis to gain SPI back.
    P0_B1,      //  7 P1-26 (masks: SPI0_CE1)
    P0_R2,      //  8 P1-24 (masks: SPI0_CE0)
    P0_G2,      //  9 P1-21 (masks: SPI0_MISO
    P0_B2,      // 10 P1-19 (masks: SPI0_MOSI)
    P0_R1,      // 11 P1-23 (masks: SPI0_SCKL)

    P1_R1,      // 12 P1-32 (only on A+/B+/Pi2)
    P1_G2,      // 13 P1-33 (only on A+/B+/Pi2)
    P2_R1,      // 14 P1-08 (masks TxD when parallel=3)
    unused_15,  // 15 P1-10 (RxD) - free to maybe use to receive data.
    P2_G2,      // 16 P1-36 (only on A+/B+/Pi2)

    CLOCK,      // 17 P1-11

    OUTPUT_ENABLE,  // 18 P1-12 (PWM pin: our timing)
    P1_R2,      // 19 P1-35 (only on A+/B+/Pi2)
    P1_B2,      // 20 P1-38 (only on A+/B+/Pi2)
    P2_B2,      // 21 P1-40 (only on A+/B+/Pi2)

    ROW_A,      // 22 P1-15  // row bits.
    ROW_B,      // 23 P1-16
    ROW_C,      // 24 P1-18
    ROW_D,      // 25 P1-22

    P2_R2,      // 26 P1-37 (only on A+/B+/Pi2)
    P0_G1,      // 27 P1-13 (Not on RPi1, Rev1)
  };

  // The frame-buffer is organized in bitplanes.
  // Highest level (slowest to cycle through) are double rows.
  // For each double-row, we store pwm-bits columns of a bitplane.
  // Each bitplane-column is pre-filled GPIO::Data, of which the colors are set.
  // Of course, that means that we store unrelated bits in the frame-buffer,
  // but it allows easy access in the critical section.
  GPIO::Data *bitplane_buffer_;

  // Pre-calculated bits to send out to GPIO. Stored here, as their address is
  // referenced by DMA.
  GPIO::Data clock_in_;
  GPIO::Data clock_reset_;
  GPIO::Data strobe_;
  GPIO::Data row_address_[16];

  // Temporary until we have something that does timing.
  GPIO::Data oe_start_;
  GPIO::Data oe_end_;

  HardwareScript *script_;

  inline GPIO::Data *ValueAt(int double_row, int column, int bit_plane);
};
}  // namespace internal
}  // namespace rgb_matrix
#endif // RPI_RGBMATRIX_FRAMEBUFFER_INTERNAL_H
