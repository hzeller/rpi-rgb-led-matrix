// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"
#include "transformer.h"
#include "graphics.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>

using std::min;
using std::max;

using namespace rgb_matrix;

/*
 * The following are demo image generators. They all use the utility
 * class ThreadedCanvasManipulator to generate new frames.
 */

// Simple generator that pulses through RGB and White.
class ColorPulseGenerator : public ThreadedCanvasManipulator {
public:
  ColorPulseGenerator(RGBMatrix *m) : ThreadedCanvasManipulator(m), matrix_(m) {
    off_screen_canvas_ = m->CreateFrameCanvas();
  }
  void Run() {
    uint32_t continuum = 0;
    while (running()) {
      usleep(5 * 1000);
      continuum += 1;
      continuum %= 3 * 255;
      int r = 0, g = 0, b = 0;
      if (continuum <= 255) {
        int c = continuum;
        b = 255 - c;
        r = c;
      } else if (continuum > 255 && continuum <= 511) {
        int c = continuum - 256;
        r = 255 - c;
        g = c;
      } else {
        int c = continuum - 512;
        g = 255 - c;
        b = c;
      }
      matrix_->transformer()->Transform(off_screen_canvas_)->Fill(r, g, b);
      off_screen_canvas_ = matrix_->SwapOnVSync(off_screen_canvas_);
    }
  }

private:
  RGBMatrix *const matrix_;
  FrameCanvas *off_screen_canvas_;
};

// Simple generator that pulses through brightness on red, green, blue and white
class BrightnessPulseGenerator : public ThreadedCanvasManipulator {
public:
  BrightnessPulseGenerator(RGBMatrix *m) : ThreadedCanvasManipulator(m), matrix_(m) {}
  void Run() {
    const uint8_t max_brightness = matrix_->brightness();
    const uint8_t c = 255;
    uint8_t count = 0;

    while (running()) {
      if (matrix_->brightness() < 1) {
        matrix_->SetBrightness(max_brightness);
        count++;
      } else {
        matrix_->SetBrightness(matrix_->brightness() - 1);
      }

      switch (count % 4) {
        case 0: matrix_->Fill(c, 0, 0); break;
        case 1: matrix_->Fill(0, c, 0); break;
        case 2: matrix_->Fill(0, 0, c); break;
        case 3: matrix_->Fill(c, c, c); break;
      }

      usleep(20 * 1000);
    }
  }

private:
  RGBMatrix *const matrix_;
};

class SimpleSquare : public ThreadedCanvasManipulator {
public:
  SimpleSquare(Canvas *m) : ThreadedCanvasManipulator(m) {}
  void Run() {
    const int width = canvas()->width() - 1;
    const int height = canvas()->height() - 1;
    // Borders
    DrawLine(canvas(), 0, 0,      width, 0,      Color(255, 0, 0));
    DrawLine(canvas(), 0, height, width, height, Color(255, 255, 0));
    DrawLine(canvas(), 0, 0,      0,     height, Color(0, 0, 255));
    DrawLine(canvas(), width, 0,  width, height, Color(0, 255, 0));

    // Diagonals.
    DrawLine(canvas(), 0, 0,        width, height, Color(255, 255, 255));
    DrawLine(canvas(), 0, height, width, 0,        Color(255,   0, 255));
  }
};

class GrayScaleBlock : public ThreadedCanvasManipulator {
public:
  GrayScaleBlock(Canvas *m) : ThreadedCanvasManipulator(m) {}
  void Run() {
    const int sub_blocks = 16;
    const int width = canvas()->width();
    const int height = canvas()->height();
    const int x_step = max(1, width / sub_blocks);
    const int y_step = max(1, height / sub_blocks);
    uint8_t count = 0;
    while (running()) {
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          int c = sub_blocks * (y / y_step) + x / x_step;
          switch (count % 4) {
          case 0: canvas()->SetPixel(x, y, c, c, c); break;
          case 1: canvas()->SetPixel(x, y, c, 0, 0); break;
          case 2: canvas()->SetPixel(x, y, 0, c, 0); break;
          case 3: canvas()->SetPixel(x, y, 0, 0, c); break;
          }
        }
      }
      count++;
      sleep(2);
    }
  }
};

