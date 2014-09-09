// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Controlling a 32x32 RGB matrix via GPIO.

#ifndef RPI_RGBMATRIX_H
#define RPI_RGBMATRIX_H

#include <stdint.h>
#include "gpio.h"
#include "canvas.h"

namespace rgb_matrix {
// The RGB matrix provides the framebuffer and the facilities to constantly
// update the LED matrix.
class RGBMatrix : public Canvas {
public:
  RGBMatrix(GPIO *io, int rows = 32, int chained_displays = 1);
  virtual ~RGBMatrix();

  virtual int width() const { return columns_; }
  virtual int height() const { return rows_; }
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue);
  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);

  // Some value between 1..7. Returns boolean to signify if value was within
  // range.
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits() { return pwm_bits_; }

  // Do luminance correction with gamma=~2.2. Experimental.
  void set_gamma_correct(bool on) { do_gamma_ = on; }
  bool gamma_correct() const { return do_gamma_; }

private:
  class UpdateThread;
  friend class UpdateThread;

  // Updates the screen, connected to the GPIO pins, once.
  // (If you were calling this before as public method in a thread to
  //  update the screen: this is not necessary anymore. The RGBMatrix does
  //  this now by itself already. You can get rid of that thread. Sorry for the
  //  API-change inconvenience).
  void UpdateScreen();

  const int rows_;     // Number of rows. 16 or 32.
  const int columns_;  // Number of columns. Number of chained boards * 32.

  uint8_t pwm_bits_;   // PWM bits to display.
  bool do_gamma_;      // do gamma correction.

  const int double_rows_;
  const uint8_t row_mask_;

  GPIO *const io_;
  UpdateThread *updater_;

  union IoBits {
    struct {
      // These reflect the GPIO mapping.
      unsigned int unused1 : 2;  // 0..1
      unsigned int output_enable : 1;  // 2
      unsigned int clock  : 1;   // 3
      unsigned int strobe : 1;   // 4
      unsigned int unused2 : 2;  // 5..6
      unsigned int row : 4;  // 7..10
      unsigned int unused3 : 6;  // 11..16
      unsigned int r1 : 1;   // 17
      unsigned int g1 : 1;   // 18
      unsigned int unused4 : 3;
      unsigned int b1 : 1;   // 22
      unsigned int r2 : 1;   // 23
      unsigned int g2 : 1;   // 24
      unsigned int b2 : 1;   // 25
    } bits;
    uint32_t raw;
    IoBits() : raw(0) {}
  };
 
  // The frame-buffer is organized in bitplanes.
  // Highest level (slowest to cycle through) are double rows.
  // For each double-row, we store pwm-bits columns of a bitplane.
  // Each bitplane-column is pre-filled IoBits, of which the colors are set.
  // Of course, that means that we store unrelated bits in the frame-buffer,
  // but it allows easy access in the critical section.
  IoBits *bitplane_framebuffer_;
  inline IoBits *ValueAt(int double_row, int column, int bit);
};

}  // end namespace rgb_matrix
#endif  // RPI_RGBMATRIX_H
