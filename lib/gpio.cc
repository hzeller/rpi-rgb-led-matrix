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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "gpio.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/*
 * nanosleep() takes longer than requested because of OS jitter.
 * In about 99.9% of the cases, this is <= 25 microcseconds on
 * the Raspberry Pi (empirically determined with a Raspbian kernel), so
 * we substract this value whenever we do nanosleep(); the remaining time
 * we then busy wait to get a good accurate result.
 *
 * You can measure the overhead using DEBUG_SLEEP_JITTER below.
 *
 * Note: A higher value here will result in more CPU use because of more busy
 * waiting inching towards the real value (for all the cases that nanosleep()
 * actually was better than this overhead).
 *
 * This might be interesting to tweak in particular if you have a realtime
 * kernel with different characteristics.
 */
#define EMPIRICAL_NANOSLEEP_OVERHEAD_US 25

/*
 * In few cases on a standard kernel, we see that the overhead is actually
 * even longer; these additional 35usec cover up for the 99.999%-ile.
 * So ideally, we always use these additional time and also busy-wait them,
 * right ?
 * However, that would take away a lot of CPU on older, one-core Raspberry Pis
 * or Pi Zeros. They rely for us to sleep when possible for it to do work.
 * So we only enable it, if we have have a newer Pi where we anyway burn
 * away on one core (And are isolated there with isolcpus=3).
 */
#define EMPIRICAL_NANOSLEEP_EXTRA_OVERHEAD_US 35

/* In order to determine useful values for above, set this to 1 and use the
 * hardware pin-pulser.
 * It will output a histogram atexit() of how much how often we were over
 * the requested time.
 * (The full histogram will be shifted by the EMPIRICAL_NANOSLEEP_OVERHEAD_US
 *  value above. To get a full histogram of OS overhead, set it to 0 first).
 */
#define DEBUG_SLEEP_JITTER 0

// Raspberry 1 and 2 have different base addresses for the periphery
#define BCM2708_PERI_BASE        0x20000000
#define BCM2709_PERI_BASE        0x3F000000

#define GPIO_REGISTER_OFFSET         0x200000
#define COUNTER_1Mhz_REGISTER_OFFSET   0x3000

#define GPIO_PWM_BASE_OFFSET	(GPIO_REGISTER_OFFSET + 0xC000)
#define GPIO_CLK_BASE_OFFSET	0x101000

#define REGISTER_BLOCK_SIZE (4*1024)

#define PWM_CTL      (0x00 / 4)
#define PWM_STA      (0x04 / 4)
#define PWM_RNG1     (0x10 / 4)
#define PWM_FIFO     (0x18 / 4)

#define PWM_CTL_CLRF1 (1<<6)	// CH1 Clear Fifo (1 Clears FIFO 0 has no effect)
#define PWM_CTL_USEF1 (1<<5)	// CH1 Use Fifo (0=data reg transmit 1=Fifo used for transmission)
#define PWM_CTL_POLA1 (1<<4)	// CH1 Polarity (0=(0=low 1=high) 1=(1=low 0=high)
#define PWM_CTL_SBIT1 (1<<3)	// CH1 Silence Bit (state of output when 0 transmission takes place)
#define PWM_CTL_MODE1 (1<<1)	// CH1 Mode (0=pwm 1=serialiser mode)
#define PWM_CTL_PWEN1 (1<<0)	// CH1 Enable (0=disable 1=enable)

#define PWM_STA_EMPT1 (1<<1)
#define PWM_STA_FULL1 (1<<0)

#define CLK_PASSWD  (0x5A<<24)

#define CLK_CTL_MASH(x)((x)<<9)
#define CLK_CTL_BUSY    (1 <<7)
#define CLK_CTL_KILL    (1 <<5)
#define CLK_CTL_ENAB    (1 <<4)
#define CLK_CTL_SRC(x) ((x)<<0)

#define CLK_CTL_SRC_PLLD 6  /* 500.0 MHz */

