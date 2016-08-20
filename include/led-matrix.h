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
#include "transformer.h"

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
// a CanvasTransformer that does coordinate remapping and which should be added
// to the transformers, like with LargeSquare64x64Transformer in demo-main.cc.
class RGBMatrix : public Canvas {
public:
  // Options to initialize the RGBMatrix.
  struct Options {
    Options();   // Creates a default option set.

    // The "rows" are the number
    // of rows supported by the display, so 32 or 16. Default: 32.
    int rows;

    // The chain_length is the number of displays daisy-chained together
    // (output of one connected to input of next). Default: 1
    int chain_length;

    // The number of parallel chains connected to the Pi; in old Pis with 26
    // GPIO pins, that is 1, in newer Pis with 40 interfaces pins, that can
    // also be 2 or 3. The effective number of pixels in vertical direction is
    // then thus rows * parallel. Default: 1
    int parallel;
  };

  // Create an RGBMatrix.
  //
  // If "io" is not NULL, initializes GPIO pins and starts refreshing the
  // screen immediately. If you need finer control, pass NULL here and see
  // SetGPIO() method.
  //
  // The resulting canvas is (options.rows * options.parallel) high and
  // (32 * options.chain_length) wide.
  RGBMatrix(GPIO *io, const Options &options);

  // Convenience constructor if you don't need the fine-control with the
  // Options object.
  RGBMatrix(GPIO *io, int rows = 32, int chained_displays = 1,
            int parallel_displays = 1);

  virtual ~RGBMatrix();

  // Set GPIO output if it was not set already in constructor (otherwise: NoOp).
  // If "start_thread" is true, starts the refresh thread.
  //
  // When would you start the thread separately from setting the GPIO ?
  // If you are becoming a daemon, you must start the thread _after_ that,
  // because all threads are stopped after daemon.
  // However, you need to set the GPIO betfore dropping privileges (which you
  // usually do when running as daemon).
  //
  // So if you write a daemon with dropping privileges, this is the pseudocode
  // of what you need to do:
  // ------------
  //   GPIO gpio;
  //   gpio.Init();
  //   RGBMatrix *matrix = new RGBMatrix(NULL);  // No init with gpio yet.
  //   matrix->SetGPIO(&gpio, false);   // First init GPIO use..
  //   drop_privileges();               // .. then drop privileges.
  //   daemon(0, 0);
  //   matrix->SetGPIO(&gpio, true);    // Now start thread.
  // -------------
  void SetGPIO(GPIO *io, bool start_thread = true);

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

  // Set brightness in percent. 1%..100%.
  // This will only affect newly set pixels.
  void SetBrightness(uint8_t brightness);
  uint8_t brightness();

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
  //
  // The optional "framerate_fraction" parameter allows to choose which
  // multiple of the global frame-count to use. So it slows down your animation
  // to an exact fraction of the refresh rate.
  // Default is 1, so immediately next available frame.
  // (Say you have 140Hz refresh rate, then a value of 5 would give you an
  // 28Hz animation, nicely locked to the frame-rate).
  FrameCanvas *SwapOnVSync(FrameCanvas *other, unsigned framerate_fraction = 1);

  // Set image transformer that maps the logical canvas we provide to the
  // physical canvas (e.g. panel mapping, rotation ...).
  // Does _not_ take ownership of the transformer.
  void SetTransformer(CanvasTransformer *transformer);
  inline CanvasTransformer *transformer() { return transformer_; }

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
  uint8_t brightness_;

  FrameCanvas *active_;

  GPIO *io_;
  Mutex active_frame_sync_;
  UpdateThread *updater_;
  std::vector<FrameCanvas*> created_frames_;
  CanvasTransformer *transformer_;
};

class FrameCanvas : public Canvas {
public:
  // Set PWM bits used for this Frame.
  // Simple comic-colors, 1 might be sufficient (111 RGB, i.e. 8 colors).
  // Lower require less CPU.
  // Returns boolean to signify if value was within range.
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits();

  // Map brightness of output linearly to input with CIE1931 profile.
  void set_luminance_correct(bool on);
  bool luminance_correct() const;

  void SetBrightness(uint8_t brightness);
  uint8_t brightness();

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
