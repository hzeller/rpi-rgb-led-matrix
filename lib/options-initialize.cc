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

#include "multiplex-mappers-internal.h"

namespace rgb_matrix {
RuntimeOptions::RuntimeOptions() :
#ifdef RGB_SLOWDOWN_GPIO
  gpio_slowdown(RGB_SLOWDOWN_GPIO),
#else
  gpio_slowdown(1),
#endif
  daemon(0),            // Don't become a daemon by default.
  drop_privileges(1),    // Encourage good practice: drop privileges by default.
  do_gpio_init(true)
{
  // Nothing to see here.
}

namespace {
typedef char** argv_iterator;

#define OPTION_PREFIX     "--led-"
#define OPTION_PREFIX_LEN strlen(OPTION_PREFIX)

static bool ConsumeBoolFlag(const char *flag_name, const argv_iterator &pos,
                            bool *result_value) {
  const char *option = *pos;
  if (strncmp(option, OPTION_PREFIX, OPTION_PREFIX_LEN) != 0)
    return false;
  option += OPTION_PREFIX_LEN;
  bool value_to_set = true;
  if (strncmp(option, "no-", 3) == 0) {
    value_to_set = false;
    option += 3;
  }
  if (strcmp(option, flag_name) != 0)
    return false;  // not consumed.
  *result_value = value_to_set;
  return true;
}

static bool ConsumeIntFlag(const char *flag_name,
                           argv_iterator &pos, const argv_iterator end,
                           int *result_value, int *error) {
  const char *option = *pos;
  if (strncmp(option, OPTION_PREFIX, OPTION_PREFIX_LEN) != 0)
    return false;
  option += OPTION_PREFIX_LEN;
  const size_t flag_len = strlen(flag_name);
  if (strncmp(option, flag_name, flag_len) != 0)
    return false;  // not consumed.
  const char *value;
  if (option[flag_len] == '=')  // --option=42  # value in same arg
    value = option + flag_len + 1;
  else if (pos + 1 < end) {     // --option 42  # value in next arg
    value = *(++pos);
  } else {
    fprintf(stderr, "Parameter expected after %s%s\n",
            OPTION_PREFIX, flag_name);
    ++*error;
    return true;  // consumed, but error.
  }
  char *end_value = NULL;
  int val = strtol(value, &end_value, 10);
  if (!*value || *end_value) {
    fprintf(stderr, "Couldn't parse parameter %s%s=%s "
            "(Expected decimal number but '%s' looks funny)\n",
            OPTION_PREFIX, flag_name, value, end_value);
    ++*error;
    return true;  // consumed, but error
  }
  *result_value = val;
  return true;  // consumed.
}

// The resulting value is allocated.
static bool ConsumeStringFlag(const char *flag_name,
                              argv_iterator &pos, const argv_iterator end,
                              const char **result_value, int *error) {
  const char *option = *pos;
  if (strncmp(option, OPTION_PREFIX, OPTION_PREFIX_LEN) != 0)
    return false;
  option += OPTION_PREFIX_LEN;
  const size_t flag_len = strlen(flag_name);
  if (strncmp(option, flag_name, flag_len) != 0)
    return false;  // not consumed.
  const char *value;
  if (option[flag_len] == '=')  // --option=hello  # value in same arg
    value = option + flag_len + 1;
  else if (pos + 1 < end) {     // --option hello  # value in next arg
    value = *(++pos);
  } else {
    fprintf(stderr, "Parameter expected after %s%s\n",
            OPTION_PREFIX, flag_name);
    ++*error;
    *result_value = NULL;
    return true;  // consumed, but error.
  }
  *result_value = strdup(value);  // This will leak, but no big deal.
  return true;
}

static bool FlagInit(int &argc, char **&argv,
                     RGBMatrix::Options *mopts,
                     RuntimeOptions *ropts,
                     bool remove_consumed_options) {
  argv_iterator it = &argv[0];
  argv_iterator end = it + argc;

  std::vector<char*> unused_options;
  unused_options.push_back(*it++);  // Not interested in program name

  bool bool_scratch;
  int err = 0;
  bool posix_end_option_seen = false;  // end of options '--'
  for (/**/; it < end; ++it) {
    posix_end_option_seen |= (strcmp(*it, "--") == 0);
    if (!posix_end_option_seen) {
      if (ConsumeStringFlag("gpio-mapping", it, end,
                            &mopts->hardware_mapping, &err))
        continue;
      if (ConsumeStringFlag("rgb-sequence", it, end,
                            &mopts->led_rgb_sequence, &err))
        continue;
      if (ConsumeStringFlag("pixel-mapper", it, end,
                            &mopts->pixel_mapper_config, &err))
        continue;
      if (ConsumeStringFlag("panel-type", it, end,
                            &mopts->panel_type, &err))
        continue;
      if (ConsumeIntFlag("rows", it, end, &mopts->rows, &err))
        continue;
      if (ConsumeIntFlag("cols", it, end, &mopts->cols, &err))
        continue;
      if (ConsumeIntFlag("chain", it, end, &mopts->chain_length, &err))
        continue;
      if (ConsumeIntFlag("parallel", it, end, &mopts->parallel, &err))
        continue;
      if (ConsumeIntFlag("multiplexing", it, end, &mopts->multiplexing, &err))
        continue;
      if (ConsumeIntFlag("brightness", it, end, &mopts->brightness, &err))
        continue;
      if (ConsumeIntFlag("scan-mode", it, end, &mopts->scan_mode, &err))
        continue;
      if (ConsumeIntFlag("pwm-bits", it, end, &mopts->pwm_bits, &err))
        continue;
      if (ConsumeIntFlag("pwm-lsb-nanoseconds", it, end,
                         &mopts->pwm_lsb_nanoseconds, &err))
        continue;
      if (ConsumeIntFlag("pwm-dither-bits", it, end,
                         &mopts->pwm_dither_bits, &err))
        continue;
      if (ConsumeIntFlag("row-addr-type", it, end,
                         &mopts->row_address_type, &err))
        continue;
      if (ConsumeBoolFlag("show-refresh", it, &mopts->show_refresh_rate))
        continue;
      if (ConsumeBoolFlag("inverse", it, &mopts->inverse_colors))
        continue;
      // We don't have a swap_green_blue option anymore, but we simulate the
      // flag for a while.
      bool swap_green_blue;
      if (ConsumeBoolFlag("swap-green-blue", it, &swap_green_blue)) {
        if (strlen(mopts->led_rgb_sequence) == 3) {
          char *new_sequence = strdup(mopts->led_rgb_sequence);
          new_sequence[0] = mopts->led_rgb_sequence[0];
          new_sequence[1] = mopts->led_rgb_sequence[2];
          new_sequence[2] = mopts->led_rgb_sequence[1];
          mopts->led_rgb_sequence = new_sequence;  // leaking. Ignore.
        }
        continue;
      }
      bool allow_hardware_pulsing = !mopts->disable_hardware_pulsing;
      if (ConsumeBoolFlag("hardware-pulse", it, &allow_hardware_pulsing)) {
        mopts->disable_hardware_pulsing = !allow_hardware_pulsing;
        continue;
      }

      bool request_help = false;
      if (ConsumeBoolFlag("help", it, &request_help) && request_help) {
        // In that case, we pretend to have failure in parsing, which will
        // trigger printing the usage(). Typically :)
        return false;
      }

      //-- Runtime options.
      if (ConsumeIntFlag("slowdown-gpio", it, end, &ropts->gpio_slowdown, &err))
        continue;
      if (ropts->daemon >= 0 && ConsumeBoolFlag("daemon", it, &bool_scratch)) {
        ropts->daemon = bool_scratch ? 1 : 0;
        continue;
      }
      if (ropts->drop_privileges >= 0 &&
          ConsumeBoolFlag("drop-privs", it, &bool_scratch)) {
        ropts->drop_privileges = bool_scratch ? 1 : 0;
        continue;
      }
      if (strncmp(*it, OPTION_PREFIX, OPTION_PREFIX_LEN) == 0) {
        fprintf(stderr, "Option %s starts with %s but it is unknown. Typo?\n",
                *it, OPTION_PREFIX);
      }
    }
    unused_options.push_back(*it);
  }

  if (err > 0) {
    return false;
  }

  if (remove_consumed_options) {
    // Success. Re-arrange flags to only include the ones not consumed.
    argc = (int) unused_options.size();
    for (int i = 0; i < argc; ++i) {
      argv[i] = unused_options[i];
    }
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

bool ParseOptionsFromFlags(int *argc, char ***argv,
                           RGBMatrix::Options *m_opt_in,
                           RuntimeOptions *rt_opt_in,
                           bool remove_consumed_options) {
  // Replace NULL arguments with some scratch-space.
  RGBMatrix::Options scratch_matrix;
  RGBMatrix::Options *mopt = (m_opt_in != NULL) ? m_opt_in : &scratch_matrix;

  RuntimeOptions scratch_rt;
  RuntimeOptions *ropt = (rt_opt_in != NULL) ? rt_opt_in : &scratch_rt;

  return FlagInit(*argc, *argv, mopt, ropt, remove_consumed_options);
}

RGBMatrix *CreateMatrixFromOptions(const RGBMatrix::Options &options,
                                   const RuntimeOptions &runtime_options) {
  std::string error;
  if (!options.Validate(&error)) {
    fprintf(stderr, "%s\n", error.c_str());
    return NULL;
  }

  // For the Pi4, we might need 2, maybe up to 4. Let's open up to 5.
  if (runtime_options.gpio_slowdown < 0 || runtime_options.gpio_slowdown > 5) {
    fprintf(stderr, "--led-slowdown-gpio=%d is outside usable range\n",
            runtime_options.gpio_slowdown);
    return NULL;
  }

  static GPIO io;  // This static var is a little bit icky.
  if (runtime_options.do_gpio_init &&
      !io.Init(runtime_options.gpio_slowdown)) {
    fprintf(stderr, "Must run as root to be able to access /dev/mem\n"
            "Prepend 'sudo' to the command\n");
    return NULL;
  }

  if (runtime_options.daemon > 0 && daemon(1, 0) != 0) {
    perror("Failed to become daemon");
  }

  RGBMatrix *result = new RGBMatrix(NULL, options);
  // Allowing daemon also means we are allowed to start the thread now.
  const bool allow_daemon = !(runtime_options.daemon < 0);
  if (runtime_options.do_gpio_init)
    result->SetGPIO(&io, allow_daemon);

  // TODO(hzeller): if we disallow daemon, then we might also disallow
  // drop privileges: we can't drop privileges until we have created the
  // realtime thread that usually requires root to be established.
  // Double check and document.
  if (runtime_options.drop_privileges > 0) {
    drop_privs("daemon", "daemon");
  }

  return result;
}

static std::string CreateAvailableMultiplexString(
  const internal::MuxMapperList &m) {
  std::string result;
  char buffer[256];
  for (size_t i = 0; i < m.size(); ++i) {
    if (i != 0) result.append("; ");
    snprintf(buffer, sizeof(buffer), "%d=%s", (int) i+1, m[i]->GetName());
    result.append(buffer);
  }
  return result;
}

// Public interface.
RGBMatrix *CreateMatrixFromFlags(int *argc, char ***argv,
                                 RGBMatrix::Options *m_opt_in,
                                 RuntimeOptions *rt_opt_in,
                                 bool remove_consumed_options) {
  RGBMatrix::Options scratch_matrix;
  RGBMatrix::Options *mopt = (m_opt_in != NULL) ? m_opt_in : &scratch_matrix;

  RuntimeOptions scratch_rt;
  RuntimeOptions *ropt = (rt_opt_in != NULL) ? rt_opt_in : &scratch_rt;

  if (!ParseOptionsFromFlags(argc, argv, mopt, ropt, remove_consumed_options))
    return NULL;
  return CreateMatrixFromOptions(*mopt, *ropt);
}

void PrintMatrixFlags(FILE *out, const RGBMatrix::Options &d,
                      const RuntimeOptions &r) {
  const internal::MuxMapperList &muxers
    = internal::GetRegisteredMultiplexMappers();

  std::vector<std::string> mapper_names = GetAvailablePixelMappers();
  std::string available_mappers;
  for (size_t i = 0; i < mapper_names.size(); ++i) {
    if (i != 0) available_mappers.append(", ");
    available_mappers.append("\"").append(mapper_names[i]).append("\"");
  }

  fprintf(out,
          "\t--led-gpio-mapping=<name> : Name of GPIO mapping used. Default \"%s\"\n"
          "\t--led-rows=<rows>         : Panel rows. Typically 8, 16, 32 or 64."
          " (Default: %d).\n"
          "\t--led-cols=<cols>         : Panel columns. Typically 32 or 64. "
          "(Default: %d).\n"
          "\t--led-chain=<chained>     : Number of daisy-chained panels. "
          "(Default: %d).\n"
          "\t--led-parallel=<parallel> : Parallel chains. range=1..3 "
          "(Default: %d).\n"
          "\t--led-multiplexing=<0..%d> : Mux type: 0=direct; %s (Default: 0)\n"
          "\t--led-pixel-mapper        : Semicolon-separated list of pixel-mappers to arrange pixels.\n"
          "\t                            Optional params after a colon e.g. \"U-mapper;Rotate:90\"\n"
          "\t                            Available: %s. Default: \"\"\n"
          "\t--led-pwm-bits=<1..11>    : PWM bits (Default: %d).\n"
          "\t--led-brightness=<percent>: Brightness in percent (Default: %d).\n"
          "\t--led-scan-mode=<0..1>    : 0 = progressive; 1 = interlaced "
          "(Default: %d).\n"
          "\t--led-row-addr-type=<0..3>: 0 = default; 1 = AB-addressed panels; 2 = direct row select; 3 = ABC-addressed panels "
          "(Default: 0).\n"
          "\t--led-%sshow-refresh        : %show refresh rate.\n"
          "\t--led-%sinverse             "
          ": Switch if your matrix has inverse colors %s.\n"
          "\t--led-rgb-sequence        : Switch if your matrix has led colors "
          "swapped (Default: \"RGB\")\n"
          "\t--led-pwm-lsb-nanoseconds : PWM Nanoseconds for LSB "
          "(Default: %d)\n"
          "\t--led-pwm-dither-bits=<0..2> : Time dithering of lower bits "
          "(Default: 0)\n"
          "\t--led-%shardware-pulse   : %sse hardware pin-pulse generation.\n"
          "\t--led-panel-type=<name>   : Needed to initialize special panels. Supported: 'FM6126A'\n",
          d.hardware_mapping,
          d.rows, d.cols, d.chain_length, d.parallel,
          (int) muxers.size(), CreateAvailableMultiplexString(muxers).c_str(),
          available_mappers.c_str(),
          d.pwm_bits, d.brightness, d.scan_mode,
          d.show_refresh_rate ? "no-" : "", d.show_refresh_rate ? "Don't s" : "S",
          d.inverse_colors ? "no-" : "",    d.inverse_colors ? "off" : "on",
          d.pwm_lsb_nanoseconds,
          !d.disable_hardware_pulsing ? "no-" : "",
          !d.disable_hardware_pulsing ? "Don't u" : "U");

  fprintf(out, "\t--led-slowdown-gpio=<0..4>: "
          "Slowdown GPIO. Needed for faster Pis/slower panels "
          "(Default: %d).\n", r.gpio_slowdown);
  if (r.daemon >= 0) {
    const bool on = (r.daemon > 0);
    fprintf(out,
            "\t--led-%sdaemon              : "
            "%sake the process run in the background as daemon.\n",
            on ? "no-" : "", on ? "Don't m" : "M");
  }
  if (r.drop_privileges >= 0) {
    const bool on = (r.drop_privileges > 0);
    fprintf(out,
            "\t--led-%sdrop-privs       : %srop privileges from 'root' "
            "after initializing the hardware.\n",
            on ? "no-" : "", on ? "Don't d" : "D");
  }
}

bool RGBMatrix::Options::Validate(std::string *err_in) const {
  std::string scratch;
  std::string *err = err_in ? err_in : &scratch;
  bool success = true;
  if (rows < 8 || rows > 64 || rows % 2 != 0) {
    err->append("Invalid number or rows per panel (--led-rows). "
                "Should be in range of [8..64] and divisible by 2.\n");
    success = false;
  }

  if (cols < 16) {
    err->append("Invlid number of columns for panel (--led-cols). "
                "Typically that is something like 32 or 64\n");
    success = false;
  }

  if (chain_length < 1) {
    err->append("Chain-length outside usable range.\n");
    success = false;
  }

  const internal::MuxMapperList &muxers
    = internal::GetRegisteredMultiplexMappers();
  if (multiplexing < 0 || multiplexing > (int)muxers.size()) {
    err->append("Multiplexing can only be one of 0=normal; ")
      .append(CreateAvailableMultiplexString(muxers));
    success = false;
  }

  if (row_address_type < 0 || row_address_type > 3) {
    err->append("Row address type values can be 0 (default), 1 (AB addressing), 2 (direct row select), 3 ABC address.\n");
    success = false;
  }

  if (parallel < 1 || parallel > 3) {
    err->append("Parallel outside usable range (1..3 allowed).\n");
    success = false;
  }

  if (brightness < 1 || brightness > 100) {
    err->append("Brightness outside usable range (Percent 1..100 allowed).\n");
    success = false;
  }

  if (pwm_bits <= 0 || pwm_bits > 11) {
    err->append("Invalid range of pwm-bits (1..11 allowed).\n");
    success = false;
  }

  if (scan_mode < 0 || scan_mode > 1) {
    err->append("Invalid scan mode (0 or 1 allowed).\n");
    success = false;
  }

  if (pwm_lsb_nanoseconds < 50 || pwm_lsb_nanoseconds > 3000) {
    err->append("Invalid range of pwm-lsb-nanoseconds (50..3000 allowed).\n");
    success = false;
  }

  if (pwm_dither_bits < 0 || pwm_dither_bits > 2) {
    err->append("Inavlid range of pwm-dither-bits (0..2 allowed).\n");
    success = false;
  }

  if (led_rgb_sequence == NULL || strlen(led_rgb_sequence) != 3) {
    err->append("led-sequence needs to be three characters long.\n");
    success = false;
  } else {
    if ((!strchr(led_rgb_sequence, 'R') && !strchr(led_rgb_sequence, 'r'))
        || (!strchr(led_rgb_sequence, 'G') && !strchr(led_rgb_sequence, 'g'))
        || (!strchr(led_rgb_sequence, 'B') && !strchr(led_rgb_sequence, 'b'))) {
      err->append("led-sequence needs to contain all of letters 'R', 'G' "
                  "and 'B'\n");
      success = false;
    }
  }

  if (!success && !err_in) {
    // If we didn't get a string to write to, we write things to stderr.
    fprintf(stderr, "%s", err->c_str());
  }

  return success;
}

// Linker trick: is someone was linking the old library that didn't have the
// optional parameter defined, the linking will fail it wouldn't find the symbol
// with less parameters. But we don't want to clutter the header with simple
// delegation calls.
//
// So we define this symbol here and doing the delegation call until
// really everyone had recompiled their code with the new header.
//
// Should be removed in a couple of months (March 2017ish)
bool ParseOptionsFromFlags(int *argc, char ***argv,
                           RGBMatrix::Options *default_options,
                           RuntimeOptions *rt_options) {
  return ParseOptionsFromFlags(argc, argv, default_options, rt_options,
                               true);
}
RGBMatrix *CreateMatrixFromFlags(int *argc, char ***argv,
                                 RGBMatrix::Options *default_options,
                                 RuntimeOptions *default_rt_opts) {
  return CreateMatrixFromFlags(argc, argv, default_options, default_rt_opts,
                               true);
}

}  // namespace rgb_matrix
