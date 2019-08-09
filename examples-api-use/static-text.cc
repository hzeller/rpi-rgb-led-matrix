// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how write text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

#ifdef JSONC
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#endif

#include <jansson.h>

using namespace rgb_matrix;

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Reads text from stdin and displays it. "
          "Empty string: clear screen\n");
  fprintf(stderr, "Options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  fprintf(stderr,
          "\t-f <font-file>    : Use given font.\n"
          "\t-b <brightness>   : Sets brightness percent. Default: 100.\n"
          "\t-x <x-origin>     : X-Origin of displaying text (Default: 0)\n"
          "\t-y <y-origin>     : Y-Origin of displaying text (Default: 0)\n"
          "\t-S <spacing>      : Spacing pixels between letters (Default: 0)\n"
          "\t-C <r,g,b>        : Color. Default 255,255,0\n"
          "\t-B <r,g,b>        : Background-Color. Default 0,0,0\n"
          "\t-O <r,g,b>        : Outline-Color, e.g. to increase contrast.\n"
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
 
#ifdef JSONC
void json_recurse(json_object* jobj, int indent) {
  enum json_type type;
  char prefix[indent + 1];
  for (int i = 0; i < indent; ++i) {
    prefix[i] = ' ';
  }
  prefix[indent] = '\0';
  json_object_object_foreach(jobj, key, val) {
    type = json_object_get_type(val);
    printf("%s type: ");
    switch (type) {
    case json_type_null:
      printf("json_type_null\n");
      break;
    case json_type_boolean:
      printf("json_type_boolean: %s\n",
             json_object_get_boolean(jobj) ? "True" : "False");
      break;
    case json_type_double:
      printf("json_type_double: %f\n",
             json_object_get_double(jobj));
      break;
    case json_type_int:
      printf("json_type_int: %d\n"k
             json_object_get_int(jobj));
      break;
    case json_type_object:
      printf("json_type_object: \n");
      json_recurse(
      break;
    case json_type_array: printf("json_type_arrayn");
      break;
    case json_type_string: printf("json_type_stringn");
      break;
    }
  }
#endif

void janson_recurse(json_t* jobj, int indent) {
  //  printf("\nrecursing %d\n", indent);

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





//CURLcode curl_easy_setopt(CURL *handle, CURLOPT_WRITEFUNCTION, write_callback);


int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  Color color(255, 255, 0);
  Color bg_color(0, 0, 0);
  Color outline_color(0,0,0);
  bool with_outline = false;

  const char *bdf_font_file = NULL;
  std::string cline;
  int x_orig = 0;
  int y_orig = 0;
  int brightness = 100;
  int letter_spacing = 0;

  int opt;
  while ((opt = getopt(argc, argv, "x:y:f:C:B:O:b:S:")) != -1) {
    switch (opt) {
    case 'b': brightness = atoi(optarg); break;
    case 'x': x_orig = atoi(optarg); break;
    case 'y': y_orig = atoi(optarg); break;
    case 'f': bdf_font_file = strdup(optarg); break;
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
    case 'O':
      if (!parseColor(&outline_color, optarg)) {
        fprintf(stderr, "Invalid outline color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      with_outline = true;
      break;
    default:
      return usage(argv[0]);
    }
  }

  struct MemoryStruct chunk;
  chunk.memory = malloc(1); // will be grown as needed by the realloc above
  chunk.size = 0;           // no data at this point 
  curl_global_init(CURL_GLOBAL_ALL);
  CURL* curler = curl_easy_init();
  curl_easy_setopt(curler, CURLOPT_URL, "http://dataservice.accuweather.com/forecasts/v1/daily/1day/523789_PC?apikey=KdGjVBTcRtAZbVqcyVb4nIvAH7qdqZrS&language=en-us&details=true&metric=false");
  curl_easy_setopt(curler, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curler, CURLOPT_WRITEDATA, (void*)&chunk);
  curl_easy_setopt(curler, CURLOPT_USERAGENT, "SPERRY-UNIVAC 1100/60");
  CURLcode res = curl_easy_perform(curler);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    exit(1);
  }
  printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
  printf("%s\n", chunk.memory);
  printf("-------------------------\n\n");
#ifdef JSONC
  struct json_object* jobj = json_tokener_parse(chunk.memory);
  json_recurse(jobj, 0);
#endif

  json_error_t jerr;
  json_t* j = json_loadb(chunk.memory, chunk.size, 0, &jerr);
  if (j == nullptr) {
    fprintf(stderr, "%s from %s at %d, %d pos %d\n",
            jerr.text, jerr.source, jerr.line, jerr.column, jerr.position);
    exit(1);
  }

  janson_recurse(j, 0);

  curl_easy_cleanup(curler);
  free(chunk.memory);
  curl_global_cleanup();
  exit(0);
  for (int i = optind; i < argc; ++i) {
    cline.append(argv[i]).append(" ");
    fprintf(stderr, "%s\n", cline.c_str());
  }

  if (cline.empty()) {
    fprintf(stderr, "Add the text you want to print on the command-line.\n");
    return usage(argv[0]);
  }

  if (bdf_font_file == NULL) {
    fprintf(stderr, "Need to specify BDF font-file with -f\n");
    return usage(argv[0]);
  }

  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return 1;
  }

  /*
   * If we want an outline around the font, we create a new font with
   * the original font as a template that is just an outline font.
   */
  rgb_matrix::Font *outline_font = NULL;
  if (with_outline) {
      outline_font = font.CreateOutlineFont();
  }

  if (brightness < 1 || brightness > 100) {
    fprintf(stderr, "Brightness is outside usable range.\n");
    return 1;
  }

  RGBMatrix *canvas = rgb_matrix::CreateMatrixFromOptions(matrix_options,
                                                          runtime_opt);
  if (canvas == NULL)
    return 1;

  canvas->SetBrightness(brightness);

  const bool all_extreme_colors = (brightness == 100)
      && FullSaturation(color)
      && FullSaturation(bg_color)
      && FullSaturation(outline_color);
  if (all_extreme_colors)
    canvas->SetPWMBits(1);

  const int x = x_orig;
  int y = y_orig;

  char* line = strdup(cline.c_str());
  fprintf(stderr, "<%s>\n", line);
  {
    const size_t last = strlen(line);
    bool line_empty = strlen(line) == 0;
    if ((y + font.height() > canvas->height()) || line_empty) {
      canvas->Clear();
      y = y_orig;
    }
    if (outline_font) {
      // The outline font, we need to write with a negative (-2) text-spacing,
      // as we want to have the same letter pitch as the regular text that
      // we then write on top.
      rgb_matrix::DrawText(canvas, *outline_font,
                           x - 1, y + font.baseline(),
                           outline_color, &bg_color, line, letter_spacing - 2);
    }
    // The regular text. Unless we already have filled the background with
    // the outline font, we also fill the background here.
    rgb_matrix::DrawText(canvas, font, x, y + font.baseline(),
                         color, outline_font ? NULL : &bg_color, line,
                         letter_spacing);
    y += font.height();
  }

  sleep(1000000);

  // Finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
