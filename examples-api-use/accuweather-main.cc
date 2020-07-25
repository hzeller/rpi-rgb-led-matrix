#include "accuweather.h"

#include <curl/curl.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
  curl_global_init(CURL_GLOBAL_ALL);
  weather("currentconditions/v1");
  cout << endl << endl << "----------------------------" << endl << endl;
  weather("forecasts/v1/daily/1day");
  curl_global_cleanup();

}
