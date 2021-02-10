// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// This code is public domain
// (but note, once linked against the led-matrix library, this is
// covered by the GPL v2)
//
// This is a grab-bag of various demos and not very readable.
#include "led-matrix.h"

#include "pixel-mapper.h"
#include "graphics.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

using std::max;
using std::min;

#define TERM_ERR "\033[1;31m"
#define TERM_NORM "\033[0m"

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
  interrupt_received = true;
}

class DemoRunner
{
protected:
  DemoRunner(Canvas *canvas) : canvas_(canvas) {}
  inline Canvas *canvas() { return canvas_; }

public:
  virtual ~DemoRunner() {}
  virtual void Run() = 0;

private:
  Canvas *const canvas_;
};

/*
 * The following are demo image generators. They all use the utility
 * class DemoRunner to generate new frames.
 */

// Simple generator that pulses through RGB and White.
class ColorPulseGenerator : public DemoRunner
{
public:
  ColorPulseGenerator(RGBMatrix *m) : DemoRunner(m), matrix_(m)
  {
    off_screen_canvas_ = m->CreateFrameCanvas();
  }
  void Run() override
  {
    uint32_t continuum = 0;
    while (!interrupt_received)
    {
      usleep(5 * 1000);
      continuum += 1;
      continuum %= 3 * 255;
      int r = 0, g = 0, b = 0;
      if (continuum <= 255)
      {
        int c = continuum;
        b = 255 - c;
        r = c;
      }
      else if (continuum > 255 && continuum <= 511)
      {
        int c = continuum - 256;
        r = 255 - c;
        g = c;
      }
      else
      {
        int c = continuum - 512;
        g = 255 - c;
        b = c;
      }
      off_screen_canvas_->Fill(r, g, b);
      off_screen_canvas_ = matrix_->SwapOnVSync(off_screen_canvas_);
    }
  }

private:
  RGBMatrix *const matrix_;
  FrameCanvas *off_screen_canvas_;
};

// Simple generator that pulses through brightness on red, green, blue and white
class BrightnessPulseGenerator : public DemoRunner
{
public:
  BrightnessPulseGenerator(RGBMatrix *m)
      : DemoRunner(m), matrix_(m) {}
  void Run() override
  {
    const uint8_t max_brightness = matrix_->brightness();
    const uint8_t c = 255;
    uint8_t count = 0;

    while (!interrupt_received)
    {
      if (matrix_->brightness() < 1)
      {
        matrix_->SetBrightness(max_brightness);
        count++;
      }
      else
      {
        matrix_->SetBrightness(matrix_->brightness() - 1);
      }

      switch (count % 4)
      {
      case 0:
        matrix_->Fill(c, 0, 0);
        break;
      case 1:
        matrix_->Fill(0, c, 0);
        break;
      case 2:
        matrix_->Fill(0, 0, c);
        break;
      case 3:
        matrix_->Fill(c, c, c);
        break;
      }

      usleep(20 * 1000);
    }
  }

private:
  RGBMatrix *const matrix_;
};

class SimpleSquare : public DemoRunner
{
public:
  SimpleSquare(Canvas *m) : DemoRunner(m) {}
  void Run() override
  {
    const int width = canvas()->width() - 1;
    const int height = canvas()->height() - 1;
    // Borders
    DrawLine(canvas(), 0, 0, width, 0, Color(255, 0, 0));
    DrawLine(canvas(), 0, height, width, height, Color(255, 255, 0));
    DrawLine(canvas(), 0, 0, 0, height, Color(0, 0, 255));
    DrawLine(canvas(), width, 0, width, height, Color(0, 255, 0));

    // Diagonals.
    DrawLine(canvas(), 0, 0, width, height, Color(255, 255, 255));
    DrawLine(canvas(), 0, height, width, 0, Color(255, 0, 255));
  }
};

