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

// Controlling 16x32 or 32x32 RGB matrixes via GPIO. It allows daisy chaining
// of a string of these, and also connecting a parallel string on newer
// Raspberry Pis with more GPIO pins available.

#ifndef RPI_RGBMATRIX_H
#define RPI_RGBMATRIX_H

#include <stdint.h>
#include <vector>

#include "gpio.h"
#include "canvas.h"
#include "thread.h"

namespace rgb_matrix {
class FrameCanvas;   // Canvas for Double- and Multibuffering
namespace internal { class Framebuffer; }

// The RGB matrix provides the framebuffer and the facilities to constantly
// update the LED matrix.
//
// This implement the Canvas interface that represents the display with
// (32 * chained_displays)x(rows * parallel_displays) pixels.
//
// If can do multi-buffering using the CreateFrameCanvas() and SwapOnVSync()
// methods. This is useful for animations and to prevent tearing.
//
// If you arrange the panels in a different way in the physical space, write
// a delegating Canvas that does coordinate remapping, like the
// LargeSquare64x64Canvas in demo-main.cc.
class RGBMatrix : public Canvas {
public:
  // Initialize RGB matrix with GPIO to write to.
  //
  // The "rows" are the number
  // of rows supported by the display, so 32 or 16. Number of "chained_display"s
  // tells many of these are daisy-chained together (output of one connected
  // to input of next).
  //
  // The "parallel_display" number determines if there is one or two displays
  // connected in parallel to the GPIO port - this only works with newer
  // Raspberry Pi that have 40 interface pins.
  //
  // If "io" is not NULL, starts refreshing the screen immediately; you can
  // defer that by setting GPIO later with SetGPIO().
  //
  // The resulting canvas is (rows * parallel_displays) high and
  // (32 * chained_displays) wide.
  RGBMatrix(GPIO *io, int rows = 32, int chained_displays = 1,
            int parallel_displays = 1);
  virtual ~RGBMatrix();

  // Set GPIO output if it was not set already in constructor (otherwise: NoOp).
  // Starts display refresh thread if this is the first setting.
  void SetGPIO(GPIO *io);

  // Set PWM bits used for output. Default is 11, but if you only deal with
  // limited comic-colors, 1 might be sufficient. Lower require less CPU and
  // increases refresh-rate.
  //
  // Returns boolean to signify if value was within range.
  //
  // This sets the PWM bits for the current active FrameCanvas and future
  // ones that are created with CreateFrameCanvas().
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits();   // return the pwm-bits of the currently active buffer.

  // Map brightness of output linearly to input with CIE1931 profile.
  void set_luminance_correct(bool on);
  bool luminance_correct() const;

  void SetBrightness(float brightness);
  float brightness();

  //-- Double- and Multibuffering.

  // Create a new buffer to be used for multi-buffering. The returned new
  // Buffer implements a Canvas with the same size of thie RGBMatrix.
  // You can use it to draw off-screen on it, then swap it with the active
  // buffer using SwapOnVSync(). That would be classic double-buffering.
  //
  // You can also create as many FrameCanvas as you like and for instance use
  // them to pre-fill scenes of an animation for fast playback later.
  //
  // The ownership of the created Canvases remains with the RGBMatrix, so you
  // don't have to worry about deleting them.
  FrameCanvas *CreateFrameCanvas();

  // This method waits to the next VSync and swaps the active buffer with the
  // supplied buffer. The formerly active buffer is returned.
  //
  // If you pass in NULL, the active buffer is returned, but it won't be
  // replaced with NULL. You can use the NULL-behavior to just wait on
  // VSync or to retrieve the initial buffer when preparing a multi-buffer
  // animation.
  FrameCanvas *SwapOnVSync(FrameCanvas *other);

  // -- Canvas interface. These write to the active FrameCanvas
  // (see documentation in canvas.h)
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue);
  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);

private:
  class UpdateThread;
  friend class UpdateThread;

  const int rows_;
  const int chained_displays_;
  const int parallel_displays_;

  uint8_t pwm_bits_;
  bool do_luminance_correct_;
  float brightness_;

  FrameCanvas *active_;

  GPIO *io_;
  Mutex active_frame_sync_;
  UpdateThread *updater_;
  std::vector<FrameCanvas*> created_frames_;
};

class FrameCanvas : public Canvas {
public:
  // Set PWM bits used for this Frame.
  // Simple comic-colors, 1 might be sufficient (111 RGB, i.e. 8 colors).
  // Lower require less CPU.
  // Returns boolean to signify if value was within range.
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits();

  // -- Canvas interface.
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue);
  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);

private:
  friend class RGBMatrix;

  FrameCanvas(internal::Framebuffer *frame) : frame_(frame){}
  virtual ~FrameCanvas();
  internal::Framebuffer *framebuffer() { return frame_; }

  internal::Framebuffer *const frame_;
};
}  // end namespace rgb_matrix
#endif  // RPI_RGBMATRIX_H
