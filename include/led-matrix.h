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
#include <string>
#include <vector>

#include "gpio.h"
#include "canvas.h"
#include "thread.h"
#include "transformer.h"

namespace rgb_matrix {
class RGBMatrix;
class FrameCanvas;   // Canvas for Double- and Multibuffering
namespace internal {
class Framebuffer;
class PixelMapper;
}

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
// to the transformers, like with UArrangementTransformer in demo-main.cc.
class RGBMatrix : public Canvas {
public:
  // Options to initialize the RGBMatrix. Also see the main README.md for
  // detailed descriptions of the command line flags.
  struct Options {
    Options();   // Creates a default option set.

    // Validate the options and possibly output a message to string. If
    // "err" is NULL, outputs validation problems to stderr.
    // Returns 'true' if all options look good.
    bool Validate(std::string *err) const;

    // Name of the hardware mapping. Something like "regular" or "adafruit-hat"
    const char *hardware_mapping;

    // The "rows" are the number
    // of rows supported by the display, so 32 or 16. Default: 32.
    // Flag: --led-rows
    int rows;

    // The chain_length is the number of displays daisy-chained together
    // (output of one connected to input of next). Default: 1
    // Flag: --led-chain
    int chain_length;

    // The number of parallel chains connected to the Pi; in old Pis with 26
    // GPIO pins, that is 1, in newer Pis with 40 interfaces pins, that can
    // also be 2 or 3. The effective number of pixels in vertical direction is
    // then thus rows * parallel. Default: 1
    // Flag: --led-parallel
    int parallel;

    // Set PWM bits used for output. Default is 11, but if you only deal with
    // limited comic-colors, 1 might be sufficient. Lower require less CPU and
    // increases refresh-rate.
    // Flag: --led-pwm-bits
    int pwm_bits;

    // Change the base time-unit for the on-time in the lowest
    // significant bit in nanoseconds.
    // Higher numbers provide better quality (more accurate color, less
    // ghosting), but have a negative impact on the frame rate.
    // Flag: --led-pwm-lsb-nanoseconds
    int pwm_lsb_nanoseconds;

    // The initial brightness of the panel in percent. Valid range is 1..100
    // Default: 100
    // Flag: --led-brightness
    int brightness;

    // Scan mode: 0=progressive, 1=interlaced
    // Flag: --led-scan-mode
    int scan_mode;

    // Disable the PWM hardware subsystem to create pulses.
    // Typically, you don't want to disable hardware pulsing, this is mostly
    // for debugging and figuring out if there is interference with the
    // sound system.
    // This won't do anything if output enable is not connected to GPIO 18 in
    // non-standard wirings.
    // Flag: --led-hardware-pulse
    bool disable_hardware_pulsing;
    bool show_refresh_rate;  // Flag: --led-show-refresh
    bool swap_green_blue;    // Flag: --led-swap-green-blue
    bool inverse_colors;     // Flag: --led-inverse
  };

  // Create an RGBMatrix.
  //
  // Needs an initialized GPIO object and configuration options from the
  // RGBMatrix::Options struct.
  //
  // If you pass an GPIO object (which has to be Init()ialized), it will start
  // the internal thread to start the screen immediately.
  //
  // If you need finer control over when the refresh thread starts (which you
  // might when you become a daemon), pass NULL here and see SetGPIO() method.
  //
  // The resulting canvas is (options.rows * options.parallel) high and
  // (32 * options.chain_length) wide.
  RGBMatrix(GPIO *io, const Options &options);

  // Simple constructor if you don't need the fine-control with the
  // Options object.
  RGBMatrix(GPIO *io, int rows = 32, int chained_displays = 1,
            int parallel_displays = 1);

  virtual ~RGBMatrix();

  // Set GPIO output if it was not set already in constructor (otherwise: NoOp).
  // If "start_thread" is true, starts the refresh thread.
  //
  // When would you want to start the thread separately from setting the GPIO ?
  // If you are becoming a daemon, you must start the thread _after_ that,
  // because all threads are stopped at the fork().
  // However, you need to set the GPIO before dropping privileges (which you
  // usually do when running as daemon).
  //
  // So if want to manually crate a daemon with dropping privileges, this is
  // the pseudocode of what you need to do:
  // ------------
  //   RGBMatrix::Options opts;
  //   RGBMatrix *matrix = new RGBMatrix(NULL, opts);  // No init with gpio yet.
  //   GPIO gpio;
  //   gpio.Init();
  //   matrix->SetGPIO(&gpio, false);   // First init GPIO use, but no thread.
  //   // Now, GPIOs are all initialized, so we can drop privileges
  //   drop_privileges();               // .. then drop privileges.
  //   daemon(0, 0);                    // .. start daemon before threads.
  //   matrix->StartRefresh();          // Now start thread.
  // -------------
  // (Note, that there is a convenience function (CreateMatrixFromOptions())
  // that does these things).
  void SetGPIO(GPIO *io, bool start_thread = true);

  // Start thread. Typically, you don't need to call this, see SetGPIO()
  // description when you might want it.
  // It doesn't harm to call if the thread is already started.
  // Returns 'false' if it couldn't start because GPIO was not set yet.
  bool StartRefresh();

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

