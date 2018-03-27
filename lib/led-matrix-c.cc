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
//
// C-bridge for led matrix.
#include "led-matrix-c.h"

#include <string.h>
#include <stdio.h>

#include "led-matrix.h"
#include "graphics.h"


// Our opaque dummy structs to communicate with the c-world
struct RGBLedMatrix {};
struct LedCanvas {};
struct LedFont {};


static rgb_matrix::RGBMatrix *to_matrix(struct RGBLedMatrix *matrix) {
  return reinterpret_cast<rgb_matrix::RGBMatrix*>(matrix);
}
static struct RGBLedMatrix *from_matrix(rgb_matrix::RGBMatrix *matrix) {
  return reinterpret_cast<struct RGBLedMatrix*>(matrix);
}

static rgb_matrix::FrameCanvas *to_canvas(struct LedCanvas *canvas) {
  return reinterpret_cast<rgb_matrix::FrameCanvas*>(canvas);
}
static struct LedCanvas *from_canvas(rgb_matrix::FrameCanvas *canvas) {
  return reinterpret_cast<struct LedCanvas*>(canvas);
}

static rgb_matrix::Font *to_font(struct LedFont *font) {
	return reinterpret_cast<rgb_matrix::Font*>(font);
}
static struct LedFont *from_font(rgb_matrix::Font *font) {
	return reinterpret_cast<struct LedFont*>(font);
}


struct RGBLedMatrix *led_matrix_create_from_options(
  struct RGBLedMatrixOptions *opts, int *argc, char ***argv) {
  rgb_matrix::RuntimeOptions default_rt;
  default_rt.drop_privileges = 0;  // Usually, this is on, but let user choose.
  default_rt.daemon = 0;

  rgb_matrix::RGBMatrix::Options default_opts;

  if (opts) {
    // Copy between C struct and C++ struct. The C++ struct already has a
    // default constructor that sets some values. These we override with the
    // C-struct values if available.
    // We assume everything non-zero has an explicit value.
#define OPT_COPY_IF_SET(o) if (opts->o) default_opts.o = opts->o
    OPT_COPY_IF_SET(hardware_mapping);
    OPT_COPY_IF_SET(rows);
    OPT_COPY_IF_SET(cols);
    OPT_COPY_IF_SET(chain_length);
    OPT_COPY_IF_SET(parallel);
    OPT_COPY_IF_SET(multiplexing);
    OPT_COPY_IF_SET(pwm_bits);
    OPT_COPY_IF_SET(pwm_lsb_nanoseconds);
    OPT_COPY_IF_SET(pwm_dither_bits);
    OPT_COPY_IF_SET(brightness);
    OPT_COPY_IF_SET(scan_mode);
    OPT_COPY_IF_SET(disable_hardware_pulsing);
    OPT_COPY_IF_SET(show_refresh_rate);
    OPT_COPY_IF_SET(led_rgb_sequence);
    OPT_COPY_IF_SET(pixel_mapper_config);
    OPT_COPY_IF_SET(inverse_colors);
    OPT_COPY_IF_SET(row_address_type);
#undef OPT_COPY_IF_SET
  }

  rgb_matrix::RGBMatrix::Options matrix_options = default_opts;
  rgb_matrix::RuntimeOptions runtime_opt = default_rt;
  if (argc != NULL && argv != NULL) {
    if (!ParseOptionsFromFlags(argc, argv, &matrix_options, &runtime_opt)) {
      rgb_matrix::PrintMatrixFlags(stderr, default_opts, default_rt);
      return NULL;
    }
  }

  if (opts) {
#define ACTUAL_VALUE_BACK_TO_OPT(o) opts->o = default_opts.o
    ACTUAL_VALUE_BACK_TO_OPT(hardware_mapping);
    ACTUAL_VALUE_BACK_TO_OPT(rows);
    ACTUAL_VALUE_BACK_TO_OPT(cols);
    ACTUAL_VALUE_BACK_TO_OPT(chain_length);
    ACTUAL_VALUE_BACK_TO_OPT(parallel);
    ACTUAL_VALUE_BACK_TO_OPT(multiplexing);
    ACTUAL_VALUE_BACK_TO_OPT(pwm_bits);
    ACTUAL_VALUE_BACK_TO_OPT(pwm_lsb_nanoseconds);
    ACTUAL_VALUE_BACK_TO_OPT(pwm_dither_bits);
    ACTUAL_VALUE_BACK_TO_OPT(brightness);
    ACTUAL_VALUE_BACK_TO_OPT(scan_mode);
    ACTUAL_VALUE_BACK_TO_OPT(disable_hardware_pulsing);
    ACTUAL_VALUE_BACK_TO_OPT(show_refresh_rate);
    ACTUAL_VALUE_BACK_TO_OPT(led_rgb_sequence);
    ACTUAL_VALUE_BACK_TO_OPT(pixel_mapper_config);
    ACTUAL_VALUE_BACK_TO_OPT(inverse_colors);
    ACTUAL_VALUE_BACK_TO_OPT(row_address_type);
#undef ACTUAL_VALUE_BACK_TO_OPT
  }

  rgb_matrix::RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options,
                                                          runtime_opt);
  return from_matrix(matrix);
}

