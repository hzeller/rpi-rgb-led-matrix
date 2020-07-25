// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "accuweather.h"
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

string map_get(const map<string, string>& themap, const string& key) {
  //map<string, string>::iterator it;
  auto it = themap.find(key);
  if (it == themap.end()) {
    return string("?") + key + string("?");
  } else {
    return it->second;
  }
}

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
          "\t-i <.ini filename>  : Default 'cclock.ini'\n"
          "\t-p polling interval : Default 5400 (seconds)\n" 
          "\t-w                  : Display weather data\n"
          "\t-W                  : Display mock weather data\n"
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
  // cerr << "!! [" << section << "] " << name << " = " << value << endl;
  auto it1 = ini.find(section);
  if (it1 != ini.end()) {
    // cerr << "  " << section << " already exists!" << endl;
    auto it2 = it1->second.find(name);
    if (it2 != it1->second.end()) {
      // cerr << "  " << name << " already exists, = " << it2->second << endl;
      it2->second.append(" ");
      it2->second.append(value);
      return 0;
    }
  }
  // cerr << "  newstuff, inserting" << endl;
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
  const char* ini_filename = "cclock.ini";
  int opt;
  int polling_interval = 5400;
  int do_weather = 0;
  int mock_weather = 0;
  while ((opt = getopt(argc, argv, "i:P:wW")) != -1) {
    switch (opt) {
    case 'i':
      ini_filename = strdup(optarg);
      break;
    case 'P':
      polling_interval = atoi(optarg);
      break;
    case 'w':
      ++do_weather; 
      break;
    case 'W':
      ++do_weather;
      ++mock_weather;
      break;
    default:
      return usage(argv[0]);
    }
  }
  ini_parse(ini_filename, inihandler, NULL);
  for (auto& m : ini) {
    cout << m.first << endl;
    for (auto& p : m.second) {
      cout << "  " << p.first << ": " << p.second.length() << " " << p.second << endl;
    }
  }
  Color time_fg;
  Color time_bg;
  Color date_fg;
  Color date_bg;
  Color ctemp_fg;
  Color hitemp_fg;
  Color lotemp_fg;
  Color day_fg;
  Color night_fg;
  Color temp_bg;
  Color prec_fg;
  int colon_adjust_x = atoi(ini["time"]["colon_adjust_x"].c_str());
  int colon_adjust_y = atoi(ini["time"]["colon_adjust_y"].c_str());
  parseColor(&time_fg, ini["time"]["fg"].c_str());
  parseColor(&time_bg, ini["time"]["bg"].c_str());
  parseColor(&date_fg, ini["date"]["fg"].c_str());
  parseColor(&date_bg, ini["date"]["bg"].c_str());
  parseColor(&ctemp_fg, ini["temp"]["ctemp"].c_str());  
  parseColor(&hitemp_fg, ini["temp"]["hitemp"].c_str());  
  parseColor(&lotemp_fg, ini["temp"]["lotemp"].c_str());  
  parseColor(&day_fg, ini["temp"]["day"].c_str());  
  parseColor(&night_fg, ini["temp"]["night"].c_str());  
  parseColor(&prec_fg, ini["temp"]["precipitation"].c_str());  
  parseColor(&temp_bg, ini["temp"]["bg"].c_str());  
  int time_x = atoi(ini["time"]["x"].c_str()) + colon_adjust_x;
  int time_y = atoi(ini["time"]["y"].c_str());
  int temp_x = atoi(ini["temp"]["x"].c_str());
  int temp_y = atoi(ini["temp"]["y"].c_str());
  int date_x = atoi(ini["date"]["x"].c_str());
  int date_y = atoi(ini["date"]["y"].c_str());
  const char* bdf_time_font_file = ini["time"]["font"].c_str();
  const char* bdf_date_font_file = ini["date"]["font"].c_str();
  const char* bdf_temp_font_file = ini["temp"]["font"].c_str();
  const char* bdf_temp_smallfont_file = ini["temp"]["smallfont"].c_str();
  Font time_font;
  Font date_font;
  Font temp_font;
  Font temp_smallfont;
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
  if (!temp_smallfont.LoadFont(bdf_temp_smallfont_file)) {
    fprintf(stderr, "Couldn't load temp font '%s'\n", bdf_temp_smallfont_file);
    return 1;
  }

  RGBMatrix* matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) {
    return 1;
  }
  matrix->SetBrightness(100);
  bool raining = false;
  bool isdaytime;
  string precipitation_type;
  string temperature;
  string weather_icon;
  bool day_fc_raining = false;
  string day_fc_icon;
  string night_fc_icon;
  bool night_fc_raining = false;
  string max_temp;
  string min_temp;
    
  int tfw = temp_font.CharacterWidth('M');
  int tfh = temp_font.height();


  FrameCanvas *offscreen = matrix->CreateFrameCanvas();
  char time_buffer[256];
  char previous_time_buffer[256];
  char date_buffer[256];
  char previous_date_buffer[256];
  struct timespec next_time;
  next_time.tv_sec = time(NULL);
  next_time.tv_nsec = 0;
  struct timespec last_weather = next_time;
  struct tm tm;
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  if (putenv(kTzNyc) != 0) {
    err(1, "putenv");
  }
  tzset();
  
  while (!interrupt_received) {
    offscreen->Fill(0, 0, 0);
    localtime_r(&next_time.tv_sec, &tm);
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", &tm);
    strftime(date_buffer, sizeof(date_buffer), "%a %b %d", &tm);

    int c_x = time_x + 2 * time_font.CharacterWidth('0');
    int m_x = time_x + 3 * time_font.CharacterWidth('0') - 1;
    int c_y = time_y - colon_adjust_y;
    if (colon_adjust_x) {
      c_x -= colon_adjust_x;
      m_x -= 2 * colon_adjust_y;
    }
    textat(":", offscreen, time_font, time_fg, time_bg, c_x, c_y);
    time_buffer[2] = '\0';
    textat(time_buffer, offscreen, time_font, time_fg, time_bg, time_x, time_y);
    textat(time_buffer + 3, offscreen, time_font, time_fg, time_bg, m_x, time_y);
    textat(date_buffer, offscreen, date_font, date_fg, date_bg, date_x, date_y);
    // cerr << "next_time=" << next_time.tv_sec << "  last_weather = " << last_weather.tv_sec << endl;
    if (do_weather && (last_weather.tv_sec < next_time.tv_sec)) {
      cerr << "Polling " << last_weather.tv_sec;
      last_weather.tv_sec = next_time.tv_sec + polling_interval;
      cerr << " -> " << last_weather.tv_sec << endl;

      string what;
      map<string, string> jmap;
      for (auto endpoint : { "currentconditions", "forecasts"}) {
        if (mock_weather) {
          what = ini["mockdata"][endpoint + string((mock_weather == 2) ? "-detail" : "")];
          cerr << "mockdata[" << what << endl;
        } else {
          what = ini["url"]["base"] +
            "/" + ini["url"][endpoint] +
            "/" + ini["url"]["location"] +
            "?apikey=" + ini["url"]["apikey"] +
            "&language=" + ini["url"]["language"] +
            "&details=" + string((do_weather == 2) ? "true" : "false");
        }
        weather(what, &jmap);
      }
      for (auto& m : jmap) {
        cout << m.first << " = " << m.second << endl;
      }
      /*
        [0]/EpochTime = 1565446980
        [0]/HasPrecipitation = false
        [0]/IsDayTime = true
        [0]/Link = http://www.accuweather.com/en/us/new-york-ny/10007/current-weather/3719_pc?lang=en-us
        [0]/LocalObservationDateTime = 2019-08-10T10:23:00-04:00
        [0]/MobileLink = http://m.accuweather.com/en/us/new-york-ny/10007/current-weather/3719_pc?lang=en-us
        [0]/PrecipitationType = NULL
        [0]/Temperature/Imperial/Unit = F
        [0]/Temperature/Imperial/UnitType = 18
        [0]/Temperature/Imperial/Value = 73
        [0]/Temperature/Metric/Unit = C
        [0]/Temperature/Metric/UnitType = 17
        [0]/Temperature/Metric/Value = 22.800000
        [0]/WeatherIcon = 1
        [0]/WeatherText = Sunny
      */
      /*
        /DailyForecasts[0]/Date = 2019-08-10T07:00:00-04:00
        /DailyForecasts[0]/Day/HasPrecipitation = false
        /DailyForecasts[0]/Day/Icon = 2
        /DailyForecasts[0]/Day/IconPhrase = Mostly sunny
        /DailyForecasts[0]/EpochDate = 1565434800
        /DailyForecasts[0]/Link = http://www.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/3719_pc?day=1&lang=en-us
        /DailyForecasts[0]/MobileLink = http://m.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/3719_pc?day=1&lang=en-us
        /DailyForecasts[0]/Night/HasPrecipitation = false
        /DailyForecasts[0]/Night/Icon = 34
        /DailyForecasts[0]/Night/IconPhrase = Mostly clear
        /DailyForecasts[0]/Sources[0] = AccuWeather
        /DailyForecasts[0]/Temperature/Maximum/Unit = F
        /DailyForecasts[0]/Temperature/Maximum/UnitType = 18
        /DailyForecasts[0]/Temperature/Maximum/Value = 82.000000
        /DailyForecasts[0]/Temperature/Minimum/Unit = F
        /DailyForecasts[0]/Temperature/Minimum/UnitType = 18
        /DailyForecasts[0]/Temperature/Minimum/Value = 63.000000
        /Headline/Category = NULL
        /Headline/EffectiveDate = 2019-08-10T08:00:00-04:00
        /Headline/EffectiveEpochDate = 1565438400
        /Headline/EndDate = NULL
        /Headline/EndEpochDate = NULL
        /Headline/Link = http://www.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/3719_pc?lang=en-us
        /Headline/MobileLink = http://m.accuweather.com/en/us/new-york-ny/10007/extended-weather-forecast/3719_pc?lang=en-us
        /Headline/Severity = 4
        /Headline/Text = Pleasant this weekend
      */

      raining = map_get(jmap, "[0]/HasPrecipitation") == "true";
      isdaytime = map_get(jmap, "[0]/IsDayTime") == "true";
      precipitation_type = map_get(jmap, "[0]/PrecipitationType");
      temperature = to_string(int(atof(map_get(jmap, "[0]/Temperature/Imperial/Value").c_str()) + .5));
      weather_icon = map_get(jmap, "[0]/WeatherIcon");
      day_fc_raining = map_get(jmap, "/DailyForecasts[0]/Day/HasPrecipitation") == "true";
      night_fc_raining = map_get(jmap, "/DailyForecasts[0]/Night/HasPrecipitation") == "true";
      day_fc_icon =  map_get(jmap, "/DailyForecasts[0]/Day/Icon");
      night_fc_icon =  map_get(jmap, "/DailyForecasts[0]/Night/Icon");
      max_temp = to_string(int(atof(map_get(jmap, "/DailyForecasts[0]/Temperature/Maximum/Value").c_str()) + .5));
      min_temp = to_string(int(atof(map_get(jmap, "/DailyForecasts[0]/Temperature/Minimum/Value").c_str()) + .5));
    }
      
    int row1 = temp_y;
    int row2 = row1 + tfh + 3;
    int row3 = row2 + tfh + 2;
    int col1 = temp_x;
    int col2 = col1 + 3 * tfw;

    auto weather_ptr = ini["wi" + weather_icon];
    auto day_fc_ptr = ini["wi" + day_fc_icon];
    auto night_fc_ptr = ini["wi" + night_fc_icon];
    string weather_text = weather_ptr["short"];
    string day_fc_text = day_fc_ptr["short"];
    string night_fc_text = night_fc_ptr["short"];
    if (day_fc_ptr["daymode"] != "true") {
      day_fc_text.append("?");
    }
    if (night_fc_ptr["nightmode"] != "true") {
      night_fc_text.append("?");
    }

    textat(max_temp.c_str(), offscreen, temp_font, hitemp_fg, temp_bg, col1, row1);
    textat(temperature.c_str(), offscreen, temp_font, ctemp_fg, temp_bg, col1, row2);
    textat(min_temp.c_str(), offscreen, temp_font, lotemp_fg, temp_bg, col1, row3);
    if (precipitation_type == "NULL") {
      if (raining) {
        precipitation_type = "?''''''?";
      } else {
        precipitation_type = "";
      }
    }
    textat(precipitation_type.c_str(), offscreen, temp_smallfont, prec_fg, temp_bg, col2, 0);
    textat(weather_text.c_str(), offscreen, temp_smallfont, ctemp_fg, temp_bg, col2, 8);
    textat(day_fc_text.c_str(), offscreen, temp_smallfont, day_fg, temp_bg, col2, 16);
    textat(night_fc_text.c_str(), offscreen, temp_smallfont, night_fg, temp_bg, col2, 24);
      

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
