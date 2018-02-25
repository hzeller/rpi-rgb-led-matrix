// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
// Copyright (C) 2015 Christoph Friedrich <christoph.friedrich@vonaffenfels.de>
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

/*
 * Deprecated.
 *
 * Use PixelMapper instead. See pixel-mapper.h
 */
#ifndef RPI_TRANSFORMER_H
#define RPI_TRANSFORMER_H

#ifndef REMOVE_DEPRECATED_TRANSFORMERS

#include <vector>
#include <cstddef>

#include "canvas.h"

namespace rgb_matrix {

// Transformer for RotateCanvas
class RotateTransformer : public CanvasTransformer {
public:
  RotateTransformer(int angle = 0);
  virtual ~RotateTransformer();

  void SetAngle(int angle);
  inline int angle() { return angle_; }

  virtual Canvas *Transform(Canvas *output);

private:
  // Transformer canvas to rotate the input canvas in 90° steps
  class TransformCanvas;

  int angle_;
  TransformCanvas *const canvas_;
};

// Transformer for linked transformer objects
// First transformer added will be considered last
// (so it would the transformer that gets the original Canvas object)
class LinkedTransformer : public CanvasTransformer {
public:
  typedef std::vector<CanvasTransformer*> List;

  LinkedTransformer() {}
  LinkedTransformer(List transformer_list) : list_(transformer_list) {}

  // The ownership of the given transformers is _not_ taken over unless
  // you explicitly call DeleteTransformers().
  void AddTransformer(CanvasTransformer *transformer);
  void AddTransformer(List transformer_list);
  void SetTransformer(List transformer_list);

  // Delete transformers that have been added or set.
  void DeleteTransformers();

  // -- CanvasTransformer interface
  virtual Canvas *Transform(Canvas *output);

private:
  List list_;
};

// If we take a long chain of panels and arrange them in a U-shape, so
// that after half the panels we bend around and continue below. This way
// we have a panel that has double the height but only uses one chain.
// A single chain display with four 32x32 panels can then be arranged in this
// 64x64 display:
//    [<][<][<][<] }- Raspbery Pi connector
//
// can be arranged in this U-shape
//    [<][<] }----- Raspberry Pi connector
//    [>][>]
//
// This works for more than one chain as well. Here an arrangement with
// two chains with 8 panels each
//   [<][<][<][<]  }-- Pi connector #1
//   [>][>][>][>]
//   [<][<][<][<]  }--- Pi connector #2
//   [>][>][>][>]
class UArrangementTransformer : public CanvasTransformer {
public:
  UArrangementTransformer(int parallel = 1);
  ~UArrangementTransformer();

  virtual Canvas *Transform(Canvas *output);

private:
  class TransformCanvas;

  TransformCanvas *const canvas_;
};

// Something used before, but it had a confusing 180 degree turn and was not
// ready for multiple parallel chains. So consider using the
// U-ArrangementTransformer instead.
class LargeSquare64x64Transformer : public CanvasTransformer {
public:
  LargeSquare64x64Transformer();
  virtual Canvas *Transform(Canvas *output);

private:
  // This old transformer was a little off and rotated the whole result in
  // the end. simulated that here.
  UArrangementTransformer arrange_;
  RotateTransformer rotated_;
}  __attribute__((deprecated));

} // namespace rgb_matrix

#endif  // REMOVE_DEPRECATED_TRANSFORMERS
#endif  // RPI_TRANSFORMER_H