// Simple class that generates a rotating block on the screen.
class RotatingBlockGenerator : public ThreadedCanvasManipulator {
public:
  RotatingBlockGenerator(Canvas *m) : ThreadedCanvasManipulator(m) {}

  uint8_t scale_col(int val, int lo, int hi) {
    if (val < lo) return 0;
    if (val > hi) return 255;
    return 255 * (val - lo) / (hi - lo);
  }

  void Run() {
    const int cent_x = canvas()->width() / 2;
    const int cent_y = canvas()->height() / 2;

    // The square to rotate (inner square + black frame) needs to cover the
    // whole area, even if diagnoal. Thus, when rotating, the outer pixels from
    // the previous frame are cleared.
    const int rotate_square = min(canvas()->width(), canvas()->height()) * 1.41;
    const int min_rotate = cent_x - rotate_square / 2;
    const int max_rotate = cent_x + rotate_square / 2;

    // The square to display is within the visible area.
    const int display_square = min(canvas()->width(), canvas()->height()) * 0.7;
    const int min_display = cent_x - display_square / 2;
    const int max_display = cent_x + display_square / 2;

    const float deg_to_rad = 2 * 3.14159265 / 360;
    int rotation = 0;
    while (running()) {
      ++rotation;
      usleep(15 * 1000);
      rotation %= 360;
      for (int x = min_rotate; x < max_rotate; ++x) {
        for (int y = min_rotate; y < max_rotate; ++y) {
          float rot_x, rot_y;
          Rotate(x - cent_x, y - cent_x,
                 deg_to_rad * rotation, &rot_x, &rot_y);
          if (x >= min_display && x < max_display &&
              y >= min_display && y < max_display) { // within display square
            canvas()->SetPixel(rot_x + cent_x, rot_y + cent_y,
                               scale_col(x, min_display, max_display),
                               255 - scale_col(y, min_display, max_display),
                               scale_col(y, min_display, max_display));
          } else {
            // black frame.
            canvas()->SetPixel(rot_x + cent_x, rot_y + cent_y, 0, 0, 0);
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

class ImageScroller : public ThreadedCanvasManipulator {
public:
  // Scroll image with "scroll_jumps" pixels every "scroll_ms" milliseconds.
  // If "scroll_ms" is negative, don't do any scrolling.
  ImageScroller(RGBMatrix *m, int scroll_jumps, int scroll_ms = 30)
    : ThreadedCanvasManipulator(m), scroll_jumps_(scroll_jumps),
      scroll_ms_(scroll_ms),
      horizontal_position_(0),
      matrix_(m) {
      offscreen_ = matrix_->CreateFrameCanvas();
  }

  virtual ~ImageScroller() {
    Stop();
    WaitStopped();   // only now it is safe to delete our instance variables.
  }

  // _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
  // Not really robust. Use at your own risk :)
  // This allows reload of an image while things are running, e.g. you can
  // life-update the content.
  bool LoadPPM(const char *filename) {
    FILE *f = fopen(filename, "r");
    // check if file exists
    if (f == NULL && access(filename, F_OK) == -1) {
      fprintf(stderr, "File \"%s\" doesn't exist\n", filename);
      return false;
    }
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
    fprintf(stderr, "Read image '%s' with %dx%d\n", filename,
            new_width, new_height);
    horizontal_position_ = 0;
    MutexLock l(&mutex_new_image_);
    new_image_.Delete();  // in case we reload faster than is picked up
    new_image_.image = new_image;
    new_image_.width = new_width;
    new_image_.height = new_height;
    return true;
  }

  void Run() {
    const int screen_height = matrix_->transformer()->Transform(offscreen_)->height();
    const int screen_width = matrix_->transformer()->Transform(offscreen_)->width();
    while (running()) {
      {
        MutexLock l(&mutex_new_image_);
        if (new_image_.IsValid()) {
          current_image_.Delete();
          current_image_ = new_image_;
          new_image_.Reset();
        }
      }
      if (!current_image_.IsValid()) {
        usleep(100 * 1000);
        continue;
      }
      for (int x = 0; x < screen_width; ++x) {
        for (int y = 0; y < screen_height; ++y) {
          const Pixel &p = current_image_.getPixel(
                     (horizontal_position_ + x) % current_image_.width, y);
          matrix_->transformer()->Transform(offscreen_)->SetPixel(x, y, p.red, p.green, p.blue);
        }
      }
      offscreen_ = matrix_->SwapOnVSync(offscreen_);
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
    Pixel() : red(0), green(0), blue(0){}
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
      if (x < 0 || x >= width || y < 0 || y >= height) return black;
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

  // Current image is only manipulated in our thread.
  Image current_image_;

  // New image can be loaded from another thread, then taken over in main thread.
  Mutex mutex_new_image_;
  Image new_image_;

  int32_t horizontal_position_;

  RGBMatrix* matrix_;
  FrameCanvas* offscreen_;
};


// Abelian sandpile
// Contributed by: Vliedel
class Sandpile : public ThreadedCanvasManipulator {
public:
  Sandpile(Canvas *m, int delay_ms=50)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
    width_ = canvas()->width() - 1; // We need an odd width
    height_ = canvas()->height() - 1; // We need an odd height

    // Allocate memory
    values_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      values_[x] = new int[height_];
    }
    newValues_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      newValues_[x] = new int[height_];
    }

    // Init values
    srand(time(NULL));
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = 0;
      }
    }
  }

  ~Sandpile() {
    for (int x=0; x<width_; ++x) {
      delete [] values_[x];
    }
    delete [] values_;
    for (int x=0; x<width_; ++x) {
      delete [] newValues_[x];
    }
    delete [] newValues_;
  }

  void Run() {
    while (running()) {
      // Drop a sand grain in the centre
      values_[width_/2][height_/2]++;
      updateValues();

      for (int x=0; x<width_; ++x) {
        for (int y=0; y<height_; ++y) {
          switch (values_[x][y]) {
            case 0:
              canvas()->SetPixel(x, y, 0, 0, 0);
              break;
            case 1:
              canvas()->SetPixel(x, y, 0, 0, 200);
              break;
            case 2:
              canvas()->SetPixel(x, y, 0, 200, 0);
              break;
            case 3:
              canvas()->SetPixel(x, y, 150, 100, 0);
              break;
            default:
              canvas()->SetPixel(x, y, 200, 0, 0);
          }
        }
      }
      usleep(delay_ms_ * 1000); // ms
    }
  }

private:
  void updateValues() {
    // Copy values to newValues
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        newValues_[x][y] = values_[x][y];
      }
    }

    // Update newValues based on values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        if (values_[x][y] > 3) {
          // Collapse
          if (x>0)
            newValues_[x-1][y]++;
          if (x<width_-1)
            newValues_[x+1][y]++;
          if (y>0)
            newValues_[x][y-1]++;
          if (y<height_-1)
            newValues_[x][y+1]++;
          newValues_[x][y] -= 4;
        }
      }
    }
    // Copy newValues to values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = newValues_[x][y];
      }
    }
  }

  int width_;
  int height_;
  int** values_;
  int** newValues_;
  int delay_ms_;
};


