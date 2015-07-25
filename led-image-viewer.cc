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
// $ sudo aptitude install libmagick++-dev
//
// Then compile with
// $ make led-image-viewer

#include "led-matrix.h"

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <Magick++.h>
#include <magick/image.h>

using rgb_matrix::GPIO;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

namespace {
// Preprocess as much as possible, so that we can just exchange full frames
// on VSync.
class PreprocessedFrame {
public:
  PreprocessedFrame(const Magick::Image &img, rgb_matrix::FrameCanvas *output)
    : canvas_(output) {
    int delay_time = img.animationDelay();  // in 1/100s of a second.
    if (delay_time < 1) delay_time = 1;
    delay_micros_ = delay_time * 10000;

    for (size_t y = 0; y < img.rows(); ++y) {
      for (size_t x = 0; x < img.columns(); ++x) {
        const Magick::Color &c = img.pixelColor(x, y);
        int drawX, drawY;
        CoordinateMapping(x, y, &drawX, &drawY);
        if (c.alphaQuantum() < 256) {
          output->SetPixel(drawX, drawY,
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
  // In case you have a different physical layout in your matrix,
  // implement this method differently.
  void CoordinateMapping(int xin, int yin, int *xout, int *yout) {
    *xout = xin;
    *yout = yin;
  }

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
                           RGBMatrix *buffer_factory,
                           std::vector<PreprocessedFrame*> *frames) {
  fprintf(stderr, "Preprocess for display.\n");
  for (size_t i = 0; i < images.size(); ++i) {
    FrameCanvas *canvas = buffer_factory->CreateFrameCanvas();
    frames->push_back(new PreprocessedFrame(images[i], canvas));
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
  fprintf(stderr, "Options:\n"
          "\t-r <rows>     : Panel rows. '16' for 16x32 (1:8 multiplexing),\n"
	  "\t                '32' for 32x32 (1:16), '8' for 1:4 multiplexing; "
          "Default: 32\n"
          "\t-P <parallel> : For Plus-models or RPi2: parallel chains. 1..3. "
          "Default: 1\n"
          "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
          "\t-d            : Run as daemon.\n");
  return 1;
}

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);

  int rows = 32;
  int chain = 1;
  int parallel = 1;
  int pwm_bits = -1;
  bool as_daemon = false;

  int opt;
  while ((opt = getopt(argc, argv, "r:P:c:p:d")) != -1) {
    switch (opt) {
    case 'r': rows = atoi(optarg); break;
    case 'P': parallel = atoi(optarg); break;
    case 'c': chain = atoi(optarg); break;
    case 'p': pwm_bits = atoi(optarg); break;
    case 'd': as_daemon = true; break;
    default:
      return usage(argv[0]);
    }
  }

  if (rows != 8 && rows != 16 && rows != 32) {
    fprintf(stderr, "Rows can one of 8, 16 or 32 "
            "for 1:4, 1:8 and 1:16 multiplexing respectively.\n");
    return 1;
  }

  if (chain < 1) {
    fprintf(stderr, "Chain outside usable range\n");
    return usage(argv[0]);
  }
  if (chain > 8) {
    fprintf(stderr, "That is a long chain. Expect some flicker.\n");
  }
  if (parallel < 1 || parallel > 3) {
    fprintf(stderr, "Parallel outside usable range.\n");
    return usage(argv[0]);
  }

  if (optind >= argc) {
    fprintf(stderr, "Expected image filename.\n");
    return usage(argv[0]);
  }

  const char *filename = argv[optind];

  /*
   * Set up GPIO pins. This fails when not running as root.
   */
  GPIO io;
  if (!io.Init())
    return 1;

  // Start daemon before we start any threads.
  if (as_daemon) {
    if (fork() != 0)
      return 0;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  RGBMatrix *const matrix = new RGBMatrix(&io, rows, chain, parallel);
  if (pwm_bits >= 0 && !matrix->SetPWMBits(pwm_bits)) {
    fprintf(stderr, "Invalid range of pwm-bits\n");
    return 1;
  }

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
