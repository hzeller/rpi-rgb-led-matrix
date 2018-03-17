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
#include "pixel-mapper.h"
#include "content-streamer.h"

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

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
using rgb_matrix::StreamReader;

typedef int64_t tmillis_t;
static const tmillis_t distant_future = (1LL<<40); // that is a while.

struct ImageParams {
  ImageParams() : anim_duration_ms(distant_future), wait_ms(1500),
                  anim_delay_ms(-1), loops(-1) {}
  tmillis_t anim_duration_ms;  // If this is an animation, duration to show.
  tmillis_t wait_ms;           // Regular image: duration to show.
  tmillis_t anim_delay_ms;     // Animation delay override.
  int loops;
};

struct FileInfo {
  ImageParams params;      // Each file might have specific timing settings
  bool is_multi_frame;
  rgb_matrix::StreamIO *content_stream;
};

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

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

static void StoreInStream(const Magick::Image &img, int delay_time_us,
                          bool do_center,
                          rgb_matrix::FrameCanvas *scratch,
                          rgb_matrix::StreamWriter *output) {
  scratch->Clear();
  const int x_offset = do_center ? (scratch->width() - img.columns()) / 2 : 0;
  const int y_offset = do_center ? (scratch->height() - img.rows()) / 2 : 0;
  for (size_t y = 0; y < img.rows(); ++y) {
    for (size_t x = 0; x < img.columns(); ++x) {
      const Magick::Color &c = img.pixelColor(x, y);
      if (c.alphaQuantum() < 256) {
        scratch->SetPixel(x + x_offset, y + y_offset,
                          ScaleQuantumToChar(c.redQuantum()),
                          ScaleQuantumToChar(c.greenQuantum()),
                          ScaleQuantumToChar(c.blueQuantum()));
      }
    }
  }
  output->Stream(*scratch, delay_time_us);
}

static void CopyStream(rgb_matrix::StreamReader *r,
                       rgb_matrix::StreamWriter *w,
                       rgb_matrix::FrameCanvas *scratch) {
  uint32_t delay_us;
  while (r->GetNext(scratch, &delay_us)) {
    w->Stream(*scratch, delay_us);
  }
}

