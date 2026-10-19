// C wrapper implementations for frame-sequence C API
// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "frame-sequence-c.h"

#include "frame-sequence.h"
#include "led-matrix.h"

extern "C" {

FrameSequenceHandle frame_sequence_create(int width, int height) {
  if (width <= 0 || height <= 0) return nullptr;
  return reinterpret_cast<FrameSequenceHandle>(new rgb_matrix::FrameSequence(width, height));
}

void frame_sequence_destroy(FrameSequenceHandle sequence) {
  delete reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
}

int frame_sequence_width(FrameSequenceHandle sequence) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  if (s == nullptr) return 0;
  return s->width();
}

int frame_sequence_height(FrameSequenceHandle sequence) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  if (s == nullptr) return 0;
  return s->height();
}

size_t frame_sequence_frame_count(FrameSequenceHandle sequence) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  if (s == nullptr) return 0;
  return s->frame_count();
}

int frame_sequence_add_frame(FrameSequenceHandle sequence,
                             const uint8_t *rgb24,
                             size_t byte_count,
                             uint32_t hold_time_us) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  if (s == nullptr) return 0;
  return s->AddFrame(rgb24, byte_count, hold_time_us) ? 1 : 0;
}

void frame_sequence_clear(FrameSequenceHandle sequence) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  if (s == nullptr) return;
  s->Clear();
}

int frame_sequence_write_to_file(FrameSequenceHandle sequence, const char *path) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  if (s == nullptr || path == nullptr) return 0;
  return s->WriteToFile(path) ? 1 : 0;
}

int frame_sequence_read_from_file(FrameSequenceHandle sequence, const char *path) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  if (s == nullptr || path == nullptr) return 0;
  return s->ReadFromFile(path) ? 1 : 0;
}

static rgb_matrix::FrameSequence::StopProvider BuildStopProvider(const volatile int *interrupt_received) {
  rgb_matrix::FrameSequence::StopProvider stop_provider;
  if (interrupt_received != nullptr) {
    stop_provider = [interrupt_received]() {
      return *interrupt_received != 0;
    };
  }
  return stop_provider;
}

static rgb_matrix::FrameSequence::BrightnessProvider BuildBrightnessProvider(const int *brightness_percent) {
  rgb_matrix::FrameSequence::BrightnessProvider brightness_provider;
  if (brightness_percent != nullptr) {
    brightness_provider = [brightness_percent]() {
      return *brightness_percent;
    };
  }
  return brightness_provider;
}

void frame_sequence_play_forever(FrameSequenceHandle sequence,
                                 struct RGBLedMatrix *matrix,
                                 const volatile int *interrupt_received,
                                 const int *brightness_percent) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  auto m = reinterpret_cast<rgb_matrix::RGBMatrix*>(matrix);
  if (s == nullptr || m == nullptr) return;

  s->PlayForever(m, BuildStopProvider(interrupt_received), BuildBrightnessProvider(brightness_percent));
}

void frame_sequence_play_count(FrameSequenceHandle sequence,
                               struct RGBLedMatrix *matrix,
                               uint32_t play_count,
                               const volatile int *interrupt_received,
                               const int *brightness_percent) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  auto m = reinterpret_cast<rgb_matrix::RGBMatrix*>(matrix);
  if (s == nullptr || m == nullptr) return;

  s->PlayCount(m, play_count, BuildStopProvider(interrupt_received), BuildBrightnessProvider(brightness_percent));
}

void frame_sequence_play_duration(FrameSequenceHandle sequence,
                                  struct RGBLedMatrix *matrix,
                                  uint32_t duration_ms,
                                  const volatile int *interrupt_received,
                                  const int *brightness_percent) {
  auto s = reinterpret_cast<rgb_matrix::FrameSequence*>(sequence);
  auto m = reinterpret_cast<rgb_matrix::RGBMatrix*>(matrix);
  if (s == nullptr || m == nullptr) return;

  s->PlayDuration(m, duration_ms, BuildStopProvider(interrupt_received), BuildBrightnessProvider(brightness_percent));
}

void frame_sequence_play(FrameSequenceHandle sequence,
                         struct RGBLedMatrix *matrix,
                         const volatile int *interrupt_received,
                         const int *brightness_percent) {
  frame_sequence_play_forever(sequence, matrix, interrupt_received, brightness_percent);
}

}  // extern "C"
