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

namespace rgb_matrix
{

    // Helper to remove command line option
    static void remove_option(int *argc, char ***argv, int index)
    {
        for (int i = index; i < (*argc) - 1; ++i)
        {
            (*argv)[i] = (*argv)[i + 1];
        }
        (*argv)[--(*argc)] = NULL;
    }

    MatrixFactory::Options::Options()
        : use_emulator(false)
    {
    }

    RGBMatrixBase *MatrixFactory::CreateMatrix(const MatrixFactory::Options &options)
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

    bool MatrixFactory::ParseOptionsFromFlags(int *argc, char ***argv,
                                              MatrixFactory::Options *options,
                                              bool remove_consumed_flags)
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