// Conway's game of life
// Contributed by: Vliedel
class GameLife : public ThreadedCanvasManipulator {
public:
  GameLife(Canvas *m, int delay_ms=500, bool torus=true)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms), torus_(torus) {
    width_ = canvas()->width();
    height_ = canvas()->height();

    // Allocate memory
    values_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      values_[x] = new int[height_];
    }
    newValues_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      newValues_[x] = new int[height_];
    }

    // Init values randomly
    srand(time(NULL));
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y]=rand()%2;
      }
    }
    r_ = rand()%255;
    g_ = rand()%255;
    b_ = rand()%255;

    if (r_<150 && g_<150 && b_<150) {
      int c = rand()%3;
      switch (c) {
        case 0:
          r_ = 200;
          break;
        case 1:
          g_ = 200;
          break;
        case 2:
          b_ = 200;
          break;
      }
    }
  }

  ~GameLife() {
    for (int x=0; x<width_; ++x) {
      delete [] values_[x];
    }
    delete [] values_;
    for (int x=0; x<width_; ++x) {
      delete [] newValues_[x];
    }
    delete [] newValues_;
  }

  void Run() {
    while (running()) {

      updateValues();

      for (int x=0; x<width_; ++x) {
        for (int y=0; y<height_; ++y) {
          if (values_[x][y])
            canvas()->SetPixel(x, y, r_, g_, b_);
          else
            canvas()->SetPixel(x, y, 0, 0, 0);
        }
      }
      usleep(delay_ms_ * 1000); // ms
    }
  }

