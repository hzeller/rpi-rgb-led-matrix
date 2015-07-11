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

#include <assert.h>

#include "transformer.h"

namespace rgb_matrix {

RotateTransformer::RotateCanvas::RotateCanvas(int angle) : delegatee_(NULL), angle_(angle) {
  // Only allow angles in 90° steps
  assert(angle % 90 == 0);
}

void RotateTransformer::RotateCanvas::SetDelegatee(Canvas* delegatee) {
  assert(delegatee != NULL);

  pivot_x_ = (delegatee->width() - 1) / 2;
  pivot_y_ = (delegatee->height() - 1) / 2;
  delegatee_ = delegatee;
}

void RotateTransformer::RotateCanvas::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  assert(delegatee_ != NULL);

  const float s = (angle_ == 90 ? 1 : (angle_ == 270 ? -1 : 0));
  const float c = (angle_ == 180 ? -1 : (angle_ == 0 ? 1 : 0));

  // Offset needed cause of little precision errors on 180° and 270°
  const int offset_x = (angle_ == 90 || angle_ == 180 ? 1 : 0);
  const int offset_y = (angle_ == 180 || angle_ == 270 ? 1 : 0);

  // translate point to origin
  x -= pivot_x_;
  y -= pivot_y_;

  float rot_x = x * c - y * s;
  float rot_y = x * s + y * c;

  // translate back
  x = rot_x + pivot_x_ + offset_x;
  y = rot_y + pivot_y_ + offset_y;

  delegatee_->SetPixel(x, y, red, green, blue);
}

int RotateTransformer::RotateCanvas::width() const { 
  assert(delegatee_ != NULL);
  return (angle_ % 180 == 0) ? delegatee_->width() : delegatee_->height();
}

int RotateTransformer::RotateCanvas::height() const { 
  assert(delegatee_ != NULL); 
  return (angle_ % 180 == 0) ? delegatee_->height() : delegatee_->width();
}

void RotateTransformer::RotateCanvas::Clear() { 
  assert(delegatee_ != NULL); 
  delegatee_->Clear(); 
}

void RotateTransformer::RotateCanvas::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  assert(delegatee_ != NULL);
  delegatee_->Fill(red, green, blue);
}

void RotateTransformer::RotateCanvas::SetAngle(int angle) {
  assert(angle % 90 == 0);
  angle_ = angle;
}

RotateTransformer::RotateTransformer(int angle) : angle_(angle) {
  assert(angle % 90 == 0);
  canvas_ = new RotateCanvas(angle);
}

Canvas *RotateTransformer::Transform(Canvas *output) {
  canvas_->SetDelegatee(output);
  return canvas_;
}

void RotateTransformer::SetAngle(int angle) {
  assert(angle % 90 == 0);
  canvas_->SetAngle(angle);
  angle_ = angle;
}

int RotateTransformer::angle() {
  return angle_;
}

void LinkedTransformer::AddTransformer(CanvasTransformer *transformer) {
  list_.push_back(transformer);
}

void LinkedTransformer::AddTransformer(List transformer_list) {
  list_.insert(list_.end(), transformer_list.begin(), transformer_list.end());
}
void LinkedTransformer::SetTransformer(List transformer_list) {
  list_ = transformer_list;
}

Canvas *LinkedTransformer::Transform(Canvas *output) {
  for (size_t i = 0; i < list_.size(); ++i) {
    output = list_[i]->Transform(output);
  }

  return output;
}

void LargeSquare64x64Transformer::LargeSquare64x64Canvas::SetDelegatee(Canvas* delegatee) {
  assert(delegatee != NULL);
  // Our assumptions of the underlying geometry:
  assert(delegatee->height() == 32);
  assert(delegatee->width() == 128);

  delegatee_ = delegatee;
}

void LargeSquare64x64Transformer::LargeSquare64x64Canvas::Clear() { 
  assert(delegatee_ != NULL); 
  delegatee_->Clear(); 
}

void LargeSquare64x64Transformer::LargeSquare64x64Canvas::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  assert(delegatee_ != NULL);
  delegatee_->Fill(red, green, blue);
}

int LargeSquare64x64Transformer::LargeSquare64x64Canvas::width() const { 
  return 64; 
}

int LargeSquare64x64Transformer::LargeSquare64x64Canvas::height() const { 
  return 64; 
}

void LargeSquare64x64Transformer::LargeSquare64x64Canvas::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  assert(delegatee_ != NULL);

  if (x < 0 || x >= width() || y < 0 || y >= height()) return;
  // We have up to column 64 one direction, then folding around. Lets map
  if (y > 31) {
    x = 127 - x;
    y = 63 - y;
  }
  delegatee_->SetPixel(x, y, red, green, blue);
}

LargeSquare64x64Transformer::LargeSquare64x64Transformer() {
  canvas_ = new LargeSquare64x64Canvas();
}

Canvas *LargeSquare64x64Transformer::Transform(Canvas *output) {
  canvas_->SetDelegatee(output);
  return canvas_;
}

} // namespace rgb_matrix