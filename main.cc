// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "led-matrix.h"
#include "threaded-matrix-manipulator.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

using std::min;
using std::max;

/*
 * The following are demo image generators. They all use the utility
 * class ThreadedMatrixManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.
class ColorPulseGenerator : public ThreadedMatrixManipulator {
public:
  ColorPulseGenerator(RGBMatrix *m) : ThreadedMatrixManipulator(m) {}
  void Run() {
    const int width = matrix_->width();
    const int height = matrix_->height();
    uint32_t count = 0;
    while (running_) {
      sleep(2);
      ++count;
      int color = count % 6;
      int value = 0xff;
      int r, g, b;
      switch (color) {
      case 0: r = value; g = b = 0; break;
      case 1: r = g = value; b = 0; break;
      case 2: g = value; r = b = 0; break;
      case 3: g = b = value; r = 0; break;
      case 4: b = value; r = g = 0; break;
      default: r = g = b = value; break;
      }
      for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
          matrix_->SetPixel(x, y, r, g, b);
    }
  }
};

class SimpleSquare : public ThreadedMatrixManipulator {
public:
  SimpleSquare(RGBMatrix *m) : ThreadedMatrixManipulator(m) {}
  void Run() {
    const int width = matrix_->width();
    const int height = matrix_->height();
    // Diagonal
    for (int x = 0; x < width; ++x) {
      matrix_->SetPixel(x, x, 255, 255, 255);           // white
      matrix_->SetPixel(height -1 - x, x, 255, 0, 255); // magenta
    }
    for (int x = 0; x < width; ++x) {
      matrix_->SetPixel(x, 0, 255, 0, 0);              // top line: red
      matrix_->SetPixel(x, height - 1, 255, 255, 0);   // bottom line: yellow
    }
    for (int y = 0; y < height; ++y) {
      matrix_->SetPixel(0, y, 0, 0, 255);              // left line: blue
      matrix_->SetPixel(width - 1, y, 0, 255, 0);      // right line: green
    }
  }
};

// Simple class that generates a rotating block on the screen.
class RotatingBlockGenerator : public ThreadedMatrixManipulator {
public:
  RotatingBlockGenerator(RGBMatrix *m) : ThreadedMatrixManipulator(m) {}

  uint8_t scale_col(int val, int lo, int hi) {
    if (val < lo) return 0;
    if (val > hi) return 255;
    return 255 * (val - lo) / (hi - lo);
  }

  void Run() {
    const int cent_x = matrix_->width() / 2;
    const int cent_y = matrix_->height() / 2;

    // The square to rotate (inner square + black frame) needs to cover the
    // whole area, even if diagnoal. Thus, when rotating, the outer pixels from
    // the previous frame are cleared.
    const int rotate_square = min(matrix_->width(), matrix_->height()) * 1.41;
    const int min_rotate = cent_x - rotate_square / 2;
    const int max_rotate = cent_x + rotate_square / 2;

    // The square to display is within the visible area.
    const int display_square = min(matrix_->width(), matrix_->height()) * 0.7;
    const int min_display = cent_x - display_square / 2;
    const int max_display = cent_x + display_square / 2;

    const float deg_to_rad = 2 * 3.14159265 / 360;
    int rotation = 0;
    while (running_) {
      ++rotation;
      usleep(15 * 1000);
      rotation %= 360;
      for (int x = min_rotate; x < max_rotate; ++x) {
        for (int y = min_rotate; y < max_rotate; ++y) {
          float disp_x, disp_y;
          Rotate(x - cent_x, y - cent_x,
                 deg_to_rad * rotation, &disp_x, &disp_y);
          if (x >= min_display && x < max_display &&
              y >= min_display && y < max_display) { // within display square
            matrix_->SetPixel(disp_x + cent_x, disp_y + cent_y,
                              scale_col(x, min_display, max_display),
                              255 - scale_col(y, min_display, max_display),
                              scale_col(y, min_display, max_display));
          } else {
            // black frame.
            matrix_->SetPixel(disp_x + cent_x, disp_y + cent_y, 0, 0, 0);
          }
        }
      }
    }
  }

private:
  void Rotate(int x, int y, float angle,
              float *new_x, float *new_y) {
    *new_x = x * cosf(angle) - y * sinf(angle);
    *new_y = x * sinf(angle) + y * cosf(angle);
  }
};

class ImageScroller : public ThreadedMatrixManipulator {
public:
  // Scroll image with "scroll_jumps" pixels every "scroll_ms" milliseconds.
  // If "scroll_ms" is negative, don't do any scrolling.
  ImageScroller(RGBMatrix *m, int scroll_jumps, int scroll_ms = 30)
    : ThreadedMatrixManipulator(m), scroll_jumps_(scroll_jumps),
      scroll_ms_(scroll_ms),
      horizontal_position_(0) {
  }

  // _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
  // Not really robust. Use at your own risk :)
  // This allows reload of an image while things are running, e.g. you can
  // life-update the content.
  bool LoadPPM(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;
    char header_buf[256];
    const char *line = ReadLine(f, header_buf, sizeof(header_buf));
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
      fclose(f); return false; }
    if (sscanf(line, "P6 ") == EOF)
      EXIT_WITH_MSG("Can only handle P6 as PPM type.");
    line = ReadLine(f, header_buf, sizeof(header_buf));
    int new_width, new_height;
    if (!line || sscanf(line, "%d %d ", &new_width, &new_height) != 2)
      EXIT_WITH_MSG("Width/height expected");
    int value;
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
      EXIT_WITH_MSG("Only 255 for maxval allowed.");
    const size_t pixel_count = new_width * new_height;
    Pixel *new_image = new Pixel [ pixel_count ];
    assert(sizeof(Pixel) == 3);   // we make that assumption.
    if (fread(new_image, sizeof(Pixel), pixel_count, f) != pixel_count) {
      line = "";
      EXIT_WITH_MSG("Not enough pixels read.");
    }
#undef EXIT_WITH_MSG
    fclose(f);
    new_image_.image = new_image;
    new_image_.width = new_width;
    new_image_.height = new_height;
    fprintf(stderr, "Read image '%s' with %dx%d\n", filename,
            new_width, new_height);
    horizontal_position_ = 0;
    return true;
  }

  void Run() {
    const int screen_height = matrix_->height();
    const int screen_width = matrix_->width();
    while (running_) {
      if (new_image_.IsValid()) {
        current_image_.Delete();
        current_image_ = new_image_;
        new_image_.Reset();
      }
      if (!current_image_.IsValid()) {
        usleep(100 * 1000);
        continue;
      }
      for (int x = 0; x < screen_width; ++x) {
        for (int y = 0; y < screen_height; ++y) {
          const Pixel &p = current_image_.getPixel(
                       (horizontal_position_ + x) % current_image_.width, y);
          matrix_->SetPixel(x, y, p.red, p.green, p.blue);
        }
      }
      horizontal_position_ += scroll_jumps_;
      if (horizontal_position_ < 0) horizontal_position_ = current_image_.width;
      if (scroll_ms_ <= 0) {
        // No scrolling. We don't need the image anymore.
        current_image_.Delete();
      } else {
        usleep(scroll_ms_ * 1000);
      }
    }
  }

private:
  struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };

  struct Image {
    Image() : width(-1), height(-1), image(NULL) {}
    ~Image() { Delete(); }
    void Delete() { delete [] image; Reset(); }
    void Reset() { image = NULL; width = -1; height = -1; }
    inline bool IsValid() { return image && height > 0 && width > 0; }
    const Pixel &getPixel(int x, int y) {
      static Pixel black;
      if (x < 0 || x > width || y < 0 || y > height) return black;
      return image[x + width * y];
    }

    int width;
    int height;
    Pixel *image;
  };

  // Read line, skip comments.
  char *ReadLine(FILE *f, char *buffer, size_t len) {
    char *result;
    do {
      result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#');
    return result;
  }

  const int scroll_jumps_;
  const int scroll_ms_;
  Image current_image_;
  Image new_image_;
  int32_t horizontal_position_;
};

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options> -D <demo-nr> [optional parameter]\n",
          progname);
  fprintf(stderr, "Options:\n"
          "\t-r <rows>     : Display rows. 16 for 16x32, 32 for 32x32. "
          "Default: 32\n"
          "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
          "\t-p <pwm-bits> : PWM bits used. Somewhere between 1 and 7\n"
          "\t-D <demo-nr>  : Always needs to be set\n"
          "\t-d            : run as daemon. Use this when starting in\n"
          "\t                /etc/init.d, but also when running without\n"
          "\t                terminal.\n"
          "\t-t <seconds>  : Run for these number of seconds, then exit\n"
          "\t       (if neither -d nor -t are supplied, waits for <RETURN>)\n");
  fprintf(stderr, "Demos, choosen with -D\n");
  fprintf(stderr, "\t0  - some rotating square\n"
          "\t1  - forward scrolling an image (-m <scroll-ms>)\n"
          "\t2  - backward scrolling an image (-m <scroll-ms>)\n"
          "\t3  - test image: a square\n"
          "\t4  - Pulsing color\n");
  fprintf(stderr, "Example:\n\t%s -t 10 -D 1 runtext.ppm\n"
          "Scrolls the runtext for 10 seconds\n", progname);
  return 1;
}

int main(int argc, char *argv[]) {
  bool as_daemon = false;
  int runtime_seconds = -1;
  int demo = -1;
  int rows = 32;
  int pwm_bits = -1;
  int chain = 1;
  int scroll_ms = 30;

  const char *demo_parameter = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "D:t:d:r:p:c:m:")) != -1) {
    switch (opt) {
    case 'D':
      demo = atoi(optarg);
      break;

    case 'd':
      as_daemon = true;
      break;

    case 't':
      runtime_seconds = atoi(optarg);
      break;

    case 'r':
      rows = atoi(optarg);
      break;

    case 'p':
      pwm_bits = atoi(optarg);
      break;

    case 'c':
      chain = atoi(optarg);
      break;

    case 'm':
      scroll_ms = atoi(optarg);
      break;

    default: /* '?' */
      return usage(argv[0]);
    }
  }

  if (optind < argc) {
    demo_parameter = argv[optind];
  }

  if (demo < 0) {
    fprintf(stderr, "Expected required option -D <demo>\n");
    return usage(argv[0]);
  }

  if (getuid() != 0) {
    fprintf(stderr, "Must run as root to be able to access /dev/mem\n"
            "Prepend 'sudo' to the command:\n\tsudo %s ...\n", argv[0]);
    return 1;
  }

  if (rows != 16 && rows != 32) {
    fprintf(stderr, "Rows can either be 16 or 32\n");
    return 1;
  }

  if (chain < 1 || chain > 8) {
    fprintf(stderr, "Chain outside usable range\n");
    return 1;
  }

  // Initialize GPIO pins. This might fail when we don't have permissions.
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

  // The matrix, our 'frame buffer' and display updater.
  RGBMatrix m(&io, rows, chain);

  if (pwm_bits > 0 && !m.SetPWMBits(pwm_bits)) {
    fprintf(stderr, "PWM bits outside supported range\n");
    return 1;
  }
    
  // The ThreadedMatrixManipulator objects are filling
  // the matrix continuously.
  ThreadedMatrixManipulator *image_gen = NULL;
  switch (demo) {
  case 0:
    image_gen = new RotatingBlockGenerator(&m);
    break;

  case 1:
  case 2:
    if (demo_parameter) {
      ImageScroller *scroller = new ImageScroller(&m,
                                                  demo == 1 ? 1 : -1,
                                                  scroll_ms);
      if (!scroller->LoadPPM(demo_parameter))
        return 1;
      image_gen = scroller;
    } else {
      fprintf(stderr, "Demo %d Requires PPM image as parameter\n", demo);
      return 1;
    }
    break;

  case 3:
    image_gen = new SimpleSquare(&m);
    break;

  case 4:
    image_gen = new ColorPulseGenerator(&m);
    break;
  }

  if (image_gen == NULL)
    return usage(argv[0]);

  // Image generating demo is crated. Now start the thread.
  image_gen->Start();

  // Now, the image genreation runs in the background. We can do arbitrary
  // things here in parallel. In this demo, we're essentially just
  // waiting for one of the conditions to exit.
  if (as_daemon) {
    sleep(runtime_seconds > 0 ? runtime_seconds : INT_MAX);
  } else if (runtime_seconds > 0) {
    sleep(runtime_seconds);
  } else {
    // Things are set up. Just wait for <RETURN> to be pressed.
    printf("Press <RETURN> to exit and reset LEDs\n");
    getchar();
  }

  // Stop image generating thread.
  delete image_gen;

  return 0;
}
