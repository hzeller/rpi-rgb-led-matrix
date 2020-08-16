// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the input bits
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"

#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <deque>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static void InteractiveUseMessage() {
  fprintf(stderr,
          "Move around with common movement keysets \n"
          " W,A,S,D (gaming move style) or\n"
          " H,J,K,L (vi console move style)\n"
          " Quit with 'q' or <ESC>\n"
          "The pixel position cannot be less than 0 or greater than the "
          "display height and width.\n");
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Display single pixel with any colour.\n");
  InteractiveUseMessage();
  fprintf(stderr, "Options:\n\n");
  fprintf(stderr,
          "\t-C <r,g,b>                : Color at front of trail. Default 255,255,0\n"
          "\t-t <trail-len>            : Length of trail behind dot (default:0)\n"
          "\t-c <r,g,b>                : Color at end of trail. Default 0,0,255\n\n"
          );
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static bool parseColor(Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

static char getch() {
  static bool is_terminal = isatty(STDIN_FILENO);

  struct termios old;
  if (is_terminal) {
    if (tcgetattr(0, &old) < 0)
      perror("tcsetattr()");

    // Set to unbuffered mode
    struct termios no_echo = old;
    no_echo.c_lflag &= ~ICANON;
    no_echo.c_lflag &= ~ECHO;
    no_echo.c_cc[VMIN] = 1;
    no_echo.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &no_echo) < 0)
      perror("tcsetattr ICANON");
  }

  char buf = 0;
  if (read(STDIN_FILENO, &buf, 1) < 0)
    perror ("read()");

  if (is_terminal) {
    // Back to original terminal settings.
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
      perror ("tcsetattr ~ICANON");
  }

  return buf;
}

// Interpolation of color between head and tail of trail.
static uint8_t quantize(float c) {
  return c < 0 ? 0 : c > 255 ? 255 : roundf(c);
}
static Color interpolate(const Color &c1, const Color &c2, float fraction) {
  float c2_fraction = 1 - fraction;
  return { quantize(c1.r * fraction + c2.r * c2_fraction),
           quantize(c1.g * fraction + c2.g * c2_fraction),
           quantize(c1.b * fraction + c2.b * c2_fraction)};
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  Color front_color(255, 255, 0);
  Color back_color(0, 0, 255);

  int trail_len = 0;
  int opt;
  while ((opt = getopt(argc, argv, "C:c:t:")) != -1) {
    switch (opt) {
    case 't':
      trail_len = std::max(0, atoi(optarg));
      break;
    case 'C':
      if (!parseColor(&front_color, optarg)) {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'c':
      if (!parseColor(&back_color, optarg)) {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    default:
      return usage(argv[0]);
    }
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL)
    return usage(argv[0]);

  rgb_matrix::FrameCanvas *canvas = matrix->CreateFrameCanvas();
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  std::deque<std::pair<int, int>> trail;
  int x_pos = 0;
  int y_pos = 0;
  trail.push_back({x_pos, y_pos});

  InteractiveUseMessage();
  const bool output_is_terminal = isatty(STDOUT_FILENO);

  bool running = true;
  while (!interrupt_received && running) {
    canvas->Clear();
    int distance_from_head = trail.size();
    for (const auto &pos : trail) {   // Draw from tail -> head
      distance_from_head--;
      Color c = interpolate(front_color, back_color,
                            1.0 - 1.0f * distance_from_head / trail.size());
      canvas->SetPixel(pos.first, pos.second, c.r, c.g, c.b);
    }
    canvas = matrix->SwapOnVSync(canvas);

    printf("%sX,Y = %d,%d%s",
           output_is_terminal ? "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" : "",
           x_pos, y_pos,
           output_is_terminal ? " " : "\n");
    fflush(stdout);

    const char c = tolower(getch());
    switch (c) {
    case 'w': case 'k':   // Up
      if (y_pos > 0) {
        y_pos--;
        trail.push_back({x_pos, y_pos});
      }
      break;
    case 's': case 'j':  // Down
      if (y_pos < canvas->height() - 1) {
        y_pos++;
        trail.push_back({x_pos, y_pos});
      }
      break;
    case 'a': case 'h':  // Left
      if (x_pos > 0) {
        x_pos--;
        trail.push_back({x_pos, y_pos});
      }
      break;
    case 'd': case 'l':  // Right
      if (x_pos < canvas->width() - 1) {
        x_pos++;
        trail.push_back({x_pos, y_pos});
      }
      break;
      // All kinds of conditions which we use to exit
    case 0x1B:           // Escape
    case 'q':            // 'Q'uit
    case 0x04:           // End of file
    case 0x00:           // Other issue from getch()
      running = false;
      break;
    }

    while ((int)trail.size() > trail_len + 1)
      trail.pop_front();   // items on front are the oldest dots
  }

  // Finished. Shut down the RGB matrix.
  delete matrix;
  printf("\n");
  return 0;
}
