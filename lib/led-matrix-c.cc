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

#include "led-matrix.h"
#include "gpio.h"

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

struct RGBLedMatrix *led_matrix_create(int rows, int chained, int parallel) {
  static rgb_matrix::GPIO gpio;
  if (!gpio.Init()) {
    return NULL;
  }
  rgb_matrix::RGBMatrix::Options options;
  options.rows = rows;
  options.chain_length = chained;
  options.parallel = parallel;
  rgb_matrix::RGBMatrix *matrix = new rgb_matrix::RGBMatrix(&gpio, options);
  return from_matrix(matrix);
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
