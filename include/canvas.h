// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
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

#ifndef RPI_CANVAS_H
#define RPI_CANVAS_H
#include <stdint.h>

namespace rgb_matrix {
// An interface for things a Canvas can do. The RGBMatrix implements this
// interface, so you can use it directly wherever a canvas is needed.
//
// This abstraction also allows you to e.g. create delegating
// implementations that do a particular transformation, e.g. re-map
// pixels (as you might lay out the physical RGB matrix in a different way),
// compose images (OR, XOR, transparecy), scale, rotate, anti-alias or
// translate coordinates in a funky way.
//
// It is a good idea to have your applications use the concept of
// a Canvas to write the content to instead of directly using the RGBMatrix.
class Canvas {
public:
  virtual ~Canvas() {}
  virtual int width() const = 0;  // Pixels available in x direction.
  virtual int height() const = 0; // Pixels available in y direction.

  // Set pixel at coordinate (x,y) with given color. Pixel (0,0) is the
  // top left corner.
  // Each color is 8 bit (24bpp), 0 black, 255 brightest.
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue) = 0;

  // Clear screen to be all black.
  virtual void Clear() = 0;

  // Fill screen with given 24bpp color.
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue) = 0;
};

// A canvas transformer is an object that, given a Canvas, returns a
// canvas that applies transformations before writing to the original
// Canvas.
//
// To simplify assumptions for implementations and users of implementations,
// the following conditions and constraints apply:
//
//   1) The CanvasTransformer _never_ takes ownership of the delegatee.
//   2) The ownership of the returned Canvas is _not_ passed to the caller.
//   3) The returned Canvas can only be assumed to be valid for the lifetime
//      of the CanvasTranformer and the lifetime of the original canvas.
//   4) The returned canvas is only valid up to the next call of Transform().
//
//   * Point 2)-4) imply that the CanvasTransformer can hand out the same
//     canvas object every time, just configured to write to the new
//     delegatee.
//   * Point 4) implies that one instance of CanvasTransformer cannot be used in
//     parallel in multiple threads calling Transform().
//   * The constraints also imply that it is valid for a CanvasTransformer to
//     return the passed in canvas itself.
class CanvasTransformer {
public:
  virtual ~CanvasTransformer() {}

  // Return a Canvas* that applies transformations before delegating to
  // the output canvas.
  virtual Canvas *Transform(Canvas *output) = 0;
};

}  // namespace rgb_matrix
#endif  // RPI_CANVAS_H
