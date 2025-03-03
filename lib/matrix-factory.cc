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

#include "matrix-factory.h"
#include <string.h>
#include <iostream>

namespace rgb_matrix
{

    MatrixFactory::Options::Options()
        : use_emulator(false)
    {
    }

    RGBMatrixBase* MatrixFactory::CreateMatrix(const MatrixFactory::Options& options)
    {
        RGBMatrixBase *result = nullptr;

#ifdef ENABLE_EMULATOR
        if (options.use_emulator)
        {
            // Create an emulator
            result = EmulatorMatrix::Create(options.led_options, options.emulator_options);
            if (result)
            {
                // Start the refresh thread for the emulator
                std::cout << "Starting refresh thread" << std::endl;
                static_cast<EmulatorMatrix *>(result)->StartRefresh();
            }
        }
        else
        {
            // Create a real hardware matrix
            result = RGBMatrix::CreateFromOptions(options.led_options, options.runtime_options);
        }
#else
        result = RGBMatrix::CreateFromOptions(options.led_options, options.runtime_options);

#endif

        return result;
    }

    bool MatrixFactory::ParseOptionsFromFlags(int *argc, char ***argv,
                                              MatrixFactory::Options *options,
                                              bool remove_consumed_flags)
    {
        if (!options)
            return false;

        // First, check for --led-emulator flag
        for (int i = 1; i < *argc; ++i)
        {
            if (strcmp((*argv)[i], "--led-emulator") == 0)
            {
                options->use_emulator = true;
                if (remove_consumed_flags)
                {
                    (*argv)[i] = nullptr;
                }
            }
        }

        // Parse options for the appropriate matrix type
        bool result = true;
        rgb_matrix::RuntimeOptions *rt_options = &options->runtime_options;

        // Parse core RGB matrix options regardless of type
        result &= rgb_matrix::ParseOptionsFromFlags(argc, argv, &options->led_options, rt_options, remove_consumed_flags);
#ifdef ENABLE_EMULATOR
        if (options->use_emulator)
        {
            // Parse emulator-specific options
            result &= rgb_matrix::ParseEmulatorOptionsFromFlags(argc, argv, &options->emulator_options, remove_consumed_flags);
        }
        else
        {
            // Parse hardware-specific runtime options
            // Use a temporary variable with the exact type to help the compiler pick the right overload
            result &= rgb_matrix::ParseOptionsFromFlags(argc, argv, &options->led_options, rt_options, remove_consumed_flags);
        }
#else
        result &= rgb_matrix::ParseOptionsFromFlags(argc, argv, &options->led_options, rt_options, remove_consumed_flags);
#endif

        return result;
    }

    void MatrixFactory::PrintMatrixFactoryFlags(FILE *out, const MatrixFactory::Options &defaults)
    {
        fprintf(out, "The following flags are available for RGB matrix configuration:\n");
        fprintf(out, "General matrix selection:\n");

#ifdef ENABLE_EMULATOR
        fprintf(out, "\t--led-emulator          : Use emulator instead of actual hardware\n\n");
#endif
        // Print both sets of options
        fprintf(out, "Hardware matrix options:\n");
        rgb_matrix::PrintMatrixFlags(out, defaults.led_options, defaults.runtime_options);

#ifdef ENABLE_EMULATOR
        fprintf(out, "\nEmulator options:\n");
        rgb_matrix::PrintEmulatorFlags(out, defaults.emulator_options);
#endif
    }

} // namespace rgb_matrix