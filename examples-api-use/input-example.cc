// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the input bits
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  RGBMatrix *matrix = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
  if (matrix == NULL)
    return 1;

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // Let's request all input bits and see which are actually available.
  // This will differ depending on which hardware mapping you use and how
  // many parallel chains you have.
  const uint64_t available_inputs = matrix->RequestInputs(0xffffffff);
  fprintf(stderr, "Available GPIO-bits: ");
  for (int b = 0; b < 32; ++b) {
      if (available_inputs & (1<<b))
          fprintf(stderr, "%d ", b);
  }
  fprintf(stderr, "\n");

  while (!interrupt_received) {
      // Block and wait until any input bit changed or 100ms passed
      uint32_t inputs = matrix->AwaitInputChange(100);

      // Minimal output: let's show the bits with LEDs in the first row
      for (int b = 0; b < 32; ++b) {
          uint8_t col = (inputs & (1<<b)) ? 255 : 0;
          matrix->SetPixel(32-b, 0, col, col, col);
      }
  }

  fprintf(stderr, "Exiting.\n");
  matrix->Clear();
  delete matrix;

  return 0;
}
