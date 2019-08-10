#include "accuweather.h"

weather_icons_t icons(
    {
      { 1, { true, false, "SUNNY   ", "Sunny"}},
      { 2, { true, false, "M SUNNY ", "Mostly Sunny"}},
      { 3, { true, false, "P SUNNY ", "Partly Sunny"}},
      { 4, { true, false, "INT CLDS", "Intermittent Clouds"}},
      { 5, { true, false, "HAZY SSN", "Hazy Sunshine"}},
      { 6, { true, false, "M CLOUDY", "Mostly Cloudy"}},
      { 7, { true,  true, "CLOUDY  ", "Cloudy"}},
      { 8, { true,  true, "DREARY  ", "Dreary (Overcast"}},
      {11, { true,  true, "FOG     ", "Fog"}},
      {12, { true,  true, "SHOWERS ", "Showers"}},
      {13, { true, false, "M CLD SH", "Mostly Cloudy w/ Showers"}},
      {14, { true, false, "P CLD SH", "Partly Sunny w/ Showers"}},
      {15, { true,  true, "THDRSTRM", "T-Storms"}},
      {16, { true, false, "M CLD TS", "Mostly Cloudy w/ T-Storms"}},
      {17, { true, false, "P CLD TS", "Partly Sunny w/ T-Storms"}},
      {18, { true,  true, "RAIN    ", "Rain"}},
      {19, { true,  true, "FLURRIES", "Flurries"}},
      {20, { true, false, "M CLD FL", "Mostly Cloudy w/ Flurries"}},
      {21, { true, false, "P CLD FL", "Partly Sunny w/ Flurries"}},
      {22, { true,  true, "SNOW    ", "Snow"}},
      {23, { true, false, "M CLD SN", "Mostly Cloudy w/ Snow"}},
      {24, { true,  true, "ICE     ", "Ice"}},
      {25, { true,  true, "SLEET   ", "Sleet"}},
      {26, { true,  true, "FRZ RAIN", "Freezing Rain"}},
      {29, { true,  true, "RAIN+SNW", "Rain and Snow"}},
      {30, { true,  true, "HOT     ", "Hot"}},
      {31, { true,  true, "COLD    ", "Cold"}},
      {32, { true,  true, "WINDY   ", "Windy"}},
      {33, {false,  true, "CLEAR   ", "Clear"}},
      {34, {false,  true, "M CLEAR ", "Mostly Clear"}},
      {35, {false,  true, "P CLEAR ", "Partly Cloudy"}},
      {36, {false,  true, "I CLOUDS", "Intermittent Clouds"}},
      {37, {false,  true, "HAZY    ", "Hazy Moonlight"}},
      {38, {false,  true, "M CLOUDY", "Mostly Cloudy"}},
      {39, {false,  true, "P CLD SH", "Partly Cloudy w/ Showers"}},
      {40, {false,  true, "M CLD SH", "Mostly Cloudy w/ Showers"}},
      {41, {false,  true, "P CLD TS", "Partly Cloudy w/ T-Storms"}},
      {42, {false,  true, "M CLD TS", "Mostly Cloudy w/ T-Storms"}},
      {43, {false,  true, "M CLD FL", "Mostly Cloudy w/ Flurries"}},
      {44, {false,  true, "M CLD SN", "Mostly Cloudy w/ Snow"}},
    }
);

		      

