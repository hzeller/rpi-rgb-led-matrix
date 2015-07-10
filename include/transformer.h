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

#ifndef RPI_TRANSFORMER_H
#define RPI_TRANSFORMER_H

#include <vector>
#include <cstddef>

#include "canvas.h"

namespace rgb_matrix {

// Transformer canvas to rotate the input canvas in 90° steps
class RotateCanvas : public Canvas {
public:
  // This class takes over ownership of the delegatee.
  RotateCanvas(int angle);
  virtual ~RotateCanvas() { delete delegatee_; }

  void SetDelegatee(Canvas* delegatee);

  void SetAngle(int angle);
  int angle();

  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);
  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);

private:
  Canvas *delegatee_;
  int angle_;
  float pivot_x_;
  float pivot_y_;
};

// Transformer for RotateCanvas
class RotateCanvasTransformer : public CanvasTransformer {
public:
  RotateCanvasTransformer(int angle);
  virtual ~RotateCanvasTransformer() { delete canvas_; }

  void SetAngle(int angle);
  int angle();
  
  virtual Canvas *Transform(Canvas *output);

private:
  RotateCanvas *canvas_;
  int angle_;
};

// Transformer for linked transformer objects
// First transformer added will be considered last
// (so it would the transformer that gets the original Canvas object)
typedef std::vector<CanvasTransformer*> CanvasTransformerList;
class LinkedTransformerCanvasTransformer : public CanvasTransformer {
public:
  
  LinkedTransformerCanvasTransformer() {}
  LinkedTransformerCanvasTransformer(CanvasTransformerList transformer_list) : transformer_list_(transformer_list) {}

  void AddTransformer(CanvasTransformer *transformer);
  void AddTransformer(CanvasTransformerList transformer_list);
  void SetTransformer(CanvasTransformerList transformer_list);

  virtual Canvas *Transform(Canvas *output);

private:
  CanvasTransformerList transformer_list_;
};

class LargeSquare64x64Canvas : public Canvas {
public:
  LargeSquare64x64Canvas() : delegatee_(NULL) {}
  virtual ~LargeSquare64x64Canvas() { delete delegatee_; }

  void SetDelegatee(Canvas* delegatee);

  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);

private:
  Canvas *delegatee_;
};

class LargeSquare64x64CanvasTransformer : public CanvasTransformer {
public:
  LargeSquare64x64CanvasTransformer();
  virtual ~LargeSquare64x64CanvasTransformer() { delete canvas_; }

  virtual Canvas *Transform(Canvas *output);

private:
  LargeSquare64x64Canvas *canvas_;
};

} // namespace rgb_matrix

#endif // RPI_TRANSFORMER_H