// Load still image or animation.
// Scale, so that it fits in "width" and "height" and store in "result".
static bool LoadImageAndScale(const char *filename,
                              int target_width, int target_height,
                              bool fill_width, bool fill_height,
                              std::vector<Magick::Image> *result,
                              std::string *err_msg) {
  std::vector<Magick::Image> frames;
  try {
    readImages(&frames, filename);
  } catch (std::exception& e) {
    if (e.what()) *err_msg = e.what();
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

void DisplayAnimation(const FileInfo *file,
                      RGBMatrix *matrix, FrameCanvas *offscreen_canvas,
                      int vsync_multiple) {
  const tmillis_t duration_ms = (file->is_multi_frame
                                 ? file->params.anim_duration_ms
                                 : file->params.wait_ms);
  rgb_matrix::StreamReader reader(file->content_stream);
  int loops = file->params.loops;
  const tmillis_t end_time_ms = GetTimeInMillis() + duration_ms;
  const tmillis_t override_anim_delay = file->params.anim_delay_ms;
  for (int k = 0;
       (loops < 0 || k < loops)
         && !interrupt_received
         && GetTimeInMillis() < end_time_ms;
       ++k) {
    uint32_t delay_us = 0;
    while (!interrupt_received && GetTimeInMillis() <= end_time_ms
           && reader.GetNext(offscreen_canvas, &delay_us)) {
      const tmillis_t anim_delay_ms =
        override_anim_delay >= 0 ? override_anim_delay : delay_us / 1000;
      const tmillis_t start_wait_ms = GetTimeInMillis();
      offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas, vsync_multiple);
      const tmillis_t time_already_spent = GetTimeInMillis() - start_wait_ms;
      SleepMillis(anim_delay_ms - time_already_spent);
    }
    reader.Rewind();
  }
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options] <image> [option] [<image> ...]\n",
          progname);

  fprintf(stderr, "Options:\n"
          "\t-O<streamfile>            : Output to stream-file instead of matrix (Don't need to be root).\n"
          "\t-C                        : Center images.\n"

          "\nThese options affect images following them on the command line:\n"
          "\t-w<seconds>               : Regular image: "
          "Wait time in seconds before next image is shown (default: 1.5).\n"
          "\t-t<seconds>               : "
          "For animations: stop after this time.\n"
          "\t-l<loop-count>            : "
          "For animations: number of loops through a full cycle.\n"
          "\t-D<animation-delay-ms>    : "
          "For animations: override the delay between frames given in the\n"
          "\t                            gif/stream animation with this value. Use -1 to use default value.\n"

          "\nOptions affecting display of multiple images:\n"
          "\t-f                        : "
          "Forever cycle through the list of files on the command line.\n"
          "\t-s                        : If multiple images are given: shuffle.\n"
          "\nDisplay Options:\n"
          "\t-V<vsync-multiple>        : Expert: Only do frame vsync-swaps on multiples of refresh (default: 1)\n"
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

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);

  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  int vsync_multiple = 1;
  bool do_forever = false;
  bool do_center = false;
  bool do_shuffle = false;

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

  const char *stream_output = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "w:t:l:fr:c:P:LhCR:sO:V:D:")) != -1) {
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
    case 'D':
      img_param.anim_delay_ms = atoi(optarg);
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
      fprintf(stderr, "Instead of deprecated -r, use --led-rows=%s instead.\n",
              optarg);
      matrix_options.rows = atoi(optarg);
      break;
    case 'c':
      fprintf(stderr, "Instead of deprecated -c, use --led-chain=%s instead.\n",
              optarg);
      matrix_options.chain_length = atoi(optarg);
      break;
    case 'P':
      matrix_options.parallel = atoi(optarg);
      break;
    case 'L':
      fprintf(stderr, "-L is deprecated. Use\n\t--led-pixel-mapper=\"U-mapper\" --led-chain=4\ninstead.\n");
      return 1;
      break;
    case 'R':
      fprintf(stderr, "-R is deprecated. "
              "Use --led-pixel-mapper=\"Rotate:%s\" instead.\n", optarg);
      return 1;
      break;
    case 'O':
      stream_output = strdup(optarg);
      break;
    case 'V':
      vsync_multiple = atoi(optarg);
      if (vsync_multiple < 1) vsync_multiple = 1;
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
  runtime_opt.do_gpio_init = (stream_output == NULL);
  RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL)
    return 1;

  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

  printf("Size: %dx%d. Hardware gpio mapping: %s\n",
         matrix->width(), matrix->height(), matrix_options.hardware_mapping);

  // These parameters are needed once we do scrolling.
  const bool fill_width = false;
  const bool fill_height = false;

  // In case the output to stream is requested, set up the stream object.
  rgb_matrix::StreamIO *stream_io = NULL;
  rgb_matrix::StreamWriter *global_stream_writer = NULL;
  if (stream_output) {
    int fd = open(stream_output, O_CREAT|O_WRONLY, 0644);
    if (fd < 0) {
      perror("Couldn't open output stream");
      return 1;
    }
    stream_io = new rgb_matrix::FileStreamIO(fd);
    global_stream_writer = new rgb_matrix::StreamWriter(stream_io);
  }

  const tmillis_t start_load = GetTimeInMillis();
  fprintf(stderr, "Loading %d files...\n", argc - optind);
  // Preparing all the images beforehand as the Pi might be too slow to
  // be quickly switching between these. So preprocess.
  std::vector<FileInfo*> file_imgs;
  for (int imgarg = optind; imgarg < argc; ++imgarg) {
    const char *filename = argv[imgarg];
    FileInfo *file_info = NULL;

    std::string err_msg;
    std::vector<Magick::Image> image_sequence;
    if (LoadImageAndScale(filename, matrix->width(), matrix->height(),
                          fill_width, fill_height, &image_sequence, &err_msg)) {
      file_info = new FileInfo();
      file_info->params = filename_params[filename];
      file_info->content_stream = new rgb_matrix::MemStreamIO();
      file_info->is_multi_frame = image_sequence.size() > 1;
      rgb_matrix::StreamWriter out(file_info->content_stream);
      for (size_t i = 0; i < image_sequence.size(); ++i) {
        const Magick::Image &img = image_sequence[i];
        int64_t delay_time_us;
        if (file_info->is_multi_frame) {
          delay_time_us = img.animationDelay() * 10000; // unit in 1/100s
        } else {
          delay_time_us = file_info->params.wait_ms * 1000;  // single image.
        }
        if (delay_time_us <= 0) delay_time_us = 100 * 1000;  // 1/10sec
        StoreInStream(img, delay_time_us, do_center, offscreen_canvas,
                      global_stream_writer ? global_stream_writer : &out);
      }
    } else {
      // Ok, not an image. Let's see if it is one of our streams.
      int fd = open(filename, O_RDONLY);
      if (fd >= 0) {
        file_info = new FileInfo();
        file_info->params = filename_params[filename];
        file_info->content_stream = new rgb_matrix::FileStreamIO(fd);
        StreamReader reader(file_info->content_stream);
        if (reader.GetNext(offscreen_canvas, NULL)) {  // header+size ok
          file_info->is_multi_frame = reader.GetNext(offscreen_canvas, NULL);
          reader.Rewind();
          if (global_stream_writer) {
            CopyStream(&reader, global_stream_writer, offscreen_canvas);
          }
        } else {
          err_msg = "Can't read as image or compatible stream";
          delete file_info->content_stream;
          delete file_info;
          file_info = NULL;
        }
      }
    }

    if (file_info) {
      file_imgs.push_back(file_info);
    } else {
      fprintf(stderr, "%s skipped: Unable to open (%s)\n",
              filename, err_msg.c_str());
    }
  }

  if (stream_output) {
    delete global_stream_writer;
    delete stream_io;
    if (file_imgs.size()) {
      fprintf(stderr, "Done: Output to stream %s; "
              "this can now be opened with led-image-viewer with the exact same panel configuration settings such as rows, chain, parallel and hardware-mapping\n", stream_output);
    }
    if (do_shuffle)
      fprintf(stderr, "Note: -s (shuffle) does not have an effect when generating streams.\n");
    if (do_forever)
      fprintf(stderr, "Note: -f (forever) does not have an effect when generating streams.\n");
    // Done, no actual output to matrix.
    return 0;
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

  fprintf(stderr, "Loading took %.3fs; now: Display.\n",
          (GetTimeInMillis() - start_load) / 1000.0);

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  do {
    if (do_shuffle) {
      std::random_shuffle(file_imgs.begin(), file_imgs.end());
    }
    for (size_t i = 0; i < file_imgs.size() && !interrupt_received; ++i) {
      DisplayAnimation(file_imgs[i], matrix, offscreen_canvas, vsync_multiple);
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
