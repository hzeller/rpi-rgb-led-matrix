// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2017 Henner Zeller <h.zeller@acm.org>
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
#ifndef RPI_RGBMATRIX_MULTIPLEX_TRANSFORMERS_INTERNAL_H
#define RPI_RGBMATRIX_MULTIPLEX_TRANSFORMERS_INTERNAL_H

#include "canvas.h"

// A couple of 1:8 multiplexing mappings found on some common led matrices.
// There are constantly new ones coming out, so please help adding one if your
// matrix is different.

namespace rgb_matrix {
namespace internal {
class StripeTransformer : public CanvasTransformer {
public:
  StripeTransformer(int panel_rows, int panel_cols);
  virtual ~StripeTransformer();

  virtual Canvas *Transform(Canvas *output);

private:
  class TransformCanvas;
  TransformCanvas *const canvas_;
};

// multiplexing is done in checkered patches.
class CheckeredTransformer : public CanvasTransformer {
public:
  CheckeredTransformer(int panel_rows, int panel_cols);
  virtual ~CheckeredTransformer();

  virtual Canvas *Transform(Canvas *output);

private:
  class TransformCanvas;
  TransformCanvas *const canvas_;
};

class SpiralTransformer : public CanvasTransformer {
public:
  SpiralTransformer(int panel_rows, int panel_cols);
  virtual ~SpiralTransformer();

  virtual Canvas *Transform(Canvas *output);

private:
  class TransformCanvas;
  TransformCanvas *const canvas_;
};


}  // namespace internal
}  // namespace rgb_matrix
#endif  // RPI_RGBMATRIX_MULTIPLEX_TRANSFORMERS_INTERNAL_H
