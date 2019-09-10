#include "accuweather.h"

#include <curl/curl.h>
#include <iostream>
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

using namespace std;

extern void janson_recurse(json_t* jobj, string pfx, map<string, string>* dstmap, int debug);



static weather_icons_t icons(
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

struct MemoryStruct {
  char* memory;
  size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t realsize = size* nmemb;
  struct MemoryStruct* mem = (struct MemoryStruct*)userp;
  char* ptr = reinterpret_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));
  if (ptr == NULL) {
    /* out of memory! */
    fprintf(stderr, "not enough memory (realloc returned NULL)\n");
    return 0;
  }
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

char mock_forecast[] =
  "{"
  "  \"Headline\": {"
  "    \"EffectiveDate\": \"2019-08-10T08:00:00-04:00\","
  "    \"EffectiveEpochDate\": 1565438400,"
  "    \"Severity\": 4,"
  "    \"Text\": \"Pleasant this weekend\","
  "    \"Category\": null,"
  "    \"EndDate\": null,"
  "    \"EndEpochDate\": null,"
  "    \"MobileLink\":"
  "    \"http://m.accuweather.com/en/us/new-york-ny/10007/extended-weather-forecast/3719_pc?lang=en-us\","
  "    \"Link\": \"http://www.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/3719_pc?lang=en-us\""
  "  },"
  "  \"DailyForecasts\": ["
  "    {"
  "      \"Date\": \"2019-08-10T07:00:00-04:00\","
  "      \"EpochDate\": 1565434800,"
  "      \"Temperature\": {"
  "        \"Minimum\": {"
  "          \"Value\": 63.0,"
  "          \"Unit\": \"F\", "
  "          \"UnitType\": 18"
  "        },"
  "        \"Maximum\": {"
  "          \"Value\": 82.0,"
  "          \"Unit\": \"F\","
  "          \"UnitType\": 18"
  "        }"
  "      },"
  "      \"Day\": {"
  "        \"Icon\": 2,"
  "        \"IconPhrase\": \"Mostly sunny\","
  "        \"HasPrecipitation\": false"
  "      },"
  "      \"Night\": {"
  "        \"Icon\": 34,"
  "        \"IconPhrase\": \"Mostly clear\","
  "        \"HasPrecipitation\": false"
  "      },"
  "      \"Sources\": ["
  "        \"AccuWeather\""
  "      ],"
  "      \"MobileLink\": \"http://m.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/3719_pc?day=1&lang=en-us\","
  "      \"Link\": \"http://www.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/3719_pc?day=1&lang=en-us\""
  "    }"
  "  ]"
  "}";

char mock_currentconditions[] =
  "["
  "  {"
  "    \"LocalObservationDateTime\": \"2019-08-10T10:23:00-04:00\","
  "    \"EpochTime\": 1565446980,"
  "    \"WeatherText\": \"Sunny\","
  "    \"WeatherIcon\": 1,"
  "    \"HasPrecipitation\": false,"
  "    \"PrecipitationType\": null,"
  "    \"IsDayTime\": true,"
  "    \"Temperature\": {"
  "      \"Metric\": {"
  "        \"Value\": 22.8,"
  "        \"Unit\": \"C\","
  "        \"UnitType\": 17"
  "      },"
  "      \"Imperial\": {"
  "        \"Value\": 73,"
  "        \"Unit\": \"F\","
  "        \"UnitType\": 18"
  "      }"
  "    },"
  "    \"MobileLink\": \"http://m.accuweather.com/en/us/new-york-ny/10007/current-weather/3719_pc?lang=en-us\","
  "    \"Link\": \"http://www.accuweather.com/en/us/new-york-ny/10007/current-weather/3719_pc?lang=en-us\""
  "  }"
  "]";

int weather(const string& what, map<string, string>* resmap) {
  cerr << "weather(" << what << endl;
  struct MemoryStruct chunk = {nullptr, 0};
  size_t chunk_size;
  char* json_result;
  if (what.substr(0, 4) == "http") {
    chunk.memory = malloc(1); // will be grown as needed by the realloc above
    chunk.size = 0;           // no data at this point
    //curl_global_init(CURL_GLOBAL_ALL);
    CURL* curler = curl_easy_init();
    curl_easy_setopt(curler, CURLOPT_URL, what.c_str());
    curl_easy_setopt(curler, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curler, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curler, CURLOPT_USERAGENT, "SPERRY-UNIVAC 1100/60");
    CURLcode res = curl_easy_perform(curler);
    curl_easy_cleanup(curler);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      return 1;
    }
    fprintf(stderr, "\n\n----\n%s\n----\n", chunk.memory);
    json_result = chunk.memory;
    chunk_size = chunk.size;
  } else {
    json_result = what.c_str();
    chunk_size = strlen(json_result);
  }
  json_error_t jerr;
  json_t* j = json_loadb(json_result, chunk_size, 0, &jerr);
  if (j == nullptr) {
    fprintf(stderr, "%s from %s at %d, %d pos %d\n",
            jerr.text, jerr.source, jerr.line, jerr.column, jerr.position);
    return 1;
  }
  janson_recurse(j, string(""), resmap, 0);
  if (chunk.memory) {
    free(chunk.memory);
  }
  // curl_global_cleanup();
  return 0;
}
