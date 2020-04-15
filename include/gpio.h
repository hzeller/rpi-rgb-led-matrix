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

#ifndef RPI_GPIO_H
#define RPI_GPIO_H

#include <stdint.h>

#include <vector>

// Putting this in our namespace to not collide with other things called like
// this.
namespace rgb_matrix {
// For now, everything is initialized as output.
class GPIO {
public:
  // Available bits that actually have pins.
  static const uint64_t kValidBits;

  GPIO();

  // Initialize before use. Returns 'true' if successful, 'false' otherwise
  // (e.g. due to a permission problem).
  bool Init(int
          #if RGB_SLOWDOWN_GPIO
            slowdown = RGB_SLOWDOWN_GPIO
    #else
            slowdown = 1
    #endif
      );

  // Initialize outputs.
  // Returns the bits that were available and could be set for output.
  // (never use the optional adafruit_hack_needed parameter, it is used
  // internally to this library).
  uint64_t InitOutputs(uint64_t outputs, bool adafruit_hack_needed = false);

  // Request given bitmap of GPIO inputs.
  // Returns the bits that were available and could be reserved.
  uint64_t RequestInputs(uint64_t inputs);

  // Set the bits that are '1' in the output. Leave the rest untouched.
  inline void SetBits(uint64_t value) {
    if (!value) return;
    WriteSetBits(value);
    for (int i = 0; i < slowdown_; ++i) {
      WriteSetBits(value);
    }
  }

  // Clear the bits that are '1' in the output. Leave the rest untouched.
  inline void ClearBits(uint64_t value) {
    if (!value) return;
    WriteClrBits(value);
    for (int i = 0; i < slowdown_; ++i) {
      WriteClrBits(value);
    }
  }

  // Write all the bits of "value" mentioned in "mask". Leave the rest untouched.
  inline void WriteMaskedBits(uint64_t value, uint64_t mask) {
    // Writing a word is two operations. The IO is actually pretty slow, so
    // this should probably  be unnoticable.
    ClearBits(~value & mask);
    SetBits(value & mask);
  }

  inline void Write(uint64_t value) { WriteMaskedBits(value, output_bits_); }
  inline uint64_t Read() const { return ReadRegisters() & input_bits_; }

private:
  inline uint64_t ReadRegisters() const { return *gpio_read_bits_low_ | (static_cast<uint64_t>(*gpio_read_bits_low_) << 32);}
  inline void WriteSetBits(uint64_t value) {
    *gpio_set_bits_low_ = static_cast<uint32_t>(value & 0xFFFFFFFF);
    *gpio_set_bits_high_ = static_cast<uint32_t>((value & 0xFFFFFFFF00000000ull) >> 32);
  }

  inline void WriteClrBits(uint64_t value) {
    *gpio_clr_bits_low_ = static_cast<uint32_t>(value & 0xFFFFFFFF);
    *gpio_clr_bits_high_ = static_cast<uint32_t>((value & 0xFFFFFFFF00000000ull) >> 32);
  }

private:
  uint64_t output_bits_;
  uint64_t input_bits_;
  uint64_t reserved_bits_;
  int slowdown_;
  volatile uint32_t *gpio_set_bits_low_;
  volatile uint32_t *gpio_set_bits_high_;
  volatile uint32_t *gpio_clr_bits_low_;
  volatile uint32_t *gpio_clr_bits_high_;
  volatile uint32_t *gpio_read_bits_low_;
  volatile uint32_t *gpio_read_bits_high_;
};

// A PinPulser is a utility class that pulses a GPIO pin. There can be various
// implementations.
class PinPulser {
public:
  // Factory for a PinPulser. Chooses the right implementation depending
  // on the context (CPU and which pins are affected).
  // "gpio_mask" is the mask that should be output (since we only
  //   need negative pulses, this is what it does)
  // "nano_wait_spec" contains a list of time periods we'd like
  //   invoke later. This can be used to pre-process timings if needed.
  static PinPulser *Create(GPIO *io, uint64_t gpio_mask,
                           bool allow_hardware_pulsing,
                           const std::vector<int> &nano_wait_spec);

  virtual ~PinPulser() {}

  // Send a pulse with a given length (index into nano_wait_spec array).
  virtual void SendPulse(int time_spec_number) = 0;

  // If SendPulse() is asynchronously implemented, wait for pulse to finish.
  virtual void WaitPulseFinished() {}
};

// Get rolling over microsecond counter. We get this from a hardware register
// if possible and a terrible slow fallback otherwise.
uint32_t GetMicrosecondCounter();

}  // end namespace rgb_matrix

#endif  // RPI_GPIO_H
