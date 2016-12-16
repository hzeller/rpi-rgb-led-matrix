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

// Our opaque dummy structs to communicate with the c-world
struct RGBLedMatrix {};
struct LedCanvas {};

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
    OPT_COPY_IF_SET(chain_length);
    OPT_COPY_IF_SET(parallel);
    OPT_COPY_IF_SET(pwm_bits);
    OPT_COPY_IF_SET(brightness);
    OPT_COPY_IF_SET(scan_mode);
    OPT_COPY_IF_SET(disable_hardware_pulsing);
    OPT_COPY_IF_SET(show_refresh_rate);
    OPT_COPY_IF_SET(swap_green_blue);
    OPT_COPY_IF_SET(inverse_colors);
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
    ACTUAL_VALUE_BACK_TO_OPT(chain_length);
    ACTUAL_VALUE_BACK_TO_OPT(parallel);
    ACTUAL_VALUE_BACK_TO_OPT(pwm_bits);
    ACTUAL_VALUE_BACK_TO_OPT(brightness);
    ACTUAL_VALUE_BACK_TO_OPT(scan_mode);
    ACTUAL_VALUE_BACK_TO_OPT(disable_hardware_pulsing);
    ACTUAL_VALUE_BACK_TO_OPT(show_refresh_rate);
    ACTUAL_VALUE_BACK_TO_OPT(swap_green_blue);
    ACTUAL_VALUE_BACK_TO_OPT(inverse_colors);
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
  rgb_matrix::PrintMatrixFlags(out);
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
