// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2013, 2016 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "led-matrix.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

namespace rgb_matrix {
namespace {
typedef char** argv_iterator;

static bool ConsumeIntFlag(const char *flag_name,
                           argv_iterator &pos, const argv_iterator end,
                           int *result_value, int *error) {
  const char *option = *pos;
  const size_t flag_len = strlen(flag_name);
  if (strncmp(option, flag_name, flag_len) != 0)
    return false;  // not consumed.
  const char *value;
  if (option[flag_len] == '=')  // --option=42  # value in same arg
    value = option + flag_len + 1;
  else if (pos + 1 < end) {     // --option 42  # value in next arg
    value = *(++pos);
  } else {
    fprintf(stderr, "Parameter expected after %s\n", flag_name);
    ++*error;
    return true;  // consumed, but error.
  }
  char *end_value = NULL;
  int val = strtol(value, &end_value, 10);
  if (!*value || *end_value) {
    fprintf(stderr, "Couldn't parse parameter %s=%s "
            "(Expected number but '%s' looks funny)\n",
            flag_name, value, end_value);
    ++*error;
    return true;  // consumed, but error
  }
  *result_value = val;
  return true;  // consumed.
}

static bool OptFlagInit(int &argc, char **&argv, RGBMatrix::Options *mopts) {
  argv_iterator it = &argv[0];
  argv_iterator end = it + argc;

  std::vector<char*> unused_options;
  unused_options.push_back(*it++);  // Not interested in program name

  int err = 0;
  bool posix_end_option_seen = false;
  for (/**/; it < end; ++it) {
    posix_end_option_seen |= (strcmp(*it, "--") == 0);
    if (!posix_end_option_seen) {
      if (ConsumeIntFlag("--led-rows", it, end, &mopts->rows, &err))
        continue;
      if (ConsumeIntFlag("--led-chain", it, end, &mopts->chain_length, &err))
        continue;
      if (ConsumeIntFlag("--led-parallel", it, end, &mopts->parallel, &err))
        continue;
    }
    unused_options.push_back(*it);
  }

  if (err > 0) {
    return false;
  }

  // Success. Re-arrange flags to only include the ones not consumed.
  argc = (int) unused_options.size();
  for (int i = 0; i < argc; ++i) {
    argv[i] = unused_options[i];
  }
  return true;
}
}  // namespace

bool RGBMatrix::Options::InitializeFromFlags(int *argc, char ***argv) {
  // Unfortunately, we can't use getopt_long(), as it does not provide a
  // way to only fish out some of the flags and leave the rest as-is without
  // much complaining. So we have to do this here ourselves.
  return OptFlagInit(*argc, *argv, this);
}

void RGBMatrix::Options::FlagUsageMessage() {
  fprintf(stderr,
          "\t--led-rows <rows>         : Panel rows. 8, 16, 32 or 64. "
          "Default: 32\n"
          "\t--led-parallel <parallel> : For Plus-models or RPi2: parallel "
          "chains. 1..3. Default: 1\n"
          "\t--led-chain <chained>     : Number of daisy-chained boards. "
          "Default: 1.\n");
}

bool RGBMatrix::Options::Validate(std::string *err) {
  bool any_error = false;
  if (rows != 8 && rows != 16 && rows != 32 && rows != 64) {
    err->append("Invalid number or panel rows. "
                "Should be one of 8, 16, 32 or 64\n");
    any_error = true;
  }

  if (chain_length < 1) {
    err->append("Chain-length outside usable range\n");
    any_error = true;
  }

  if (parallel < 1 || parallel > 3) {
    err->append("Parallel outside usable range.\n");
    any_error = true;
  }
  return !any_error;
}

}  // namespace rgb_matrix
