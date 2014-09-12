// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how write text.

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace rgb_matrix;

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Reads text from stdin and displays it. "
          "Empty string: clear screen\n");
  fprintf(stderr, "Options:\n"
          "\t-f <font-file>: Use given font.\n"
          "\t-r <rows>     : Display rows. 16 for 16x32, 32 for 32x32. "
          "Default: 32\n"
          "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
          "\t-x <x-origin> : X-Origin of displaying text (Default: 0)\n"
          "\t-y <y-origin> : Y-Origin of displaying text (Default: 0)\n"
          "\t-C <r,g,b>    : Color. Default 255,255,0\n");
  return 1;
}

static bool parseColor(Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

int main(int argc, char *argv[]) {
  Color color(255, 255, 0);
  const char *bdf_font_file = NULL;
  int rows = 32;
  int chain = 1;
  int x_orig = 0;
  int y_orig = -1;

  int opt;
  while ((opt = getopt(argc, argv, "r:c:x:y:")) != -1) {
    switch (opt) {
    case 'r': rows = atoi(optarg); break;
    case 'c': chain = atoi(optarg); break;
    case 'x': x_orig = atoi(optarg); break;
    case 'y': y_orig = atoi(optarg); break;
    case 'f': bdf_font_file = strdup(optarg); break;
    case 'C':
      if (!parseColor(&color, optarg)) {
        fprintf(stderr, "Invalid color spec.\n");
        return usage(argv[0]);
      }
      break;
    }
  }

  if (bdf_font_file == NULL) {
    fprintf(stderr, "Need to specify BDF font-file with -f\n");
    return usage(argv[0]);
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return usage(argv[0]);
  }

  /*
   * Set up GPIO pins. This fails when not running as root.
   */
  GPIO io;
  if (!io.Init())
    return 1;
    
  /*
   * Set up the RGBMatrix. It implements a 'Canvas' interface.
   */
  Canvas *canvas = new RGBMatrix(&io, rows, chain);

  const int x = x_orig;
  int y = y_orig;

  if (isatty(STDIN_FILENO)) {
    // Only give a message if we are interactive. If connected via pipe, be quiet
    printf("Enter lines. Full screen or empty line clears screen.\n"
           "Supports UTF-8. CTRL-D for exit.\n");
  }

  char line[1024];
  while (fgets(line, sizeof(line), stdin)) {
    const size_t last = strlen(line);
    if (last > 0) line[last - 1] = '\0';  // remove newline.
    bool line_empty = strlen(line) == 0;
    if ((y + font.height() > canvas->height()) || line_empty) {
      canvas->Clear();
      y = y_orig;
    }
    if (line_empty)
      continue;
    rgb_matrix::DrawText(canvas, font, x, y + font.baseline(), color, line);
    y += font.height();
  }

  // Finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
