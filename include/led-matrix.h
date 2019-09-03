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
#include "pixel-mapper.h"

namespace rgb_matrix {
class RGBMatrix;
class FrameCanvas;   // Canvas for Double- and Multibuffering

namespace internal {
class Framebuffer;
class PixelDesignatorMap;
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

    // The "cols" are the number of columns per panel. Typically something
    // like 32, but also 64 is possible. Sometimes even 40.
    // cols * chain_length is the total length of the display, so you can
    // represent a 64 wide display as cols=32, chain=2 or cols=64, chain=1;
    // same thing, but more convenient to think of.
    // Flag: --led-cols
    int cols;

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

    // The lower bits can be time-dithered for higher refresh rate.
    // Flag: --led-pwm-dither-bits
    int pwm_dither_bits;

    // The initial brightness of the panel in percent. Valid range is 1..100
    // Default: 100
    // Flag: --led-brightness
    int brightness;

    // Scan mode: 0=progressive, 1=interlaced
    // Flag: --led-scan-mode
    int scan_mode;

    // Default row address type is 0, corresponding to direct setting of the
    // row, while row address type 1 is used for panels that only have A/B,
    // typically some 64x64 panels
    int row_address_type;  // Flag --led-row-addr-type

    // Type of multiplexing. 0 = direct, 1 = stripe, 2 = checker (typical 1:8)
    int multiplexing;

    // Disable the PWM hardware subsystem to create pulses.
    // Typically, you don't want to disable hardware pulsing, this is mostly
    // for debugging and figuring out if there is interference with the
    // sound system.
    // This won't do anything if output enable is not connected to GPIO 18 in
    // non-standard wirings.
    // Flag: --led-hardware-pulse
    bool disable_hardware_pulsing;
    bool show_refresh_rate;    // Flag: --led-show-refresh
    // bool swap_green_blue; (Deprecated: use led_sequence instead)
    bool inverse_colors;       // Flag: --led-inverse

    // In case the internal sequence of mapping is not "RGB", this contains the
    // real mapping. Some panels mix up these colors.
    const char *led_rgb_sequence;  // Flag: --led-rgb-sequence

    // A string describing a sequence of pixel mappers that should be applied
    // to this matrix. A semicolon-separated list of pixel-mappers with optional
    // parameter.
    const char *pixel_mapper_config;   // Flag: --led-pixel-mapper

    // Panel type. Typically an empty string or NULL, but some panels need
    // a particular initialization sequence, so this is used for that.
    const char *panel_type;  // Flag: --led-panel-type
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
  // that does all these things).
  void SetGPIO(GPIO *io, bool start_thread = true);

  // Start thread. Typically, you don't need to call this, see SetGPIO()
  // description when you might want it.
  // It doesn't harm to call if the thread is already started, it is a no-op
  // then.
  // Returns 'false' if it couldn't start because GPIO was not set yet.
  bool StartRefresh();

  // Apply a pixel mapper. This is used to re-map pixels according to some
  // scheme implemented by the PixelMapper. Does not take ownership of the
  // mapper. Mapper can be NULL, in which case nothing happens.
  // Returns a boolean indicating if this was successful.
  bool ApplyPixelMapper(const PixelMapper *mapper);

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

  // Set brightness in percent for all created FrameCanvas. 1%..100%.
  // This will only affect newly set pixels.
  void SetBrightness(uint8_t brightness);
  uint8_t brightness();

  //-- GPIO interaction

  // Return pointer to GPIO object for your own interaction with free
  // pins. But don't mess with bits already in use by the matrix :)
  GPIO *gpio() { return io_; }

  // This function will return whenever the GPIO input pins
  // change (pins that are not already in use for output, that is) or the
  // timeout is reached. You need to have reserved the inputs with
  // gpio()->RequestInputs(...) first (e.g.
  //   gpio()->RequestInputs((1<<25)|(1<<24));
  //
  // A positive timeout waits the given amount of milliseconds for a change
  // (e.g. a button-press) to occur; if there is no change, it will just
  // return the last value.
  // If you just want to know how the pins are right now, call with zero
  // timeout.
  // A negative number waits forever and will only return if there is a change.
  //
  // Note, while you can poll the gpio()->Read() function directly, it is
  // not recommended as this might occur during the display update which might
  // result in flicker.
  // This function only samples between display refreshes and is more
  // convenient as it allows to wait for a change.
  //
  // Returns the bitmap of all GPIO input pins.
  uint32_t AwaitInputChange(int timeout_ms);

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

