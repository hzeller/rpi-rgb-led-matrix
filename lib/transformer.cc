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

/*****************************/
/* Rotate Transformer Canvas */
/*****************************/
class RotateTransformer::TransformCanvas : public Canvas {
public:
  TransformCanvas(int angle);

  void SetDelegatee(Canvas* delegatee);
  void SetAngle(int angle);

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
  int cos_;
  int sin_;
  int offset_x_;
  int offset_y_;
};

RotateTransformer::TransformCanvas::TransformCanvas(int angle)
  : delegatee_(NULL) {
  SetAngle(angle);
}

void RotateTransformer::TransformCanvas::SetDelegatee(Canvas* delegatee) {
  pivot_x_ = (delegatee->width() - 1) / 2;
  pivot_y_ = (delegatee->height() - 1) / 2;
  delegatee_ = delegatee;
}

void RotateTransformer::TransformCanvas::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  // translate point to origin
  x -= pivot_x_;
  y -= pivot_y_;

  float rot_x = x * cos_ - y * sin_;
  float rot_y = x * sin_ + y * cos_;

  // translate back
  x = rot_x + pivot_x_ + offset_x_;
  y = rot_y + pivot_y_ + offset_y_;

  delegatee_->SetPixel(x, y, red, green, blue);
}

int RotateTransformer::TransformCanvas::width() const {
  return (angle_ % 180 == 0) ? delegatee_->width() : delegatee_->height();
}

int RotateTransformer::TransformCanvas::height() const {
  return (angle_ % 180 == 0) ? delegatee_->height() : delegatee_->width();
}

void RotateTransformer::TransformCanvas::Clear() {
  delegatee_->Clear();
}

void RotateTransformer::TransformCanvas::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  delegatee_->Fill(red, green, blue);
}

void RotateTransformer::TransformCanvas::SetAngle(int angle) {
  assert(angle % 90 == 0);  // We currenlty enforce that for more pretty output
  angle_ = angle % 360;

  sin_ = (angle_ ==  90 ?  1 : (angle_ == 270 ? -1 : 0));
  cos_ = (angle_ == 180 ? -1 : (angle_ ==   0 ?  1 : 0));

  // Offset needed cause of little precision errors on 180° and 270°
  offset_x_ = (angle_ == 90 || angle_ == 180 ? 1 : 0);
  offset_y_ = (angle_ == 180 || angle_ == 270 ? 1 : 0);
}

/**********************/
/* Rotate Transformer */
/**********************/
RotateTransformer::RotateTransformer(int angle)
  : angle_(angle), canvas_(new TransformCanvas(angle)) {
}

RotateTransformer::~RotateTransformer() {
  delete canvas_;
}

Canvas *RotateTransformer::Transform(Canvas *output) {
  assert(output != NULL);

  canvas_->SetDelegatee(output);
  return canvas_;
}

void RotateTransformer::SetAngle(int angle) {
  canvas_->SetAngle(angle);
  angle_ = angle;
}

/**********************/
/* Linked Transformer */
/**********************/
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

void LinkedTransformer::DeleteTransformers() {
  for (size_t i = 0; i < list_.size(); ++i) {
    delete list_[i];
  }
  list_.clear();
}

/***********************************/
/* Large Square Transformer Canvas */
/***********************************/
class LargeSquare64x64Transformer::TransformCanvas : public Canvas {
public:
  TransformCanvas() : delegatee_(NULL) {}

  void SetDelegatee(Canvas* delegatee);

  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);

private:
  Canvas *delegatee_;
};

void LargeSquare64x64Transformer::TransformCanvas::SetDelegatee(Canvas* delegatee) {
  // Our assumptions of the underlying geometry:
  assert(delegatee->height() == 32);
  assert(delegatee->width() == 128);

  delegatee_ = delegatee;
}

void LargeSquare64x64Transformer::TransformCanvas::Clear() {
  delegatee_->Clear();
}