class GrayScaleBlock : public DemoRunner
{
public:
  GrayScaleBlock(Canvas *m) : DemoRunner(m) {}
  void Run() override
  {
    const int sub_blocks = 16;
    const int width = canvas()->width();
    const int height = canvas()->height();
    const int x_step = max(1, width / sub_blocks);
    const int y_step = max(1, height / sub_blocks);
    uint8_t count = 0;
    while (!interrupt_received)
    {
      for (int y = 0; y < height; ++y)
      {
        for (int x = 0; x < width; ++x)
        {
          int c = sub_blocks * (y / y_step) + x / x_step;
          switch (count % 4)
          {
          case 0:
            canvas()->SetPixel(x, y, c, c, c);
            break;
          case 1:
            canvas()->SetPixel(x, y, c, 0, 0);
            break;
          case 2:
            canvas()->SetPixel(x, y, 0, c, 0);
            break;
          case 3:
            canvas()->SetPixel(x, y, 0, 0, c);
            break;
          }
        }
      }
      count++;
      sleep(2);
    }
  }
};

// 회전하는 블럭을 스크린에 생성하는 클라스 입니다.
class RotatingBlockGenerator : public DemoRunner
{
public:
  // 해당 클라스의 생성자는 부모 클라스의 생성자 규칙을 따릅니다.
  // 자세한건 DemoRunner() 클라스를 참조하세요!
  RotatingBlockGenerator(Canvas *m) : DemoRunner(m) {}

  uint8_t scale_col(int val, int lo, int hi)
  {
    if (val < lo)
      return 0;
    if (val > hi)
      return 255;
    return 255 * (val - lo) / (hi - lo);
  }

  // 실행 메서드
  void Run() override
  {
    // 일단 주어진 캔버스의 사이즈 값 (높이랑 너비 값)을 2로 나누어서 중앙 좌표를 구합니다.
    const int cent_x = canvas()->width() / 2;
    const int cent_y = canvas()->height() / 2;

    // 데모 시연 시에 회전을 실행할 최대/최소 범위입니다.
    // 이 데모의 경우, 사각형이 그냥 회전만 하는 게 아니라, 돌면서 막 이동하거든요 ㅎㅎ
    // 이 때 x,y축 으로 각각 어디에서 어디까지 이동할지 그냥 설정하는 겁니다.
    const int rotate_square = min(canvas()->width(), canvas()->height()) * 1.41;
    const int min_rotate = cent_x - rotate_square / 2;
    const int max_rotate = cent_x + rotate_square / 2;

    // The square to display is within the visible area.
    // 데모 시연시에 디스플레이 할 최대/최소 범위를 설정합니다.
    const int display_square = min(canvas()->width(), canvas()->height()) * 0.7;
    const int min_display = cent_x - display_square / 2;
    const int max_display = cent_x + display_square / 2;

    // 회전에 쓰일 단위를 설정합니다.
    // 라디안을 쓸거기 때문에 (1 라디안 = 2*원주율/360도) 공식을 정의해줍니다.
    const float deg_to_rad = 2 * 3.14159265 / 360;
    int rotation = 0;
    // Interrupt가 발생하지 않는 이상은 계속 while문 속의 코드를 반복합니다.
    // 지금 쯤이면 인터럽트는 다 아실거라고 믿음..!
    while (!interrupt_received)
    {
      ++rotation;
      // usleep(microseconds)는 10^(-6) 초 단위로 잠시 코드 실행을 멈추는 함수 입니다.
      // 보통 신호 처리에서 전파 속도에 따른 답장을 받아올때 어느정도 지연시간 필요한 경우 사용되지요.
      usleep(15 * 1000);
      rotation %= 360;
      for (int x = min_rotate; x < max_rotate; ++x)
      {
        for (int y = min_rotate; y < max_rotate; ++y)
        {
          float rot_x, rot_y;
          // 회전을 한번 진행 할떄 그 다음 진행해야 할 새로운 x, y값을 rot_x, rot_y에 저장합니다.
          // 밑에 private 에 Rotate 메서드를 참고하세용
          Rotate(x - cent_x, y - cent_x,
                 deg_to_rad * rotation, &rot_x, &rot_y);
          if (x >= min_display && x < max_display &&
              y >= min_display && y < max_display)
          { // within display square
            // SetPixel() 메서드를 이용해서 실제로 LED에 표현합니다.
            canvas()->SetPixel(rot_x + cent_x, rot_y + cent_y,
                               scale_col(x, min_display, max_display),
                               255 - scale_col(y, min_display, max_display),
                               scale_col(y, min_display, max_display));
          }
          else
          {
            // 배경 (검정 프레임)도 그려줍니다.
            canvas()->SetPixel(rot_x + cent_x, rot_y + cent_y, 0, 0, 0);
          }
        }
      }
    }
  }

private:
  void Rotate(int x, int y, float angle,
              float *new_x, float *new_y)
  {
    *new_x = x * cosf(angle) - y * sinf(angle);
    *new_y = x * sinf(angle) + y * cosf(angle);
  }
};