struct RGBLedMatrix *led_matrix_create(int rows, int chained, int parallel) {
  struct RGBLedMatrixOptions opts;
  memset(&opts, 0, sizeof(opts));
  opts.rows = rows;
  opts.chain_length = chained;
  opts.parallel = parallel;
  return led_matrix_create_from_options(&opts, NULL, NULL);
}

void led_matrix_print_flags(FILE *out) {
  rgb_matrix::RGBMatrix::Options defaults;
  rgb_matrix::RuntimeOptions rt_opt;
  rt_opt.daemon = -1;
  rt_opt.drop_privileges = -1;
  rgb_matrix::PrintMatrixFlags(out, defaults, rt_opt);
}

void led_matrix_delete(struct RGBLedMatrix *matrix) {
  delete to_matrix(matrix);
}

struct LedCanvas *led_matrix_get_canvas(struct RGBLedMatrix *matrix) {
  return from_canvas(to_matrix(matrix)->SwapOnVSync(NULL));
}

struct LedCanvas *led_matrix_create_offscreen_canvas(struct RGBLedMatrix *m) {
  return from_canvas(to_matrix(m)->CreateFrameCanvas());
}

struct LedCanvas *led_matrix_swap_on_vsync(struct RGBLedMatrix *matrix,
                                           struct LedCanvas *canvas) {
  return from_canvas(to_matrix(matrix)->SwapOnVSync(to_canvas(canvas)));
}

void led_canvas_get_size(const struct LedCanvas *canvas,
                         int *width, int *height) {
  rgb_matrix::FrameCanvas *c = to_canvas((struct LedCanvas*)canvas);
  if (c == NULL ) return;
  if (width != NULL) *width = c->width();
  if (height != NULL) *height = c->height();
}

void led_canvas_set_pixel(struct LedCanvas *canvas, int x, int y,
			  uint8_t r, uint8_t g, uint8_t b) {
  to_canvas(canvas)->SetPixel(x, y, r, g, b);
}

void led_canvas_clear(struct LedCanvas *canvas) {
  to_canvas(canvas)->Clear();
}

void led_canvas_fill(struct LedCanvas *canvas, uint8_t r, uint8_t g, uint8_t b) {
  to_canvas(canvas)->Fill(r, g, b);
}

struct LedFont *load_font(const char *bdf_font_file) {
	rgb_matrix::Font* font = new rgb_matrix::Font();
	font->LoadFont(bdf_font_file);
	return from_font(font);
}

void delete_font(struct LedFont *font) {
	delete to_font(font);
}


// -- Some utility functions.

// Draw text, a standard NUL terminated C-string encoded in UTF-8,
// with given "font" at "x","y" with "color".
// "color" always needs to be set (hence it is a reference),
// "background_color" is a pointer to optionally be NULL for transparency.
// "kerning_offset" allows for additional spacing between characters (can be
// negative)
// Returns how many pixels we advanced on the screen.
int draw_text(struct LedCanvas *c, struct LedFont *font, int x, int y,
	uint8_t r, uint8_t g, uint8_t b, const char *utf8_text, int kerning_offset) {
	const rgb_matrix::Color col = rgb_matrix::Color(r, g, b);
	return DrawText(to_canvas(c), *to_font(font), x, y, col, NULL, utf8_text, kerning_offset);
}

// Draw text, a standard NUL terminated C-string encoded in UTF-8,
// with given "font" at "x","y" with "color".
// Draw text as above, but vertically (top down).
// The text is a standard NUL terminated C-string encoded in UTF-8.
// "font, "x", "y", "color" and "background_color" are same as DrawText().
// "kerning_offset" allows for additional spacing between characters (can be
// negative).
// Returns font height to advance up on the screen.
int vertical_draw_text(struct LedCanvas *c, struct LedFont *font, int x, int y,
	uint8_t r, uint8_t g, uint8_t b,
	const char *utf8_text, int kerning_offset = 0) {
	const rgb_matrix::Color col = rgb_matrix::Color(r, g, b);
	return VerticalDrawText(to_canvas(c), *to_font(font), x, y, col, NULL, utf8_text, kerning_offset);
}

// Draw a circle centered at "x", "y", with a radius of "radius" and with "color"
void draw_circle(struct LedCanvas *c, int xx, int y, int radius, uint8_t r, uint8_t g, uint8_t b) {
	const rgb_matrix::Color col = rgb_matrix::Color( r,g,b );
	DrawCircle(to_canvas(c), xx, y, radius, col);
}

// Draw a line from "x0", "y0" to "x1", "y1" and with "color"
void draw_line(struct LedCanvas *c, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
	const rgb_matrix::Color col = rgb_matrix::Color(r, g, b);
	DrawLine(to_canvas(c), x0, y0, x1, y1, col);
}
