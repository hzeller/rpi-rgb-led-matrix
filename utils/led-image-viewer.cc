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

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <Magick++.h>
#include <magick/image.h>

using rgb_matrix::GPIO;
using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

typedef int64_t tmillis_t;
static const tmillis_t distant_future = (1LL<<40); // that is a while.
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
  PreprocessedFrame(const Magick::Image &img, bool do_center,
                    rgb_matrix::FrameCanvas *output)
    : canvas_(output) {
    int delay_time = img.animationDelay();  // in 1/100s of a second.
    if (delay_time < 1) delay_time = 1;
    delay_millis_ = delay_time * 10;

    const int x_offset = do_center ? (output->width() - img.columns()) / 2 : 0;
    const int y_offset = do_center ? (output->height() - img.rows()) / 2 : 0;
    for (size_t y = 0; y < img.rows(); ++y) {
      for (size_t x = 0; x < img.columns(); ++x) {
        const Magick::Color &c = img.pixelColor(x, y);
        if (c.alphaQuantum() < 256) {
          output->SetPixel(x + x_offset, y + y_offset,
                           ScaleQuantumToChar(c.redQuantum()),
                           ScaleQuantumToChar(c.greenQuantum()),
                           ScaleQuantumToChar(c.blueQuantum()));
        }
      }
    }
  }

  FrameCanvas *canvas() const { return canvas_; }

  tmillis_t delay_millis() const { return delay_millis_; }
  void set_delay(tmillis_t delay) { delay_millis_ = delay; }

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
            SleepMillis(frame->delay_millis());
        }
    }
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options] <image> [option] [<image> ...]\n",
          progname);

  fprintf(stderr, "Options:\n"
          "\t-C                        : Center images.\n"
          "\t-w<seconds>               : If multiple images given: "
          "Wait time between in seconds (default: 1.5).\n"
          "\t-f                        : "
          "Forever cycle through the list of files on the command line.\n"
          "\t-t<seconds>               : "
          "For gif animations: stop after this time.\n"
          "\t-l<loop-count>            : "
          "For gif animations: number of loops through a full cycle.\n"
          "\t-s                        : If multiple images are given: shuffle.\n"
          "\t-L                        : Large display, in which each chain is 'folded down'\n"
          "\t                            in the middle in an U-arrangement to get more vertical space.\n"
          "\t-R<angle>                 : Rotate output; steps of 90 degrees\n"
          );

  fprintf(stderr, "\nGeneral LED matrix options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);

  fprintf(stderr,
          "\nSwitch time between files: "
          "-w for static images; -t/-l for animations\n"
          "Animated gifs: If both -l and -t are given, "
          "whatever finishes first determines duration.\n");

  fprintf(stderr, "\nThe -w, -t and -l options apply to the following images "
          "until a new instance of one of these options is seen.\n"
          "So you can choose different durations for different images.\n");

  return 1;
}

struct ImageParams {
  ImageParams() : anim_duration_ms(distant_future), wait_ms(1500), loops(-1){}
  tmillis_t anim_duration_ms;
  tmillis_t wait_ms;
  int loops;
};