private:
  int numAliveNeighbours(int x, int y) {
    int num=0;
    if (torus_) {
      // Edges are connected (torus)
      num += values_[(x-1+width_)%width_][(y-1+height_)%height_];
      num += values_[(x-1+width_)%width_][y                    ];
      num += values_[(x-1+width_)%width_][(y+1        )%height_];
      num += values_[(x+1       )%width_][(y-1+height_)%height_];
      num += values_[(x+1       )%width_][y                    ];
      num += values_[(x+1       )%width_][(y+1        )%height_];
      num += values_[x                  ][(y-1+height_)%height_];
      num += values_[x                  ][(y+1        )%height_];
    }
    else {
      // Edges are not connected (no torus)
      if (x>0) {
        if (y>0)
          num += values_[x-1][y-1];
        if (y<height_-1)
          num += values_[x-1][y+1];
        num += values_[x-1][y];
      }
      if (x<width_-1) {
        if (y>0)
          num += values_[x+1][y-1];
        if (y<31)
          num += values_[x+1][y+1];
        num += values_[x+1][y];
      }
      if (y>0)
        num += values_[x][y-1];
      if (y<height_-1)
        num += values_[x][y+1];
    }
    return num;
  }

  void updateValues() {
    // Copy values to newValues
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        newValues_[x][y] = values_[x][y];
      }
    }
    // update newValues based on values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        int num = numAliveNeighbours(x,y);
        if (values_[x][y]) {
          // cell is alive
          if (num < 2 || num > 3)
            newValues_[x][y] = 0;
        }
        else {
          // cell is dead
          if (num == 3)
            newValues_[x][y] = 1;
        }
      }
    }
    // copy newValues to values
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = newValues_[x][y];
      }
    }
  }

  int** values_;
  int** newValues_;
  int delay_ms_;
  int r_;
  int g_;
  int b_;
  int width_;
  int height_;
  bool torus_;
};

// Langton's ant
// Contributed by: Vliedel
class Ant : public ThreadedCanvasManipulator {
public:
  Ant(Canvas *m, int delay_ms=500)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
    numColors_ = 4;
    width_ = canvas()->width();
    height_ = canvas()->height();
    values_ = new int*[width_];
    for (int x=0; x<width_; ++x) {
      values_[x] = new int[height_];
    }
  }

  ~Ant() {
    for (int x=0; x<width_; ++x) {
      delete [] values_[x];
    }
    delete [] values_;
  }

  void Run() {
    antX_ = width_/2;
    antY_ = height_/2-3;
    antDir_ = 0;
    for (int x=0; x<width_; ++x) {
      for (int y=0; y<height_; ++y) {
        values_[x][y] = 0;
        updatePixel(x, y);
      }
    }

    while (running()) {
      // LLRR
      switch (values_[antX_][antY_]) {
        case 0:
        case 1:
          antDir_ = (antDir_+1+4) % 4;
          break;
        case 2:
        case 3:
          antDir_ = (antDir_-1+4) % 4;
          break;
      }

      values_[antX_][antY_] = (values_[antX_][antY_] + 1) % numColors_;
      int oldX = antX_;
      int oldY = antY_;
      switch (antDir_) {
        case 0:
          antX_++;
          break;
        case 1:
          antY_++;
          break;
        case 2:
          antX_--;
          break;
        case 3:
          antY_--;
          break;
      }
      updatePixel(oldX, oldY);
      if (antX_ < 0 || antX_ >= width_ || antY_ < 0 || antY_ >= height_)
        return;
      updatePixel(antX_, antY_);
      usleep(delay_ms_ * 1000);
    }
  }

