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

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

// Raspberry 1 and 2 have different base addresses for the periphery
#define BCM2708_PERI_BASE        0x20000000
#define BCM2709_PERI_BASE        0x3F000000

#define GPIO_REGISTER_OFFSET         0x200000
#define COUNTER_1Mhz_REGISTER_OFFSET   0x3000

#define GPIO_PWM_BASE_OFFSET	(GPIO_REGISTER_OFFSET + 0xC000)
#define GPIO_CLK_BASE_OFFSET	0x101000

#define REGISTER_BLOCK_SIZE (4*1024)

#define PWM_CTL      0
#define PWM_STA      1
#define PWM_FIFO     6

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
    fprintf(stderr, "mmap error %p\n", result);
    return NULL;
  }
  return result;
}

// Based on code example found in http://elinux.org/RPi_Low-level_peripherals
bool GPIO::Init() {
  gpio_port_ = mmap_bcm_register(IsRaspberryPi2(), GPIO_REGISTER_OFFSET);
  if (gpio_port_ == NULL) {
    return false;
  }
  gpio_set_bits_ = gpio_port_ + (0x1C / sizeof(uint32_t));
  gpio_clr_bits_ = gpio_port_ + (0x28 / sizeof(uint32_t));
  return true;
}

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

// ----------
// TODO: timing needs to be improved. It is jittery due to the nature of running
// in a non-realtime operating system, and apparently the nanosleep() does not
// make any effort to even be close to accurate.
// ----------

static volatile uint32_t *timer1Mhz = NULL;

static void sleep_nanos_rpi_1(long nanos);
static void sleep_nanos_rpi_2(long nanos);
static void (*busy_sleep_impl)(long) = sleep_nanos_rpi_1;

bool Timers::Init() {
  const bool isRPi2 = IsRaspberryPi2();
  uint32_t *timereg = mmap_bcm_register(isRPi2, COUNTER_1Mhz_REGISTER_OFFSET);
  if (timereg == NULL) {
    return false;
  }
  timer1Mhz = &timereg[1];

  busy_sleep_impl = isRPi2 ? sleep_nanos_rpi_2 : sleep_nanos_rpi_1;
  return true;
}

