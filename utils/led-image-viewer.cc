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
#include <sys/time.h>

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

typedef int64_t tmillis_t;
static tmillis_t GetTimeInMillis() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static void SleepMillis(tmillis_t milli_seconds) {
    if (milli_seconds <= 0) return;
    struct timespec ts;
    ts.tv_sec = milli_seconds / 1000;
    ts.tv_nsec = (milli_seconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
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
    delay_millis_ = delay_time * 10;

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

  tmillis_t delay_millis() const { return delay_millis_; }

private:
  FrameCanvas *const canvas_;
  tmillis_t delay_millis_;
};
}  // end anonymous namespace

typedef std::vector<PreprocessedFrame*> PreprocessedList;

// Load still image or animation.
// Scale, so that it fits in "width" and "height" and store in "result".
static bool LoadImageAndScale(const char *filename,
                              int target_width, int target_height,
                              bool fill_width, bool fill_height,
                              std::vector<Magick::Image> *result) {
    std::vector<Magick::Image> frames;
    try {
        readImages(&frames, filename);
    } catch (std::exception& e) {
        fprintf(stderr, "Trouble loading %s (%s)\n", filename, e.what());
        return false;
    }
    if (frames.size() == 0) {
        fprintf(stderr, "No image found.");
        return false;
    }

    // Put together the animation from single frames. GIFs can have nasty
    // disposal modes, but they are handled nicely by coalesceImages()
    if (frames.size() > 1) {
        Magick::coalesceImages(result, frames.begin(), frames.end());
    } else {
        result->push_back(frames[0]);   // just a single still image.
    }

    const int img_width = (*result)[0].columns();
    const int img_height = (*result)[0].rows();
    const float width_fraction = (float)target_width / img_width;
    const float height_fraction = (float)target_height / img_height;
    if (fill_width && fill_height) {
        // Scrolling diagonally. Fill as much as we can get in available space.
        // Largest scale fraction determines that.
        const float larger_fraction = (width_fraction > height_fraction)
            ? width_fraction
            : height_fraction;
        target_width = (int) roundf(larger_fraction * img_width);
        target_height = (int) roundf(larger_fraction * img_height);
    }
    else if (fill_height) {
        // Horizontal scrolling: Make things fit in vertical space.
        // While the height constraint stays the same, we can expand to full
        // width as we scroll along that axis.
        target_width = (int) roundf(height_fraction * img_width);
    }
    else if (fill_width) {
        // dito, vertical. Make things fit in horizontal space.
        target_height = (int) roundf(width_fraction * img_height);
    }

    for (size_t i = 0; i < result->size(); ++i) {
        (*result)[i].scale(Magick::Geometry(target_width, target_height));
    }

    return true;
}

void DisplayAnimation(const PreprocessedList &frames,
                      tmillis_t duration_ms, int loops, RGBMatrix *matrix) {
    const tmillis_t end_time_ms = GetTimeInMillis() + duration_ms;
    if (frames.size() == 1)
        loops = 1;   // If there is no animation, nothing to repeat.
    for (int k = 0;
         (loops < 0 || k < loops)
             && !interrupt_received
             && GetTimeInMillis() < end_time_ms;
         ++k) {
        for (unsigned int i = 0; i < frames.size() && !interrupt_received; ++i) {
            if (interrupt_received || GetTimeInMillis() > end_time_ms)
                break;
            PreprocessedFrame *frame = frames[i];
            matrix->SwapOnVSync(frame->canvas());
            SleepMillis(frames.size() == 1
                        ? duration_ms : frame->delay_millis());
        }
    }
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options] <image> [<image> ...]\n", progname);
  fprintf(stderr, "Options:\n"
          "\t-w<seconds>               : If multiple images given: "
          "Wait time between in seconds (default: 1.5).\n"
          "\t-f                        : "
          "Forever cycle through the list of files on the command line.\n"
          "\t-t<seconds>               : "
          "For gif animations: stop after this time.\n"
          "\t-l<loop-count>            : "
          "For gif animations: number of loops through a full cycle.\n");

  fprintf(stderr, "\nGeneral LED matrix options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);

  fprintf(stderr,
          "Animated gifs                     : If both -l and -t are given, "
          "whatever comes first stops.\n"
          "Switch time between multiple files: "
          "-w for static images; -t/-l for animations\n");
  return 1;
}

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);
  RGBMatrix *const matrix = rgb_matrix::CreateMatrixFromFlags(&argc, &argv);

  bool do_forever = false;
  const tmillis_t distant_future = (1LL<<40); // that is a while.
  tmillis_t anim_duration_ms = distant_future;
  tmillis_t wait_ms = 1500;
  int loops  = -1;

  int opt;
  while ((opt = getopt(argc, argv, "w:t:l:fh")) != -1) {
    switch (opt) {
    case 'w':
      wait_ms = roundf(atof(optarg) * 1000.0f);
      break;
    case 't':
      anim_duration_ms = roundf(atof(optarg) * 1000.0f);
      break;
    case 'l':
      loops = atoi(optarg);
      break;
    case 'f':
      do_forever = true;
      break;
    case 'h':
    default:
      return usage(argv[0]);
    }
  }

  const int filename_count = argc - optind;
  if (filename_count == 0) {
    fprintf(stderr, "Expected image filename.\n");
    return usage(argv[0]);
  }

  if (filename_count == 1) {
    wait_ms = distant_future;
  }
  else if (filename_count > 1 &&
           loops < 0 && anim_duration_ms == distant_future) {
    // More than one image but parameters for animations are default ? Set them
    // to requested wait time, otherwise the first animation would just run
    // forever.
    anim_duration_ms = wait_ms;
  }

  if (matrix == NULL)
    return 1;

  const bool fill_width = false;
  const bool fill_height = false;

  // Preparing all the images beforehand as the Pi might be too slow to
  // be quickly switching between these.
  std::vector<PreprocessedList> file_imgs;
  for (int imgarg = optind; imgarg < argc && !interrupt_received; ++imgarg) {
      const char *filename = argv[imgarg];

      std::vector<Magick::Image> image_sequence;
      if (!LoadImageAndScale(filename, matrix->width(), matrix->height(),
                             fill_width, fill_height, &image_sequence)) {
        continue;
      }

      PreprocessedList frames;
      CanvasTransformer *const transformer = matrix->transformer();
      // Convert to preprocessed frames.
      for (size_t i = 0; i < image_sequence.size(); ++i) {
        FrameCanvas *canvas = matrix->CreateFrameCanvas();
        frames.push_back(new PreprocessedFrame(image_sequence[i],
                                               transformer, canvas));
      }
      file_imgs.push_back(frames);
  }

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  do {
    for (size_t i = 0; i < file_imgs.size() && !interrupt_received; ++i) {
      const PreprocessedList frames = file_imgs[i];

      const tmillis_t duration = ((frames.size() == 1)
                                  ? wait_ms
                                  : anim_duration_ms);
      DisplayAnimation(frames, duration , loops, matrix);
    }
  } while (do_forever && !interrupt_received);

  if (interrupt_received)
    fprintf(stderr, "Caught signal. Exiting.\n");

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  return 0;
}
