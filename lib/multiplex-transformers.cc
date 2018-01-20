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

#include "multiplex-transformers-internal.h"

#include <stdio.h>
#include <assert.h>

namespace rgb_matrix {
namespace internal {
class StripeTransformer::TransformCanvas : public Canvas {
public:
  TransformCanvas(int panel_rows, int panel_cols)
    : panel_cols_(panel_cols), panel_rows_(panel_rows) {
    if (panel_rows_ % 4 != 0 || panel_cols_ % 2 != 0) {
      printf("For this multiplexing to work, we'd expect the number of "
             "panel rows to be divisible by 4 and the columsn to be "
             "divisible by two. But is %dx%d\n", panel_rows_, panel_cols_);
      assert(0);
    }
  }

  void SetDelegatee(Canvas* delegatee) {
    // The delegatee is already stretched double long and half-heighted.
    width_ = delegatee->width() / 2;
    height_ = delegatee->height() * 2;
    assert(width_ % panel_cols_ == 0);
    assert(height_ % panel_rows_ == 0);
    delegatee_ = delegatee;
  }

  virtual void Clear() { delegatee_->Clear(); }
  virtual void Fill(uint8_t r, uint8_t g, uint8_t b) {
    delegatee_->Fill(r, g, b);
  }
  virtual int width() const { return width_; }
  virtual int height() const { return height_; }

  virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);

private:
  const int panel_cols_;
  const int panel_rows_;
  int width_;
  int height_;
  Canvas *delegatee_;
};

void StripeTransformer::TransformCanvas::SetPixel(
  int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) return;

  const int chained_panel  = x / panel_cols_;
  const int parallel_panel = y / panel_rows_;

  const int within_panel_x = x % panel_cols_;
  const int within_panel_y = y % panel_rows_;

  const bool is_top_stripe = (within_panel_y % (panel_rows_/2)) < panel_rows_/4;
  int new_x = is_top_stripe ? within_panel_x + panel_cols_ : within_panel_x;
  int new_y = (within_panel_y / (panel_rows_/2)) * (panel_rows_/4)
                + within_panel_y % (panel_rows_/4);
  delegatee_->SetPixel(chained_panel * 2*panel_cols_ + new_x,
                       parallel_panel * panel_rows_/2 + new_y,
                       red, green, blue);
}

StripeTransformer::StripeTransformer(int panel_cols, int panel_rows)
    : canvas_(new TransformCanvas(panel_cols, panel_rows)) {
}

StripeTransformer::~StripeTransformer() {
  delete canvas_;
}

Canvas *StripeTransformer::Transform(Canvas *output) {
  assert(output != NULL);

  canvas_->SetDelegatee(output);
  return canvas_;
}

// ------------------------

class CheckeredTransformer::TransformCanvas : public Canvas {
public:
  TransformCanvas(int panel_rows, int panel_cols)
    : panel_cols_(panel_cols), panel_rows_(panel_rows) {
    if (panel_rows_ % 4 != 0 || panel_cols_ % 2 != 0) {
      printf("For this multiplexing to work, we'd expect the number of "
             "panel rows to be divisible by 4 and the columsn to be "
             "divisible by two. But is %dx%d\n", panel_rows_, panel_cols_);
      assert(0);
    }
  }

  void SetDelegatee(Canvas* delegatee) {
    // The delegatee is already stretched double long and half-heighted.
    width_ = delegatee->width() / 2;
    height_ = delegatee->height() * 2;
    assert(width_ % panel_cols_ == 0);
    assert(height_ % panel_rows_ == 0);
    delegatee_ = delegatee;
  }

  virtual void Clear() { delegatee_->Clear(); }
  virtual void Fill(uint8_t r, uint8_t g, uint8_t b) {
    delegatee_->Fill(r, g, b);
  }
  virtual int width() const { return width_; }
  virtual int height() const { return height_; }

  virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);

private:
  const int panel_cols_;
  const int panel_rows_;
  int width_;
  int height_;
  Canvas *delegatee_;
};

void CheckeredTransformer::TransformCanvas::SetPixel(
  int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) return;

  const int chained_panel  = x / panel_cols_;
  const int parallel_panel = y / panel_rows_;

  const int within_panel_x = x % panel_cols_;
  const int within_panel_y = y % panel_rows_;

  const bool is_top_check = (within_panel_y % (panel_rows_/2)) < panel_rows_/4;
  const bool is_left_check = (within_panel_x < panel_cols_/2);
  int new_x, new_y;
  if (is_top_check) {
    new_x = is_left_check ? within_panel_x+panel_cols_/2 : within_panel_x+panel_cols_;
  } else {
    new_x = is_left_check ? within_panel_x : within_panel_x + panel_cols_/2;
  }
  new_y = (within_panel_y / (panel_rows_/2)) * (panel_rows_/4)
      + within_panel_y % (panel_rows_/4);
  delegatee_->SetPixel(chained_panel * 2*panel_cols_ + new_x,
                       parallel_panel * panel_rows_/2 + new_y,
                       red, green, blue);
}

CheckeredTransformer::CheckeredTransformer(int panel_cols, int panel_rows)
    : canvas_(new TransformCanvas(panel_cols, panel_rows)) {
}

CheckeredTransformer::~CheckeredTransformer() {
  delete canvas_;
}

Canvas *CheckeredTransformer::Transform(Canvas *output) {
  assert(output != NULL);

  canvas_->SetDelegatee(output);
  return canvas_;
}

}  // namespace internal
}  // namespace rgb_matrix
