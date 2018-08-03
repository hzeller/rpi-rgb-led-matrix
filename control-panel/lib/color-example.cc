// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static void DrawOnCanvas(Canvas *canvas) {
  /*
   * Let's create a simple animation. We use the canvas to draw
   * pixels. We wait between each step to have a slower animation.
   */
  canvas->Fill(0, 3, 0);

  for (float a = 0; a < 100000; a++) {
    if (interrupt_received)
      return;

/*

    int red = rand() % 256;
    int green = rand() % 256;
    int blue = rand() % 256;*/

    int bow = rand() % 5;

      int red = 0;
      int green = 0;
      int blue = 0;

    if(bow == 0){
      red = 0;
      green = 10;
      blue = 0;
    } else if(bow == 1) {
      red = 0;
      green = 0;
      blue = 10;
    }  else if(bow == 2) {
      red = 10;
      green = 0;
      blue = 0;
    }   else if(bow == 3) {
      red = 10;
      green = 10;
      blue = 0;
    } 

    int x = rand() % 64;
    int y = rand() % 16;
    canvas->SetPixel(x, y,
                     red, green, blue);
    usleep(1 * 100);  // wait a little to slow down things.
  }
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 16;
  defaults.chain_length = 2;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;
  Canvas *canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL)
    return 1;

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  DrawOnCanvas(canvas);    // Using the canvas.

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