// 오늘은 이미지 스클로러를 한번 분석해 보겠습니다.
class ImageScroller : public DemoRunner
{
public:
  // 매 scroll_ms 밀리세컨드마다 scroll_jumps 픽셀 수 만큼 이미지를 스크롤합니다.
  // 예) ImageScroller(RGBMatrix *m, int scroll_jumps = 4, int scroll_ms = 30)
  //     => 30 밀리세컨드마다 4 픽셀 씩 이미지를 스크롤합니다.
  // scroll_jumps 값이 음수라면, 스크롤링이 진행되지 않습니다.
  ImageScroller(RGBMatrix *m, int scroll_jumps, int scroll_ms = 30)
      : DemoRunner(m), scroll_jumps_(scroll_jumps),
        scroll_ms_(scroll_ms),
        horizontal_position_(0),
        matrix_(m)
  {
    offscreen_ = matrix_->CreateFrameCanvas();
  }
  // 이진 P6 PPM 파일만을 읽을 수 있는 메서드 입니다.
  // 개발자의 말을 빌리자면 아직 완벽한 메서드는 아닌듯 하며 사용시 문제가 발생할 수 있답니다.
  // 이 메서드는 주로 이미지를 라이브 업데이트 하는 용도로 사용됩니다.
  bool LoadPPM(const char *filename)
  {
    // 파일 주소을 저장하는 포인터입니다.
    FILE *f = fopen(filename, "r");
    // 파일 존재 유무, 파일 권한(access from unistd) 확인
    if (f == NULL && access(filename, F_OK) == -1)
    {
      // printf 를 쓸 경우, 출력 버퍼에 저장되어 있다가 출력하므로 오류 메시지를 출력하지 못할 수 도 있습니다.
      // fprintf(stderr, ...) 를 쓸 경우 버퍼 없이 바로 출력 됩니다.
      fprintf(stderr, "File \"%s\" doesn't exist\n", filename);
      return false;
    }
    if (f == NULL)
      return false;
    char header_buf[256];
    const char *line = ReadLine(f, header_buf, sizeof(header_buf));
    // 오류 상황에 맞는 line을 출력하고 종료합니다.
#define EXIT_WITH_MSG(m)                              \
  {                                                   \
    fprintf(stderr, "%s: %s |%s", filename, m, line); \
    fclose(f);                                        \
    return false;                                     \
  }
    // 이부분을 이해하기 위해서는 sscanf 에 대해서 알아봅시다.
    // 만약 P6 포맷의 PPM 파일이 아니라면
    if (sscanf(line, "P6 ") == EOF)
      EXIT_WITH_MSG("Can only handle P6 as PPM type.");

    // 필드 높이, 너비 둘 중 하나라도 저장이 안된다면 값이 2가 안되겠죠?
    line = ReadLine(f, header_buf, sizeof(header_buf));
    int new_width, new_height;
    if (!line || sscanf(line, "%d %d ", &new_width, &new_height) != 2)
      EXIT_WITH_MSG("Width/height expected");

    // value 에 제대로 픽셀이 할당되었는지 확인, 그리고 그 (최대)값이 255
    int value;
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
      EXIT_WITH_MSG("Only 255 for maxval allowed.");

    const size_t pixel_count = new_width * new_height;
    Pixel *new_image = new Pixel[pixel_count];
    // assert(bool ok)은 Ok가 거짓일 경우에 프로그램을 종료합니다.
    assert(sizeof(Pixel) == 3); // we make that assumption.
    // fread는 결과적으로 읽어들인 원소의 갯수를 반환합니다. 의도대로 읽어들이지 못하면 다음 에러가 발생하겠죠?
    if (fread(new_image, sizeof(Pixel), pixel_count, f) != pixel_count)
    {
      line = "";
      EXIT_WITH_MSG("Not enough pixels read.");
    }
#undef EXIT_WITH_MSG
    fclose(f);
    fprintf(stderr, "Read image '%s' with %dx%d\n", filename,
            new_width, new_height);
    horizontal_position_ = 0;
    MutexLock l(&mutex_new_image_);
    new_image_.Delete(); // in case we reload faster than is picked up
    new_image_.image = new_image;
    new_image_.width = new_width;
    new_image_.height = new_height;
    return true;
  }