struct FileInfo {
  ImageParams params;      // Each file might have specific timing settings
  PreprocessedList frames; // For animations: possibly multiple frames.
};

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);

  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  bool do_forever = false;
  bool do_center = false;
  bool do_shuffle = false;
  bool large_display = false;  // 64x64 made out of 4 in sequence.
  int angle = -361;

  // We remember ImageParams for each image, which will change whenever
  // there is a flag modifying them. This map keeps track of filenames
  // and their image params (also for unrelated elements of argv[], but doesn't
  // matter).
  // We map the pointer instad of the string of the argv parameter so that
  // we can have two times the same image on the commandline list with different
  // parameters.
  std::map<const void *, struct ImageParams> filename_params;

  // Set defaults.
  ImageParams img_param;
  for (int i = 0; i < argc; ++i) {
    filename_params[argv[i]] = img_param;
  }

  int opt;
  while ((opt = getopt(argc, argv, "w:t:l:fr:c:P:LhCR:s")) != -1) {
    switch (opt) {
    case 'w':
      img_param.wait_ms = roundf(atof(optarg) * 1000.0f);
      break;
    case 't':
      img_param.anim_duration_ms = roundf(atof(optarg) * 1000.0f);
      break;
    case 'l':
      img_param.loops = atoi(optarg);
      break;
    case 'f':
      do_forever = true;
      break;
    case 'C':
      do_center = true;
      break;
    case 's':
      do_shuffle = true;
      break;
    case 'r':
      matrix_options.rows = atoi(optarg);
      break;
    case 'c':
      matrix_options.chain_length = atoi(optarg);
      break;
    case 'P':
      matrix_options.parallel = atoi(optarg);
      break;
    case 'L':
      if (matrix_options.chain_length == 1) {
        // If this is still default, force the 64x64 arrangement.
        matrix_options.chain_length = 4;
      }
      large_display = true;
      break;
    case 'R':
      angle = atoi(optarg);
      break;
    case 'h':
    default:
      return usage(argv[0]);
    }

    // Starting from the current file, set all the remaining files to
    // the latest change.
    for (int i = optind; i < argc; ++i) {
      filename_params[argv[i]] = img_param;
    }
  }

  const int filename_count = argc - optind;
  if (filename_count == 0) {
    fprintf(stderr, "Expected image filename.\n");
    return usage(argv[0]);
  }

  // Prepare matrix
  RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL)
    return 1;

  if (large_display) {
    // Mapping the coordinates of a 32x128 display mapped to a square of 64x64,
    // or any other U-shape.
    matrix->ApplyStaticTransformer(rgb_matrix::UArrangementTransformer(
                                     matrix_options.parallel));
  }

  if (angle >= -360) {
    matrix->ApplyStaticTransformer(rgb_matrix::RotateTransformer(angle));
  }

  printf("Size: %dx%d. Hardware gpio mapping: %s\n",
         matrix->width(), matrix->height(), matrix_options.hardware_mapping);

  // These parameters are needed once we do scrolling.
  const bool fill_width = false;
  const bool fill_height = false;

  fprintf(stderr, "Load images...\n");
  // Preparing all the images beforehand as the Pi might be too slow to
  // be quickly switching between these. So preprocess.
  std::vector<FileInfo*> file_imgs;
  for (int imgarg = optind; imgarg < argc; ++imgarg) {
      const char *filename = argv[imgarg];

      std::vector<Magick::Image> image_sequence;
      if (!LoadImageAndScale(filename, matrix->width(), matrix->height(),
                             fill_width, fill_height, &image_sequence)) {
        continue;
      }

      FileInfo *file_info = new FileInfo();
      file_info->params = filename_params[filename];
      // Convert to preprocessed frames.
      for (size_t i = 0; i < image_sequence.size(); ++i) {
        FrameCanvas *canvas = matrix->CreateFrameCanvas();
        file_info->frames.push_back(
          new PreprocessedFrame(image_sequence[i], do_center, canvas));
      }
      // The 'animation delay' of a single image is the time to the next image.
      if (file_info->frames.size() == 1) {
        file_info->frames.back()->set_delay(file_info->params.wait_ms);
      }
      file_imgs.push_back(file_info);
  }

  // Some parameter sanity adjustments.
  if (file_imgs.empty()) {
    // e.g. if all files could not be interpreted as image.
    fprintf(stderr, "No image could be loaded.\n");
    return 1;
  } else if (file_imgs.size() == 1) {
    // Single image: show forever.
    file_imgs[0]->params.wait_ms = distant_future;
  } else {
    for (size_t i = 0; i < file_imgs.size(); ++i) {
      ImageParams &params = file_imgs[i]->params;
      // Forever animation ? Set to loop only once, otherwise that animation
      // would just run forever, stopping all the images after it.
      if (params.loops < 0 && params.anim_duration_ms == distant_future) {
        params.loops = 1;
      }
    }
  }

  fprintf(stderr, "Display.\n");

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  do {
    if (do_shuffle) {
      std::random_shuffle(file_imgs.begin(), file_imgs.end());
    }
    for (size_t i = 0; i < file_imgs.size() && !interrupt_received; ++i) {
      const FileInfo *file = file_imgs[i];

      const tmillis_t duration = ((file->frames.size() == 1)
                                  ? file->params.wait_ms
                                  : file->params.anim_duration_ms);
      DisplayAnimation(file->frames, duration, file->params.loops, matrix);
    }
  } while (do_forever && !interrupt_received);

  if (interrupt_received) {
    fprintf(stderr, "Caught signal. Exiting.\n");
  }

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  // Leaking the FileInfos, but don't care at program end.
  return 0;
}
