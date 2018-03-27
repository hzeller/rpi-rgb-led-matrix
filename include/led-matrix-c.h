/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 * Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>
 *
 * Controlling 16x32 or 32x32 RGB matrixes via GPIO. It allows daisy chaining
 * of a string of these, and also connecting a parallel string on newer
 * Raspberry Pis with more GPIO pins available.
 *
 * This is a C-binding (for the C++ library) to allow easy binding and
 * integration with other languages. The symbols are exported in librgbmatrix.a
 * and librgbmatrix.so. You still need to call the final link with
 *
 * See examples-api-use/c-example.c for a usage example.
 *
 */
#ifndef RPI_RGBMATRIX_C_H
#define RPI_RGBMATRIX_C_H

#include <stdint.h>
#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct RGBLedMatrix;
struct LedCanvas;
struct LedFont;

/**
 * Parameters to create a new matrix.
 *
 * To get the defaults, non-set values have to be initialized to zero, so you
 * should zero out this struct before setting anything.
 */
struct RGBLedMatrixOptions {
  /*
   * Name of the hardware mapping used. If passed NULL here, the default
   * is used.
   */
  const char *hardware_mapping;

  /* The "rows" are the number of rows supported by the display, so 32 or 16.
   * Default: 32.
   * Corresponding flag: --led-rows
   */
  int rows;

  /* The "cols" are the number of columns per panel. Typically something
   * like 32, but also 64 is possible. Sometimes even 40.
   * cols * chain_length is the total length of the display, so you can
   * represent a 64 wide display as cols=32, chain=2 or cols=64, chain=1;
   * same thing.
   * Flag: --led-cols
   */
  int cols;

  /* The chain_length is the number of displays daisy-chained together
   * (output of one connected to input of next). Default: 1
   * Corresponding flag: --led-chain
   */
  int chain_length;

  /* The number of parallel chains connected to the Pi; in old Pis with 26
   * GPIO pins, that is 1, in newer Pis with 40 interfaces pins, that can
   * also be 2 or 3. The effective number of pixels in vertical direction is
   * then thus rows * parallel. Default: 1
   * Corresponding flag: --led-parallel
   */
  int parallel;

  /* Set PWM bits used for output. Default is 11, but if you only deal with
   * limited comic-colors, 1 might be sufficient. Lower require less CPU and
   * increases refresh-rate.
   * Corresponding flag: --led-pwm-bits
   */
  int pwm_bits;

  /* Change the base time-unit for the on-time in the lowest
   * significant bit in nanoseconds.
   * Higher numbers provide better quality (more accurate color, less
   * ghosting), but have a negative impact on the frame rate.
   * Corresponding flag: --led-pwm-lsb-nanoseconds
   */
  int pwm_lsb_nanoseconds;

  /* The lower bits can be time-dithered for higher refresh rate.
   * Corresponding flag: --led-pwm-dither-bits
   */
  int pwm_dither_bits;

  /* The initial brightness of the panel in percent. Valid range is 1..100
   * Corresponding flag: --led-brightness
   */
  int brightness;

  /* Scan mode: 0=progressive, 1=interlaced
   * Corresponding flag: --led-scan-mode
   */
  int scan_mode;

  /* Default row address type is 0, corresponding to direct setting of the
   * row, while row address type 1 is used for panels that only have A/B,
   * typically some 64x64 panels
   */
  int row_address_type;  /* Corresponding flag: --led-row-addr-type */

  /*  Type of multiplexing. 0 = direct, 1 = stripe, 2 = checker (typical 1:8)
   */
  int multiplexing;

  /* In case the internal sequence of mapping is not "RGB", this contains the
   * real mapping. Some panels mix up these colors.
   */
  const char *led_rgb_sequence;     /* Corresponding flag: --led-rgb-sequence */

  /* A string describing a sequence of pixel mappers that should be applied
   * to this matrix. A semicolon-separated list of pixel-mappers with optional
   * parameter.
   */
  const char *pixel_mapper_config;  /* Corresponding flag: --led-pixel-mapper */

  /** The following are boolean flags, all off by default **/

  /* Allow to use the hardware subsystem to create pulses. This won't do
   * anything if output enable is not connected to GPIO 18.
   * Corresponding flag: --led-hardware-pulse
   */
  unsigned disable_hardware_pulsing:1;
  unsigned show_refresh_rate:1;  /* Corresponding flag: --led-show-refresh    */
  // unsigned swap_green_blue:1; /* deprecated, use led_sequence instead */
  unsigned inverse_colors:1;     /* Corresponding flag: --led-inverse         */
};

