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

#ifndef RPI_RGBMATRIX_FACTORY_H
#define RPI_RGBMATRIX_FACTORY_H

#include "led-matrix.h"
#include "emulator.h"

namespace rgb_matrix {

// MatrixFactory allows creating either a real hardware RGBMatrix or an
// EmulatorMatrix based on command line options
class MatrixFactory {
public:
  // Factory options for creating matrices
  struct Options {
    Options();
    
    // If true, use emulator instead of real hardware
    bool use_emulator;
    
    // RGBMatrix options for hardware or emulator
    RGBMatrix::Options led_options;
    
    // Runtime options specific to the hardware matrix
    RuntimeOptions runtime_options;
    
    // Options specific to the emulator
    EmulatorOptions emulator_options;
  };
  
  // Updated to return RGBMatrixBase instead of Canvas
  static RGBMatrixBase* CreateMatrix(const Options& options);
  
  // Parse options from command line arguments
  static bool ParseOptionsFromFlags(int *argc, char ***argv,
                                   MatrixFactory::Options *options,
                                   bool remove_consumed_flags = true);
                                   
  // Print help for all matrix options (hardware and emulator)
  static void PrintMatrixFactoryFlags(FILE *out, const MatrixFactory::Options& defaults = MatrixFactory::Options());
};

}  // namespace rgb_matrix

#endif  // RPI_RGBMATRIX_FACTORY_H