private:
  void updatePixel(int x, int y) {
    switch (values_[x][y]) {
      case 0:
        canvas()->SetPixel(x, y, 200, 0, 0);
        break;
      case 1:
        canvas()->SetPixel(x, y, 0, 200, 0);
        break;
      case 2:
        canvas()->SetPixel(x, y, 0, 0, 200);
        break;
      case 3:
        canvas()->SetPixel(x, y, 150, 100, 0);
        break;
    }
    if (x == antX_ && y == antY_)
      canvas()->SetPixel(x, y, 0, 0, 0);
  }

  int numColors_;
  int** values_;
  int antX_;
  int antY_;
  int antDir_; // 0 right, 1 up, 2 left, 3 down
  int delay_ms_;
  int width_;
  int height_;
};



// Imitation of volume bars
// Purely random height doesn't look realistic
// Contributed by: Vliedel
class VolumeBars : public ThreadedCanvasManipulator {
public:
  VolumeBars(Canvas *m, int delay_ms=50, int numBars=8)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms),
      numBars_(numBars), t_(0) {
  }

  ~VolumeBars() {
    delete [] barHeights_;
    delete [] barFreqs_;
    delete [] barMeans_;
  }

  void Run() {
    const int width = canvas()->width();
    height_ = canvas()->height();
    barWidth_ = width/numBars_;
    barHeights_ = new int[numBars_];
    barMeans_ = new int[numBars_];
    barFreqs_ = new int[numBars_];
    heightGreen_  = height_*4/12;
    heightYellow_ = height_*8/12;
    heightOrange_ = height_*10/12;
    heightRed_    = height_*12/12;

    // Array of possible bar means
    int numMeans = 10;
    int means[10] = {1,2,3,4,5,6,7,8,16,32};
    for (int i=0; i<numMeans; ++i) {
      means[i] = height_ - means[i]*height_/8;
    }
    // Initialize bar means randomly
    srand(time(NULL));
    for (int i=0; i<numBars_; ++i) {
      barMeans_[i] = rand()%numMeans;
      barFreqs_[i] = 1<<(rand()%3);
    }

    // Start the loop
    while (running()) {
      if (t_ % 8 == 0) {
        // Change the means
        for (int i=0; i<numBars_; ++i) {
          barMeans_[i] += rand()%3 - 1;
          if (barMeans_[i] >= numMeans)
            barMeans_[i] = numMeans-1;
          if (barMeans_[i] < 0)
            barMeans_[i] = 0;
        }
      }

      // Update bar heights
      t_++;
      for (int i=0; i<numBars_; ++i) {
        barHeights_[i] = (height_ - means[barMeans_[i]])
          * sin(0.1*t_*barFreqs_[i]) + means[barMeans_[i]];
        if (barHeights_[i] < height_/8)
          barHeights_[i] = rand() % (height_/8) + 1;
      }

      for (int i=0; i<numBars_; ++i) {
        int y;
        for (y=0; y<barHeights_[i]; ++y) {
          if (y<heightGreen_) {
            drawBarRow(i, y, 0, 200, 0);
          }
          else if (y<heightYellow_) {
            drawBarRow(i, y, 150, 150, 0);
          }
          else if (y<heightOrange_) {
            drawBarRow(i, y, 250, 100, 0);
          }
          else {
            drawBarRow(i, y, 200, 0, 0);
          }
        }
        // Anything above the bar should be black
        for (; y<height_; ++y) {
          drawBarRow(i, y, 0, 0, 0);
        }
      }
      usleep(delay_ms_ * 1000);
    }
  }

