// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the input bits
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Display single pixel with any colour and brightness.\n"
          "Move around with W,A,S,D keys.\n"
          "The pixel position cannot be less than 0 or greater than the display height and width.\n");
  fprintf(stderr, "Options:\n\n");
  fprintf(stderr,
          "\t-b <brightness>           : Sets brightness percent. Default: 100.\n"
          "\t-C <r,g,b>                : Color. Default 255,255,0\n\n"
          );
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static bool parseColor(Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

static bool FullSaturation(const Color &c) {
  return (c.r == 0 || c.r == 255)
    && (c.g == 0 || c.g == 255)
    && (c.b == 0 || c.b == 255);
}

static char getch() {
  struct termios old;
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

  char buf = 0;
  if (read(0, &buf, 1) < 0)
    perror ("read()");

  // Back to original terminal settings.
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror ("tcsetattr ~ICANON");

  return buf;
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  Color color(255, 255, 0);
  int brightness = 100;

  int opt;
  while ((opt = getopt(argc, argv, "C:b:")) != -1) {
    switch (opt) {
    case 'b': brightness = atoi(optarg); break;
    case 'C':
      if (!parseColor(&color, optarg)) {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    default:
      return usage(argv[0]);
    }
  }
  if (brightness < 1 || brightness > 100) {
    fprintf(stderr, "Brightness is outside usable range.\n");
    return 1;
  }

  RGBMatrix *canvas = rgb_matrix::CreateMatrixFromOptions(matrix_options,
                                                          runtime_opt);
  if (canvas == NULL)
    return 1;

  canvas->SetBrightness(brightness);

  const bool all_extreme_colors = (brightness == 100)
    && FullSaturation(color);
  if (all_extreme_colors)
    canvas->SetPWMBits(1);

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  int x_pos=0;
  int y_pos=0;

  fprintf(stderr, "Move around with W,A,S,D keys.\n"
          "The pixel position cannot be less than 0 or greater than "
          "the display height or width.\n");

  while (!interrupt_received) {
    canvas->Clear();
    canvas->SetPixel(x_pos,y_pos, color.r, color.g, color.b);
    fprintf(stderr, "X,Y Position : %d,%d\n",x_pos,y_pos);   //display the pixel position
    switch(getch()) {
    case 'w':
      if (y_pos > 0)
        y_pos--;
      break;
    case 's':
      if (y_pos < canvas->height() - 1)
        y_pos++;
      break;
    case 'a':
      if (x_pos > 0)
        x_pos--;
      break;
    case 'd':
      if (x_pos < canvas->width() - 1)
        x_pos++;
      break;
    }
  }

  // Finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
