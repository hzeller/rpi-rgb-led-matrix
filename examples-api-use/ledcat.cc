// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// A program that reads frames form STDIN as RGB24, much like
// https://github.com/polyfloyd/ledcat does.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define FPS 60

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular"; // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  Canvas *canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL) {
    return 1;
  }

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  ssize_t frame_size = canvas->width() * canvas->height() * 3;
  uint8_t buf[frame_size];

  while (1) {
    struct timespec start;
    timespec_get(&start, TIME_UTC);

    ssize_t nread;
    ssize_t total_nread = 0;
    while ((nread = read(STDIN_FILENO, &buf[total_nread], frame_size - total_nread)) > 0) {
      if (interrupt_received) {
        return 1;
      }
      total_nread += nread;
    }
    if (total_nread < frame_size){
      break;
    }

    for (int y = 0; y < canvas->height(); y++) {
      for (int x = 0; x < canvas->width(); x++) {
        uint8_t *p = &buf[(y * canvas->width() + x) * 3];
        uint8_t r = *(p+0), g = *(p+1), b = *(p+2);
        canvas->SetPixel(x, y, r, g, b);
      }
    }

    struct timespec end;
    timespec_get(&end, TIME_UTC);
    long tudiff = (end.tv_nsec / 1000 + end.tv_sec * 1000000) - (start.tv_nsec / 1000 + start.tv_sec * 1000000);
    if (tudiff < 1000000l / FPS) {
      usleep(1000000l / FPS - tudiff);
    }
  }

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;
  return 0;
}
