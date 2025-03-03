// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2023 Hendrik
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

#ifndef RPI_RGBMATRIX_EMULATOR_H
#define RPI_RGBMATRIX_EMULATOR_H

#include "led-matrix.h"
#include <string>

// Only define emulator functionality if ENABLE_EMULATOR is set
#ifdef ENABLE_EMULATOR

namespace rgb_matrix {

// EmulatorOptions contains configuration for the matrix emulator
struct EmulatorOptions {
  EmulatorOptions();
  
  // Window scaling factor (default: 10)
  int display_scale;
  
  // Window title
  std::string window_title;
  
  // Emulate slowness/hardware timing (default: false)
  // When true, it will simulate timing issues to match hardware behavior
  bool emulate_hardware_timing;
  
  // Display refresh rate in Hz (default: 60)
  int refresh_rate_hz;
};

// EmulatorMatrix is an implementation of RGBMatrix that shows the matrix content
// in a window rather than requiring physical GPIO hardware
class EmulatorMatrix : public RGBMatrixBase {
public:
  // Create a new EmulatorMatrix with the given options
  static EmulatorMatrix *Create(const RGBMatrix::Options &options,
                                const EmulatorOptions &emulator_options);
  
  // Destructor
  virtual ~EmulatorMatrix();
  
  // Canvas interface implementation
  virtual int width() const;
  virtual int height() const;
  virtual void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);
  virtual void Clear();
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue);
  
  // Create a new off-screen canvas
  FrameCanvas *CreateFrameCanvas();
  
  // Swap the current canvas with the new one on VSync
  FrameCanvas *SwapOnVSync(FrameCanvas *other, unsigned framerate_fraction = 1);
  
  // Apply pixel mapper to the matrix
  bool ApplyPixelMapper(const PixelMapper *mapper);
  
  // Set brightness
  void SetBrightness(uint8_t brightness);
  uint8_t brightness();
  
  // Set PWM bits
  bool SetPWMBits(uint8_t value);
  uint8_t pwmbits();
  
  // Luminance correction
  void set_luminance_correct(bool on);
  bool luminance_correct() const;
  
  // Start the refresh thread
  bool StartRefresh();

private:
  class Impl;
  EmulatorMatrix(Impl *impl);
  
  Impl *const impl_;
};

// Parse emulator options from command line
bool ParseEmulatorOptionsFromFlags(int *argc, char ***argv,
                                  EmulatorOptions *default_options,
                                  bool remove_consumed_flags = true);

// Print emulator-specific flags help
void PrintEmulatorFlags(FILE *out, const EmulatorOptions &defaults = EmulatorOptions());

}  // namespace rgb_matrix

#else  // !ENABLE_EMULATOR

// When ENABLE_EMULATOR is not defined, provide minimal stubs for EmulatorOptions
// to maintain API compatibility without any functionality
namespace rgb_matrix {

struct EmulatorOptions {
  EmulatorOptions() {}
  
  int display_scale = 10;
  std::string window_title = "RGB Matrix Emulator";
  bool emulate_hardware_timing = false;
  int refresh_rate_hz = 60;
};

// Stub functions that do nothing when emulator is disabled
inline bool ParseEmulatorOptionsFromFlags(int *argc, char ***argv,
                                         EmulatorOptions *default_options,
                                         bool remove_consumed_flags = true) {
  (void)argc;
  (void)argv;
  (void)default_options;
  (void)remove_consumed_flags;
  return true;  // Nothing to do, just return success
}

inline void PrintEmulatorFlags(FILE *out, const EmulatorOptions &defaults = EmulatorOptions()) {
  (void)out;
  (void)defaults;
  // No emulator flags to print when disabled
}

}  // namespace rgb_matrix

#endif  // ENABLE_EMULATOR

#endif  // RPI_RGBMATRIX_EMULATOR_H
