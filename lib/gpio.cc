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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

extern "C" {
#include "mailbox.h"
}

// Raspberry 1 and 2 have different base addresses for the periphery
#define BCM2708_PERI_BASE        0x20000000
#define BCM2709_PERI_BASE        0x3F000000

//#define UNCACHED_START_MAP     0x40000000
#define UNCACHED_START_MAP       0xC0000000


#define COUNTER_1Mhz_REGISTER_OFFSET   0x3000
#define GPIO_REGISTER_OFFSET     0x200000
#define DMA_BASE_OFFSET          0x007000
#define GPIO_PWM_BASE_OFFSET	 (GPIO_REGISTER_OFFSET + 0xC000)
#define GPIO_CLK_BASE_OFFSET	 0x101000

#define PAGE_SIZE 4096
#define REGISTER_BLOCK_SIZE (4*1024)

#define BUS_TO_PHYS(x) ((x)&~0xC0000000)

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

#define DMA_ENABLE 0xff0   // Set bit n to enable channel n.

#define DMA_CB_TI_NO_WIDE_BURSTS (1<<26)
#define DMA_CB_TI_SRC_INC     (1<<8)
#define DMA_CB_TI_DEST_INC    (1<<4)
#define DMA_CB_TI_TDMODE      (1<<1)

#define DMA_CS_RESET (1<<31)
#define DMA_CS_ABORT (1<<30)
#define DMA_CS_END (1<<1)
#define DMA_CS_ACTIVE (1<<0)
#define DMA_CS_PRIORITY(x) ((x)&0xf << 16) //higher priority DMA transfers are serviced first, it would appear
#define DMA_CS_PRIORITY_MAX DMA_CS_PRIORITY(7)
#define DMA_CS_PANIC_PRIORITY(x) ((x)&0xf << 20)
#define DMA_CS_PANIC_PRIORITY_MAX DMA_CS_PANIC_PRIORITY(7)
#define DMA_CS_DISDEBUG (1<<28) //DMA will not stop when debug signal is asserted

#define DMA_DEBUG_READ_ERROR (1<<2)
#define DMA_DEBUG_FIFO_ERROR (1<<1)
#define DMA_DEBUG_READ_LAST_NOT_SET_ERROR (1<<0)

// Same for Pi1 and Pi2
#define GPIO_DMA_BASE_BUS 0x7E200000 // Physical GPIO bus GPIO address.
#define GPSET_START   0x0000001C     // first set register. next word: set1, next 2 words: clear.
#define DMA_CB_TXFR_LEN_YLENGTH(y) (((y-1)&0x4fff) << 16)
#define DMA_CB_TXFR_LEN_XLENGTH(x) ((x)&0xffff)
#define DMA_CB_STRIDE_D_STRIDE(x)  (((x)&0xffff) << 16)
#define DMA_CB_STRIDE_S_STRIDE(x)  ((x)&0xffff)

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
  return true;
}

void GPIO::SetBits(uint32_t value) {
  gpio_port_[0x1C / sizeof(uint32_t)] = value;
#ifdef RGB_SLOWDOWN_GPIO
  gpio_port_[0x1C / sizeof(uint32_t)] = value;
#endif
}

