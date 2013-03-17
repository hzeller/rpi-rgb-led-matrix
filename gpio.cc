
#include "gpio.h"

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio_port_+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_port_+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

/*static*/ const uint32_t GPIO::kValidBits 
= ((1 <<  2) | (1 <<  3) | (1 <<  4) | (1 <<  7)| (1 << 8) | (1 <<  9) |
   (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15)| (1 <<17) | (1 << 18)|
   (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25)| (1 << 27));
   

GPIO::GPIO() : output_bits_(0), gpio_port_(NULL) {
}
   
uint32_t GPIO::InitOutputs(uint32_t outputs) {
  if (gpio_port_ == NULL) {
    fprintf(stderr, "Attempt to init outputs but initialized.\n");
    return 0;
  }
  outputs &= kValidBits;   // Sanitize input.
  output_bits_ = outputs;
  for (uint32_t b = 0; b < 27; ++b) {
    if (outputs & (1 << b)) {
      INP_GPIO(b);   // for writing, we first need to set as input.
      OUT_GPIO(b);
    }
  }
  return output_bits_;
}

// Based on code example found in http://elinux.org/RPi_Low-level_peripherals
bool GPIO::Init() {
  int mem_fd;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    perror("can't open /dev/mem: ");
    return false;
  }

  char *gpio_map =
    (char*) mmap(NULL,             //Any adddress in our space will do
         BLOCK_SIZE,       //Map length
         PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
         MAP_SHARED,       //Shared with other processes
         mem_fd,           //File to map
         GPIO_BASE         //Offset to GPIO peripheral
         );

  close(mem_fd); //No need to keep mem_fd open after mmap

  if (gpio_map == MAP_FAILED) {
    fprintf(stderr, "mmap error %ld\n", (long)gpio_map);
    return false;
  }

  gpio_port_ = (volatile uint32_t *)gpio_map;
  return true;
}
