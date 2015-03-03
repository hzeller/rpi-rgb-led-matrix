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
#include <magick/quantum.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

namespace {
class PreprocessedFrame {
public:
  PreprocessedFrame(const Magick::Image &img)
    : width_(img.columns()), height_(img.rows()),
      content_(new Pixel[width_ * height_]) {
    int delay_time = img.animationDelay();  // in 1/100s of a second.
    if (delay_time < 1) delay_time = 1;
    delay_micros_ = delay_time * 10000;

    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        const Magick::Color &c = img.pixelColor(x, y);
        content_[y * width_ + x] =
          Pixel(MagickCore::ScaleQuantumToChar(c.redQuantum()),
                MagickCore::ScaleQuantumToChar(c.greenQuantum()),
                MagickCore::ScaleQuantumToChar(c.blueQuantum()));
      }
    }
  }

  ~PreprocessedFrame() { delete [] content_; }

  void CopyToCanvas(rgb_matrix::Canvas *canvas) const {
    Pixel *pixel = content_;
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        canvas->SetPixel(x, y, pixel->r, pixel->g, pixel->b);
        ++pixel;
      }
    }
  }

  int delay_micros() const {
    return delay_micros_;
  }

private:
  struct Pixel {
    Pixel() : r(0), g(0), b(0){}
    Pixel(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb){}
    uint8_t r; uint8_t g; uint8_t b;
  };
  const int width_;
  const int height_;
  int delay_micros_;

  Pixel *content_;
};
}

// Load still image or animation.
static bool LoadAnimation(const char *filename, int width, int height,
                          std::vector<PreprocessedFrame*> *sequence_pics) {
  std::vector<Magick::Image> frames;
  fprintf(stderr, "Read image...\n");
  readImages(&frames, filename);
  if (frames.size() == 0) {
    fprintf(stderr, "No image found.");
    return false;
  }

  // Put together the animation from single frames. GIFs can have nasty
  // disposal modes, but they are handled nicely by coalesceImages()
  std::vector<Magick::Image> coalesced;
  if (frames.size() > 1) {
    fprintf(stderr, "Assembling animation with %d frames.\n",
            (int)frames.size());
    Magick::coalesceImages(&coalesced, frames.begin(), frames.end());
  } else {
    coalesced.push_back(frames[0]);   // just a single still image.
  }

  fprintf(stderr, "Scale ... %dx%d -> %dx%d\n",
          (int)coalesced[0].columns(), (int)coalesced[0].rows(),
          width, height);
  for (size_t i = 0; i < coalesced.size(); ++i) {
    coalesced[i].zoom(Magick::Geometry(width, height));
  }
  fprintf(stderr, "Preprocess for display.\n");
  for (size_t i = 0; i < coalesced.size(); ++i) {
    sequence_pics->push_back(new PreprocessedFrame(coalesced[i]));
  }
  return true;
}

static void DisplayAnimation(const std::vector<PreprocessedFrame*> &frames,
                             rgb_matrix::Canvas *canvas) {
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);
  fprintf(stderr, "Display.\n");
  for (unsigned int i = 0; !interrupt_received; ++i) {
    const PreprocessedFrame *frame = frames[i % frames.size()];
    frame->CopyToCanvas(canvas);
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
          "\t-r <rows>     : Display rows. 16 for 16x32, 32 for 32x32. "
          "Default: 32\n"
          "\t-P <parallel> : For Plus-models or RPi2: parallel chains. 1..3. "
          "Default: 1\n"
          "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
          "\t-d            : Run as daemon.\n");
  return 1;
}

int main(int argc, char *argv[]) {
  int rows = 32;
  int chain = 1;
  int parallel = 1;
  int pwm_bits = -1;
  bool as_daemon = false;

  int opt;
  while ((opt = getopt(argc, argv, "r:P:c:p:")) != -1) {
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

  if (rows != 16 && rows != 32) {
    fprintf(stderr, "Rows can either be 16 or 32\n");
    return usage(argv[0]);
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

  std::vector<PreprocessedFrame*> sequence_pics;
  if (!LoadAnimation(filename, matrix->width(), matrix->height(),
                     &sequence_pics)) {
    return 0;
  }

  DisplayAnimation(sequence_pics, matrix);

  fprintf(stderr, "Caught signal. Exiting.\n");

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}
