// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "frame-sequence.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>

#include "led-matrix.h"

namespace rgb_matrix {

namespace {
static const uint32_t kFseqMagic = 0x51455346;  // "FSEQ"
static const useconds_t kStopPollIntervalUs = 10000;

struct FseqHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t width;
  uint32_t height;
  uint32_t frame_count;
  uint32_t frame_size;
};

struct FseqFrameHeader {
  uint32_t hold_time_us;
};

static inline uint8_t ClampBrightness(int value) {
  if (value < 1) return 1;
  if (value > 100) return 100;
  return value;
}

}  // namespace

FrameSequence::FrameSequence(int width, int height)
    : width_(width), height_(height), frame_size_(0) {
  if (width_ > 0 && height_ > 0) {
    frame_size_ = static_cast<size_t>(width_) * static_cast<size_t>(height_) * 3;
  }
}

int FrameSequence::width() const { return width_; }
int FrameSequence::height() const { return height_; }
size_t FrameSequence::frame_count() const { return frames_.size(); }

bool FrameSequence::AddFrame(const uint8_t *rgb24,
                             size_t byte_count,
                             uint32_t hold_time_us) {
  if (rgb24 == NULL || frame_size_ == 0 || byte_count != frame_size_) {
    return false;
  }

  Frame frame;
  frame.rgb24.resize(frame_size_);
  memcpy(frame.rgb24.data(), rgb24, frame_size_);
  frame.hold_time_us = hold_time_us;
  frames_.push_back(frame);
  return true;
}

bool FrameSequence::WriteToFile(const char *path) const {
  if (path == NULL || frame_size_ == 0) {
    return false;
  }
  if (frames_.size() > 0xFFFFFFFFu) {
    return false;
  }

  FILE *f = fopen(path, "wb");
  if (f == NULL) {
    perror("Unable to open output file");
    return false;
  }

  FseqHeader header = {};
  header.magic = kFseqMagic;
  header.version = 1;
  header.width = static_cast<uint32_t>(width_);
  header.height = static_cast<uint32_t>(height_);
  header.frame_count = static_cast<uint32_t>(frames_.size());
  header.frame_size = static_cast<uint32_t>(frame_size_);

  if (fwrite(&header, sizeof(header), 1, f) != 1) {
    fclose(f);
    return false;
  }

  for (std::vector<Frame>::const_iterator it = frames_.begin();
       it != frames_.end(); ++it) {
    FseqFrameHeader fh = {};
    fh.hold_time_us = it->hold_time_us;
    if (fwrite(&fh, sizeof(fh), 1, f) != 1) {
      fclose(f);
      return false;
    }
    if (fwrite(it->rgb24.data(), 1, frame_size_, f) != frame_size_) {
      fclose(f);
      return false;
    }
  }

  const bool ok = (fclose(f) == 0);
  return ok;
}

bool FrameSequence::ReadFromFile(const char *path) {
  if (path == NULL || frame_size_ == 0) {
    return false;
  }

  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    perror("Unable to open input file");
    return false;
  }

  FseqHeader header = {};
  if (fread(&header, sizeof(header), 1, f) != 1) {
    fclose(f);
    return false;
  }

  if (header.magic != kFseqMagic || header.version != 1) {
    fclose(f);
    fprintf(stderr, "Unsupported .fseq format in '%s'\n", path);
    return false;
  }

  if ((int) header.width != width_ || (int) header.height != height_) {
    fclose(f);
    fprintf(stderr,
            "Input .fseq is %ux%u but current matrix target is %dx%d\n",
            header.width, header.height, width_, height_);
    return false;
  }

  if (header.frame_size != frame_size_) {
    fclose(f);
    fprintf(stderr,
            "Input .fseq frame payload size %u does not match expected %u\n",
            header.frame_size, (unsigned) frame_size_);
    return false;
  }

  std::vector<Frame> loaded;
  loaded.reserve(header.frame_count);
  for (uint32_t i = 0; i < header.frame_count; ++i) {
    FseqFrameHeader fh = {};
    if (fread(&fh, sizeof(fh), 1, f) != 1) {
      fclose(f);
      return false;
    }

    Frame frame;
    frame.hold_time_us = fh.hold_time_us;
    frame.rgb24.resize(frame_size_);
    if (fread(frame.rgb24.data(), 1, frame_size_, f) != frame_size_) {
      fclose(f);
      return false;
    }
    loaded.push_back(frame);
  }

  if (fclose(f) != 0) {
    return false;
  }

  frames_.swap(loaded);
  return true;
}

