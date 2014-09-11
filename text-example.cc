// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how write text.

#include "led-matrix.h"
#include "graphics.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace rgb_matrix;

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <path/to/bdf-font>\n", progname);
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return usage(argv[0]);
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  const char *bdf_font = argv[1];
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font);
    return usage(argv[0]);
  }
  printf("Font height: %d; baseline: %d\n", font.height(), font.baseline());

  /*
   * Set up GPIO pins. This fails when not running as root.
   */
  GPIO io;
  if (!io.Init())
    return 1;
    
  /*
   * Set up the RGBMatrix. It implements a 'Canvas' interface.
   */
  int rows = 32;   // A 32x32 display. Use 16 when this is a 16x32 display.
  int chain = 1;   // Number of boards chained together.
  Canvas *canvas = new RGBMatrix(&io, rows, chain);

  const int x = 0;                 // x position we start to write from
  int y = 0;                       // y position is advanced for each line.
  const Color color(255, 255, 0);  // yellow.

  printf("Enter lines. Supports UTF-8. CTRL-D for exit.\n");
  char line[1024];
  while (fgets(line, sizeof(line), stdin)) {
    if (y + font.height() > canvas->height()) {  // screen full. Make new.
      canvas->Clear();
      y = 0;
    }
    const size_t last = strlen(line);
    if (last > 0) line[last - 1] = '\0';  // remove newline.
    rgb_matrix::DrawText(canvas, font, x, y + font.baseline(), color, line);
    y += font.height();
  }

  // Finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
