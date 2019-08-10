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
#include <ini.h>
#include <iostream>
#include <map>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace rgb_matrix;
using namespace std;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

char tz_NYC[] = "TZ=America/New_York";
char tz_SFO[] = "TZ=America/Los_Angeles";
char tz_UTC[] = "TZ=UTC";
char tz_LON[] = "TZ=Europe/London";
char tz_PAR[] = "TZ=Europe/Paris";
char tz_ATH[] = "TZ=Europe/Athens";
char tz_SIN[] = "TZ=Asia/Singapore";

const char left_arrow[] = "\xe2\x86\x90";
const char up_arrow[] = "\xe2\x86\x91";
const char right_arrow[] = "\xe2\x86\x92";
const char down_arrow[] = "\xe2\x86\x93";


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

void textat(const char* text,
            FrameCanvas* offscreen,
            const Font& font,
            const Color& color,
            const Color& bgcolor,
            int x,
            int y) {
  DrawText(offscreen,
           font,
           x,
           y + font.baseline(),
           color,
           &bgcolor,
           text);
}

const char* kBigFont = "../fonts/9x15B.bdf";
const char* kSmallFont = "../fonts/5x8.bdf";

Color time_fg(255, 128,   0);
Color time_bg(  0,   0,   0);
Color date_fg(255, 255,   0);
Color date_bg(  0,   0,   0);
Color temp_fg(  0, 128, 255);
Color temp_bg(255,   0,   0);

map<string, map<string, string>> ini;

int inihandler(void* user,
               const char* section,
               const char* name,
               const char* value) {
  // cerr << section << " " << name << " " << value << " " << endl;
  ini[string(section)][string(name)] = string(value);
  return 0;
}

int main(int argc, char *argv[]) {
  ini_parse("cclock.ini", inihandler, NULL);
  for (auto& m : ini) {
    cout << m.first << endl;
    for (auto& p : m.second) {
      cout << "  " << p.first << ": " << p.second << endl;
    }
  }
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
  const char* bdf_time_font_file = ini["time"]["font"].c_str();
  const char* bdf_date_font_file = ini["date"]["font"].c_str();
  const char* bdf_temp_font_file = ini["temp"]["font"].c_str();
  parseColor(&time_fg, ini["time"]["fg"].c_str());
  parseColor(&time_bg, ini["time"]["bg"].c_str());
  parseColor(&temp_fg, ini["temp"]["fg"].c_str());
  parseColor(&temp_bg, ini["temp"]["bg"].c_str());
  parseColor(&date_fg, ini["date"]["fg"].c_str());
  parseColor(&date_bg, ini["date"]["bg"].c_str());
  int time_x = atoi(ini["time"]["x"].c_str());
  int time_y = atoi(ini["time"]["y"].c_str());
  int temp_x = atoi(ini["temp"]["x"].c_str());
  int temp_y = atoi(ini["temp"]["y"].c_str());
  int date_x = atoi(ini["date"]["x"].c_str());
  int date_y = atoi(ini["date"]["y"].c_str());
  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  Font time_font;
  if (!time_font.LoadFont(bdf_time_font_file)) {
    fprintf(stderr, "Couldn't load time font '%s'\n", bdf_time_font_file);
    return 1;
  }
  Font date_font;
  if (!date_font.LoadFont(bdf_date_font_file)) {
    fprintf(stderr, "Couldn't load date font '%s'\n", bdf_date_font_file);
    return 1;
  }
  Font temp_font;
  if (!temp_font.LoadFont(bdf_temp_font_file)) {
    fprintf(stderr, "Couldn't load temp font '%s'\n", bdf_temp_font_file);
    return 1;
  }

  RGBMatrix* matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) {
    return 1;
  }
  matrix->SetBrightness(brightness);

  const bool all_extreme_colors = (brightness == 100)
    && FullSaturation(color)
    && FullSaturation(bg_color);
  if (all_extreme_colors) {
    //    matrix->SetPWMBits(1);
  }
  FrameCanvas *offscreen = matrix->CreateFrameCanvas();
  char time_buffer[256];
  char date_buffer[256];
  char temp_buffer[256];
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
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", &tm);
    strftime(date_buffer, sizeof(date_buffer), "%a %b %d", &tm);
    temp_buffer[0] = '7';
    temp_buffer[1] = '2';
    temp_buffer[2] = 176;
    temp_buffer[3] = 0;
    temp_buffer[4] = 0;

    

    textat(time_buffer, offscreen, time_font, time_fg, time_bg, time_x, time_y);
    textat(date_buffer, offscreen, date_font, date_fg, date_bg, date_x, date_y);
    textat(temp_buffer, offscreen, temp_font, temp_fg, temp_bg, temp_x, temp_y);
    


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
