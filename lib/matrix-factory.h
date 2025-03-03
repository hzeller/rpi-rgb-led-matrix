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
#include <cstring>

// Only include emulator.h when ENABLE_EMULATOR is defined
#ifdef ENABLE_EMULATOR
#include "emulator.h"
#endif

namespace rgb_matrix
{

    class MatrixFactory
    {
    public:
        struct Options
        {
            Options() : use_emulator(false) {}

            // Standard RGBMatrix options for hardware matrix
            RGBMatrix::Options led_options;

            // Runtime options for hardware matrix
            RuntimeOptions runtime_options;

            // Whether to use emulator instead of hardware
            bool use_emulator;

#ifdef ENABLE_EMULATOR
            // Emulator specific options (only available when emulator is enabled)
            EmulatorOptions emulator_options;
#endif
        };

        // Create a matrix based on the provided options
        static RGBMatrixBase *CreateMatrix(const Options &options)
        {
            // Try to create hardware matrix first
            if (!options.use_emulator)
            {
                return RGBMatrix::CreateFromOptions(options.led_options,
                                                    options.runtime_options);
            }

#ifdef ENABLE_EMULATOR
            // Create emulator if requested and available
            return EmulatorMatrix::Create(options.led_options, options.emulator_options);
#else
            // If emulator requested but not available, fallback to hardware matrix
            fprintf(stderr, "Emulator requested but not available in this build.\n"
                            "Rebuild with -DENABLE_EMULATOR=ON to enable emulator support.\n"
                            "Falling back to hardware matrix.\n");
            return RGBMatrix::CreateFromOptions(options.led_options, options.runtime_options);
#endif
        }

        // Parse options from command line
        static bool ParseOptionsFromFlags(int *argc, char ***argv, Options *options)
        {
            // Parse regular RGBMatrix options
            if (!rgb_matrix::ParseOptionsFromFlags(argc, argv,
                                                   &options->led_options,
                                                   &options->runtime_options))
            {
                return false;
            }

            // Parse factory-specific options
            for (int i = 1; i < *argc; ++i)
            {
                if (*argv == NULL || *argv[i] == NULL)
                    continue;
                if (strcmp(*argv[i], "--led-emulator") == 0)
                {
                    options->use_emulator = true;
                    remove_option(argc, argv, i);
                    --i;
                }
            }

#ifdef ENABLE_EMULATOR
            // Parse emulator options if emulator is enabled
            if (options->use_emulator)
            {
                if (!ParseEmulatorOptionsFromFlags(argc, argv, &options->emulator_options))
                {
                    return false;
                }
            }
#else
            if (options->use_emulator)
            {
                fprintf(stderr, "Emulator requested but not available in this build.\n"
                                "Rebuild with -DENABLE_EMULATOR=ON to enable emulator support.\n");
                return false;
            }
#endif

            return true;
        }

        // Print command line flags help
        static void PrintMatrixFactoryFlags(FILE *out, const Options &defaults)
        {
            // Print standard RGBMatrix flags
            rgb_matrix::PrintMatrixFlags(out);

            fprintf(out, "\nFactory-specific options:\n");
#ifdef ENABLE_EMULATOR
            fprintf(out, "\t--led-emulator                : Use emulator instead of hardware matrix%s\n",
                    defaults.use_emulator ? " (Default: on)" : " (Default: off)");

            // Print emulator options if enabled
            fprintf(out, "\nEmulator options:\n");
            PrintEmulatorFlags(out, defaults.emulator_options);
#else
            fprintf(out, "\nEmulator options not available in this build.\n"
                         "Rebuild with -DENABLE_EMULATOR=ON to enable emulator support.\n");
#endif
        }

    private:
        // Helper to remove command line option
        static void remove_option(int *argc, char ***argv, int index)
        {
            for (int i = index; i < (*argc) - 1; ++i)
            {
                (*argv)[i] = (*argv)[i + 1];
            }
            (*argv)[--(*argc)] = NULL;
        }
    };

} // namespace rgb_matrix

#endif // RPI_RGBMATRIX_FACTORY_H
