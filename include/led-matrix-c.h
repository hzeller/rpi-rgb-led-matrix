// -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
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
//
// This is a C-binding (for the C++ library) to allow easy binding and
// integration with other languages. The symbols are exported in librgbmatrix.a
// and librgbmatrix.so.
//
// Usage: -------
//   struct RGBLedMatrix *matrix = led_matrix_create(32, 3, 3);
//
//   // Get canvas and use it for drawing:
//   struct LedCanvas *canvas = led_matrix_get_canvas(matrix);
//   led_canvas_set_pixel(canvas, x, y, red, green, blue);
//
//   // Make sure to always call led_matrix_delete() in the end to reset the
//   // display. Installing signal handlers for defined exit is a good idea.
//   led_matrix_delete(matrix);
// ---------------
// For double-buffering, see example at led_matrix_swap_on_vsync() documentation.
//
#ifndef RPI_RGBMATRIX_C_H
#define RPI_RGBMATRIX_C_H

#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct RGBLedMatrix;
struct LedCanvas;

// Create matrix and initialize hardware. Returns NULL if that was not
// possible.
// The "rows" are the number of rows supported by the display, so 32, 16 or 8.
//
// Number of "chained_display"s tells many of these are daisy-chained together
// (output of one connected to input of next).
//
// The "parallel_display" number determines if there is one or two displays
// connected in parallel to the GPIO port - this only works with newer
// Raspberry Pi that have 40 interface pins.
//
// This creates a realtime thread and requires root access to access the GPIO
// pins.
// So if you run this in a daemon, this should be called after becoming a
// daemon (as fork/exec stops threads) and before dropping privileges.
struct RGBLedMatrix *led_matrix_create(int rows, int chained, int parallel);

// Stop matrix and free.
void led_matrix_delete(struct RGBLedMatrix *matrix);

// Get active canvas from LED matrix for you to draw on.
// Ownership of returned pointer stays with the matrix, don't free().
struct LedCanvas *led_matrix_get_canvas(struct RGBLedMatrix *matrix);

// Return size of canvas.
void led_canvas_get_size(const struct LedCanvas *canvas,
                         int *width, int *height);

// Set pixel at (x, y) with color (r,g,b).
void led_canvas_set_pixel(struct LedCanvas *canvas, int x, int y,
			  uint8_t r, uint8_t g, uint8_t b);

// Clear screen (black).
void led_canvas_clear(struct LedCanvas *canvas);

// Fill matrix with given color.
void led_canvas_fill(struct LedCanvas *canvas, uint8_t r, uint8_t g, uint8_t b);

// -- API to provide double-buffering.

// Create a new canvas to be used with led_matrix_swap_on_vsync()
// Ownership of returned pointer stays with the matrix, don't free().
struct LedCanvas *led_matrix_create_offscreen_canvas(struct RGBLedMatrix *matrix);

// Swap the given canvas (created with create_offscreen_canvas) with the
// currently active canvas on vsync (blocks until vsync is reached).
// Returns the previously active canvas. So with that, you can create double
// buffering.
//   struct LedCanvas *offscreen = led_matrix_create_offscreen_canvas(...);
//   led_canvas_set_pixel(offscreen, ...);   // not shown until swap-on-vsync
//   offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
//   // The returned buffer, assigned to offscreen, is now the inactive buffer
//   // fill, then swap again.
struct LedCanvas *led_matrix_swap_on_vsync(struct RGBLedMatrix *matrix,
                                           struct LedCanvas *canvas);

#ifdef  __cplusplus
}  // extern C
#endif

#endif  // RPI_RGBMATRIX_C_H
