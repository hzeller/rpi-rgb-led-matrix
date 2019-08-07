// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Example of a clock. This is very similar to the text-example,
// except that it shows the time :)
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"

#include <err.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

const char* kBigFont = "../fonts/8x13.bdf";
const char* kSmallFont = "../fonts/5x7.bdf";
char tz_NYC[] = "TZ=America/New_York";
char tz_SFO[] = "TZ=America/Los_Angeles";
char tz_UTC[] = "TZ=UTC";
char tz_LON[] = "TZ=Europe/London";
char tz_PAR[] = "TZ=Europe/Paris";
char tz_ATH[] = "TZ=Europe/Athens";
char tz_SIN[] = "TZ=Asia/Singapore";

struct SmallClock {
  char* tz;
  const char* label;
  Color color;
  int x;
  int y;
  
};

SmallClock small_clocks[] = {
  {tz_UTC, "UTC", Color(255, 255, 255),  0, 16},
  {tz_SFO, "SFO", Color(255, 128, 128), 32, 16},
  {tz_LON, "LON", Color(  0, 255, 255),  0, 24},
  {tz_ATH, "ATH", Color( 64,  64, 255), 32, 24},
};

const int small_clocks_NCLOCKS = sizeof(small_clocks) / sizeof(small_clocks[0]);
// SmallClock small_clocks[] = {
//   {tz_SFO, "SFO", "255,128,128", 0, 16},
//   {tz_UTC, "UTC", "255,255,255", 21, 16},
//   {tz_LON, "LON", "0,255,255", 42, 16},
//   {tz_PAR, "PAR", "64,64,64",  0, 24},
//   {tz_ATH, "ATH", "64,64,255", 21, 24},
//   {tz_SIN, "SIN", "255,0,0", 42, 24},
// };

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Reads text from stdin and displays it. "
          "Empty string: clear screen\n");
  fprintf(stderr, "Options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  fprintf(stderr,
          "\t-d <time-format>  : Default '%%H:%%M'. See strftime()\n"
          "\t-f <font-file>    : Use given font for small font.\n"
          "\t-F <font-file>    : Use given font for big font.\n"
          "\t-b <brightness>   : Sets brightness percent. Default: 100.\n"
          "\t-S <spacing>      : Spacing pixels between letters (Default: 0)\n"
          "\t-C <r,g,b>        : Color. Default 255,255,0\n"
          "\t-B <r,g,b>        : Background-Color. Default 0,0,0\n"
          );

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

  const char *bdf_big_font_file = kBigFont;
  const char *bdf_small_font_file = kSmallFont;
  int brightness = 100;
  int letter_spacing = 0;
  int opt;
  while ((opt = getopt(argc, argv, "x:y:f:F:C:B:b:S:d:")) != -1) {
    switch (opt) {
    case 'b': brightness = atoi(optarg); break;
    case 'f': bdf_small_font_file = strdup(optarg); break;
    case 'F': bdf_big_font_file = strdup(optarg); break;
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
    default:
      return usage(argv[0]);
    }
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font big_font;
  if (!big_font.LoadFont(bdf_big_font_file)) {
    fprintf(stderr, "Couldn't load big font '%s'\n", bdf_big_font_file);
    return 1;
  }
  int big_font_baseline = big_font.baseline();
  int big_font_height = big_font.height();
  int big_font_width = big_font.CharacterWidth(77);
  int big_x_orig = (64 - 7 * big_font_width) / 2;
  int big_y_orig = big_font_baseline;
  rgb_matrix::Font small_font;
  if (!small_font.LoadFont(bdf_small_font_file)) {
    fprintf(stderr, "Couldn't load small font '%s'\n", bdf_small_font_file);
    return 1;
  }
  int small_font_baseline = small_font.baseline();
  int small_font_height = small_font.height();
  int small_font_width = small_font.CharacterWidth(77);
  if (brightness < 1 || brightness > 100) {
    fprintf(stderr, "Brightness is outside usable range.\n");
    return 1;
  }

  RGBMatrix *matrix = rgb_matrix::CreateMatrixFromOptions(matrix_options,
                                                          runtime_opt);
  if (matrix == NULL)
    return 1;

  matrix->SetBrightness(brightness);

  const bool all_extreme_colors = (brightness == 100)
    && FullSaturation(color)
    && FullSaturation(bg_color);
  if (all_extreme_colors)
    matrix->SetPWMBits(1);

  FrameCanvas *offscreen = matrix->CreateFrameCanvas();

  char hours_buffer[256];
  char minutes_buffer[256];
  char seconds_buffer[256];
  struct timespec next_time;
  next_time.tv_sec = time(NULL);
  next_time.tv_nsec = 0;
  struct tm tm;

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  while (!interrupt_received) {
    offscreen->Fill(bg_color.r, bg_color.g, bg_color.b);
    if (putenv(tz_NYC) != 0) {
      err(1, "putenv");
    }
    tzset();
    localtime_r(&next_time.tv_sec, &tm);
    strftime(hours_buffer, sizeof(hours_buffer), "%H", &tm);
    strftime(minutes_buffer, sizeof(minutes_buffer), "%M", &tm);
    strftime(seconds_buffer, sizeof(seconds_buffer), "%S", &tm);
    int x = big_x_orig;
    rgb_matrix::DrawText(offscreen, big_font, x, big_y_orig,
                         color, NULL, hours_buffer, 0);
    x = big_x_orig + 1.75 * big_font_width;
    rgb_matrix::DrawText(offscreen, big_font, x, big_y_orig,
                         color, NULL, ":", 0);
    x = big_x_orig + 2.5 * big_font_width;
    rgb_matrix::DrawText(offscreen, big_font, x, big_y_orig,
                         color, NULL, minutes_buffer, 0);
    x = big_x_orig + 4.25 * big_font_width;
    rgb_matrix::DrawText(offscreen, big_font, x, big_y_orig,
                         color, NULL, ":", 0);
    x = big_x_orig + 5 * big_font_width;
    rgb_matrix::DrawText(offscreen, big_font, x, big_y_orig,
                         color, NULL, seconds_buffer, 0);
    for (int i = 0; i < small_clocks_NCLOCKS; ++i) {
      SmallClock* sc = &small_clocks[i];
      if (putenv(sc->tz) != 0) {
        err(1, "putenv(%s)", sc->tz);
      }
      tzset();
      localtime_r(&next_time.tv_sec, &tm);
      strftime(hours_buffer, sizeof(hours_buffer), "%H", &tm);
      strftime(minutes_buffer, sizeof(minutes_buffer), "%M", &tm);
      int x = sc->x;
      rgb_matrix::DrawText(offscreen, small_font, x, sc->y + small_font_baseline,
                         sc->color, NULL, hours_buffer, 0);
      x = sc->x + 1.75 * small_font_width;
      rgb_matrix::DrawText(offscreen, small_font, x, sc->y + small_font_baseline,
                           sc->color, NULL, ":", 0);
      x = sc->x + 2.5 * small_font_width;
      rgb_matrix::DrawText(offscreen, small_font, x, sc->y + small_font_baseline,
                           sc->color, NULL, minutes_buffer, 0);
      x = sc->x + 4.5 * small_font_width;
      rgb_matrix::DrawText(offscreen, small_font, x, sc->y + small_font_baseline,
                           sc->color, NULL, sc->label, 0);

    }

    // Wait until we're ready to show it.
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, NULL);

    // Atomic swap with double buffer
    offscreen = matrix->SwapOnVSync(offscreen);

    next_time.tv_sec += 1;
  }

  // Finished. Shut down the RGB matrix.
  matrix->Clear();
  delete matrix;

  write(STDOUT_FILENO, "\n", 1);  // Create a fresh new line after ^C on screen
  return 0;
}
