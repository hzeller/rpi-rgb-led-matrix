// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2015 Henner Zeller <h.zeller@acm.org>
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

// To use this image viewer, first get image-magick development files
// $ sudo apt-get install libgraphicsmagick++-dev libwebp-dev
//
// Then compile with
// $ make led-image-viewer

#include "led-matrix.h"
#include "transformer.h"

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <Magick++.h>
#include <magick/image.h>

using rgb_matrix::GPIO;
using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::CanvasTransformer;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

namespace {
// Preprocess as much as possible, so that we can just exchange full frames
// on VSync.
class PreprocessedFrame {
public:
  PreprocessedFrame(const Magick::Image &img,
                    CanvasTransformer *transformer,
                    rgb_matrix::FrameCanvas *output)
    : canvas_(output) {
    int delay_time = img.animationDelay();  // in 1/100s of a second.
    if (delay_time < 1) delay_time = 1;
    delay_micros_ = delay_time * 10000;

    Canvas *const transformed_draw_canvas = transformer->Transform(output);
    for (size_t y = 0; y < img.rows(); ++y) {
      for (size_t x = 0; x < img.columns(); ++x) {
        const Magick::Color &c = img.pixelColor(x, y);
        if (c.alphaQuantum() < 256) {
          transformed_draw_canvas
            ->SetPixel(x, y,
                       ScaleQuantumToChar(c.redQuantum()),
                       ScaleQuantumToChar(c.greenQuantum()),
                       ScaleQuantumToChar(c.blueQuantum()));
        }
      }
    }
  }

  FrameCanvas *canvas() const { return canvas_; }

  int delay_micros() const {
    return delay_micros_;
  }

private:
  FrameCanvas *const canvas_;
  int delay_micros_;
};
}  // end anonymous namespace

// Load still image or animation.
// Scale, so that it fits in "width" and "height" and store in "image_sequence".
// If this is a still image, "image_sequence" will contain one image, otherwise
// all animation frames.
static bool LoadAnimation(const char *filename, int width, int height,
                          std::vector<Magick::Image> *image_sequence) {
  std::vector<Magick::Image> frames;
  fprintf(stderr, "Read image...\n");
  readImages(&frames, filename);
  if (frames.size() == 0) {
    fprintf(stderr, "No image found.");
    return false;
  }

  // Put together the animation from single frames. GIFs can have nasty
  // disposal modes, but they are handled nicely by coalesceImages()
  if (frames.size() > 1) {
    fprintf(stderr, "Assembling animation with %d frames.\n",
            (int)frames.size());
    Magick::coalesceImages(image_sequence, frames.begin(), frames.end());
  } else {
    image_sequence->push_back(frames[0]);   // just a single still image.
  }

  fprintf(stderr, "Scale ... %dx%d -> %dx%d\n",
          (int)(*image_sequence)[0].columns(), (int)(*image_sequence)[0].rows(),
          width, height);
  for (size_t i = 0; i < image_sequence->size(); ++i) {
    (*image_sequence)[i].scale(Magick::Geometry(width, height));
  }
  return true;
}

// Preprocess buffers: create readily filled frame-buffers that can be
// swapped with the matrix to minimize computation time when we're displaying.
static void PrepareBuffers(const std::vector<Magick::Image> &images,
                           RGBMatrix *matrix,
                           std::vector<PreprocessedFrame*> *frames) {
  fprintf(stderr, "Preprocess for display.\n");
  CanvasTransformer *const transformer = matrix->transformer();
  for (size_t i = 0; i < images.size(); ++i) {
    FrameCanvas *canvas = matrix->CreateFrameCanvas();
    frames->push_back(new PreprocessedFrame(images[i], transformer, canvas));
  }
}

static void DisplayAnimation(const std::vector<PreprocessedFrame*> &frames,
                             RGBMatrix *matrix) {
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  fprintf(stderr, "Display.\n");
  for (unsigned int i = 0; !interrupt_received; ++i) {
    const PreprocessedFrame *frame = frames[i % frames.size()];
    matrix->SwapOnVSync(frame->canvas());
    if (frames.size() == 1) {
      sleep(86400);  // Only one image. Nothing to do.
    } else {
      usleep(frame->delay_micros());
    }
  }
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options] <image>\n", progname);
  fprintf(stderr, "Options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static bool WarnAboutDeprecatedOption(int argc, char **argv);

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);
  RGBMatrix *const matrix = rgb_matrix::CreateMatrixFromFlags(&argc, &argv);

  if (WarnAboutDeprecatedOption(argc, argv))
    return usage(argv[0]);

  if (argc <= 1) {
    fprintf(stderr, "Expected image filename.\n");
    return usage(argv[0]);
  }

  if (matrix == NULL)
    return 1;

  const char *filename = argv[argc-1];

  std::vector<Magick::Image> sequence_pics;
  if (!LoadAnimation(filename, matrix->width(), matrix->height(),
                     &sequence_pics)) {
    return 0;
  }

  std::vector<PreprocessedFrame*> frames;
  PrepareBuffers(sequence_pics, matrix, &frames);

  DisplayAnimation(frames, matrix);

  fprintf(stderr, "Caught signal. Exiting.\n");

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}

static bool WarnAboutDeprecatedOption(int argc, char **argv) {
    // These used to be options we understood, but deprecate now. Accept them
  // for now, but tell the user.
  bool any_deprecated_option = false;
  int opt;
  while ((opt = getopt(argc, argv, "r:P:c:p:b:d")) != -1) {
    switch (opt) {
    case 'r':
      fprintf(stderr, "-r is a deprecated option. "
              "Please use --led-rows=... instead!\n");
      any_deprecated_option = true;
      break;

    case 'P':
      fprintf(stderr, "-P is a deprecated option. "
              "Please use --led-parallel=... instead!\n");
      any_deprecated_option = true;
      break;

    case 'c':
      fprintf(stderr, "-c is a deprecated option. "
              "Please use --led-chain=... instead!\n");
      any_deprecated_option = true;
      break;

    case 'p':
      fprintf(stderr, "-p is a deprecated option. "
              "Please use --led-pwm-bits=... instead!\n");
      any_deprecated_option = true;
      break;
    case 'b':
      fprintf(stderr, "-b is a deprecated option. "
              "Please use --led-brightness=... instead!\n");
      any_deprecated_option = true;
      break;
    }
  }
  return any_deprecated_option;
}