  // Set image transformer that maps the logical canvas coordinates to the
  // physical canvas coordinates.
  // This preprocesses the transformation for static pixel mapping once.
  //
  // (In the rate case that you have transformers that dynamically change
  //  their behavior at runtime or do transformations on the color, you have to
  //  manually use them to wrap canvases.)
  void ApplyStaticTransformer(const CanvasTransformer &transformer);

  // Don't use this function anymore, use ApplyStaticTransformer() instead.
  // See demo-main.cc how.
  //
  // This used to somewhat work with dynamic tranformations, but it
  // was confusing as that didn't apply to FrameCanvases as well.
  // If you have static transformations that can be done at program start
  // (such as rotation or creating your particular pysical display mapping),
  // use ApplyStaticTransformer().
  // If you use the Transformer concept to modify writes to canvases on-the-fly,
  // use them directly as such.
  //
  // DO NOT USE. WILL BE REMOVED.
  void SetTransformer(CanvasTransformer *t) __attribute__((deprecated)) {
    transformer_ = t;
    if (t) ApplyStaticTransformer(*t);
  }

  // DO NOT USE. WILL BE REMOVED.
  CanvasTransformer *transformer() __attribute__((deprecated)) {
    return transformer_;
  }

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

  Options params_;
  bool do_luminance_correct_;

  FrameCanvas *active_;

  GPIO *io_;
  Mutex active_frame_sync_;
  CanvasTransformer *transformer_;  // deprecated. To be removed.
  UpdateThread *updater_;
  std::vector<FrameCanvas*> created_frames_;
  internal::PixelMapper *shared_pixel_mapper_;
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
  virtual ~FrameCanvas();   // Any FrameCanvas is owned by RGBMatrix.
  internal::Framebuffer *framebuffer() { return frame_; }

  internal::Framebuffer *const frame_;
};

// Runtime options to simplify doing common things for many programs such as
// dropping privileges and becoming a daemon.
struct RuntimeOptions {
  RuntimeOptions();

  int gpio_slowdown;    // 0 = no slowdown.          Flag: --led-slowdown-gpio
  // If the following are disabled, the following options will not be offered.
  // If daemon is disabled, the user has to call StartRefresh() once the
  // matrix is created.
  int daemon;           // -1 disabled. 0=off, 1=on. Flag: --led-daemon
  int drop_privileges;  // -1 disabled. 0=off, 1=on. flag: --led-drop-privs
};

// Convenience utility functions to read standard rgb-matrix flags and create
// a RGBMatrix. Commandline flags are something like --led-rows, --led-chain,
// --led-parallel. See output of PrintMatrixFlags() for all available options
// and detailed description in
// https://github.com/hzeller/rpi-rgb-led-matrix#changing-parameters-via-command-line-flags
//
// Example use:
/*
using rgb_matrix::RGBMatrix;
int main(int argc, char **argv) {
  RGBMatrix::Options led_options;
  rgb_matrix::RuntimeOptions runtime;

  // Set defaults
  led_options.chain_length = 3;
  led_options.show_refresh_rate = true;
  runtime.drop_privileges = 1;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &led_options, &runtime)) {
    rgb_matrix::PrintMatrixFlags(stderr);
    return 1;
  }

  // Do your own command line handling with the remaining flags.
  while (getopt()) {...}

  // Looks like we're ready to start
  RGBMatrix *matrix = CreateMatrixFromOptions(led_options, runtime);
  if (matrix == NULL) {
    return 1;
  }

  //  .. now use matrix

  delete matrix;   // Make sure to delete it in the end to switch off LEDs.
  return 0;
}
*/
// This parses the flags from argv and updates the structs with the parsed-out
// values. Structs can be NULL if you are not interested in it.
// The recongized flags are removed from argv if "remove_consumed_flags"
// is true; this simplifies your command line processing.
// Returns 'true' on success, 'false' if there was flag parsing problem.
bool ParseOptionsFromFlags(int *argc, char ***argv,
                           RGBMatrix::Options *default_options,
                           RuntimeOptions *rt_options,
                           bool remove_consumed_flags = true);

// Factory to create a matrix and possibly other things such as dropping
// privileges and becoming a daemon.
// Returns NULL, if there was a problem (a message then is written to stderr).
RGBMatrix *CreateMatrixFromOptions(const RGBMatrix::Options &options,
                                   const RuntimeOptions &runtime_options);

// A convenience function that combines the previous two steps. Optionally,
// you can pass in option structs with a couple of defaults. A matrix is
// created and returned; also the options structs are updated to reflect
// the values that were used.
// Returns NULL, if there was a problem (a message then is written to stderr).
RGBMatrix *CreateMatrixFromFlags(int *argc, char ***argv,
                                 RGBMatrix::Options *default_options = NULL,
                                 RuntimeOptions *default_runtime_opts = NULL,
                                 bool remove_consumed_flags = true);

// Show all the available options for CreateMatrixFromFlags().
void PrintMatrixFlags(FILE *out,
                      const RGBMatrix::Options &defaults = RGBMatrix::Options(),
                      const RuntimeOptions &rt_opt = RuntimeOptions());

}  // end namespace rgb_matrix
#endif  // RPI_RGBMATRIX_H