void GPIO::ClearBits(uint32_t value) {
  gpio_port_[0x28 / sizeof(uint32_t)] = value;
#ifdef RGB_SLOWDOWN_GPIO
  gpio_port_[0x28 / sizeof(uint32_t)] = value;
#endif
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
 TODO hardware timing: more CPU use and less refresh rate than 'regular'
   - darker, i.e more setup time in-between (= more dark time) (less current use)
   - lower refresh rate
*/

// A PinPulser that uses the PWM hardware to create accurate pulses.
// It only works on GPIO-18 though.
// Based in part on discussion found on
// https://www.raspberrypi.org/forums/viewtopic.php?t=67741&p=494768
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

    usleep(1);  // TODO: what is a good time here ?
    //for (int i = 0; i < 300; ++i) { asm(""); }  // Registers need a while to settle.
    last_divider_ = divider;
  }

  pwm_fifo_config CreatePwmFifoConfig(int divider, int bit_count, int fyi_nanos) {
    pwm_fifo_config result;
    bzero(&result, sizeof(result));

    fprintf(stderr, "%7dns div:%3d bits:%4d = %6dns (Δ%3d) ",
            fyi_nanos, divider, bit_count,
            divider * PWM_BASE_TIME_NS * bit_count,
            divider * PWM_BASE_TIME_NS * bit_count - fyi_nanos);

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

    for (int i = 0; i < 8; ++i) {
      fprintf(stderr, "%08x ", result.pwm_pattern[i]);
    }
    fprintf(stderr, "\n");

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
  if (HardwarePinPulser::CanHandle(gpio_mask) && false) {  // disabled for now.
    fprintf(stderr, "Using hardware pulser\n");
    return new HardwarePinPulser(gpio_mask, nano_wait_spec);
  } else {
    return new TimerBasedPinPulser(io, gpio_mask, nano_wait_spec);
  }
}

struct HardwareScript::dma_chan {
  // 4.2.1.2 RegisterMap. p 41.
  uint32_t cs;        // control and status.
  uint32_t cblock;    // control block address.
  // -- A copy of the currently active control block.
  uint32_t info;      // transfer information.
  uint32_t src;       // physical source address.
  uint32_t dst;       // physical destination address.
  uint32_t length;    // transfer length.
  uint32_t stride;    // stride mode.
  uint32_t next_cb;   // physical address next control block (cache-line=32byte-aligned)
  // --
  uint32_t debug;     // control debug settings.
};

struct HardwareScript::dma_cb {  // 32 bytes.
  uint32_t info;   // transfer information.
  uint32_t src;    // physical source address.
  uint32_t dst;    // physical destination address.
  uint32_t length; // transfer length.
  uint32_t stride; // stride mode.
  uint32_t next;   // next control block.
  uint32_t pad[2];
};

class HardwareScript::ScriptElement {
public:
  virtual ~ScriptElement() {}
  virtual void Run() = 0;
  virtual void FillDMABlock(dma_cb *control) {}
};

class HardwareScript::DataElement : public HardwareScript::ScriptElement {
public:
  DataElement(GPIO *io, uintptr_t physical_addr, const GPIO::Data *data)
    : io_(io), data_physical_(physical_addr), data_(data) {}
  virtual void Run() { io_->Write(*data_); }
  virtual void FillDMABlock(dma_cb *cb) {
    cb->info   = (DMA_CB_TI_SRC_INC | DMA_CB_TI_DEST_INC |
                  DMA_CB_TI_NO_WIDE_BURSTS | DMA_CB_TI_TDMODE);
    cb->src    = data_physical_;
    cb->dst    = GPIO_DMA_BASE_BUS + GPSET_START;
    cb->length = DMA_CB_TXFR_LEN_YLENGTH(2) | DMA_CB_TXFR_LEN_XLENGTH(8);
    cb->stride = DMA_CB_STRIDE_D_STRIDE(4) | DMA_CB_STRIDE_S_STRIDE(0);
  }

private:
  GPIO *const io_;
  uintptr_t data_physical_;
  const GPIO::Data *data_;
};

class HardwareScript::PulseElement : public HardwareScript::ScriptElement {
public:
  PulseElement(PinPulser *pulser, int spec) : pulser_(pulser), spec_(spec) {}
  virtual void Run() {
    pulser_->SendPulse(spec_);
    pulser_->WaitPulseFinished();  // todo: make that a separate element.
  }
  virtual void FillDMABlock(dma_cb *control) {
    assert(0);  // can't deal with this yet.
  }
private:
  PinPulser *const pulser_;
  const int spec_;
};


HardwareScript::HardwareScript(GPIO *io, PinPulser *pulser)
  : io_(io), pulser_(pulser), script_block_(NULL) {
  const int channel_num = 5;
  char *dmaBase = (char*)mmap_bcm_register(IsRaspberryPi2(),
                                               DMA_BASE_OFFSET);
  dma_channel_ = (dma_chan*)(dmaBase + 0x100 * channel_num);  // 4.2.1.2
  *(uint32_t*)(dmaBase + DMA_ENABLE) |= (1 << channel_num);
}

