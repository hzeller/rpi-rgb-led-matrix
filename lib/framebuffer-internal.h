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

  void SetBrightness(float brightness) {  brightness_ = brightness; }
  float brightness() { return brightness_; }

  void DumpToMatrix(GPIO *io);

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
  float brightness_;

  const int double_rows_;
  const uint8_t row_mask_;

#ifdef ADAFRUIT_RGBMATRIX_HAT
  // Adafruit made a HAT to work with this library, but it has a slightly
  // different GPIO mapping. This is this mapping. See #else for regular mapping.
  union IoBits {
    struct {
      // This bitset reflects the GPIO mapping. The naming of the
      // pins of type 'p0_r1' means 'first parallel chain, red-bit one'
      unsigned int unused_0_3         : 4;  // 0..3
      unsigned int output_enable      : 1;  // 4
      unsigned int p0_r1              : 1;  // 5
      unsigned int p0_b1              : 1;  // 6
      unsigned int unused_7_11        : 5;  // 7..11
      unsigned int p0_r2              : 1;  // 12
      unsigned int p0_g1              : 1;  // 13
      unsigned int unused_14_15       : 2;  // 14,15
      unsigned int p0_g2              : 1;  // 16
      unsigned int clock              : 1;  // 17
      unsigned int unused_18_19       : 2;  // 18,19
      unsigned int d                  : 1;  // 20
      unsigned int strobe             : 1;  // 21
      unsigned int a                  : 1;  // 22
      unsigned int p0_b2              : 1;  // 23
      unsigned int unused_24_25       : 2;  // 24,25
      unsigned int b                  : 1;  // 26
      unsigned int c                  : 1;  // 27
    } bits;
    uint32_t raw;
    IoBits() : raw(0) {}
  };
#elif defined(EXPERIMENTAL_NEW_PINOUT)
  // Currently experimental new layout. Reshuffling completely to better meet
  // constraints:
  //   - Make work with all RPis (and leave a chance to make work with Rev1)
  //   - put output_enable bit on pin 18, which is the PWM pin.
  //   - have the ability to leave out certain critical pins
  //      o If not chain=2 is used, then SDA/SCL and UART (TxD/RxD) are free
  //      o Only on 40Pin: if chain=0 is left out, SPI bus can be re-used.
  union IoBits {
    struct {
      // This bitset reflects the GPIO mapping. The naming of the
      // pins of type 'p0_r1' means 'first parallel chain, red-bit one'
      //                                 GPIO Header-pos
      unsigned int unused_0_1     : 2;  //  0..1  (only on RPi 1, Revision 1)
      unsigned int p2_g1          : 1;  //  2 P1-03 (masks SDA when parallel=3)
      unsigned int p2_b1          : 1;  //  3 P1-05 (masks SCL when parallel=3)
      unsigned int strobe         : 1;  //  4 P1-07
      unsigned int p1_g1          : 1;  //  5 P1-29 (only on A+/B+/Pi2)
      unsigned int p1_b1          : 1;  //  6 P1-31 (only on A+/B+/Pi2)
      // TODO: be able to disable chain 0 for higher-pin RPis to gain SPI back.
      unsigned int p0_b1          : 1;  //  7 P1-26 (masks: SPI0_CE1)
      unsigned int p0_r2          : 1;  //  8 P1-24 (masks: SPI0_CE0)
      unsigned int p0_g2          : 1;  //  9 P1-21 (masks: SPI0_MISO
      unsigned int p0_b2          : 1;  // 10 P1-19 (masks: SPI0_MOSI)
      unsigned int p0_r1          : 1;  // 11 P1-23 (masks: SPI0_SCKL)

      unsigned int p1_r1          : 1;  // 12 P1-32 (only on A+/B+/Pi2)
      unsigned int p1_g2          : 1;  // 13 P1-33 (only on A+/B+/Pi2)
      unsigned int p2_r1          : 1;  // 14 P1-08 (masks TxD when parallel=3)
      unsigned int unused_15      : 1;  // 15 P1-10 (RxD)
      unsigned int p2_g2          : 1;  // 16 P1-36 (only on A+/B+/Pi2)

      unsigned int clock          : 1;  // 17 P1-11

      unsigned int output_enable  : 1;  // 18 P1-12 (PWM pin: our timing)
      unsigned int p1_r2          : 1;  // 19 P1-35 (only on A+/B+/Pi2)
      unsigned int p1_b2          : 1;  // 20 P1-38 (only on A+/B+/Pi2)
      unsigned int p2_b2          : 1;  // 21 P1-40 (only on A+/B+/Pi2)

      unsigned int a              : 1;  // 22 P1-15  // row bits.
      unsigned int b              : 1;  // 23 P1-16
      unsigned int c              : 1;  // 24 P1-18
      unsigned int d              : 1;  // 25 P1-22

      unsigned int p2_r2          : 1;  // 26 P1-37 (only on A+/B+/Pi2)
      unsigned int p0_g1          : 1;  // 27 P1-13 (Not on RPi1, Rev1)
    } bits;
    uint32_t raw;
    IoBits() : raw(0) {}
  };