#define CLK_DIV_DIVI(x) ((x)<<12)
#define CLK_DIV_DIVF(x) ((x)<< 0)

#define CLK_PWMCTL 40
#define CLK_PWMDIV 41

// We want to have the last word in the fifo free
#define MAX_PWM_BIT_USE 224
#define PWM_BASE_TIME_NS 2

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x).
#define INP_GPIO(g) *(gpio_port_+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_port_+((g)/10)) |=  (1<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

namespace rgb_matrix {
/*static*/ const uint32_t GPIO::kValidBits
= ((1 <<  0) | (1 <<  1) | // RPi 1 - Revision 1 accessible
   (1 <<  2) | (1 <<  3) | // RPi 1 - Revision 2 accessible
   (1 <<  4) | (1 <<  7) | (1 << 8) | (1 <<  9) |
   (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15)| (1 <<17) | (1 << 18) |
   (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25)| (1 << 27) |
   // support for A+/B+ and RPi2 with additional GPIO pins.
   (1 <<  5) | (1 <<  6) | (1 << 12) | (1 << 13) | (1 << 16) |
   (1 << 19) | (1 << 20) | (1 << 21) | (1 << 26)
);

GPIO::GPIO() : output_bits_(0), input_bits_(0), reserved_bits_(0),
               slowdown_(1), gpio_port_(NULL) {
}

uint32_t GPIO::InitOutputs(uint32_t outputs,
                           bool adafruit_pwm_transition_hack_needed) {
  if (gpio_port_ == NULL) {
    fprintf(stderr, "Attempt to init outputs but not yet Init()-ialized.\n");
    return 0;
  }

  // Hack: for the PWM mod, the user soldered together GPIO 18 (new OE)
  // with GPIO 4 (old OE).
  // Since they are connected inside the HAT, want to make extra sure that,
  // whatever the outside system set as pinmux, the old OE is _not_ also
  // set as output so that these GPIO outputs don't fight each other.
  //
  // So explicitly set both of these pins as input initially, so the user
  // can switch between the two modes "adafruit-hat" and "adafruit-hat-pwm"
  // without trouble.
  if (adafruit_pwm_transition_hack_needed) {
    INP_GPIO(4);
    INP_GPIO(18);
    // Even with PWM enabled, GPIO4 still can not be used, because it is
    // now connected to the GPIO18 and thus must stay an input.
    // So reserve this bit if it is not set in outputs.
    reserved_bits_ = (1<<4) & ~outputs;
  }

  outputs &= kValidBits;     // Sanitize: only bits on GPIO header allowed.
  outputs &= ~(output_bits_ | input_bits_ | reserved_bits_);
  for (uint32_t b = 0; b <= 27; ++b) {
    if (outputs & (1 << b)) {
      INP_GPIO(b);   // for writing, we first need to set as input.
      OUT_GPIO(b);
    }
  }
  output_bits_ |= outputs;
  return outputs;
}

uint32_t GPIO::RequestInputs(uint32_t inputs) {
  if (gpio_port_ == NULL) {
    fprintf(stderr, "Attempt to init inputs but not yet Init()-ialized.\n");
    return 0;
  }

  inputs &= kValidBits;     // Sanitize: only bits on GPIO header allowed.
  inputs &= ~(output_bits_ | input_bits_ | reserved_bits_);
  for (uint32_t b = 0; b <= 27; ++b) {
    if (inputs & (1 << b)) {
      INP_GPIO(b);
    }
  }
  input_bits_ |= inputs;
  return inputs;
}

static bool DetermineIsRaspberryPi2() {
  // TODO: there must be a better, more robust way. Can we ask the processor ?
  char buffer[2048];
  const int fd = open("/proc/cmdline", O_RDONLY);
  ssize_t r = read(fd, buffer, sizeof(buffer) - 1); // returns all in one read.
  buffer[r >= 0 ? r : 0] = '\0';
  close(fd);
  const char *mem_size_key;
  uint64_t mem_size = 0;
  if ((mem_size_key = strstr(buffer, "mem_size=")) != NULL
      && (sscanf(mem_size_key + strlen("mem_size="), "%" PRIx64, &mem_size) == 1)
      && (mem_size >= 0x3F000000)) {
    return true;
  }
  return false;
}

static bool IsRaspberryPi2() {
  static bool ispi2 = DetermineIsRaspberryPi2();
  return ispi2;
}

static uint32_t JitterAllowanceMicroseconds() {
  // If this is a Raspberry Pi2 or 3, we can allow to burn a bit more busy-wait
  // CPU cycles to get the timing accurate as we have more CPU to spare.
  static int allowance_us = EMPIRICAL_NANOSLEEP_OVERHEAD_US
    + (IsRaspberryPi2() ? EMPIRICAL_NANOSLEEP_EXTRA_OVERHEAD_US : 0);
  return allowance_us;
}

static uint32_t *mmap_bcm_register(bool isRPi2, off_t register_offset) {
  const off_t base = (isRPi2 ? BCM2709_PERI_BASE : BCM2708_PERI_BASE);

  int mem_fd;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    perror("can't open /dev/mem: ");
    return NULL;
  }

