#pragma once

#include <map>
#include <string>

struct WeatherIcon {
  bool for_day;
  bool for_night;
  std::string long_text;
  std::string short_text;
};

typedef std::map<int, WeatherIcon> weather_icons_t;

int weather(const std::string& what);
