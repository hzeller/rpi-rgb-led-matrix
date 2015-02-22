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
  // Initialize RGB matrix with GPIO to write to. The "rows" are the number
  // of rows supported by the display, so 32 or 16. Number of "chained_display"s
  // tells many of these are daisy-chained together.
  // If "io" is not NULL, starts refreshing the screen immediately; you can
  // defer that by setting GPIO later with SetGPIO().
  RGBMatrix(GPIO *io, int rows = 32, int chained_displays = 1);
  virtual ~RGBMatrix();

  // Set GPIO output if it was not set already in constructor (otherwise: no-op).
  // Starts display refresh thread if this is the first setting.
  void SetGPIO(GPIO *io);

  // Set PWM bits used for output. Default is 11, but if you only deal with
  // simple comic-colors, 1 might be sufficient. Lower require less CPU.
  // Returns boolean to signify if value was within range.
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits();

  // Map brightness of output linearly to input with CIE1931 profile.
  void set_luminance_correct(bool on);
  bool luminance_correct() const;

  // -- Canvas interface. These write to the active FrameCanvas
  // (see documentation in canvas.h)
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue);
  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);

private:
  class Framebuffer;
  class UpdateThread;
  friend class UpdateThread;
  friend class FrameCanvas;

  // Updates the screen regularly.
  void UpdateScreen();

  Framebuffer *frame_;
  GPIO *io_;
  UpdateThread *updater_;
};
}  // end namespace rgb_matrix
#endif  // RPI_RGBMATRIX_H