void FrameSequence::Clear() {
  frames_.clear();
}

void FrameSequence::PlayForever(
    RGBMatrix *matrix,
    const StopProvider &stop_provider,
    const BrightnessProvider &brightness_provider) const {
  auto always_continue = []() {
    return false;
  };
  while (!stop_provider || !stop_provider()) {
    if (!PlayOneSequence(matrix, stop_provider, brightness_provider, always_continue)) {
      break;
    }
  }
}

void FrameSequence::PlayCount(
    RGBMatrix *matrix,
    uint32_t play_count,
    const StopProvider &stop_provider,
    const BrightnessProvider &brightness_provider) const {
  if (play_count == 0) {
    return;
  }
  auto always_continue = []() {
    return false;
  };
  for (uint32_t i = 0; i < play_count; ++i) {
    if (stop_provider && stop_provider()) {
      break;
    }
    if (!PlayOneSequence(matrix, stop_provider, brightness_provider, always_continue)) {
      break;
    }
  }
}

void FrameSequence::PlayDuration(
    RGBMatrix *matrix,
    uint32_t duration_ms,
    const StopProvider &stop_provider,
    const BrightnessProvider &brightness_provider) const {
  if (duration_ms == 0) {
    return;
  }

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(duration_ms);
  auto should_stop = [&deadline]() {
    return std::chrono::steady_clock::now() >= deadline;
  };

  while ((!stop_provider || !stop_provider()) && !should_stop()) {
    if (!PlayOneSequence(matrix, stop_provider, brightness_provider, should_stop)) {
      break;
    }
  }
}

void FrameSequence::Play(
    RGBMatrix *matrix,
    volatile bool *interrupt_received,
    const BrightnessProvider &brightness_provider) const {
  auto stop_provider = [interrupt_received]() {
    return interrupt_received != NULL && *interrupt_received;
  };
  PlayForever(matrix, stop_provider, brightness_provider);
}

bool FrameSequence::PlayOneSequence(
    RGBMatrix *matrix,
    const StopProvider &stop_provider,
    const BrightnessProvider &brightness_provider,
    const std::function<bool(void)> &should_stop) const {
  if (matrix == NULL || frames_.empty()) {
    return false;
  }

  if (matrix->width() != width_ || matrix->height() != height_) {
    fprintf(stderr, "FrameSequence is %dx%d but matrix is %dx%d\n",
            width_, height_, matrix->width(), matrix->height());
    return false;
  }

  FrameCanvas *offscreen = matrix->SwapOnVSync(NULL);
  if (offscreen == NULL) {
    return false;
  }

  int last_brightness = -1;
  for (std::vector<Frame>::const_iterator it = frames_.begin();
       it != frames_.end(); ++it) {
    if ((stop_provider && stop_provider()) || (should_stop && should_stop())) {
      break;
    }

    if (brightness_provider) {
      const int wanted = ClampBrightness(brightness_provider());
      if (wanted != last_brightness) {
        matrix->SetBrightness(wanted);
        last_brightness = wanted;
      }
    }

    const uint8_t *p = it->rgb24.data();
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        offscreen->SetPixel(x, y, p[0], p[1], p[2]);
        p += 3;
      }
    }

    offscreen = matrix->SwapOnVSync(offscreen);
    if (it->hold_time_us > 0) {
      uint32_t remaining_us = it->hold_time_us;
      while (remaining_us > 0) {
        if ((stop_provider && stop_provider()) || (should_stop && should_stop())) {
          break;
        }

        const useconds_t sleep_us = remaining_us > kStopPollIntervalUs ? kStopPollIntervalUs : remaining_us;
        usleep(sleep_us);
        remaining_us -= sleep_us;
      }
    }
  }

  return true;
}

}  // namespace rgb_matrix