private:
  void drawBarRow(int bar, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t x=bar*barWidth_; x<(bar+1)*barWidth_; ++x) {
      canvas()->SetPixel(x, height_-1-y, r, g, b);
    }
  }

  int delay_ms_;
  int numBars_;
  int* barHeights_;
  int barWidth_;
  int height_;
  int heightGreen_;
  int heightYellow_;
  int heightOrange_;
  int heightRed_;
  int* barFreqs_;
  int* barMeans_;
  int t_;
};

/// Genetic Colors
/// A genetic algorithm to evolve colors
/// by bbhsu2 + anonymous
class GeneticColors : public ThreadedCanvasManipulator {
public:
  GeneticColors(Canvas *m, int delay_ms = 200)
    : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
    width_ = canvas()->width();
    height_ = canvas()->height();
    popSize_ = width_ * height_;

    // Allocate memory
    children_ = new citizen[popSize_];
    parents_ = new citizen[popSize_];
    srand(time(NULL));
  }

  ~GeneticColors() {
    delete [] children_;
    delete [] parents_;
  }

  static int rnd (int i) { return rand() % i; }

  void Run() {
    // Set a random target_
    target_ = rand() & 0xFFFFFF;

    // Create the first generation of random children_
    for (int i = 0; i < popSize_; ++i) {
      children_[i].dna = rand() & 0xFFFFFF;
    }

    while(running()) {
      swap();
      sort();
      mate();
      std::random_shuffle (children_, children_ + popSize_, rnd);

      // Draw citizens to canvas
      for(int i=0; i < popSize_; i++) {
        int c = children_[i].dna;
        int x = i % width_;
        int y = (int)(i / width_);
        canvas()->SetPixel(x, y, R(c), G(c), B(c));
      }

      // When we reach the 85% fitness threshold...
      if(is85PercentFit()) {
        // ...set a new random target_
        target_ = rand() & 0xFFFFFF;

        // Randomly mutate everyone for sake of new colors
        for (int i = 0; i < popSize_; ++i) {
          mutate(children_[i]);
        }
      }
      usleep(delay_ms_ * 1000);
    }
  }

