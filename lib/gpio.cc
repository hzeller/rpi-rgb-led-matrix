// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
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

#include "gpio.h"

// Raspberry 1 and 2 have different base addresses for the periphery
#define BCM2708_PERI_BASE        0x20000000
#define BCM2709_PERI_BASE        0x3F000000

#define GPIO_REGISTER_OFFSET       0x200000

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define GPIO_REGISTER_BLOCK_SIZE (4*1024)

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio_port_+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_port_+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

/*static*/ const uint32_t ::rgb_matrix::GPIO::kValidBits 
= ((1 <<  0) | (1 <<  1) | // RPi 1 - Revision 1 accessible
   (1 <<  2) | (1 <<  3) | // RPi 1 - Revision 2 accessible
   (1 <<  4) | (1 <<  7) | (1 << 8) | (1 <<  9) |
   (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15)| (1 <<17) | (1 << 18) |
   (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25)| (1 << 27) |
   // support for A+/B+ and RPi2 with additional GPIO pins.
   (1 <<  5) | (1 <<  6) | (1 << 12) | (1 << 13) | (1 << 16) |
   (1 << 19) | (1 << 20) | (1 << 21) | (1 << 26)
);

namespace rgb_matrix {
GPIO::GPIO() : output_bits_(0), gpio_port_(NULL) {
}
   
uint32_t GPIO::InitOutputs(uint32_t outputs) {
  if (gpio_port_ == NULL) {
    fprintf(stderr, "Attempt to init outputs but not yet Init()-ialized.\n");
    return 0;
  }
  outputs &= kValidBits;   // Sanitize input.
  output_bits_ = outputs;
  for (uint32_t b = 0; b <= 27; ++b) {
    if (outputs & (1 << b)) {
      INP_GPIO(b);   // for writing, we first need to set as input.
      OUT_GPIO(b);
    }
  }
  return output_bits_;
}

static bool IsRaspberryPi2() {
  // TODO: there must be a better, more robust way. Can we ask the processor ?
  char buffer[2048];
  const int fd = open("/proc/cmdline", O_RDONLY);
  ssize_t r = read(fd, buffer, sizeof(buffer) - 1); // returns all in one read.
  buffer[r >= 0 ? r : 0] = '\0';
  close(fd);
  const char *mem_size_key;
  off_t mem_size;
  if ((mem_size_key = strstr(buffer, "mem_size=")) != NULL
      && (sscanf(mem_size_key + strlen("mem_size="), "%lx", &mem_size) == 1)
      && (mem_size == 0x3F000000)) {
    return true;
  }
  return false;
}

// Based on code example found in http://elinux.org/RPi_Low-level_peripherals
bool GPIO::Init() {
  int mem_fd;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    perror("can't open /dev/mem: ");
    return false;
  }

  const bool isRPI2 = IsRaspberryPi2();
  const off_t gpio_offset = (isRPI2 ? BCM2709_PERI_BASE : BCM2708_PERI_BASE)
    + GPIO_REGISTER_OFFSET;

  char *gpio_map =
    (char*) mmap(NULL,                     // Any adddress in our space will do
                 GPIO_REGISTER_BLOCK_SIZE, // Map length
                 PROT_READ|PROT_WRITE,     // Enable r/w on GPIO registers.
                 MAP_SHARED,
                 mem_fd,                   // File to map
                 gpio_offset               // Offset to GPIO peripheral
                 );
  close(mem_fd);

  if (gpio_map == MAP_FAILED) {
    fprintf(stderr, "mmap error %ld\n", (long)gpio_map);
    return false;
  }

  gpio_port_ = (volatile uint32_t *)gpio_map;

  Timers::Init(); // Will call IsRaspberryPi2() again, but not time critical.
  return true;
}

// ----------
// TODO: timing needs to be improved. It is jittery due to the nature of running
// in a non-realtime operating system, and apparently the nanosleep() does not
// make any effort to even be close to accurate. Here are some half-ass
// implementations that are choosen depending on the platform - but they are
// still lacking. In particular in darker areas in full 11bit PWM, there are
// brightness glitches.
//
// Various ideas:
//   - use build-in timers, e.g. RPi2 apparently has one at 0x3F003000
//   - use CPU cycle counter (probably not available in user-space though)
//     for accurate time measurement, then use regular nanosleep() to inch
//     towards the time, then use busy loop to get there.
//   - reconsider DMA. DMA proofed to be much slower than direct GPIO access
//     in the past, but if it can give more predictable timing, maybe it is
//     worth investigating that.
// ----------


// We use different implementations to work around somewhat messed up nanosleep
// on linux. We choose the actual implementation when we set up GPIOs.
static void sleep_nanos_rpi_1(long nanos);
static void sleep_nanos_rpi_2(long nanos);
static void sleep_nanos_rpi_2_forcebusyloop(long nanos);
static void (*sleep_impl)(long) = sleep_nanos_rpi_1;

void Timers::Init(bool experimental_low_jitter) {
  const bool isRPI2 = IsRaspberryPi2();
  if (isRPI2) {
    if (experimental_low_jitter)
      sleep_impl = sleep_nanos_rpi_2_forcebusyloop;  // (yeah, ick)
    else
      sleep_impl = sleep_nanos_rpi_2;
  } else {
    sleep_impl = sleep_nanos_rpi_1;
  }
}

void Timers::sleep_nanos(long nanos) {
  sleep_impl(nanos);
}

static void sleep_nanos_rpi_1(long nanos) {
  // For sleep times above 20usec, nanosleep seems to be fine, but it has
  // an offset of about 20usec (on the RPi distribution I was testing it on).
  // That means, we need to give it 80us to get 100us.
  // For values lower than roughly 30us, this is not accurate anymore and we
  // need to switch to busy wait.
  // TODO: compile Linux kernel realtime extensions and watch if the offset-time
  // changes and hope for less jitter.
  if (nanos > 28000) {
    struct timespec sleep_time = { 0, nanos - 20000 };
    nanosleep(&sleep_time, NULL);
  } else {
    // The following loop is determined empirically on a 700Mhz RPi
    for (uint32_t i = (nanos - 70) >> 2; i != 0; --i) {
      asm("nop");
    }
  }
}

static void sleep_nanos_rpi_2(long nanos) {
  // For sleep times above 20usec, nanosleep seems to be fine, but it has
  // an offset of about 15usec.
  // TODO: Play around with built-in timers of CPU, like the one at 0x3F003000
  // TODO: also, we might trick linux into giving us a dedicated core.
  if (nanos > 28000) {
    struct timespec sleep_time = { 0, nanos - 15000 };
    nanosleep(&sleep_time, NULL);
  } else {
    // The following loop is determined empirically on a 900Mhz RPi 2
    for (uint32_t i = (nanos - 20) * 100 / 110; i != 0; --i) {
      asm("");
    }
  }
}

// On RPi2, we can choose to be wasteful and essentially dedicate a full
// core to this task (TODO: this should probably access the CPU counter, but
// it might not be available in user-space. Verify).
static void sleep_nanos_rpi_2_forcebusyloop(long nanos) {
  // The following loop is determined empirically on a 900Mhz RPi 2
  for (uint32_t i = (nanos - 20) * 100 / 110; i != 0; --i) {
    asm("");
  }
}

} // namespace rgb_matrix