  void Run() override
  {
    const int screen_height = offscreen_->height();
    const int screen_width = offscreen_->width();
    while (!interrupt_received)
    {
      {
        MutexLock l(&mutex_new_image_);
        if (new_image_.IsValid())
        {
          current_image_.Delete();
          current_image_ = new_image_;
          new_image_.Reset();
        }
      }
      if (!current_image_.IsValid())
      {
        usleep(100 * 1000);
        continue;
      }
      for (int x = 0; x < screen_width; ++x)
      {
        for (int y = 0; y < screen_height; ++y)
        {
          const Pixel &p = current_image_.getPixel(
              (horizontal_position_ + x) % current_image_.width, y);
          offscreen_->SetPixel(x, y, p.red, p.green, p.blue);
        }
      }
      offscreen_ = matrix_->SwapOnVSync(offscreen_);
      horizontal_position_ += scroll_jumps_;
      if (horizontal_position_ < 0)
        horizontal_position_ = current_image_.width;
      if (scroll_ms_ <= 0)
      {
        // No scrolling. We don't need the image anymore.
        current_image_.Delete();
      }
      else
      {
        usleep(scroll_ms_ * 1000);
      }
    }
  }

private:
  struct Pixel
  {
    Pixel() : red(0), green(0), blue(0) {}
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };

  struct Image
  {
    Image() : width(-1), height(-1), image(NULL) {}
    ~Image() { Delete(); }
    void Delete()
    {
      delete[] image;
      Reset();
    }
    void Reset()
    {
      image = NULL;
      width = -1;
      height = -1;
    }
    inline bool IsValid() { return image && height > 0 && width > 0; }
    const Pixel &getPixel(int x, int y)
    {
      static Pixel black;
      if (x < 0 || x >= width || y < 0 || y >= height)
        return black;
      return image[x + width * y];
    }

    int width;
    int height;
    Pixel *image;
  };

  // Read line, skip comments.
  // result 문자열 속에 있는 글을 읽습니다. 주석은 제외 합니다.
  char *ReadLine(FILE *f, char *buffer, size_t len)
  {
    char *result;
    do
    {
      result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#'); // 주석 "#" 제외
    return result;
  }

  const int scroll_jumps_;
  const int scroll_ms_;

  // Current image is only manipulated in our thread.
  Image current_image_;

  // New image can be loaded from another thread, then taken over in main thread
  Mutex mutex_new_image_;
  Image new_image_;

  int32_t horizontal_position_;

  RGBMatrix *matrix_;
  FrameCanvas *offscreen_;
};

// Abelian sandpile
// Contributed by: Vliedel
class Sandpile : public DemoRunner
{
public:
  Sandpile(Canvas *m, int delay_ms = 50)
      : DemoRunner(m), delay_ms_(delay_ms)
  {
    width_ = canvas()->width() - 1;   // We need an odd width
    height_ = canvas()->height() - 1; // We need an odd height

    // Allocate memory
    values_ = new int *[width_];
    for (int x = 0; x < width_; ++x)
    {
      values_[x] = new int[height_];
    }
    newValues_ = new int *[width_];
    for (int x = 0; x < width_; ++x)
    {
      newValues_[x] = new int[height_];
    }

    // Init values
    srand(time(NULL));
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        values_[x][y] = 0;
      }
    }
  }

  ~Sandpile()
  {
    for (int x = 0; x < width_; ++x)
    {
      delete[] values_[x];
    }
    delete[] values_;
    for (int x = 0; x < width_; ++x)
    {
      delete[] newValues_[x];
    }
    delete[] newValues_;
  }

  void Run() override
  {
    while (!interrupt_received)
    {
      // Drop a sand grain in the centre
      values_[width_ / 2][height_ / 2]++;
      updateValues();

      for (int x = 0; x < width_; ++x)
      {
        for (int y = 0; y < height_; ++y)
        {
          switch (values_[x][y])
          {
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
  void updateValues()
  {
    // Copy values to newValues
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        newValues_[x][y] = values_[x][y];
      }
    }

    // Update newValues based on values
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        if (values_[x][y] > 3)
        {
          // Collapse
          if (x > 0)
            newValues_[x - 1][y]++;
          if (x < width_ - 1)
            newValues_[x + 1][y]++;
          if (y > 0)
            newValues_[x][y - 1]++;
          if (y < height_ - 1)
            newValues_[x][y + 1]++;
          newValues_[x][y] -= 4;
        }
      }
    }
    // Copy newValues to values
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        values_[x][y] = newValues_[x][y];
      }
    }
  }

  int width_;
  int height_;
  int **values_;
  int **newValues_;
  int delay_ms_;
};