/**
 * Universal way to create and initialize a matrix.
 * The "options" struct (if not NULL) contains all default configuration values
 * chosen by the programmer to create the matrix.
 *
 * If "argc" and "argv" are provided, this function also reads command line
 * flags provided, that then can override any of the defaults given.
 * The arguments that have been used from the command line are removed from
 * the argv list (and argc is adjusted) - that way these don't mess with your
 * own command line handling.
 *
 * The actual options used are filled back into the "options" struct if not
 * NULL.
 *
 * Usage:
 * ----------------
 * int main(int argc, char **argv) {
 *   struct RGBLedMatrixOptions options;
 *   memset(&options, 0, sizeof(options));
 *   options.rows = 32;            // You can set defaults if you want.
 *   options.chain_length = 1;
 *   struct RGBLedMatrix *matrix = led_matrix_create_from_options(&options,
 *                                                                &argc, &argv);
 *   if (matrix == NULL) {
 *      led_matrix_print_flags(stderr);
 *      return 1;
 *   }
 *   // do additional commandline handling; then use matrix...
 * }
 * ----------------
 */
struct RGBLedMatrix *led_matrix_create_from_options(
             struct RGBLedMatrixOptions *options, int *argc, char ***argv);


/**
 * Print available LED matrix options.
 */
void led_matrix_print_flags(FILE *out);

/**
 * Simple form of led_matrix_create_from_options() with just the few
 * main options. Returns NULL if that was not possible.
 * The "rows" are the number of rows supported by the display, so 32, 16 or 8.
 *
 * Number of "chained_display"s tells many of these are daisy-chained together
 * (output of one connected to input of next).
 *
 * The "parallel_display" number determines if there is one or two displays
 * connected in parallel to the GPIO port - this only works with newer
 * Raspberry Pi that have 40 interface pins.
 *
 * This creates a realtime thread and requires root access to access the GPIO
 * pins.
 * So if you run this in a daemon, this should be called after becoming a
 * daemon (as fork/exec stops threads) and before dropping privileges.
 */
struct RGBLedMatrix *led_matrix_create(int rows, int chained, int parallel);


/**
 * Stop matrix and free memory.
 * Always call before the end of the program to properly reset the hardware
 */
void led_matrix_delete(struct RGBLedMatrix *matrix);


/**
 * Get active canvas from LED matrix for you to draw on.
 * Ownership of returned pointer stays with the matrix, don't free().
 */
struct LedCanvas *led_matrix_get_canvas(struct RGBLedMatrix *matrix);

/** Return size of canvas. */
void led_canvas_get_size(const struct LedCanvas *canvas,
                         int *width, int *height);

/** Set pixel at (x, y) with color (r,g,b). */
void led_canvas_set_pixel(struct LedCanvas *canvas, int x, int y,
			  uint8_t r, uint8_t g, uint8_t b);

/** Clear screen (black). */
void led_canvas_clear(struct LedCanvas *canvas);

/** Fill matrix with given color. */
void led_canvas_fill(struct LedCanvas *canvas, uint8_t r, uint8_t g, uint8_t b);

/*** API to provide double-buffering. ***/

/**
 * Create a new canvas to be used with led_matrix_swap_on_vsync()
 * Ownership of returned pointer stays with the matrix, don't free().
 */
struct LedCanvas *led_matrix_create_offscreen_canvas(struct RGBLedMatrix *matrix);

/**
 * Swap the given canvas (created with create_offscreen_canvas) with the
 * currently active canvas on vsync (blocks until vsync is reached).
 * Returns the previously active canvas. So with that, you can create double
 * buffering:
 *
 *   struct LedCanvas *offscreen = led_matrix_create_offscreen_canvas(...);
 *   led_canvas_set_pixel(offscreen, ...);   // not shown until swap-on-vsync
 *   offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
 *   // The returned buffer, assigned to offscreen, is now the inactive buffer
 *   // fill, then swap again.
 */
struct LedCanvas *led_matrix_swap_on_vsync(struct RGBLedMatrix *matrix,
                                           struct LedCanvas *canvas);


struct LedFont *load_font(const char *bdf_font_file);
void delete_font(struct LedFont *font);

int draw_text(struct LedCanvas *c, struct LedFont *font, int x, int y,
	uint8_t r, uint8_t g, uint8_t b,
	const char *utf8_text, int kerning_offset);

int vertical_draw_text(struct LedCanvas *c, struct LedFont *font, int x, int y,
	uint8_t r, uint8_t g, uint8_t b, const char *utf8_text, int kerning_offset);

void draw_circle(struct LedCanvas *c, int xx, int y, int radius, uint8_t r, uint8_t g, uint8_t b);

void draw_line(struct LedCanvas *c, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b);

#ifdef  __cplusplus
}  // extern C
#endif

#endif
