// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how write text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

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
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "\t-f <font-file>    : Use given font.\n"
          "\t-x <x-origin>     : X-Origin of displaying text (Default: 0)\n"
          "\t-y <y-origin>     : Y-Origin of displaying text (Default: 0)\n"
          "\t-S <spacing>      : Spacing pixels between letters (Default: 0)\n"
          "\t-C <r,g,b>        : Color. Default 255,255,0\n"
          "\t-B <r,g,b>        : Font Background-Color. Default 0,0,0\n"
          "\t-O <r,g,b>        : Outline-Color, e.g. to increase contrast.\n"
          "\t-F <r,g,b>        : Panel flooding-background color. Default 0,0,0\n"
          "\n"
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

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  Color color(255, 255, 0);
  Color bg_color(0, 0, 0);
  Color flood_color(0, 0, 0);
  Color outline_color(0,0,0);
  bool with_outline = false;

  const char *bdf_font_file = NULL;
  int x_orig = 0;
  int y_orig = 0;
  int letter_spacing = 0;

  int opt;
  while ((opt = getopt(argc, argv, "x:y:f:C:B:O:S:F:")) != -1) {
    switch (opt) {
    case 'x': x_orig = atoi(optarg); break;
    case 'y': y_orig = atoi(optarg); break;
    case 'f': bdf_font_file = strdup(optarg); break;
    case 'S': letter_spacing = atoi(optarg); break;
    case 'C':
      if (!parseColor(&color, optarg)) {
        fprintf(stderr, "Invalid color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'B':
      if (!parseColor(&bg_color, optarg)) {
        fprintf(stderr, "Invalid background color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'O':
      if (!parseColor(&outline_color, optarg)) {
        fprintf(stderr, "Invalid outline color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      with_outline = true;
      break;
    case 'F':
      if (!parseColor(&flood_color, optarg)) {
        fprintf(stderr, "Invalid background color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    default:
      return usage(argv[0]);
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
    return 1;
  }

  /*
   * If we want an outline around the font, we create a new font with
   * the original font as a template that is just an outline font.
   */
  rgb_matrix::Font *outline_font = NULL;
  if (with_outline) {
    outline_font = font.CreateOutlineFont();
  }

  RGBMatrix *canvas = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (canvas == NULL)
    return 1;

  const bool all_extreme_colors = (matrix_options.brightness == 100)
    && FullSaturation(color)
    && FullSaturation(bg_color)
    && FullSaturation(outline_color);
  if (all_extreme_colors)
    canvas->SetPWMBits(1);

  const int x = x_orig;
  int y = y_orig;

  if (isatty(STDIN_FILENO)) {
    // Only give a message if we are interactive. If connected via pipe, be quiet
    printf("Enter lines. Full screen or empty line clears screen.\n"
           "Supports UTF-8. CTRL-D for exit.\n");
  }

  canvas->Fill(flood_color.r, flood_color.g, flood_color.b);
  char line[1024];
  while (fgets(line, sizeof(line), stdin)) {
    const size_t last = strlen(line);
    if (last > 0) line[last - 1] = '\0';  // remove newline.
    bool line_empty = strlen(line) == 0;
    if ((y + font.height() > canvas->height()) || line_empty) {
      canvas->Fill(flood_color.r, flood_color.g, flood_color.b);
      y = y_orig;
    }
    if (line_empty)
      continue;
    if (outline_font) {
      // The outline font, we need to write with a negative (-2) text-spacing,
      // as we want to have the same letter pitch as the regular text that
      // we then write on top.
      rgb_matrix::DrawText(canvas, *outline_font,
                           x - 1, y + font.baseline(),
                           outline_color, &bg_color, line, letter_spacing - 2);
    }
    // The regular text. Unless we already have filled the background with
    // the outline font, we also fill the background here.
    rgb_matrix::DrawText(canvas, font, x, y + font.baseline(),
                         color, outline_font ? NULL : &bg_color, line,
                         letter_spacing);
    y += font.height();
  }

  // Finished. Shut down the RGB matrix.
  delete canvas;

  return 0;
}