void Timers::sleep_nanos(long nanos) {
  // For smaller durations, we go straight to busy wait.

  // For larger duration, we use nanosleep() to give the operating system
  // a chance to do something else.
  // However, these timings have a lot of jitter, so we do a two way
  // approach: we use nanosleep(), but for some shorter time period so
  // that we can tolerate some jitter (also, we need at least an offset of
  // 20usec as the nanosleep implementations on RPi actually have such offset).
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

/*
 TODO hardware timing: current observation:
   - More CPU use and less refresh rate than 'regular' (USE_HARDWARE_PWM_TIMER=0)
   - Darker, i.e more setup time in-between (= more dark time) (less current use)
   - Lower refresh rate
   - The usleep() are annoying. Ideally, we can wait for the pulsing to finish
     with some interrupt.
   - usleep() in PWM setup is needed to get the registers settled, but it is not
     clear what an ideal version would be. Ideally, we never switch the baseline
     but rather write to the fifo more bits (but: it is too short and we
     shouldn't wait.
   - More work needed.
*/

// A PinPulser that uses the PWM hardware to create accurate pulses.
// It only works on GPIO-18 though.
// Based in part on discussion found on
// https://www.raspberrypi.org/forums/viewtopic.php?t=67741&p=494768
//
// TODO: - divider switching is very slow.
class HardwarePinPulser : public PinPulser {
public:
  static bool CanHandle(uint32_t gpio_mask) { return gpio_mask == (1 << 18); }

  HardwarePinPulser(uint32_t pins, const std::vector<int> &specs)
    : any_pulse_sent_(false), last_divider_(-1) {
    assert(CanHandle(pins));

    int base = specs[0];
    for (size_t i = 0; i < specs.size(); ++i) {
#if 0
      // Experimental. Exact pulses, but due to higher divider high
      // recovery time and low refresh-rate.
      if (specs[i] > MAX_PWM_BIT_USE * base) {
        base <<= 5;
      }
#endif
      if (specs[i] <= MAX_PWM_BIT_USE * base) {
        pwm_configs_.push_back(CreatePwmFifoConfig(base / PWM_BASE_TIME_NS,
                                                   specs[i] / base, specs[i]));
      } else {
        pwm_configs_.push_back(ConfigFromNanos(specs[i]));
      }
    }

    // Get relevant registers
    const bool isPI2 = IsRaspberryPi2();
    volatile uint32_t *gpioReg = mmap_bcm_register(isPI2, GPIO_REGISTER_OFFSET);
    pwm_reg_  = mmap_bcm_register(isPI2, GPIO_PWM_BASE_OFFSET);
    clk_reg_  = mmap_bcm_register(isPI2, GPIO_CLK_BASE_OFFSET);
    assert((clk_reg_ != NULL) && (pwm_reg_ != NULL));  // init error.

    SetGPIOMode(gpioReg, 18, 2); // set GPIO 18 to PWM0 mode (Alternative 5)
    SetPWMDivider(5);
  }

  virtual void SendPulse(int c) {
    SetPWMDivider(pwm_configs_[c].clk_div);
    //pwm_reg_[PWM_CTL] = PWM_CTL_USEF1 | PWM_CTL_MODE1 | PWM_CTL_PWEN1 | PWM_CTL_POLA1;
    //pwm_reg_[PWM_STA] = -1;   // clear status bits.
    for (uint32_t *pattern = pwm_configs_[c].pwm_pattern; *pattern; ++pattern) {
      pwm_reg_[PWM_FIFO] = *pattern;
    }
    pwm_reg_[PWM_FIFO] = 0;
    any_pulse_sent_ = true;
  }

  virtual void WaitPulseFinished() {
    if (!any_pulse_sent_) return;
    // Wait until FIFO is empty.
    // TODO(hzeller): this is a very crude way to wait for the result.
    // ideally, we could use the interrupt feature to wait for the result.
    pwm_reg_[PWM_FIFO] = 0;
    while ((pwm_reg_[PWM_STA] & PWM_STA_EMPT1) == 0) {
      usleep(1);
    }
  }

private:
  struct pwm_fifo_config {
    uint32_t clk_div;
    uint32_t pwm_pattern[8];  // fifo length. We only fill up to the first 7
  };

  void SetGPIOMode(volatile uint32_t *gpioReg, unsigned gpio, unsigned mode) {
    const int reg = gpio / 10;
    const int mode_pos = (gpio % 10) * 3;
    gpioReg[reg] = (gpioReg[reg] & ~(7 << mode_pos)) | (mode << mode_pos);
  }

  void SetPWMDivider(uint32_t divider) {
    if (divider == last_divider_)
      return;

    // reset PWM clock
    clk_reg_[CLK_PWMCTL] = CLK_PASSWD | CLK_CTL_KILL;

    // set PWM clock source as 500 MHz PLLD
    clk_reg_[CLK_PWMCTL] = CLK_PASSWD | CLK_CTL_SRC(CLK_CTL_SRC_PLLD);

    // set PWM clock divider
    clk_reg_[CLK_PWMDIV] = CLK_PASSWD | CLK_DIV_DIVI(divider) | CLK_DIV_DIVF(0);

    // enable PWM clock
    clk_reg_[CLK_PWMCTL] = CLK_PASSWD | CLK_CTL_ENAB | CLK_CTL_SRC(CLK_CTL_SRC_PLLD);

    // set our mode
    pwm_reg_[PWM_CTL] = PWM_CTL_USEF1 | PWM_CTL_MODE1 | PWM_CTL_PWEN1 | PWM_CTL_POLA1;
    pwm_reg_[PWM_STA] = -1;   // clear status bits.

    usleep(1);  // TODO: what is a good time here ? Are there better ways ?
    //for (int i = 0; i < 300; ++i) { asm(""); }  // Registers need a while to settle.
    last_divider_ = divider;
  }

  pwm_fifo_config CreatePwmFifoConfig(int divider, int bit_count, int fyi_nanos) {
    pwm_fifo_config result;
    bzero(&result, sizeof(result));
#define PWM_DEBUG_PRINT 0
#if PWM_DEBUG_PRINT
    fprintf(stderr, "%7dns div:%3d bits:%4d = %6dns (Δ%3d) ",
            fyi_nanos, divider, bit_count,
            divider * PWM_BASE_TIME_NS * bit_count,
            divider * PWM_BASE_TIME_NS * bit_count - fyi_nanos);
#endif
    assert(bit_count <= MAX_PWM_BIT_USE);
    assert(divider < (1<<12));   // we only have 12 bits

    result.clk_div = divider;
    for (int i = 0; i < 8 && bit_count > 0; ++i) {
      if (bit_count >= 32) {
        result.pwm_pattern[i] = ~0;
        bit_count -= 32;
      } else {
        // Remaining bits.
        result.pwm_pattern[i] = ~((1u<<(32-bit_count))-1);  // fill from top
        break;
      }
    }

#if PWM_DEBUG_PRINT
    for (int i = 0; i < 8; ++i) {
      fprintf(stderr, "%08x ", result.pwm_pattern[i]);
    }
    fprintf(stderr, "\n");
#endif
#undef PWM_DEBUG_PRINT

    return result;
  }

  pwm_fifo_config ConfigFromNanos(int nano_seconds) {
    const int clock_loops = nano_seconds / PWM_BASE_TIME_NS;
    // Find the smallest divider (=high time resolution) that fits
    // in maximum allowed bits.
    // We can sometimes get more exact values with higher dividers,
    // however, they seem to impose a higher recovery rate.
    int divider = (clock_loops / MAX_PWM_BIT_USE)+1;
    if (divider < 2) divider = 2;   // Can't be 1
    int bit_count = clock_loops / divider;
    return CreatePwmFifoConfig(divider, bit_count, nano_seconds);
  }

private:
  std::vector<pwm_fifo_config> pwm_configs_;
  volatile uint32_t *pwm_reg_;
  volatile uint32_t *clk_reg_;
  bool any_pulse_sent_;
  uint32_t last_divider_;
};

} // end anonymous namespace

// Public PinPulser factory
PinPulser *PinPulser::Create(GPIO *io, uint32_t gpio_mask,
                             const std::vector<int> &nano_wait_spec) {
  // The only implementation so far.
  if (!Timers::Init()) return NULL;
  if (HardwarePinPulser::CanHandle(gpio_mask)) {
    return new HardwarePinPulser(gpio_mask, nano_wait_spec);
  } else {
    return new TimerBasedPinPulser(io, gpio_mask, nano_wait_spec);
  }
}

} // namespace rgb_matrix
