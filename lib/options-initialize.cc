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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

#include <vector>

namespace rgb_matrix {
namespace {
typedef char** argv_iterator;

static bool ConsumeBoolFlag(const char *flag_name, const argv_iterator &pos,
                            bool *result_value) {
  const char *option = *pos;
  const size_t flag_len = strlen(flag_name);
  if (strncmp(option, flag_name, flag_len) != 0)
    return false;  // not consumed.
  *result_value = !*result_value;
  return true;
}

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

struct RuntimeOptions {
  RuntimeOptions() : as_daemon(false), drop_privileges(false) {}

  bool as_daemon;
  bool drop_privileges;
};

static bool FlagInit(int &argc, char **&argv,
                     RGBMatrix::Options *mopts,
                     RuntimeOptions *ropts) {
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
      if (ConsumeIntFlag("--led-brightness", it, end, &mopts->brightness, &err))
        continue;
      if (ConsumeIntFlag("--led-pwm-bits", it, end, &mopts->pwm_bits, &err))
        continue;
      if (ConsumeBoolFlag("--led-daemon", it, &ropts->as_daemon))
        continue;
      if (ConsumeBoolFlag("--led-drop-privs", it, &ropts->drop_privileges))
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

static bool drop_privs(const char *priv_user, const char *priv_group) {
  uid_t ruid, euid, suid;
  if (getresuid(&ruid, &euid, &suid) >= 0) {
    if (euid != 0)   // not root anyway. No priv dropping.
      return true;
  }

  struct group *g = getgrnam(priv_group);
  if (g == NULL) {
    perror("group lookup.");
    return false;
  }
  if (setresgid(g->gr_gid, g->gr_gid, g->gr_gid) != 0) {
    perror("setresgid()");
    return false;
  }
  struct passwd *p = getpwnam(priv_user);
  if (p == NULL) {
    perror("user lookup.");
    return false;
  }
  if (setresuid(p->pw_uid, p->pw_uid, p->pw_uid) != 0) {
    perror("setresuid()");
    return false;
  }
  return true;
}

}  // namespace

// Public interface.
RGBMatrix *CreateMatrixFromFlags(int *argc, char ***argv, bool allow_daemon) {
  RGBMatrix::Options mopt;
  RuntimeOptions ropt;
  if (!FlagInit(*argc, *argv, &mopt, &ropt)) {
    return NULL;
  }

  std::string error;
  if (!mopt.Validate(&error)) {
    fprintf(stderr, "%s\n", error.c_str());
    return NULL;
  }

  if (getuid() != 0) {
    fprintf(stderr, "Must run as root to be able to access /dev/mem\n"
            "Prepend 'sudo' to the command:\n\tsudo %s ...\n", (*argv)[0]);
    return NULL;
  }

  static GPIO io;  // This static var is a little bit icky.
  if (!io.Init()) {
    return NULL;
  }

  if (!allow_daemon && ropt.as_daemon) {
    fprintf(stderr, "Ignoring --led-daemon which was disabled.\n");
  }

  if (allow_daemon && ropt.as_daemon && daemon(1, 0) != 0) {
    perror("Failed to become daemon");
  }

  RGBMatrix *result = new RGBMatrix(NULL, mopt);
  // Allowing daemon also means we are allowed to start the thread now.
  result->SetGPIO(&io, allow_daemon);

  if (ropt.drop_privileges) {
    drop_privs("daemon", "daemon");
  }

  return result;
}

void PrintMatrixOptions(FILE *out, bool show_daemon) {
  fprintf(out,
          "\t--led-rows=<rows>         : Panel rows. 8, 16, 32 or 64. "
          "Default: 32\n"
          "\t--led-chain=<chained>     : Number of daisy-chained panels. "
          "Default: 1.\n"
          "\t--led-parallel=<parallel> : For A/B+ models or RPi2,3b: parallel "
          "chains. 1..3. Default: 1\n"
          "\t--led-pwm-bits=<1..11>    : PWM bits. Default: 11\n"
          "\t--led-brightness=<percent>: Brightness in percent. Default: 100.\n"
          "\t--led-drop-privs          : Drop privileges from 'root' after "
          "initializing the hardware.\n");
  if (show_daemon) {
    fprintf(out,
            "\t--led-daemon              :"
            "Make the process run in the background as daemon.\n");
  }
}

bool RGBMatrix::Options::Validate(std::string *err) {
  bool success = true;
  if (rows != 8 && rows != 16 && rows != 32 && rows != 64) {
    err->append("Invalid number or panel rows. "
                "Should be one of 8, 16, 32 or 64\n");
    success = false;
  }

  if (chain_length < 1) {
    err->append("Chain-length outside usable range\n");
    success = false;
  }

  if (parallel < 1 || parallel > 3) {
    err->append("Parallel outside usable range.\n");
    success = false;
  }

  if (brightness < 1 || brightness > 100) {
    err->append("Brightness is outside usable range.\n");
    success = false;
  }

  if (pwm_bits <= 0 || pwm_bits > 11) {
    err->append("Invalid range of pwm-bits\n");
    success = false;
  }

  return success;
}

}  // namespace rgb_matrix
