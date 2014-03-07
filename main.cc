#include "thread.h"
#include "led-matrix.h"

#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Base-class for a Thread that does something with a matrix.
class RGBMatrixManipulator : public Thread {
public:
  RGBMatrixManipulator(RGBMatrix *m) : running_(true), matrix_(m) {}
  virtual ~RGBMatrixManipulator() { running_ = false; }

  // Run() implementation needs to check running_ regularly.

protected:
  volatile bool running_;  // TODO: use mutex, but this is good enough for now.
  RGBMatrix *const matrix_;
};

// Pump pixels to screen. Needs to be high priority real-time because jitter
// here will make the PWM uneven.
class DisplayUpdater : public RGBMatrixManipulator {
public:
  DisplayUpdater(RGBMatrix *m) : RGBMatrixManipulator(m) {}

  void Run() {
    while (running_) {
      matrix_->UpdateScreen();
    }
  }
};

// -- The following are demo image generators.

// Simple generator that goes through RGB for all pixel.
class ColorPulseGenerator : public RGBMatrixManipulator {
public:
  ColorPulseGenerator(RGBMatrix *m) : RGBMatrixManipulator(m) {}
  void Run() {
    const int columns = matrix_->columns();
    while (running_) {
      for (int x = 0; x < columns; ++x)
        for (int y = 0; y < 32; ++y)
          matrix_->SetPixel(x, y, 255, 0, 0);
      usleep(500000);
      for (int x = 0; x < columns; ++x)
        for (int y = 0; y < 32; ++y)
          matrix_->SetPixel(x, y, 0, 255, 0);
      usleep(500000);
      for (int x = 0; x < columns; ++x)
        for (int y = 0; y < columns; ++y)
          matrix_->SetPixel(x, y, 0, 0, 255);
      usleep(500000);
    }
  }
};

// Simple class that simulates rain on the screen.
class RotatingBlockGenerator : public RGBMatrixManipulator {
public:
  RotatingBlockGenerator(RGBMatrix *m) : RGBMatrixManipulator(m) {}
  void Run() {
	int start[65][2], weiter[65][2], geschw[65];
	srand(time(NULL));
	matrix_->FillScreen(0, 0, 0);
	for (int i = 0; i < 65; i++) {
      start[i][0] = rand() % 32;
	  start[i][1] = rand() % 32;
	  weiter[i][0] = start[i][0];
	  weiter[i][1] = start[i][1];
	  matrix_->SetPixel(start[i][0], start[i][1], 0, 0, 255);
	  geschw[i] = 1;
	}
	while (running_) {
	  matrix_->FillScreen(0, 0, 0);
	  for (int i = 0; i < 65; i++) {
	    if ((start[i][1] - 1) >= 31) {
		  start[i][0] = rand() % 32;
		  start[i][1] = 0;
		  weiter[i][0] = start[i][0];
		  weiter[i][1] = start[i][1];
		  matrix_->SetPixel(start[i][0], start[i][1], 0, 0, 255);
		}
		start[i][1] = weiter[i][1];
		weiter[i][1] = start[i][1] + geschw[i];
		matrix_->SetPixel(start[i][0], start[i][1] - 1, 0, 0, 25);
		matrix_->SetPixel(start[i][0], start[i][1], 0, 0, 80);
		matrix_->SetPixel(weiter[i][0], weiter[i][1], 0, 0, 255);
	  }
	  usleep(50000);
	}
  }
};

class ImageScroller : public RGBMatrixManipulator {
public:
  ImageScroller(RGBMatrix *m)
    : RGBMatrixManipulator(m), image_(NULL), horizontal_position_(0) {
  }

  // _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
  // Not really robust. Use at your own risk :)
  bool LoadPPM(const char *filename) {
    if (image_) {
      delete [] image_;
      image_ = NULL;
    }
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;
    char header_buf[256];
    const char *line = ReadLine(f, header_buf, sizeof(header_buf));
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
      fclose(f); return false; }
    if (sscanf(line, "P6 ") == EOF)
      EXIT_WITH_MSG("Can only handle P6 as PPM type.");
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d %d ", &width_, &height_) != 2)
      EXIT_WITH_MSG("Width/height expected");
    int value;
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
      EXIT_WITH_MSG("Only 255 for maxval allowed.");
    const size_t pixel_count = width_ * height_;
    image_ = new Pixel [ pixel_count ];
    assert(sizeof(Pixel) == 3);   // we make that assumption.
    if (fread(image_, sizeof(Pixel), pixel_count, f) != pixel_count) {
      line = "";
      EXIT_WITH_MSG("Not enough pixels read.");
    }
#undef EXIT_WITH_MSG
    fclose(f);
    fprintf(stderr, "Read image with %dx%d\n", width_, height_);
    horizontal_position_ = 0;
    return true;
  }

  void Run() {
    const int columns = matrix_->columns();
    while (running_) {
      if (image_ == NULL) {
        usleep(100 * 1000);
        continue;
      }
      usleep(30 * 1000);
      for (int x = 0; x < columns; ++x) {
        for (int y = 0; y < 32; ++y) {
          const Pixel &p = getPixel((horizontal_position_ + x) % width_, y);
          // Display upside down on my desk. Lets flip :)
          int disp_x = columns - x;
          int disp_y = 31 - y;
          matrix_->SetPixel(disp_x, disp_y, p.red, p.green, p.blue);
        }
      }
      ++horizontal_position_;
    }
  }

private:
  struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };

  // Read line, skip comments.
  char *ReadLine(FILE *f, char *buffer, size_t len) {
    char *result;
    do {
      result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#');
    return result;
  }

  const Pixel &getPixel(int x, int y) {
    static Pixel dummy;
    if (x < 0 || x > width_ || y < 0 || y > height_) return dummy;
    return image_[x + width_ * y];
  }

  int width_;
  int height_;
  Pixel *image_;
  uint32_t horizontal_position_;
};

int main(int argc, char *argv[]) {
  int demo = 0;
  if (argc > 1) {
    demo = atoi(argv[1]);
  }
  fprintf(stderr, "Using demo %d\n", demo);

  GPIO io;
  if (!io.Init())
    return 1;

  RGBMatrix m(&io);
    
  RGBMatrixManipulator *image_gen = NULL;
  switch (demo) {
  case 0:
    image_gen = new RotatingBlockGenerator(&m);
    break;

  case 1:
    if (argc > 2) {
      ImageScroller *scroller = new ImageScroller(&m);
      if (!scroller->LoadPPM(argv[2]))
        return 1;
      image_gen = scroller;
    } else {
      fprintf(stderr, "Demo %d Requires PPM image as parameter", demo);
      return 1;
    }
    break;

  default:
    image_gen = new ColorPulseGenerator(&m);
    break;
  }

  if (image_gen == NULL)
    return 1;

  RGBMatrixManipulator *updater = new DisplayUpdater(&m);
  updater->Start(10);  // high priority

  image_gen->Start();

  // Things are set up. Just wait for <RETURN> to be pressed.
  printf("Press <RETURN> to exit and reset LEDs\n");
  getchar();

  // Stopping threads and wait for them to join.
  delete image_gen;
  delete updater;

  // Final thing before exit: clear screen and update once, so that
  // we don't have random pixels burn
  m.ClearScreen();
  m.UpdateScreen();

  return 0;
}