// Conway's game of life
// Contributed by: Vliedel
class GameLife : public DemoRunner
{
public:
  GameLife(Canvas *m, int delay_ms = 500, bool torus = true)
      : DemoRunner(m), delay_ms_(delay_ms), torus_(torus)
  {
    width_ = canvas()->width();
    height_ = canvas()->height();

    // Allocate memory
    values_ = new int *[width_];
    for (int x = 0; x < width_; ++x)
    {
      values_[x] = new int[height_];
    }
    newValues_ = new int *[width_];
    for (int x = 0; x < width_; ++x)
    {
      newValues_[x] = new int[height_];
    }

    // Init values randomly
    srand(time(NULL));
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        values_[x][y] = rand() % 2;
      }
    }
    r_ = rand() % 255;
    g_ = rand() % 255;
    b_ = rand() % 255;

    if (r_ < 150 && g_ < 150 && b_ < 150)
    {
      int c = rand() % 3;
      switch (c)
      {
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

  ~GameLife()
  {
    for (int x = 0; x < width_; ++x)
    {
      delete[] values_[x];
    }
    delete[] values_;
    for (int x = 0; x < width_; ++x)
    {
      delete[] newValues_[x];
    }
    delete[] newValues_;
  }

  void Run() override
  {
    while (!interrupt_received)
    {

      updateValues();

      for (int x = 0; x < width_; ++x)
      {
        for (int y = 0; y < height_; ++y)
        {
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
  int numAliveNeighbours(int x, int y)
  {
    int num = 0;
    if (torus_)
    {
      // Edges are connected (torus)
      num += values_[(x - 1 + width_) % width_][(y - 1 + height_) % height_];
      num += values_[(x - 1 + width_) % width_][y];
      num += values_[(x - 1 + width_) % width_][(y + 1) % height_];
      num += values_[(x + 1) % width_][(y - 1 + height_) % height_];
      num += values_[(x + 1) % width_][y];
      num += values_[(x + 1) % width_][(y + 1) % height_];
      num += values_[x][(y - 1 + height_) % height_];
      num += values_[x][(y + 1) % height_];
    }
    else
    {
      // Edges are not connected (no torus)
      if (x > 0)
      {
        if (y > 0)
          num += values_[x - 1][y - 1];
        if (y < height_ - 1)
          num += values_[x - 1][y + 1];
        num += values_[x - 1][y];
      }
      if (x < width_ - 1)
      {
        if (y > 0)
          num += values_[x + 1][y - 1];
        if (y < 31)
          num += values_[x + 1][y + 1];
        num += values_[x + 1][y];
      }
      if (y > 0)
        num += values_[x][y - 1];
      if (y < height_ - 1)
        num += values_[x][y + 1];
    }
    return num;
  }

  void updateValues()
  {
    // Copy values to newValues
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        newValues_[x][y] = values_[x][y];
      }
    }
    // update newValues based on values
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        int num = numAliveNeighbours(x, y);
        if (values_[x][y])
        {
          // cell is alive
          if (num < 2 || num > 3)
            newValues_[x][y] = 0;
        }
        else
        {
          // cell is dead
          if (num == 3)
            newValues_[x][y] = 1;
        }
      }
    }
    // copy newValues to values
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        values_[x][y] = newValues_[x][y];
      }
    }
  }

  int **values_;
  int **newValues_;
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
class Ant : public DemoRunner
{
public:
  Ant(Canvas *m, int delay_ms = 500)
      : DemoRunner(m), delay_ms_(delay_ms)
  {
    numColors_ = 4;
    width_ = canvas()->width();
    height_ = canvas()->height();
    values_ = new int *[width_];
    for (int x = 0; x < width_; ++x)
    {
      values_[x] = new int[height_];
    }
  }

  ~Ant()
  {
    for (int x = 0; x < width_; ++x)
    {
      delete[] values_[x];
    }
    delete[] values_;
  }

