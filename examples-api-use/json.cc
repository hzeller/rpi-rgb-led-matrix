#include <iostream>
#include <jansson.h>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

using namespace std;
static void janson_recurse(json_t* jobj, string pfx, map<string, string>* dstmap, int debug) {
  size_t index;
  const char* jkey;
  json_t* jvalue;
  switch (json_typeof(jobj)) {
  case JSON_TRUE:
    if (debug) {
      cerr << pfx << ": True" << endl;
    }
    if (dstmap != nullptr) {
      (*dstmap)[pfx] = "true";
    }
    break;
  case JSON_FALSE:
    if (debug) {
      cerr << pfx << ": False" << endl;
    }
    if (dstmap != nullptr) {
      (*dstmap)[pfx] = "false";
    }
    break;
  case JSON_NULL:
    if (debug) {
      cerr << pfx << ": NULL" << endl;
    }
    if (dstmap != nullptr) {
      (*dstmap)[pfx] = "NULL";
    }
    break;
  case JSON_INTEGER:
    if (debug) {
      cerr << pfx << ": " << json_integer_value(jobj) << endl;
    }
    if (dstmap != nullptr) {
      (*dstmap)[pfx] = to_string(json_integer_value(jobj));
    }
    break;
  case JSON_REAL:
    if (debug) {
      cerr << pfx << ": " << json_real_value(jobj) << endl;
    }
    if (dstmap != nullptr) {
      (*dstmap)[pfx] = to_string(json_real_value(jobj));
    }
    break;
  case JSON_STRING:
    if (debug) {
      cerr << pfx << ": " << json_string_value(jobj) << endl;
    }
    if (dstmap != nullptr) {
      (*dstmap)[pfx] = (json_string_value(jobj));
    }
    break;
  case JSON_ARRAY:
    json_array_foreach(jobj, index, jvalue) {
      janson_recurse(jvalue, pfx + "[" + to_string(index) + "]", dstmap, debug);
    }
    break;
  case JSON_OBJECT:
    json_object_foreach(jobj, jkey, jvalue) {
      janson_recurse(jvalue, pfx + "/" + jkey, dstmap, debug);
    }
    break;
  }
}

#ifdef __MAIN__

char forecast[] =
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

char currentconditions[] =
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

int main(int argc, char* argv[]) {
  json_error_t jerr;
  json_t* j;
  map<string, string> dstmap;
  if (strcmp(argv[1], "f") == 0) {
    j = json_loadb(forecast, strlen(forecast), 0, &jerr);
    janson_recurse(j, string(""), &dstmap, 0);
    cout << "      dayicon: " << dstmap["/DailyForecasts[0]/Day/Icon"] << endl;
    cout << "    dayphrase: " << dstmap["/DailyForecasts[0]/Day/IconPhrase"] << endl;
    cout << "   day-precip: " << dstmap["/DailyForecasts[0]/Day/HasPrecipitation"] << endl;
    cout << "    nighticon: " << dstmap["/DailyForecasts[0]/Night/Icon"] << endl;
    cout << "  nightphrase: " << dstmap["/DailyForecasts[0]/Night/IconPhrase"] << endl;
    cout << " night-precip: " << dstmap["/DailyForecasts[0]/Night/HasPrecipitation"] << endl;
    cout << "     temp max: " << dstmap["/DailyForecasts[0]/Temperature/Maximum/Value"] << endl;
    cout << "     temp min: " << dstmap["/DailyForecasts[0]/Temperature/Minimum/Value"] << endl;
  } else if (strcmp(argv[1], "c") == 0) {
    j = json_loadb(currentconditions, strlen(currentconditions), 0, &jerr);
    janson_recurse(j, string(""), &dstmap, 0);
    cout << "precipitation: " << dstmap["[0]/HasPrecipitation"] << endl;
    cout << "  precip-type: " << dstmap["[0]/PrecipitationType"] << endl;
    cout << "       isday?: " << dstmap["[0]/IsDayTime"] << endl;
    cout << "  temperature: " << dstmap["[0]/Temperature/Imperial/Value"] << endl;
    cout << " weather icon: " << dstmap["[0]/WeatherIcon"] << endl;
    cout << " weather text: " << dstmap["[0]/WeatherText"] << endl;
  } else {
    j = json_loadb(argv[1], strlen(argv[1]), 0, &jerr);
    if (j == nullptr) {
      fprintf(stderr, "%s from %s at %d, %d pos %d\n",
              jerr.text, jerr.source, jerr.line, jerr.column, jerr.position);
      return 1;
    }
    janson_recurse(j, string(""), &dstmap, 0);
    for (auto& m : dstmap) {
      cout << m.first << ": " << m.second << endl;
    }
  }
}
#endif
