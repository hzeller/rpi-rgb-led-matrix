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

#define GPIO_REGISTER_OFFSET         0x200000
#define COUNTER_1Mhz_REGISTER_OFFSET   0x3000

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define REGISTER_BLOCK_SIZE (4*1024)

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

static char *mmap_bcm_register(off_t register_offset) {
  const bool isRPI2 = IsRaspberryPi2();
  const off_t base = (isRPI2 ? BCM2709_PERI_BASE : BCM2708_PERI_BASE);

  int mem_fd;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    perror("can't open /dev/mem: ");
    return NULL;
  }

  char *result =
    (char*) mmap(NULL,                  // Any adddress in our space will do
                 REGISTER_BLOCK_SIZE,   // Map length
                 PROT_READ|PROT_WRITE,  // Enable r/w on GPIO registers.
                 MAP_SHARED,
                 mem_fd,                // File to map
                 base + register_offset // Offset to bcm register
                 );
  close(mem_fd);

  if (result == MAP_FAILED) {
    fprintf(stderr, "mmap error %p\n", result);
  }
  return (char*) result;
}

// Based on code example found in http://elinux.org/RPi_Low-level_peripherals
bool GPIO::Init() {
  char *gpio_map = mmap_bcm_register(GPIO_REGISTER_OFFSET);
  if (gpio_map == NULL) {
    return false;
  }

  gpio_port_ = (volatile uint32_t *)gpio_map;

  // To be compatible with old code, make sure to initialize this here.
  return Timers::Init();
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

static volatile uint32_t *timer1Mhz = NULL;

static void sleep_nanos_rpi_1(long nanos);
static void sleep_nanos_rpi_2(long nanos);
static void (*busy_sleep_impl)(long) = sleep_nanos_rpi_1;

bool Timers::Init() {
  char *timer_map = mmap_bcm_register(COUNTER_1Mhz_REGISTER_OFFSET);
  if (timer_map == NULL) {
    return false;
  }
  timer1Mhz = &((volatile uint32_t *)timer_map)[1];

  const bool isRPI2 = IsRaspberryPi2();
  busy_sleep_impl = isRPI2 ? sleep_nanos_rpi_2 : sleep_nanos_rpi_1;
  return true;
}

void Timers::sleep_nanos(long nanos) {
  // For smaller durations, we go straight to busy wait.

  // For larger duration, we use nanosleep() to give the operating system
  // a chance to do something else.
  // However, these timings have a lot of jitter, so we do a two way
  // approach: we use nanosleep(), but not for a shorter time period so
  // that we can tolerate some jitter (we need at least an offset of 20usec
  // as the nanosleep implementations on RPi actually have such offset).
  //
  // We use the global 1Mhz hardware timer to measure the actual time period
  // that has passed, and then inch forward for the remaining time with
  // busy wait.
  if (nanos > 30000) {
    const uint32_t before = *timer1Mhz;
    struct timespec sleep_time = { 0, nanos - 25000 };
    nanosleep(&sleep_time, NULL);
    const uint32_t after = *timer1Mhz;
    const long nanoseconds_passed = 1000 * (uint32_t)(after - before);
    if (nanoseconds_passed > nanos) {
      return;  // darn, missed it.
    } else {
      nanos -= nanoseconds_passed; // remaining time with busy-loop
    }
  }

  busy_sleep_impl(nanos);
}

static void sleep_nanos_rpi_1(long nanos) {
  if (nanos < 70) return;
  // The following loop is determined empirically on a 700Mhz RPi
  for (uint32_t i = (nanos - 70) >> 2; i != 0; --i) {
    asm("nop");
  }
}

static void sleep_nanos_rpi_2(long nanos) {
  if (nanos < 20) return;
  // The following loop is determined empirically on a 900Mhz RPi 2
  for (uint32_t i = (nanos - 20) * 100 / 110; i != 0; --i) {
    asm("");
  }
}

} // namespace rgb_matrix
