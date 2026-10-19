// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2026
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.

#ifndef RPI_FRAME_SEQUENCE_H
#define RPI_FRAME_SEQUENCE_H

#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <vector>

namespace rgb_matrix {
class RGBMatrix;

// A lightweight in-memory sequence of RGB frames and hold-times.
//
// Frames are stored as packed RGB24 rows in display order (R,G,B per pixel).
// This allows simple playback without requiring stream files.
class FrameSequence {
public:
  typedef std::function<int(void)> BrightnessProvider;
  typedef std::function<bool(void)> StopProvider;

  FrameSequence(int width, int height);

  int width() const;
  int height() const;
  size_t frame_count() const;

  // Add one frame in packed RGB24 format.
  // byte_count must be width * height * 3.
  bool AddFrame(const uint8_t *rgb24, size_t byte_count, uint32_t hold_time_us);

  // Write the sequence to a .fseq file.
  bool WriteToFile(const char *path) const;

  // Read a sequence from a .fseq file.
  bool ReadFromFile(const char *path);

  void Clear();

  // Play the sequence forever until interrupted.
  void PlayForever(RGBMatrix *matrix,
                    const StopProvider &stop_provider = StopProvider(),
                    const BrightnessProvider &brightness_provider = BrightnessProvider()) const;

  // Play the sequence a fixed number of times.
  void PlayCount(RGBMatrix *matrix,
                 uint32_t play_count,
                 const StopProvider &stop_provider = StopProvider(),
                 const BrightnessProvider &brightness_provider = BrightnessProvider()) const;

  // Play the sequence for a fixed duration in milliseconds.
  void PlayDuration(RGBMatrix *matrix,
                    uint32_t duration_ms,
                    const StopProvider &stop_provider = StopProvider(),
                    const BrightnessProvider &brightness_provider = BrightnessProvider()) const;

  // Legacy convenience wrapper kept for compatibility with older callers.
  void Play(RGBMatrix *matrix,
            volatile bool *interrupt_received = NULL,
            const BrightnessProvider &brightness_provider = BrightnessProvider()) const;

private:
  struct Frame {
    std::vector<uint8_t> rgb24;
    uint32_t hold_time_us;
  };

  int width_;
  int height_;
  size_t frame_size_;
  std::vector<Frame> frames_;

  bool PlayOneSequence(RGBMatrix *matrix,
                      const StopProvider &stop_provider,
                      const BrightnessProvider &brightness_provider,
                      const std::function<bool(void)> &should_stop) const;
};

}  // namespace rgb_matrix

#endif  // RPI_FRAME_SEQUENCE_H