private:
  /// citizen will hold dna information, a 24-bit color value.
  struct citizen {
    citizen() { }

    citizen(int chrom)
      : dna(chrom) {
    }

    int dna;
  };

  /// for sorting by fitness
  class comparer {
  public:
    comparer(int t)
      : target_(t) { }

    inline bool operator() (const citizen& c1, const citizen& c2) {
      return (calcFitness(c1.dna, target_) < calcFitness(c2.dna, target_));
    }

  private:
    const int target_;
  };

  static int R(const int cit) { return at(cit, 16); }
  static int G(const int cit) { return at(cit, 8); }
  static int B(const int cit) { return at(cit, 0); }
  static int at(const int v, const  int offset) { return (v >> offset) & 0xFF; }

  /// fitness here is how "similar" the color is to the target
  static int calcFitness(const int value, const int target) {
    // Count the number of differing bits
    int diffBits = 0;
    for (unsigned int diff = value ^ target; diff; diff &= diff - 1) {
      ++diffBits;
    }
    return diffBits;
  }

  /// sort by fitness so the most fit citizens are at the top of parents_
  /// this is to establish an elite population of greatest fitness
  /// the most fit members and some others are allowed to reproduce
  /// to the next generation
  void sort() {
    std::sort(parents_, parents_ + popSize_, comparer(target_));
  }

  /// let the elites continue to the next generation children
  /// randomly select 2 parents of (near)elite fitness and determine
  /// how they will mate. after mating, randomly mutate citizens
  void mate() {
    // Adjust these for fun and profit
    const float eliteRate = 0.30f;
    const float mutationRate = 0.20f;

    const int numElite = popSize_ * eliteRate;
    for (int i = 0; i < numElite; ++i) {
      children_[i] = parents_[i];
    }

    for (int i = numElite; i < popSize_; ++i) {
      //select the parents randomly
      const float sexuallyActive = 1.0 - eliteRate;
      const int p1 = rand() % (int)(popSize_ * sexuallyActive);
      const int p2 = rand() % (int)(popSize_ * sexuallyActive);
      const int matingMask = (~0) << (rand() % bitsPerPixel);

      // Make a baby
      int baby = (parents_[p1].dna & matingMask)
        | (parents_[p2].dna & ~matingMask);
      children_[i].dna = baby;

      // Mutate randomly based on mutation rate
      if ((rand() / (float)RAND_MAX) < mutationRate) {
        mutate(children_[i]);
      }
    }
  }

  /// parents make children,
  /// children become parents,
  /// and they make children...
  void swap() {
    citizen* temp = parents_;
    parents_ = children_;
    children_ = temp;
  }

  void mutate(citizen& c) {
    // Flip a random bit
    c.dna ^= 1 << (rand() % bitsPerPixel);
  }

  /// can adjust this threshold to make transition to new target seamless
  bool is85PercentFit() {
    int numFit = 0;
    for (int i = 0; i < popSize_; ++i) {
        if (calcFitness(children_[i].dna, target_) < 1) {
            ++numFit;
        }
    }
    return ((numFit / (float)popSize_) > 0.85f);
  }

  static const int bitsPerPixel = 24;
  int popSize_;
  int width_, height_;
  int delay_ms_;
  int target_;
  citizen* children_;
  citizen* parents_;
};

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options> -D <demo-nr> [optional parameter]\n",
          progname);
  fprintf(stderr, "Options:\n"
          "\t-r <rows>     : Panel rows. '16' for 16x32 (1:8 multiplexing),\n"
	  "\t                '32' for 32x32 (1:16), '8' for 1:4 multiplexing; "
          "Default: 32\n"
          "\t-P <parallel> : For Plus-models or RPi2: parallel chains. 1..3. "
          "Default: 1\n"
          "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
          "\t-L            : 'Large' display, composed out of 4 times 32x32\n"
          "\t-p <pwm-bits> : Bits used for PWM. Something between 1..11\n"
          "\t-l            : Don't do luminance correction (CIE1931)\n"
          "\t-D <demo-nr>  : Always needs to be set\n"
          "\t-d            : run as daemon. Use this when starting in\n"
          "\t                /etc/init.d, but also when running without\n"
          "\t                terminal (e.g. cron).\n"
          "\t-t <seconds>  : Run for these number of seconds, then exit.\n"
          "\t                (if neither -d nor -t are supplied, waits for <RETURN>)\n"
          "\t-b <brightnes>: Sets brightness percent. Default: 100.\n"
          "\t-R <rotation> : Sets the rotation of matrix. Allowed: 0, 90, 180, 270. Default: 0.\n");
  fprintf(stderr, "Demos, choosen with -D\n");
  fprintf(stderr, "\t0  - some rotating square\n"
          "\t1  - forward scrolling an image (-m <scroll-ms>)\n"
          "\t2  - backward scrolling an image (-m <scroll-ms>)\n"
          "\t3  - test image: a square\n"
          "\t4  - Pulsing color\n"
          "\t5  - Grayscale Block\n"
          "\t6  - Abelian sandpile model (-m <time-step-ms>)\n"
          "\t7  - Conway's game of life (-m <time-step-ms>)\n"
          "\t8  - Langton's ant (-m <time-step-ms>)\n"
          "\t9  - Volume bars (-m <time-step-ms>)\n"
          "\t10 - Evolution of color (-m <time-step-ms>)\n"
          "\t11 - Brightness pulse generator\n");
  fprintf(stderr, "Example:\n\t%s -t 10 -D 1 runtext.ppm\n"
          "Scrolls the runtext for 10 seconds\n", progname);
  return 1;
}

