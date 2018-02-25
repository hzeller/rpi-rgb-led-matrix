// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2018 Henner Zeller <h.zeller@acm.org>
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
#ifndef RGBMATRIX_PIXEL_MAPPER
#define RGBMATRIX_PIXEL_MAPPER

#include <string>

namespace rgb_matrix {

// A pixel mapper is a way for you to map pixels of LED matrixes to a different
// layout. If you have an implementation of a PixelMapper, you can give it
// to the RGBMatrix::ApplyPixelMapper(), which then presents you a canvas
// that has the new "visible_width", "visible_height".
class PixelMapper {
public:
  virtual ~PixelMapper() {}

  // Get the name of this PixelMapper. Each PixelMapper needs to have a name
  // so that it can be referred to with command line flags.
  virtual const char *GetName() const = 0;

  // Pixel mappers might receive optional parameters, e.g. from command
  // line flags.
  // This is a single string containing the parameters.
  // You can be used from simple scalar parameters, such as the angle for
  // the rotate transformer, or more complex parameters that describe a mapping
  // of panels for instance.
  // Keep it concise (as people will give parameters on the command line) and
  // don't use semicolons in your string (as they are
  // used to separate pixel mappers on the command line).
  //
  // For instance, the rotate transformer is invoked like this
  //  --led-pixel-mapper=rotate:90
  // And the parameter that is passed to SetParameter() is "90".
  //
  // Returns 'true' if parameter was parsed successfully.
  virtual bool SetParameter(const std::string &parameter_string) {
    return parameter_string.empty();  // Default: expecting no parameter.
  }

  // Given a underlying matrix (width, height), returns the
  // visible (width, height) after the mapping.
  // E.g. a 90 degree rotation might map matrix=(64, 32) -> visible=(32, 64)
  // Some multiplexing matrices will double the height and half the width.
  //
  // While not technically necessary, one would expect that the number of
  // pixels stay the same, so
  // matrix_width * matrix_height == (*visible_width) * (*visible_height);
  //
  // Returns boolean "true" if the mapping can be successfully done with this
  // mapper.
  virtual bool GetSizeMapping(int matrix_width, int matrix_height,
                              int *visible_width, int *visible_height)
    const = 0;

  // Map where a visible pixel (x,y) is mapped to the underlying matrix (x,y).
  //
  // To be convienently stateless, the first parameters are the full
  // matrix width and height.
  //
  // So for many multiplexing methods this means to map a panel to a double
  // length and half height panel (32x16 -> 64x8).
  // The logic_x, logic_y are output parameters and guaranteed not to be
  // nullptr.
  virtual void MapVisibleToMatrix(int matrix_width, int matrix_height,
                                  int visible_x, int visible_y,
                                  int *matrix_x, int *matrix_y) const = 0;
};

}  // namespace rgb_matrix

#endif  // RGBMATRIX_PIXEL_MAPPER
