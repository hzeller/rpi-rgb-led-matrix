#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <pigpio.h>


const uint8_t HI = 1;
const uint8_t LO = 0;
const uint8_t kReg1[] = {
    LO, HI, HI, HI, HI, HI, HI, HI,
    HI, HI, HI, HI, HI, HI, HI, HI,
};
const uint8_t kReg2[] = {
    LO, LO, LO, LO, LO, LO, LO, LO,
    LO, HI, LO, LO, LO, LO, LO, LO,
};
typedef uint32_t gpio_bits_t;  /* this should probably come from gpio.h */
struct HardwareMapping {
  const char *name;
  int max_parallel_chains;
  gpio_bits_t output_enable;
  gpio_bits_t clock;
  gpio_bits_t strobe;
  gpio_bits_t a, b, c, d, e;
  gpio_bits_t p0_r1, p0_g1, p0_b1;
  gpio_bits_t p0_r2, p0_g2, p0_b2;
  gpio_bits_t p1_r1, p1_g1, p1_b1;
  gpio_bits_t p1_r2, p1_g2, p1_b2;
  gpio_bits_t p2_r1, p2_g1, p2_b1;
  gpio_bits_t p2_r2, p2_g2, p2_b2;
};

#define GPIO_BIT(b) (b)

struct HardwareMapping hm = {
    .name          = "ji-hat",

    .output_enable = GPIO_BIT(18),
    .clock         = GPIO_BIT(27),
    .strobe        = GPIO_BIT(21),

    /* Address lines */
    .a             = GPIO_BIT(16),
    .b             = GPIO_BIT(19),
    .c             = GPIO_BIT(20),
    .d             = GPIO_BIT(26),
    .e             = GPIO_BIT(13),

    /* Parallel chain 0, RGB for both sub-panels */
    .p0_r1         = GPIO_BIT(4),
    .p0_g1         = GPIO_BIT(15),
    .p0_b1         = GPIO_BIT(14),
    .p0_r2         = GPIO_BIT(3),
    .p0_g2         = GPIO_BIT(17),
    .p0_b2         = GPIO_BIT(2),

    /* Chain 1 */
    .p1_r1         = GPIO_BIT(10),
    .p1_g1         = GPIO_BIT(25),
    .p1_b1         = GPIO_BIT(24),
    .p1_r2         = GPIO_BIT(22),
    .p1_g2         = GPIO_BIT(9),
    .p1_b2         = GPIO_BIT(23),

    /* Chain 2 */
    .p2_r1         = GPIO_BIT(5),
    .p2_g1         = GPIO_BIT(12),
    .p2_b1         = GPIO_BIT(7),
    .p2_r2         = GPIO_BIT(8),
    .p2_g2         = GPIO_BIT(6),
    .p2_b2         = GPIO_BIT(11),
};


inline void hmset(uint8_t bit) {
  gpioWrite(hm.p0_r1, bit);
  gpioWrite(hm.p0_g1, bit);
  gpioWrite(hm.p0_b1, bit);
  gpioWrite(hm.p0_r2, bit);
  gpioWrite(hm.p0_g2, bit);
  gpioWrite(hm.p0_b2, bit);
  gpioWrite(hm.p1_r1, bit);
  gpioWrite(hm.p1_g1, bit);
  gpioWrite(hm.p1_b1, bit);
  gpioWrite(hm.p1_r2, bit);
  gpioWrite(hm.p1_g2, bit);
  gpioWrite(hm.p1_b2, bit);
  gpioWrite(hm.p2_r1, bit);
  gpioWrite(hm.p2_g1, bit);
  gpioWrite(hm.p2_b1, bit);
  gpioWrite(hm.p2_r2, bit);
  gpioWrite(hm.p2_g2, bit);
  gpioWrite(hm.p2_b2, bit);
}

int main(int argc, char* argv[]) {
  atexit(gpioTerminate);
  if (gpioInitialise() < 0) {
    err(1, "initializing");
  }
  uint32_t max_led = 64;
  if (argc > 1) {
    max_led = atoi(argv[1]) * 64;
  }
  gpioSetMode(hm.output_enable, PI_OUTPUT);
  gpioSetMode(hm.clock, PI_OUTPUT);
  gpioSetMode(hm.strobe, PI_OUTPUT);
  gpioSetMode(hm.a, PI_OUTPUT);
  gpioSetMode(hm.b, PI_OUTPUT);
  gpioSetMode(hm.c, PI_OUTPUT);
  gpioSetMode(hm.d, PI_OUTPUT);
  gpioSetMode(hm.e, PI_OUTPUT);
  gpioSetMode(hm.p0_r1, PI_OUTPUT);
  gpioSetMode(hm.p0_g1, PI_OUTPUT);
  gpioSetMode(hm.p0_b1, PI_OUTPUT);
  gpioSetMode(hm.p0_r2, PI_OUTPUT);
  gpioSetMode(hm.p0_g2, PI_OUTPUT);
  gpioSetMode(hm.p0_b2, PI_OUTPUT);
  gpioSetMode(hm.p1_r1, PI_OUTPUT);
  gpioSetMode(hm.p1_g1, PI_OUTPUT);
  gpioSetMode(hm.p1_b1, PI_OUTPUT);
  gpioSetMode(hm.p1_r2, PI_OUTPUT);
  gpioSetMode(hm.p1_g2, PI_OUTPUT);
  gpioSetMode(hm.p1_b2, PI_OUTPUT);
  gpioSetMode(hm.p2_r1, PI_OUTPUT);
  gpioSetMode(hm.p2_g1, PI_OUTPUT);
  gpioSetMode(hm.p2_b1, PI_OUTPUT);
  gpioSetMode(hm.p2_r2, PI_OUTPUT);
  gpioSetMode(hm.p2_g2, PI_OUTPUT);
  gpioSetMode(hm.p2_b2, PI_OUTPUT);

  // Send Data to control register 11
  gpioWrite(hm.clock, LO);
  gpioWrite(hm.output_enable, LO); // Display reset
  gpioWrite(hm.strobe, LO); // ???
  gpioWrite(hm.a, HI);
  gpioWrite(hm.b, LO);
  gpioWrite(hm.c, LO);
  gpioWrite(hm.d, LO);
  gpioWrite(hm.e, LO);

  for (int l = 0; l < max_led; ++l) {
    hmset(kReg2[l % 16]);;
    gpioWrite(hm.clock, HI);
    gpioWrite(hm.clock, LO);
    if (l >= max_led - 12) {
      gpioWrite(hm.strobe, HI);
    } else {
      gpioWrite(hm.strobe, LO);
    }
  }
  gpioWrite(hm.strobe, LO);

  for (int l = 0; l < max_led; ++l) {
    hmset(kReg2[l % 16]);;
    gpioWrite(hm.clock, HI);
    gpioWrite(hm.clock, LO);
    if (l >= max_led - 13) {
      gpioWrite(hm.strobe, HI);
    } else {
      gpioWrite(hm.strobe, LO);
    }
  }
  gpioWrite(hm.strobe, LO);
}