HardwareScript::~HardwareScript() {
  fprintf(stderr, "Shutting down script.\n");
  dma_channel_->cs |= DMA_CS_ABORT;
  usleep(100);
  dma_channel_->cs &= ~DMA_CS_ACTIVE;
  dma_channel_->cs |= DMA_CS_RESET;
  usleep(100);

  delete script_block_;
  Clear();
}

void HardwareScript::Clear() {
  for (size_t i = 0; i < elements_.size(); ++i) {
    delete elements_[i];
  }
  elements_.clear();
}

void HardwareScript::AppendGPIO(const MemBlock& block, const GPIO::Data *data) {
  // Make sure that we don't mess with bits we haven't configured.
  assert((data->set_bits & io_->output_bits()) == data->set_bits);
  assert((data->clear_bits & io_->output_bits()) == data->clear_bits);

  elements_.push_back(new DataElement(io_, block.ToPhysical(data), data));
}
void HardwareScript::AppendPinPulse(int spec) {
  elements_.push_back(new PulseElement(pulser_, spec));
}

int MemBlock::mbox_ = -1;

MemBlock::MemBlock(size_t size) {
  if (mbox_ < 0) {
    mbox_ = mbox_open();
  }
  // Round up to next full page.
  size = size % PAGE_SIZE == 0 ? size : (size + PAGE_SIZE) & ~(PAGE_SIZE - 1);

  size_ = size;
  int mem_flag = 0x0c;
  mem_handle_ = mem_alloc(mbox_, size, PAGE_SIZE, mem_flag);
  bus_addr_ = mem_lock(mbox_, mem_handle_);
  mem_ = mapmem(BUS_TO_PHYS(bus_addr_), size);
  fprintf(stderr, "Alloc: %6d bytes;  %p (bus=0x%08x, phys=0x%08x)\n",
          size, mem_, bus_addr_, BUS_TO_PHYS(bus_addr_));
  assert(bus_addr_);  // otherwise: couldn't allocate contiguous.
  memset(mem_, 0x00, size);
}

uintptr_t MemBlock::ToPhysical(const void *m) const {
    uint32_t offset = (uint8_t*)m - (uint8_t*)mem_;
    assert(offset < size_);
    return bus_addr_ + offset;
  }

MemBlock::~MemBlock() {
  unmapmem(mem_, size_);
  mem_unlock(mbox_, mem_handle_);
  mem_free(mbox_, mem_handle_);
}

void HardwareScript::FinishScript() {
  script_block_ = new MemBlock(sizeof(dma_cb) * elements_.size());
  dma_cb *const control_blocks = (dma_cb*) script_block_->mem();
  dma_cb *cb = 0;
  for (size_t i = 0; i < elements_.size(); ++i) {
    cb = control_blocks + i;
    elements_[i]->FillDMABlock(cb);
    cb->next = script_block_->ToPhysical(cb + 1);
  }
  cb->next = script_block_->ToPhysical(script_block_->mem());
}

void HardwareScript::Run() {
#if 1
  dma_channel_->cs = DMA_CS_RESET;
  usleep(100);
  dma_channel_->cs |= DMA_CS_END;
  dma_channel_->debug = (DMA_DEBUG_READ_ERROR | DMA_DEBUG_FIFO_ERROR
                         | DMA_DEBUG_READ_LAST_NOT_SET_ERROR);
  dma_channel_->cblock = script_block_->ToPhysical(script_block_->mem());
  dma_channel_->cs = DMA_CS_PRIORITY_MAX | DMA_CS_PANIC_PRIORITY_MAX | DMA_CS_DISDEBUG;
  dma_channel_->cs = (DMA_CS_PRIORITY_MAX | DMA_CS_PANIC_PRIORITY_MAX | DMA_CS_DISDEBUG
                      | DMA_CS_ACTIVE);  // Aaaand action.
#else
  // foreground running.
  for (size_t i = 0; i < elements_.size(); ++i) {
    elements_[i]->Run();
  }
#endif
}

} // namespace rgb_matrix