void LargeSquare64x64Transformer::TransformCanvas::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  delegatee_->Fill(red, green, blue);
}

int LargeSquare64x64Transformer::TransformCanvas::width() const {
  return 64;
}

int LargeSquare64x64Transformer::TransformCanvas::height() const {
  return 64;
}

void LargeSquare64x64Transformer::TransformCanvas::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  if (x < 0 || x >= width() || y < 0 || y >= height()) return;
  // We have up to column 64 one direction, then folding around. Lets map
  if (y > 31) {
    x = 127 - x;
    y = 63 - y;
  }
  delegatee_->SetPixel(x, y, red, green, blue);
}

/****************************/
/* Large Square Transformer */
/****************************/
LargeSquare64x64Transformer::LargeSquare64x64Transformer()
  : canvas_(new TransformCanvas()) {
}

LargeSquare64x64Transformer::~LargeSquare64x64Transformer() {
  delete canvas_;
}

Canvas *LargeSquare64x64Transformer::Transform(Canvas *output) {
  assert(output != NULL);

  canvas_->SetDelegatee(output);
  return canvas_;
}

/********************************/
/* Scrambled Transformer Canvas */
/********************************/
class Scrambled32x16Transformer::TransformCanvas : public Canvas {
public:
  TransformCanvas() : delegatee_(NULL), map_(NULL) {}
  ~TransformCanvas();

  void SetDelegatee(Canvas* delegatee);

  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);

private:
  Canvas *delegatee_;
  int width_, height_;
  unsigned int **map_;
};

Scrambled32x16Transformer::TransformCanvas::~TransformCanvas(){
  if (map_){
    for (int i=0; i<width_; i++){
      delete map_[i];
    }
    delete map_;
  }
}

void Scrambled32x16Transformer::TransformCanvas::SetDelegatee(Canvas* delegatee) {
  // The scrambled display I have has 2 chain per 32x16 panel, each of 8 rows
  // When we chain several of this scrambled display, the 8 lower rows of the entire display
  // is at the end of the chain.
  // So to use this display with the current library, we need to define 8 rows and 2 x number of diplay for chain number.
  // Here we set width and height in a more user friendly way.
  assert(delegatee->height() == 8);
  assert(delegatee->width() % 32 == 0);
  height_ = 16;
  width_ = delegatee->width()/2;

  if (!map_){
    map_ = new unsigned int*[width_];
    for (int i=0; i<width_; i++){
      map_[i] = new unsigned int[height_];
    }
    for (int i=0; i<width_*height_; i++){
      map_[i%width_][i/width_] = i%8 + (2*(i/8)+1-(i%(8*width_))/(4*width_))*8%(8*width_) + (i/(8*width_))*(8*width_);
    }
  }

  delegatee_ = delegatee;
}

void Scrambled32x16Transformer::TransformCanvas::Clear() {
  delegatee_->Clear();
}

void Scrambled32x16Transformer::TransformCanvas::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  delegatee_->Fill(red, green, blue);
}

int Scrambled32x16Transformer::TransformCanvas::width() const {
  return width_;
}

int Scrambled32x16Transformer::TransformCanvas::height() const {
  return height_;
}

void Scrambled32x16Transformer::TransformCanvas::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  int idx = map_[x][y];
  x = idx%(width_*2);
  y = idx/(width_*2);

  delegatee_->SetPixel(x, y, red, green, blue);
}

/*****************************/
/* Scrambled32x16Transformer */
/*****************************/
Scrambled32x16Transformer::Scrambled32x16Transformer()
  : canvas_(new TransformCanvas()) {
}

Scrambled32x16Transformer::~Scrambled32x16Transformer() {
  delete canvas_;
}

Canvas *Scrambled32x16Transformer::Transform(Canvas *output) {
  assert(output != NULL);

  canvas_->SetDelegatee(output);
  return canvas_;
}

} // namespace rgb_matrix