  void Run() override
  {
    antX_ = width_ / 2;
    antY_ = height_ / 2 - 3;
    antDir_ = 0;
    for (int x = 0; x < width_; ++x)
    {
      for (int y = 0; y < height_; ++y)
      {
        values_[x][y] = 0;
        updatePixel(x, y);
      }
    }

    while (!interrupt_received)
    {
      // LLRR
      switch (values_[antX_][antY_])
      {
      case 0:
      case 1:
        antDir_ = (antDir_ + 1 + 4) % 4;
        break;
      case 2:
      case 3:
        antDir_ = (antDir_ - 1 + 4) % 4;
        break;
      }

      values_[antX_][antY_] = (values_[antX_][antY_] + 1) % numColors_;
      int oldX = antX_;
      int oldY = antY_;
      switch (antDir_)
      {
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
  void updatePixel(int x, int y)
  {
    switch (values_[x][y])
    {
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
  int **values_;
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
class VolumeBars : public DemoRunner
{
public:
  VolumeBars(Canvas *m, int delay_ms = 50, int numBars = 8)
      : DemoRunner(m), delay_ms_(delay_ms),
        numBars_(numBars), t_(0)
  {
  }

  ~VolumeBars()
  {
    delete[] barHeights_;
    delete[] barFreqs_;
    delete[] barMeans_;
  }

  void Run() override
  {
    const int width = canvas()->width();
    height_ = canvas()->height();
    barWidth_ = width / numBars_;
    barHeights_ = new int[numBars_];
    barMeans_ = new int[numBars_];
    barFreqs_ = new int[numBars_];
    heightGreen_ = height_ * 4 / 12;
    heightYellow_ = height_ * 8 / 12;
    heightOrange_ = height_ * 10 / 12;
    heightRed_ = height_ * 12 / 12;

    // Array of possible bar means
    int numMeans = 10;
    int means[10] = {1, 2, 3, 4, 5, 6, 7, 8, 16, 32};
    for (int i = 0; i < numMeans; ++i)
    {
      means[i] = height_ - means[i] * height_ / 8;
    }
    // Initialize bar means randomly
    srand(time(NULL));
    for (int i = 0; i < numBars_; ++i)
    {
      barMeans_[i] = rand() % numMeans;
      barFreqs_[i] = 1 << (rand() % 3);
    }

    // Start the loop
    while (!interrupt_received)
    {
      if (t_ % 8 == 0)
      {
        // Change the means
        for (int i = 0; i < numBars_; ++i)
        {
          barMeans_[i] += rand() % 3 - 1;
          if (barMeans_[i] >= numMeans)
            barMeans_[i] = numMeans - 1;
          if (barMeans_[i] < 0)
            barMeans_[i] = 0;
        }
      }

      // Update bar heights
      t_++;
      for (int i = 0; i < numBars_; ++i)
      {
        barHeights_[i] = (height_ - means[barMeans_[i]]) * sin(0.1 * t_ * barFreqs_[i]) + means[barMeans_[i]];
        if (barHeights_[i] < height_ / 8)
          barHeights_[i] = rand() % (height_ / 8) + 1;
      }

      for (int i = 0; i < numBars_; ++i)
      {
        int y;
        for (y = 0; y < barHeights_[i]; ++y)
        {
          if (y < heightGreen_)
          {
            drawBarRow(i, y, 0, 200, 0);
          }
          else if (y < heightYellow_)
          {
            drawBarRow(i, y, 150, 150, 0);
          }
          else if (y < heightOrange_)
          {
            drawBarRow(i, y, 250, 100, 0);
          }
          else
          {
            drawBarRow(i, y, 200, 0, 0);
          }
        }
        // Anything above the bar should be black
        for (; y < height_; ++y)
        {
          drawBarRow(i, y, 0, 0, 0);
        }
      }
      usleep(delay_ms_ * 1000);
    }
  }

private:
  void drawBarRow(int bar, int y, uint8_t r, uint8_t g, uint8_t b)
  {
    for (int x = bar * barWidth_; x < (bar + 1) * barWidth_; ++x)
    {
      canvas()->SetPixel(x, height_ - 1 - y, r, g, b);
    }
  }

  int delay_ms_;
  int numBars_;
  int *barHeights_;
  int barWidth_;
  int height_;
  int heightGreen_;
  int heightYellow_;
  int heightOrange_;
  int heightRed_;
  int *barFreqs_;
  int *barMeans_;
  int t_;
};

/// Genetic Colors
/// A genetic algorithm to evolve colors
/// by bbhsu2 + anonymous
class GeneticColors : public DemoRunner
{
public:
  GeneticColors(Canvas *m, int delay_ms = 200)
      : DemoRunner(m), delay_ms_(delay_ms)
  {
    width_ = canvas()->width();
    height_ = canvas()->height();
    popSize_ = width_ * height_;

    // Allocate memory
    children_ = new citizen[popSize_];
    parents_ = new citizen[popSize_];
    srand(time(NULL));
  }

  ~GeneticColors()
  {
    delete[] children_;
    delete[] parents_;
  }

  static int rnd(int i) { return rand() % i; }

  void Run() override
  {
    // Set a random target_
    target_ = rand() & 0xFFFFFF;

    // Create the first generation of random children_
    for (int i = 0; i < popSize_; ++i)
    {
      children_[i].dna = rand() & 0xFFFFFF;
    }

    while (!interrupt_received)
    {
      swap();
      sort();
      mate();
      std::random_shuffle(children_, children_ + popSize_, rnd);

      // Draw citizens to canvas
      for (int i = 0; i < popSize_; i++)
      {
        int c = children_[i].dna;
        int x = i % width_;
        int y = (int)(i / width_);
        canvas()->SetPixel(x, y, R(c), G(c), B(c));
      }

      // When we reach the 85% fitness threshold...
      if (is85PercentFit())
      {
        // ...set a new random target_
        target_ = rand() & 0xFFFFFF;

        // Randomly mutate everyone for sake of new colors
        for (int i = 0; i < popSize_; ++i)
        {
          mutate(children_[i]);
        }
      }
      usleep(delay_ms_ * 1000);
    }
  }

private:
  /// citizen will hold dna information, a 24-bit color value.
  struct citizen
  {
    citizen() {}

    citizen(int chrom)
        : dna(chrom)
    {
    }

    int dna;
  };

  /// for sorting by fitness
  class comparer
  {
  public:
    comparer(int t)
        : target_(t) {}

    inline bool operator()(const citizen &c1, const citizen &c2)
    {
      return (calcFitness(c1.dna, target_) < calcFitness(c2.dna, target_));
    }

  private:
    const int target_;
  };

  static int R(const int cit) { return at(cit, 16); }
  static int G(const int cit) { return at(cit, 8); }
  static int B(const int cit) { return at(cit, 0); }
  static int at(const int v, const int offset) { return (v >> offset) & 0xFF; }

  /// fitness here is how "similar" the color is to the target
  static int calcFitness(const int value, const int target)
  {
    // Count the number of differing bits
    int diffBits = 0;
    for (unsigned int diff = value ^ target; diff; diff &= diff - 1)
    {
      ++diffBits;
    }
    return diffBits;
  }

  /// sort by fitness so the most fit citizens are at the top of parents_
  /// this is to establish an elite population of greatest fitness
  /// the most fit members and some others are allowed to reproduce
  /// to the next generation
  void sort()
  {
    std::sort(parents_, parents_ + popSize_, comparer(target_));
  }

  /// let the elites continue to the next generation children
  /// randomly select 2 parents of (near)elite fitness and determine
  /// how they will mate. after mating, randomly mutate citizens
  void mate()
  {
    // Adjust these for fun and profit
    const float eliteRate = 0.30f;
    const float mutationRate = 0.20f;

    const int numElite = popSize_ * eliteRate;
    for (int i = 0; i < numElite; ++i)
    {
      children_[i] = parents_[i];
    }

    for (int i = numElite; i < popSize_; ++i)
    {
      //select the parents randomly
      const float sexuallyActive = 1.0 - eliteRate;
      const int p1 = rand() % (int)(popSize_ * sexuallyActive);
      const int p2 = rand() % (int)(popSize_ * sexuallyActive);
      const unsigned matingMask = (~0u) << (rand() % bitsPerPixel);

      // Make a baby
      unsigned baby = (parents_[p1].dna & matingMask) | (parents_[p2].dna & ~matingMask);
      children_[i].dna = baby;

      // Mutate randomly based on mutation rate
      if ((rand() / (float)RAND_MAX) < mutationRate)
      {
        mutate(children_[i]);
      }
    }
  }

  /// parents make children,
  /// children become parents,
  /// and they make children...
  void swap()
  {
    citizen *temp = parents_;
    parents_ = children_;
    children_ = temp;
  }

  void mutate(citizen &c)
  {
    // Flip a random bit
    c.dna ^= 1 << (rand() % bitsPerPixel);
  }

  /// can adjust this threshold to make transition to new target seamless
  bool is85PercentFit()
  {
    int numFit = 0;
    for (int i = 0; i < popSize_; ++i)
    {
      if (calcFitness(children_[i].dna, target_) < 1)
      {
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
  citizen *children_;
  citizen *parents_;
};

static int usage(const char *progname)
{
  fprintf(stderr, "usage: %s <options> -D <demo-nr> [optional parameter]\n",
          progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "\t-D <demo-nr>              : Always needs to be set\n");

  rgb_matrix::PrintMatrixFlags(stderr);

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
  fprintf(stderr, "Example:\n\t%s -D 1 runtext.ppm\n"
                  "Scrolls the runtext until Ctrl-C is pressed\n",
          progname);
  return 1;
}

int main(int argc, char *argv[])
{
  int demo = -1;
  int scroll_ms = 30;

  const char *demo_parameter = NULL;
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;

  // These are the defaults when no command-line flags are given.
  matrix_options.rows = 32;
  matrix_options.chain_length = 1;
  matrix_options.parallel = 1;

  // First things first: extract the command line flags that contain
  // relevant matrix options.
  if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt))
  {
    return usage(argv[0]);
  }

  int opt;
  while ((opt = getopt(argc, argv, "dD:r:P:c:p:b:m:LR:")) != -1)
  {
    switch (opt)
    {
    case 'D':
      demo = atoi(optarg);
      break;

    case 'm':
      scroll_ms = atoi(optarg);
      break;

    default: /* '?' */
      return usage(argv[0]);
    }
  }

  if (optind < argc)
  {
    demo_parameter = argv[optind];
  }

  if (demo < 0)
  {
    fprintf(stderr, TERM_ERR "Expected required option -D <demo>\n" TERM_NORM);
    return usage(argv[0]);
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL)
    return 1;

  printf("Size: %dx%d. Hardware gpio mapping: %s\n",
         matrix->width(), matrix->height(), matrix_options.hardware_mapping);

  Canvas *canvas = matrix;

  // The DemoRunner objects are filling
  // the matrix continuously.
  DemoRunner *demo_runner = NULL;
  switch (demo)
  {
  case 0:
    demo_runner = new RotatingBlockGenerator(canvas);
    break;

  case 1:
  case 2:
    if (demo_parameter)
    {
      ImageScroller *scroller = new ImageScroller(matrix,
                                                  demo == 1 ? 1 : -1,
                                                  scroll_ms);
      if (!scroller->LoadPPM(demo_parameter))
        return 1;
      demo_runner = scroller;
    }
    else
    {
      fprintf(stderr, "Demo %d Requires PPM image as parameter\n", demo);
      return 1;
    }
    break;

  case 3:
    demo_runner = new SimpleSquare(canvas);
    break;

  case 4:
    demo_runner = new ColorPulseGenerator(matrix);
    break;

  case 5:
    demo_runner = new GrayScaleBlock(canvas);
    break;

  case 6:
    demo_runner = new Sandpile(canvas, scroll_ms);
    break;

  case 7:
    demo_runner = new GameLife(canvas, scroll_ms);
    break;

  case 8:
    demo_runner = new Ant(canvas, scroll_ms);
    break;

  case 9:
    demo_runner = new VolumeBars(canvas, scroll_ms, canvas->width() / 2);
    break;

  case 10:
    demo_runner = new GeneticColors(canvas, scroll_ms);
    break;

  case 11:
    demo_runner = new BrightnessPulseGenerator(matrix);
    break;
  }

  if (demo_runner == NULL)
    return usage(argv[0]);

  // Set up an interrupt handler to be able to stop animations while they go
  // on. Each demo tests for while (!interrupt_received) {},
  // so they exit as soon as they get a signal.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  printf("Press <CTRL-C> to exit and reset LEDs\n");

  // Now, run our particular demo; it will exit when it sees interrupt_received.
  demo_runner->Run();

  delete demo_runner;
  delete canvas;

  printf("Received CTRL-C. Exiting.\n");
  return 0;
}
