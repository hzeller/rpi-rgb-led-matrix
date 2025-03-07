// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2023 Hendrik
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#ifdef ENABLE_EMULATOR

#include "matrix-factory.h"
#include "graphics.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ostream>
#include <iostream>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

// Demo that draws a circle bouncing around the screen.
// Shows how the MatrixFactory can be used to create either a real 
// LED matrix or an emulator depending on command line flags.
int main(int argc, char *argv[]) {
  // Setup signal handlers
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // Set up matrix factory options
  MatrixFactory::Options options;
  
  // Set default options
  options.led_options.cols = 64;
  options.led_options.rows = 64;
  options.led_options.chain_length = 2;
  options.led_options.parallel = 2;
  options.led_options.hardware_mapping = "regular";  // Add this line to ensure hardware mapping is set
  options.emulator_options.display_scale = 5;  // Larger window for emulator
  options.emulator_options.window_title = "LED Matrix Emulator Demo";

  // Parse options from command line
  if (!MatrixFactory::ParseOptionsFromFlags(&argc, &argv, &options)) {
    // Show usage and exit
    fprintf(stderr, "Usage: %s [options]\n", argv[0]);
    fprintf(stderr, "Options:\n");
    MatrixFactory::PrintMatrixFactoryFlags(stderr, options);
    return 1;
  }

  // Create matrix from options (either real or emulated)
  RGBMatrixBase *matrix = MatrixFactory::CreateMatrix(options);
  if (matrix == NULL) {
    fprintf(stderr, "Failed to create matrix\n");
    return 1;
  }

  // Get canvas dimensions
  const int width = matrix->width();
  const int height = matrix->height();
  
  std::cout << "Canvas width: " << width << ", height: " << height << std::endl << std::flush;
  
  // Create a canvas for double-buffering
  FrameCanvas *offscreen = matrix->CreateFrameCanvas();
  if (offscreen == NULL) {
    fprintf(stderr, "Failed to create offscreen canvas\n");
    delete matrix;
    return 1;
  }

  // Setup drawing parameters
  float radius = height / 4;
  float center_x = width / 2;
  float center_y = height / 2;
  float velocity_x = 0.5;  // pixels per frame
  float velocity_y = 0.2;  // pixels per frame
  
  // Animation state variables
  float x = center_x;
  float y = center_y;
  uint8_t r = 255, g = 0, b = 0;

  printf("Press Ctrl+C to exit\n");
  
  // Main animation loop
  while (!interrupt_received) {
    // Clear the canvas and draw a circle
    offscreen->Fill(0, 0, 0);
    
    // Draw the bouncy circle
    DrawCircle(offscreen, x, y, radius, Color(r, g, b));
    
    // Update position
    x += velocity_x;
    y += velocity_y;
    
    // Bounce off edges with a little elasticity
    if (x - radius <= 0) {
      x = radius;
      velocity_x = -velocity_x * 0.9;
      
      // Change color on bounce
      r = rand() % 256;
      g = rand() % 256;
      b = rand() % 256;
    }
    if (x + radius >= width - 1) {
      x = width - 1 - radius;
      velocity_x = -velocity_x * 0.9;
      
      // Change color on bounce
      r = rand() % 256;
      g = rand() % 256;
      b = rand() % 256;
    }
    if (y - radius <= 0) {
      y = radius;
      velocity_y = -velocity_y * 0.9;
      
      // Change color on bounce
      r = rand() % 256;
      g = rand() % 256;
      b = rand() % 256;
    }
    if (y + radius >= height - 1) {
      y = height - 1 - radius;
      velocity_y = -velocity_y * 0.9;
      
      // Change color on bounce
      r = rand() % 256;
      g = rand() % 256;
      b = rand() % 256;
    }

    // Swap buffers
    offscreen = matrix->SwapOnVSync(offscreen);
    
    // Sleep for a bit to control animation speed
    usleep(16000);  // ~60 Hz refresh rate
  }

  // Cleanup
  matrix->Clear();
  delete matrix;
  
  printf("\nAnimation stopped. Exiting.\n");
  return 0;
}

#else
// Compiled without emulator support
#include <stdio.h>

int main(int argc, char *argv[]) {
  fprintf(stderr, "This demo requires ENABLE_EMULATOR to be defined.\n");
  fprintf(stderr, "Please compile with 'make ENABLE_EMULATOR=1'\n");
  return 1;
}
#endif // ENABLE_EMULATOR