  uint32_t *result =
    (uint32_t*) mmap(NULL,                  // Any adddress in our space will do
                     REGISTER_BLOCK_SIZE,   // Map length
                     PROT_READ|PROT_WRITE,  // Enable r/w on GPIO registers.
                     MAP_SHARED,
                     mem_fd,                // File to map
                     base + register_offset // Offset to bcm register
                     );
  close(mem_fd);

  if (result == MAP_FAILED) {
    perror("mmap error: ");
    fprintf(stderr, "%s: MMapping from base 0x%lx, offset 0x%lx\n",
            isRPi2 ? "RPi2,3" : "RPi1", base, register_offset);
    return NULL;
  }
  return result;
}

// Based on code example found in http://elinux.org/RPi_Low-level_peripherals
bool GPIO::Init(int slowdown) {
  slowdown_ = slowdown;
  gpio_port_ = mmap_bcm_register(IsRaspberryPi2(), GPIO_REGISTER_OFFSET);
  if (gpio_port_ == NULL) {
    return false;
  }
  gpio_set_bits_ = gpio_port_ + (0x1C / sizeof(uint32_t));
  gpio_clr_bits_ = gpio_port_ + (0x28 / sizeof(uint32_t));
  gpio_read_bits_ = gpio_port_ + (0x34 / sizeof(uint32_t));
  return true;
}

/*
 * We support also other pinouts that don't have the OE- on the hardware
 * PWM output pin, so we need to provide (impefect) 'manual' timing as well.
 * Hence all various sleep_nano() implementations depending on the hardware.
 */

// --- PinPulser. Private implementation parts.
namespace {
// Manual timers.
class Timers {
public:
  static bool Init();
  static void sleep_nanos(long t);
};

// Simplest of PinPulsers. Uses somewhat jittery and manual timers
// to get the timing, but not optimal.
class TimerBasedPinPulser : public PinPulser {
public:
  TimerBasedPinPulser(GPIO *io, uint32_t bits,
                      const std::vector<int> &nano_specs)
    : io_(io), bits_(bits), nano_specs_(nano_specs) {}

