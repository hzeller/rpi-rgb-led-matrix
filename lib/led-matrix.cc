// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
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

#include "led-matrix.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include "gpio.h"
#include "framebuffer-internal.h"

namespace rgb_matrix {

RGBMatrix::RGBMatrix(GPIO *io, int rows, int chained_displays,
                     int parallel_displays)
  : rows_(rows), chained_displays_(chained_displays),
    parallel_displays_(parallel_displays),
    io_(NULL) {
  assert(io);  // Always expect it to be set.
  SetGPIO(io);
}

RGBMatrix::~RGBMatrix() {
  // Make sure LEDs are off.
  active_->Clear();
  active_->framebuffer()->DumpToMatrix(io_);

  for (size_t i = 0; i < created_frames_.size(); ++i) {
    delete created_frames_[i];
  }
}

void RGBMatrix::SetGPIO(GPIO *io) {
  if (io == NULL) return;  // nothing to set.
  if (io_ != NULL) return;  // already set.
  io_ = io;
  internal::Framebuffer::InitGPIO(io_, parallel_displays_);
  active_ = CreateFrameCanvas();
  Clear();
  active_->framebuffer()->DumpToMatrix(io_);
}

FrameCanvas *RGBMatrix::CreateFrameCanvas() {
  FrameCanvas *result =
    new FrameCanvas(new internal::Framebuffer(io_,
                                              rows_, 32 * chained_displays_,
                                              parallel_displays_));
  if (created_frames_.empty()) {
    // First time. Get defaults from initial Framebuffer.
    pwm_bits_ = result->framebuffer()->pwmbits();
    do_luminance_correct_ = result->framebuffer()->luminance_correct();
    brightness_ = result->framebuffer()->brightness();
  } else {
    result->framebuffer()->SetPWMBits(pwm_bits_);
    result->framebuffer()->set_luminance_correct(do_luminance_correct_);
    result->framebuffer()->SetBrightness(brightness_);
  }
  result->framebuffer()->InitializeScript(io_);
  created_frames_.push_back(result);
  return result;
}

FrameCanvas *RGBMatrix::SwapOnVSync(FrameCanvas *other) {
  fprintf(stderr, "SwapOnVSync() currently disabled\n");
  assert(0);
}

bool RGBMatrix::SetPWMBits(uint8_t value) {
  const bool success = active_->framebuffer()->SetPWMBits(value);
  if (success) {
    pwm_bits_ = value;
  }
  return success;
}
uint8_t RGBMatrix::pwmbits() { return pwm_bits_; }

// Map brightness of output linearly to input with CIE1931 profile.
void RGBMatrix::set_luminance_correct(bool on) {
  active_->framebuffer()->set_luminance_correct(on);
  do_luminance_correct_ = on;
}
bool RGBMatrix::luminance_correct() const {
  return do_luminance_correct_;
}

void RGBMatrix::SetBrightness(uint8_t brightness) {
  active_->framebuffer()->SetBrightness(brightness);
  brightness_ = brightness;
}

uint8_t RGBMatrix::brightness() {
  return brightness_;
}

// -- Implementation of RGBMatrix Canvas: delegation to ContentBuffer
int RGBMatrix::width() const { return active_->framebuffer()->width(); }
int RGBMatrix::height() const { return active_->framebuffer()->height(); }
void RGBMatrix::SetPixel(int x, int y,
                         uint8_t red, uint8_t green, uint8_t blue) {
  active_->framebuffer()->SetPixel(x, y, red, green, blue);
}
void RGBMatrix::Clear() { return active_->framebuffer()->Clear(); }
void RGBMatrix::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  active_->framebuffer()->Fill(red, green, blue);
}

// FrameCanvas implementation of Canvas
FrameCanvas::~FrameCanvas() { delete frame_; }
int FrameCanvas::width() const { return frame_->width(); }
int FrameCanvas::height() const { return frame_->height(); }
void FrameCanvas::SetPixel(int x, int y,
                         uint8_t red, uint8_t green, uint8_t blue) {
  frame_->SetPixel(x, y, red, green, blue);
}
void FrameCanvas::Clear() { return frame_->Clear(); }
void FrameCanvas::Fill(uint8_t red, uint8_t green, uint8_t blue) {
  frame_->Fill(red, green, blue);
}
bool FrameCanvas::SetPWMBits(uint8_t value) { return frame_->SetPWMBits(value); }
uint8_t FrameCanvas::pwmbits() { return frame_->pwmbits(); }

}  // end namespace rgb_matrix
