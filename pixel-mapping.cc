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

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;
using namespace rgb_matrix;

int main(int argc, char *argv[]) {
  /*
   * Set up GPIO pins. This fails when not running as root.
   */
  GPIO io;
  if (!io.Init())
    return 1;

  /*
   * Set up the RGBMatrix. It implements a 'Canvas' interface.
   */
  int rows = 8;    // A 32x32 display. Use 16 when this is a 16x32 display.
  int chain = 16;    // Number of boards chained together.
  int parallel = 1; // Number of chains in parallel (1..3). > 1 for plus or Pi2
  RGBMatrix *matrix = new RGBMatrix(&io, rows, chain, parallel);
  Scrambled32x16Transformer *transformer = new Scrambled32x16Transformer();
  matrix->SetTransformer(transformer);

  printf("matrix dimen :  %d x %d\n", matrix->width(), matrix->height() );
  for (int j=0; j<matrix->height(); j++){
    for (int i=0; i<matrix->width(); i++){
      matrix->SetPixel(i,j,10,10,10);
      usleep(10000);
    }
  }

  while(1);

  // Animation finished. Shut down the RGB matrix.
  matrix->Clear();

  delete transformer;
  delete matrix;

  return 0;
}