  virtual void SendPulse(int time_spec_number) {
    io_->ClearBits(bits_);
    Timers::sleep_nanos(nano_specs_[time_spec_number]);
    io_->SetBits(bits_);
  }

private:
  GPIO *const io_;
  const uint32_t bits_;
  const std::vector<int> nano_specs_;
};

static bool LinuxHasModuleLoaded(const char *name) {
  FILE *f = fopen("/proc/modules", "r");
  if (f == NULL) return false; // don't care.
  char buf[256];
  const size_t namelen = strlen(name);
  bool found = false;
  while (fgets(buf, sizeof(buf), f) != NULL) {
    if (strncmp(buf, name, namelen) == 0) {
      found = true;
      break;
    }
  }
  fclose(f);
  return found;
}

static volatile uint32_t *timer1Mhz = NULL;

static void sleep_nanos_rpi_1(long nanos);
static void sleep_nanos_rpi_2(long nanos);
static void (*busy_sleep_impl)(long) = sleep_nanos_rpi_1;

// By default, the kernel applies some throtteling for realtime
// threads to prevent starvation of non-RT threads. But we
// really want all we can get iff the machine has more cores and
// our RT-thread is locked onto one of these.
// So let's tell it not to do that.
static void DisableRealtimeThrottling() {
  if (!IsRaspberryPi2()) return;   // Not safe if we don't have > 1 core.
  const int out = open("/proc/sys/kernel/sched_rt_runtime_us", O_WRONLY);
  if (out < 0) return;
  write(out, "-1", 2);
  close(out);
}

bool Timers::Init() {
  const bool isRPi2 = IsRaspberryPi2();
  uint32_t *timereg = mmap_bcm_register(isRPi2, COUNTER_1Mhz_REGISTER_OFFSET);
  if (timereg == NULL) {
    return false;
  }
  timer1Mhz = timereg + 1;

  busy_sleep_impl = isRPi2 ? sleep_nanos_rpi_2 : sleep_nanos_rpi_1;
  if (isRPi2) DisableRealtimeThrottling();
  return true;
}

void Timers::sleep_nanos(long nanos) {
  // For smaller durations, we go straight to busy wait.

  // For larger duration, we use nanosleep() to give the operating system
  // a chance to do something else.
  // However, these timings have a lot of jitter, so we do a two way
  // approach: we use nanosleep(), but for some shorter time period so
  // that we can tolerate some jitter (also, we need at least an offset of
  // EMPIRICAL_NANOSLEEP_OVERHEAD_US as the nanosleep implementations on RPi
  // actually have such offset).
  //
  // We use the global 1Mhz hardware timer to measure the actual time period
  // that has passed, and then inch forward for the remaining time with
  // busy wait.
  static long kJitterAllowanceNanos = JitterAllowanceMicroseconds() * 1000;
  if (nanos > kJitterAllowanceNanos + 5000) {
    const uint32_t before = *timer1Mhz;
    struct timespec sleep_time
      = { 0, nanos - kJitterAllowanceNanos };
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

#if DEBUG_SLEEP_JITTER
static int overshoot_histogram_us[256] = {0};
static void print_overshoot_histogram() {
  fprintf(stderr, "Overshoot histogram >= empirical overhead of %dus\n"
          "%6s | %7s | %7s\n",
          JitterAllowanceMicroseconds(), "usec", "count", "accum");
  int total_count = 0;
  for (int i = 0; i < 256; ++i) total_count += overshoot_histogram_us[i];
  int running_count = 0;
  for (int us = 0; us < 256; ++us) {
    const int count = overshoot_histogram_us[us];
    if (count > 0) {
      running_count += count;
      fprintf(stderr, "%s%3dus: %8d %7.3f%%\n", (us == 0) ? "<=" : " +",
              us, count, 100.0 * running_count / total_count);
    }
  }
}
#endif

// A PinPulser that uses the PWM hardware to create accurate pulses.
// It only works on GPIO-18 though.
class HardwarePinPulser : public PinPulser {
public:
  static bool CanHandle(uint32_t gpio_mask) {
#ifdef DISABLE_HARDWARE_PULSES
    return false;
#else
    return gpio_mask == (1 << 18) || gpio_mask == (1 << 12);
#endif
  }

  HardwarePinPulser(uint32_t pins, const std::vector<int> &specs)
    : triggered_(false) {
    assert(CanHandle(pins));
#if DEBUG_SLEEP_JITTER
    atexit(print_overshoot_histogram);
#endif

    if (LinuxHasModuleLoaded("snd_bcm2835")) {
      fprintf(stderr,
              "\n%s=== snd_bcm2835: found that the Pi sound module is loaded. ===%s\n"
              "Don't use the built-in sound of the Pi together with this lib; it is known to be\n"
	      "incompatible and cause trouble and hangs (you can still use external USB sound adapters).\n\n"
              "See Troubleshooting section in README how to disable the sound module.\n"
	      "You can also run with --led-no-hardware-pulse to avoid the incompatibility,\n"
	      "but you will have more flicker.\n"
              "Exiting; fix the above first or use --led-no-hardware-pulse\n\n",
              "\033[1;31m", "\033[0m");
      exit(1);
    }

    for (size_t i = 0; i < specs.size(); ++i) {
      // Hints how long to nanosleep, already corrected for system overhead.
      sleep_hints_.push_back(specs[i] / 1000 - JitterAllowanceMicroseconds());
    }

    const int base = specs[0];
    // Get relevant registers
    const bool isPI2 = IsRaspberryPi2();
    volatile uint32_t *gpioReg = mmap_bcm_register(isPI2, GPIO_REGISTER_OFFSET);
    pwm_reg_  = mmap_bcm_register(isPI2, GPIO_PWM_BASE_OFFSET);
    clk_reg_  = mmap_bcm_register(isPI2, GPIO_CLK_BASE_OFFSET);
    fifo_ = pwm_reg_ + PWM_FIFO;
    assert((clk_reg_ != NULL) && (pwm_reg_ != NULL));  // init error.

    if (pins == (1<<18)) {
      SetGPIOMode(gpioReg, 18, 2); // set GPIO 18 to PWM0 mode (Alternative 5)
    } else if (pins == (1<<12)) {
      SetGPIOMode(gpioReg, 12, 4); // set GPIO 12 to PWM0 mode (Alternative 0)
    } else {
      assert(false); // should've been caught by CanHandle()
    }
    InitPWMDivider((base/2) / PWM_BASE_TIME_NS);
    for (size_t i = 0; i < specs.size(); ++i) {
      pwm_range_.push_back(2 * specs[i] / base);
    }
  }

  virtual void SendPulse(int c) {
    if (pwm_range_[c] < 16) {
      pwm_reg_[PWM_RNG1] = pwm_range_[c];

      *fifo_ = pwm_range_[c];
    } else {
      // Keep the actual range as short as possible, as we have to
      // wait for one full period of these in the zero phase.
      // The hardware can't deal with values < 2, so only do this when
      // have enough of these.
      pwm_reg_[PWM_RNG1] = pwm_range_[c] / 8;

      *fifo_ = pwm_range_[c] / 8;
      *fifo_ = pwm_range_[c] / 8;
      *fifo_ = pwm_range_[c] / 8;
      *fifo_ = pwm_range_[c] / 8;
      *fifo_ = pwm_range_[c] / 8;
      *fifo_ = pwm_range_[c] / 8;
      *fifo_ = pwm_range_[c] / 8;
      *fifo_ = pwm_range_[c] / 8;
    }

    /*
     * We need one value at the end to have it go back to
     * default state (otherwise it just repeats the last
     * value, so will be constantly 'on').
     */
    *fifo_ = 0;   // sentinel.

    /*
     * For some reason, we need a second empty sentinel in the
     * fifo, otherwise our way to detect the end of the pulse,
     * which relies on 'is the queue empty' does not work. It is
     * not entirely clear why that is from the datasheet,
     * but probably there is some buffering register in which data
     * elements are kept after the fifo is emptied.
     */
    *fifo_ = 0;

    sleep_hint_ = sleep_hints_[c];
    start_time_ = *timer1Mhz;
    triggered_ = true;
    pwm_reg_[PWM_CTL] = PWM_CTL_USEF1 | PWM_CTL_PWEN1 | PWM_CTL_POLA1;
  }

  virtual void WaitPulseFinished() {
    if (!triggered_) return;
    // Determine how long we already spent and sleep to get close to the
    // actual end-time of our sleep period.
    //
    // TODO(hzeller): find if it is possible to get some sort of interrupt from
    //   the hardware once it is done with the pulse. Sounds silly that there is
    //   not.
    if (sleep_hint_ > 0) {
      const uint32_t already_elapsed_usec = *timer1Mhz - start_time_;
      const int to_sleep = sleep_hint_ - already_elapsed_usec;
      if (to_sleep > 0) {
        struct timespec sleep_time = { 0, 1000 * to_sleep };
        nanosleep(&sleep_time, NULL);

#if DEBUG_SLEEP_JITTER
        {
          // Record histogram of realtime jitter how much longer we actually
          // took.
          const int total_us = *timer1Mhz - start_time_;
          const int nanoslept = total_us - already_elapsed_usec;
          int overshoot = nanoslept - (to_sleep + JitterAllowanceMicroseconds());
          if (overshoot < 0) overshoot = 0;
          if (overshoot > 255) overshoot = 255;
          overshoot_histogram_us[overshoot]++;
        }
#endif
      }
    }

    while ((pwm_reg_[PWM_STA] & PWM_STA_EMPT1) == 0) {
      // busy wait until done.
    }
    pwm_reg_[PWM_CTL] = PWM_CTL_USEF1 | PWM_CTL_POLA1 | PWM_CTL_CLRF1;
    triggered_ = false;
  }

private:
  void SetGPIOMode(volatile uint32_t *gpioReg, unsigned gpio, unsigned mode) {
    const int reg = gpio / 10;
    const int mode_pos = (gpio % 10) * 3;
    gpioReg[reg] = (gpioReg[reg] & ~(7 << mode_pos)) | (mode << mode_pos);
  }

  void InitPWMDivider(uint32_t divider) {
    assert(divider < (1<<12));  // we only have 12 bits.

    pwm_reg_[PWM_CTL] = PWM_CTL_USEF1 | PWM_CTL_POLA1 | PWM_CTL_CLRF1;

    // reset PWM clock
    clk_reg_[CLK_PWMCTL] = CLK_PASSWD | CLK_CTL_KILL;

    // set PWM clock source as 500 MHz PLLD
    clk_reg_[CLK_PWMCTL] = CLK_PASSWD | CLK_CTL_SRC(CLK_CTL_SRC_PLLD);

    // set PWM clock divider
    clk_reg_[CLK_PWMDIV] = CLK_PASSWD | CLK_DIV_DIVI(divider) | CLK_DIV_DIVF(0);

    // enable PWM clock
    clk_reg_[CLK_PWMCTL] = CLK_PASSWD | CLK_CTL_ENAB | CLK_CTL_SRC(CLK_CTL_SRC_PLLD);
  }

private:
  std::vector<uint32_t> pwm_range_;
  std::vector<int> sleep_hints_;
  volatile uint32_t *pwm_reg_;
  volatile uint32_t *fifo_;
  volatile uint32_t *clk_reg_;
  uint32_t start_time_;
  int sleep_hint_;
  bool triggered_;
};

} // end anonymous namespace

// Public PinPulser factory
PinPulser *PinPulser::Create(GPIO *io, uint32_t gpio_mask,
                             bool allow_hardware_pulsing,
                             const std::vector<int> &nano_wait_spec) {
  if (!Timers::Init()) return NULL;
  if (allow_hardware_pulsing && HardwarePinPulser::CanHandle(gpio_mask)) {
    return new HardwarePinPulser(gpio_mask, nano_wait_spec);
  } else {
    return new TimerBasedPinPulser(io, gpio_mask, nano_wait_spec);
  }
}

uint32_t GetMicrosecondCounter() {
  return timer1Mhz ? *timer1Mhz : 0;
}

} // namespace rgb_matrix
