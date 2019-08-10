// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "led-matrix.h"
#include "graphics.h"

#include <curl/curl.h>
#include <err.h>
#include <getopt.h>
#include <ini.h>
#include <iostream>
#include <jansson.h>
#include <map>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace rgb_matrix;
using namespace std;

char kTzAth[] = "TZ=Europe/Athens";
char kTzLon[] = "TZ=Europe/London";
char kTzNyc[] = "TZ=America/New_York";
char kTzPar[] = "TZ=Europe/Paris";
char kTzSfo[] = "TZ=America/Los_Angeles";
char kTzSin[] = "TZ=Asia/Singapore";
char kTzUtc[] = "TZ=UTC";

const char kLeftArrow[] = "\xe2\x86\x90";
const char kUpArrow[] = "\xe2\x86\x91";
const char kRightArrow[] = "\xe2\x86\x92";
const char kDownArrow[] = "\xe2\x86\x93";

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static int usage(const char* progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Options:\n");
  PrintMatrixFlags(stderr);
  fprintf(stderr,
          "\t-i <.ini filename> : Default 'cclock.ini'\n"
          );
  return 1;
}

static bool parseColor(Color* c, const char* str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
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

map<string, map<string, string>> ini;

int inihandler(void* user,
               const char* section,
               const char* name,
               const char* value) {
  ini[string(section)][string(name)] = string(value);
  return 0;
}



int main(int argc, char* argv[]) {
  RGBMatrix::Options matrix_options;
  RuntimeOptions runtime_opt;
  if (!ParseOptionsFromFlags(&argc, &argv,
                             &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }
  char* ini_filename = "cclock.ini";
  int adjust_colon_x = 0;
  int adjust_colon_y = 0;
  int opt;
  while ((opt = getopt(argc, argv, "i:Wc:C:")) != -1) {
    switch (opt) {
    case 'i':
      ini_filename = strdup(optarg);
      break;
    case 'W':
      exit(weather());
      break;
    case 'c':
      adjust_colon_x = atoi(optarg);
      break;
    case 'C':
      adjust_colon_y = atoi(optarg);
      break;
    default:
      return usage(argv[0]);
    }
  }
  ini_parse(ini_filename, inihandler, NULL);
  for (auto& m : ini) {
    cout << m.first << endl;
    for (auto& p : m.second) {
      cout << "  " << p.first << ": " << p.second << endl;
    }
  }
  Color time_fg;
  Color time_bg;
  Color date_fg;
  Color date_bg;
  Color temp_fg;
  Color temp_bg;
  parseColor(&time_fg, ini["time"]["fg"].c_str());
  parseColor(&time_bg, ini["time"]["bg"].c_str());
  parseColor(&temp_fg, ini["temp"]["fg"].c_str());
  parseColor(&temp_bg, ini["temp"]["bg"].c_str());
  parseColor(&date_fg, ini["date"]["fg"].c_str());
  parseColor(&date_bg, ini["date"]["bg"].c_str());
  int time_x = atoi(ini["time"]["x"].c_str()) + adjust_colon_x;
  int time_y = atoi(ini["time"]["y"].c_str());
  int temp_x = atoi(ini["temp"]["x"].c_str());
  int temp_y = atoi(ini["temp"]["y"].c_str());
  int date_x = atoi(ini["date"]["x"].c_str());
  int date_y = atoi(ini["date"]["y"].c_str());
  const char* bdf_time_font_file = ini["time"]["font"].c_str();
  const char* bdf_date_font_file = ini["date"]["font"].c_str();
  const char* bdf_temp_font_file = ini["temp"]["font"].c_str();
  Font time_font;
  Font date_font;
  Font temp_font;
  if (!time_font.LoadFont(bdf_time_font_file)) {
    fprintf(stderr, "Couldn't load time font '%s'\n", bdf_time_font_file);
    return 1;
  }
  if (!date_font.LoadFont(bdf_date_font_file)) {
    fprintf(stderr, "Couldn't load date font '%s'\n", bdf_date_font_file);
    return 1;
  }
  if (!temp_font.LoadFont(bdf_temp_font_file)) {
    fprintf(stderr, "Couldn't load temp font '%s'\n", bdf_temp_font_file);
    return 1;
  }

  RGBMatrix* matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) {
    return 1;
  }
  matrix->SetBrightness(100);

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
    offscreen->Fill(0, 0, 0);
    if (putenv(kTzNyc) != 0) {
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

    int c_x = time_x + 2 * time_font.CharacterWidth('0');
    int m_x = time_x + 3 * time_font.CharacterWidth('0') - 1;
    int c_y = time_y - adjust_colon_y;
    if (adjust_colon_x) {
      c_x -= adjust_colon_x;
      m_x -= 2 * adjust_colon_y;
    }
    textat(":", offscreen, time_font, time_fg, time_bg, c_x, c_y);
    time_buffer[2] = '\0';
    textat(time_buffer, offscreen, time_font, time_fg, time_bg, time_x, time_y);
    textat(time_buffer + 3, offscreen, time_font, time_fg, time_bg, m_x, time_y);

    
    



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
