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
  static const uint32_t kValidBits;

  // GPIO has separate operations for setting and clearing bits, so
  // one datum contains information about setting and clearing.
  //
  // Even though we only use 32 bits for the usual Raspberry Pi
  // (might be different when we try the compute module), we use
  // 64 bits, as this is the width of the GPIO register.
  // This is important if used in DMA operations.
  //
  // Operations are done in the sequence set/clear.
  struct Data {
    uint64_t set_bits;
    uint64_t clear_bits;

    inline void SetMasked(uint64_t value, uint64_t mask) {
      set_bits   = (set_bits   & ~mask) | ( value & mask);
      clear_bits = (clear_bits & ~mask) | (~value & mask);
    }
  };

  GPIO();

  // Initialize before use. Returns 'true' if successful, 'false' otherwise
  // (e.g. due to a permission problem).
  bool Init();

  // Initialize pins that should act as outputs.
  // Returns the bits that are actually set.
  // Only these bits should be set in the SetMasked(), otherwise you
  // might lock-up your Pi.
  uint32_t InitOutputs(uint32_t outputs);
  uint32_t output_bits() const { return output_bits_; }

  inline void Write(const Data& data) {
    SetBits(data.set_bits);
    ClearBits(data.clear_bits);
  }

  // Set the bits that are '1' in the output. Leave the rest untouched.
  void SetBits(uint32_t value);

  // Clear the bits that are '1' in the output. Leave the rest untouched.
  void ClearBits(uint32_t value);

 private:
  uint32_t output_bits_;
  volatile uint32_t *gpio_port_;
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
  static PinPulser *Create(GPIO *io, uint32_t gpio_mask,
                           const std::vector<int> &nano_wait_spec);

  virtual ~PinPulser() {}

  // Send a pulse with a given length (index into nano_wait_spec array).
  virtual void SendPulse(int time_spec_number) = 0;

  // If SendPulse() is asynchronously implemented, wait for pulse to finish.
  virtual void WaitPulseFinished() {}
};

// Scratch API thought area for DMA. To abstract it from the actual DMA, we
// consider it a "HardwareScript", but it is close enough so that it can be
// implemented straight-forward.
// (playground right now to figure out a good API that makes things readable)
class HardwareScript {
public:
  // Does  not take ownership of io and pulser.
  HardwareScript(GPIO *io, PinPulser *pulser) : io_(io), pulser_(pulser) {}
  ~HardwareScript();

  // Clear script.
  void Clear();

  // Append a GPIO datum to be written. Ownership is not taken, but the
  // pointer must survive.
  void AppendGPIO(const GPIO::Data *data);

  // Append pulsing a pin (negative logic) for given spec
  // (TODO: this should be pin+nano-seconds, for now just spec from pulser
  // definition)
  void AppendPinPulse(int spec);

  // Run this script once.
  void RunOnce();

private:
  class ScriptElement;
  class DataElement;
  class PulseElement;

  GPIO *const io_;
  PinPulser *const pulser_;
  std::vector<ScriptElement*> elements_;
};
}  // end namespace rgb_matrix
#endif  // RPI_GPIO_H