int main(int argc, char *argv[]) {
  GPIO io;
  bool as_daemon = false;
  int runtime_seconds = -1;
  int demo = -1;
  int rows = 32;
  int chain = 1;
  int parallel = 1;
  int scroll_ms = 30;
  int pwm_bits = -1;
  int brightness = 100;
  int rotation = 0;
  bool large_display = false;
  bool do_luminance_correct = true;

  const char *demo_parameter = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "dlD:t:r:P:c:p:b:m:LR:")) != -1) {
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

    case 'P':
      parallel = atoi(optarg);
      break;

    case 'c':
      chain = atoi(optarg);
      break;

    case 'm':
      scroll_ms = atoi(optarg);
      break;

    case 'p':
      pwm_bits = atoi(optarg);
      break;

    case 'b':
      brightness = atoi(optarg);
      break;

    case 'l':
      do_luminance_correct = !do_luminance_correct;
      break;

    case 'L':
      // The 'large' display assumes a chain of four displays with 32x32
      chain = 4;
      rows = 32;
      large_display = true;
      break;

    case 'R':
      rotation = atoi(optarg);
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

  if (rows != 8 && rows != 16 && rows != 32) {
    fprintf(stderr, "Rows can one of 8, 16 or 32 "
            "for 1:4, 1:8 and 1:16 multiplexing respectively.\n");
    return 1;
  }

  if (chain < 1) {
    fprintf(stderr, "Chain outside usable range\n");
    return 1;
  }
  if (chain > 8) {
    fprintf(stderr, "That is a long chain. Expect some flicker.\n");
  }
  if (parallel < 1 || parallel > 3) {
    fprintf(stderr, "Parallel outside usable range.\n");
    return 1;
  }

  if (brightness < 1 || brightness > 100) {
    fprintf(stderr, "Brightness is outside usable range.\n");
    return 1;
  }

  if (rotation % 90 != 0) {
    fprintf(stderr, "Rotation %d not allowed! Only 0, 90, 180 and 270 are possible.\n", rotation);
    return 1;
  }

  // Initialize GPIO pins. This might fail when we don't have permissions.
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
  RGBMatrix *matrix = new RGBMatrix(&io, rows, chain, parallel);
  matrix->set_luminance_correct(do_luminance_correct);
  matrix->SetBrightness(brightness);
  if (pwm_bits >= 0 && !matrix->SetPWMBits(pwm_bits)) {
    fprintf(stderr, "Invalid range of pwm-bits\n");
    return 1;
  }

  LinkedTransformer *transformer = new LinkedTransformer();
  matrix->SetTransformer(transformer);

  if (large_display) {
    // Mapping the coordinates of a 32x128 display mapped to a square of 64x64
    transformer->AddTransformer(new LargeSquare64x64Transformer());
  }

  if (rotation > 0) {
    transformer->AddTransformer(new RotateTransformer(rotation));
  }

  Canvas *canvas = matrix;

  // The ThreadedCanvasManipulator objects are filling
  // the matrix continuously.
  ThreadedCanvasManipulator *image_gen = NULL;
  switch (demo) {
  case 0:
    image_gen = new RotatingBlockGenerator(canvas);
    break;

  case 1:
  case 2:
    if (demo_parameter) {
      ImageScroller *scroller = new ImageScroller(matrix,
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
    image_gen = new SimpleSquare(canvas);
    break;

  case 4:
    image_gen = new ColorPulseGenerator(matrix);
    break;

  case 5:
    image_gen = new GrayScaleBlock(canvas);
    break;

  case 6:
    image_gen = new Sandpile(canvas, scroll_ms);
    break;

  case 7:
    image_gen = new GameLife(canvas, scroll_ms);
    break;

  case 8:
    image_gen = new Ant(canvas, scroll_ms);
    break;

  case 9:
    image_gen = new VolumeBars(canvas, scroll_ms, canvas->width()/2);
    break;

  case 10:
    image_gen = new GeneticColors(canvas, scroll_ms);
    break;

  case 11:
    image_gen = new BrightnessPulseGenerator(matrix);
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
  delete canvas;

  return 0;
}
