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
// However, this abstraction allows you to e.g. create delegating
// implementations that do a particular transformation, e.g. compose images,
// scale, rotate, anti-alias or translate coordinates in a funky way.
// So it might be a good idea to have your demos write to a Canvas instead.
class Canvas {
public:
  virtual ~Canvas() {}
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue) = 0;
  virtual void Clear() = 0;
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue) = 0;
};

}  // namespace rgb_matrix
#endif  // RPI_CANVAS_H