  // -- Canvas interface. These write to the active FrameCanvas
  // (see documentation in canvas.h)
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue);
  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);


#ifndef REMOVE_DEPRECATED_TRANSFORMERS
  //--- deprecated section: transformers. Use PixelMapper instead.
  void ApplyStaticTransformer(const CanvasTransformer &transformer) __attribute__((deprecated)) {
    ApplyStaticTransformerDeprecated(transformer);
  }
  void SetTransformer(CanvasTransformer *t) __attribute__((deprecated)) {
    transformer_ = t;
    if (t) ApplyStaticTransformerDeprecated(*t);
  }
  CanvasTransformer *transformer() __attribute__((deprecated)) {
    return transformer_;
  }
  // --- end deprecated section.
#endif  // INCLUDE_DEPRECATED_TRANSFORMERS

private:
  class UpdateThread;
  friend class UpdateThread;

  // Apply pixel mappers that have been passed down via a configuration
  // string.
  void ApplyNamedPixelMappers(const char *pixel_mapper_config,
                              int chain, int parallel);

#ifndef REMOVE_DEPRECATED_TRANSFORMERS
  void ApplyStaticTransformerDeprecated(const CanvasTransformer &transformer);
#endif  // REMOVE_DEPRECATED_TRANSFORMERS

  Options params_;
  bool do_luminance_correct_;

  FrameCanvas *active_;

  GPIO *io_;
  Mutex active_frame_sync_;
#ifndef REMOVE_DEPRECATED_TRANSFORMERS
  CanvasTransformer *transformer_;  // deprecated. To be removed.
#endif
  UpdateThread *updater_;
  std::vector<FrameCanvas*> created_frames_;
  internal::PixelDesignatorMap *shared_pixel_mapper_;
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

  //-- Serialize()/Deserialize() are fast ways to store and re-create a canvas.

  // Provides a pointer to a buffer of the internal representation to
  // be copied out for later Deserialize().
  //
  // Returns a "data" pointer and the data "len" in the given out-paramters;
  // the content can be copied from there by the caller.
  //
  // Note, the content is not simply RGB, it is the opaque and platform
  // specific representation which allows to make deserialization very fast.
  // It is also bigger than just RGB; if you want to store it somewhere,
  // using compression is a good idea.
  void Serialize(const char **data, size_t *len) const;

  // Load data previously stored with Serialize(). Needs to be restored into
  // a FrameCanvas with exactly the same settings (rows, chain, transformer,...)
  // as serialized.
  // Returns 'false' if size is unexpected.
  // This method should only be called if FrameCanvas is off-screen.
  bool Deserialize(const char *data, size_t len);

  // Copy content from other FrameCanvas owned by the same RGBMatrix.
  void CopyFrom(const FrameCanvas &other);

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

  // ----------
  // If the following options are set to disabled with -1, they are not
  // even offered via the command line flags.
  // ----------

  // If daemon is disabled (= -1), the user has to call StartRefresh() manually
  // once the matrix is created, to leave the decision to become a daemon
  // after the call (which requires that no threads have been started yet).
  // In the other cases (off or on), the choice is already made, so the thread
  // is conveniently already started for you.
  int daemon;           // -1 disabled. 0=off, 1=on. Flag: --led-daemon

  // Drop privileges from 'root' to 'daemon' once the hardware is initialized.
  // This is usually a good idea unless you need to stay on elevated privs.
  int drop_privileges;  // -1 disabled. 0=off, 1=on. flag: --led-drop-privs

  // By default, the gpio is initialized for you, but if you want to manually
  // do that yourself, set this flag to false.
  // Then, you have to initialize the matrix yourself with SetGPIO().
  bool do_gpio_init;
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
//
// The recongized flags are removed from argv if "remove_consumed_flags" is
// true; this simplifies your command line processing for the remaining options.
//
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
//
// If you allow the user to start a daemon with --led-daemon, make sure to
// call this function before you have started any threads, so early on in
// main() (see RuntimeOptions doc above).
//
// Note, the permissions are dropped by default from 'root' to 'daemon', so
// if you are required to stay root after this, disable this option in
// the default RuntimeOptions (set drop_privileges = -1).
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
