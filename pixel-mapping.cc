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
  Canvas *canvas = new RGBMatrix(&io, rows, chain, parallel);

  int width=256, height=16;
  int map[256][16];
/*
  for (int i=0; i<256*16; i++){
    map[i%256][i/256] = i%8 + (2*(i/8)+1-(i%2048)/1024)*8%2048 + (i/2048)*2048;
  }
*/
  for (int i=0; i<width*height; i++){
    map[i%width][i/width] = i%8 + (2*(i/8)+1-(i%(8*width))/(4*width))*8%(8*width) + (i/(8*width))*(8*width);
  }

  printf("map table :\n");
  for (int i=0; i<256; i++){
    for(int j=0; j<16; j++){
      printf("%d\t",map[i][j]);
    }
    printf("\n");
  }

  for (int j=0; j<16; j++){
    for (int i=0; i<256; i++){
      int idx = map[i][j];
      canvas->SetPixel(idx%512,idx/512, 10, 10, 10);
      usleep(10000);
    }
  }

  while(1);

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
