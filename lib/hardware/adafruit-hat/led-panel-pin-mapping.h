// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Adafruit made a HAT to work with this library, but it has a slightly
// different GPIO mapping. This is this mapping. A variant of this mapping
// allows using the Raspberry Pi PWM hardware. This requires modifying the
// HAT to connect GPIO 4 and 18. See #else for regular mapping.
#define RGB_PARALLEL_CHAINS 1
union IoBits {
  struct {
    // This bitset reflects the GPIO mapping. The naming of the
    // pins of type 'p0_r1' means 'first parallel chain, red-bit one'
    unsigned int unused_0_3         : 4;  // 0..3
#ifdef ADAFRUIT_RGBMATRIX_HAT_PWM
    unsigned int unused_4           : 1;  // 4
#else
    unsigned int output_enable      : 1;  // 4
#endif
    unsigned int p0_r1              : 1;  // 5
    unsigned int p0_b1              : 1;  // 6
    unsigned int unused_7_11        : 5;  // 7..11
    unsigned int p0_r2              : 1;  // 12
    unsigned int p0_g1              : 1;  // 13
    unsigned int unused_14_15       : 2;  // 14,15
    unsigned int p0_g2              : 1;  // 16
    unsigned int clock              : 1;  // 17
#ifdef ADAFRUIT_RGBMATRIX_HAT_PWM
    unsigned int output_enable      : 1;  // 18
    unsigned int unused_19          : 1;  // 19
#else
    unsigned int unused_18_19       : 2;  // 18,19
#endif
    unsigned int d                  : 1;  // 20
    unsigned int strobe             : 1;  // 21
    unsigned int a                  : 1;  // 22
    unsigned int p0_b2              : 1;  // 23
    unsigned int e                  : 1;  // 24  // Needs manual wiring
    unsigned int unused_25          : 1;  // 25
    unsigned int b                  : 1;  // 26
    unsigned int c                  : 1;  // 27
  } bits;
  uint32_t raw;
  IoBits() : raw(0) {}
};
