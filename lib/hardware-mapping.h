/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 * Copyright (C) 2013 Henner Zeller <h.zeller@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http: *gnu.org/licenses/gpl-2.0.txt>
 */
#ifndef RPI_HARDWARE_MAPPING_H
#define RPI_HARDWARE_MAPPING_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

extern struct HardwareMapping matrix_hardware_mappings[];

#ifdef  __cplusplus
}  // extern C
#endif

#endif
