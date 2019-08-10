#include "accuweather.h"

#include <curl/curl.h>
#include <iostream>
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

using namespace std;

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

#if 0
static void janson_recurse(json_t* jobj, string prefix) {
  const char* jkey;
  json_t* jvalue;
  json_object_foreach(jobj, jkey, jvalue) {
    string pfx(prefix + string("/") + string(jkey));
    switch (json_typeof(jvalue)) {
    case JSON_TRUE:
      cout << pfx << ": True" << endl;
      break;
    case JSON_FALSE:
      cout << pfx << ": False" << endl;
      break;
    case JSON_NULL:
      cout << pfx << ": NULL" << endl;
      break;
    case JSON_INTEGER:
      // printf("%" JSON_INTEGER_FORMAT "\n", json_integer_value(jvalue));
      cout << pfx << ": " << json_integer_value(jvalue) << endl;
      break;
    case JSON_REAL:
      // printf("%f\n", json_real_value(jvalue));
      cout << pfx << ": " << json_real_value(jvalue) << endl;
      break;
    case JSON_STRING:
      // printf("\"%s\"\n", json_string_value(jvalue));
      cout << pfx << ": " << json_string_value(jvalue) << endl;
      break;
    case JSON_ARRAY:
      {
        size_t index;
        json_t *jarr;
        json_t *jv = jvalue;
        json_array_foreach(jv, index, jarr) {
          // printf("%s  [%d]:\n", prefix, index);
          janson_recurse(jarr, pfx + "[" + to_string(index) + "]");
        }
      }
      break;
    case JSON_OBJECT:
      // printf("%sOBJECT\n", prefix);
      janson_recurse(jvalue, pfx);
      break;
    }
  }
}
#endif
#if 0
static void janson_recurse(json_t* jobj, int indent) {
  char prefix[indent + 1];
  for (int i = 0; i < indent; ++i) {
    prefix[i] = ' ';
  }
  prefix[indent] = '\0';
  const char* jkey;
  json_t* jvalue;
  json_object_foreach(jobj, jkey, jvalue) {
    printf("%s%s: ", prefix, jkey);
    switch (json_typeof(jvalue)) {
    case JSON_TRUE:
      printf("True\n");
      break;
    case JSON_FALSE:
      printf("False\n");
      break;
    case JSON_NULL:
      printf("NULL\n");
      break;
    case JSON_INTEGER:
      printf("%" JSON_INTEGER_FORMAT "\n", json_integer_value(jvalue));
      break;
    case JSON_REAL:
      printf("%f\n", json_real_value(jvalue));
      break;
    case JSON_STRING:
      printf("\"%s\"\n", json_string_value(jvalue));
      break;
    case JSON_ARRAY:
      printf("[\n");
      {
        size_t index;
        json_t *jarr;
        json_t *jv = jvalue;
        json_array_foreach(jv, index, jarr) {
          printf("%s  [%d]:\n", prefix, index);
          janson_recurse(jarr, indent + 4);
        }
      }
      printf("%s]\n", prefix);
      break;
    case JSON_OBJECT:
      printf("%sOBJECT\n", prefix);
      janson_recurse(jvalue, indent + 2);
      break;
    }
  }
}
#endif

int weather(const string& what) {
  string url("http://dataservice.accuweather.com/" +
             what +
             "/" "3719_PC" +
             "?" "apikey" "=" "KdGjVBTcRtAZbVqcyVb4nIvAH7qdqZrS"
             "&" "language" "=" "en-us" +
             "&" "details" "=" "false");
  cerr << url << endl;
  struct MemoryStruct chunk;
  chunk.memory = malloc(1); // will be grown as needed by the realloc above
  chunk.size = 0;           // no data at this point
  //curl_global_init(CURL_GLOBAL_ALL);
  CURL* curler = curl_easy_init();
  curl_easy_setopt(curler, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curler, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curler, CURLOPT_WRITEDATA, (void*)&chunk);
  curl_easy_setopt(curler, CURLOPT_USERAGENT, "SPERRY-UNIVAC 1100/60");
  CURLcode res = curl_easy_perform(curler);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    return 1;
  }
  fprintf(stderr, "\n\n----\n%s\n----\n", chunk.memory);
  json_error_t jerr;
  json_t* j = json_loadb(chunk.memory, chunk.size, 0, &jerr);
  if (j == nullptr) {
    fprintf(stderr, "%s from %s at %d, %d pos %d\n",
            jerr.text, jerr.source, jerr.line, jerr.column, jerr.position);
    return 1;
  }
  janson_recurse(j, string(""));
  curl_easy_cleanup(curler);
  free(chunk.memory);
  // curl_global_cleanup();
  return 0;
}