#else
  union IoBits {
    struct {
      // This bitset reflects the GPIO mapping. The naming of the
      // pins of type 'p0_r1' means 'first parallel chain, red-bit one'
#ifdef SUPPORT_MULTI_PARALLEL
      unsigned int unused_0_1         : 2;  // 0..1   (only on RPi 1, Revision 1)
      unsigned int p2_g1              : 1;  // 2      (masks SDA when parallel=3)
      unsigned int p2_b1              : 1;  // 3      (masks SCL when parallel=3)
#else
      // The Revision1 and Revision2 boards have different GPIO mappings
      // on the pins 2 and 3. Just use both interpretations.
      // To keep the I2C pins free, we don't use these anymore.
      // We keep this backward compatible unless SUPPORT_MULTI_PARALLEL
      // is explicitly chosen.
      unsigned int output_enable_rev1 : 1;  // 0      (RPi 1, Revision 1)
      unsigned int clock_rev1         : 1;  // 1      (RPi 1, Revision 1)
      unsigned int output_enable_rev2 : 1;  // 2      (Pi1.Rev2; masks: I2C SDA)
      unsigned int clock_rev2         : 1;  // 3      (Pi1.Rev2; masks: I2C SCL)
#endif
      unsigned int strobe             : 1;  // 4
      unsigned int p1_g1              : 1;  // 5      (only on A+/B+/Pi2)
      unsigned int p1_b1              : 1;  // 6      (only on A+/B+/Pi2)
      // row: 7..10, but separated as seprate bits to make it easier to shuffle
      // bits if needed.
      unsigned int a                  : 1;  // 7      (masks: SPI0_CE1)
      unsigned int b                  : 1;  // 8      (masks: SPI0_CE0)
      unsigned int c                  : 1;  // 9      (masks: SPI0_MISO)
      unsigned int d                  : 1;  // 10     (masks: SPI0_MOSI)
      unsigned int clock              : 1;  // 11     (masks: SPI0_SCKL)
      unsigned int p1_r1              : 1;  // 12     (only on A+/B+/Pi2)
      unsigned int p1_g2              : 1;  // 13     (only on A+/B+/Pi2)
      unsigned int p2_r1              : 1;  // 14     (masks TxD when parallel=3)
      unsigned int p2_r2              : 1;  // 15     (masks RxD when parallel=3)
      unsigned int unused_16          : 1;  // 16     (only on A+/B+/Pi2)
      unsigned int p0_r1              : 1;  // 17
      unsigned int p0_g1              : 1;  // 18
      unsigned int p1_r2              : 1;  // 19     (only on A+/B+/Pi2)
      unsigned int p1_b2              : 1;  // 20     (only on A+/B+/Pi2)
      unsigned int p2_b2              : 1;  // 21     (only on A+/B+/Pi2)
      unsigned int p0_b1              : 1;  // 22
      unsigned int p0_r2              : 1;  // 23
      unsigned int p0_g2              : 1;  // 24
      unsigned int p0_b2              : 1;  // 25
      unsigned int p2_g2              : 1;  // 26     (only on A+/B+/Pi2)
      unsigned int output_enable      : 1;  // 27     (Not on RPi1, Rev1)
    } bits;
    uint32_t raw;
    IoBits() : raw(0) {}
  };
#endif

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
