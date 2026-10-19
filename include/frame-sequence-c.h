/* C API for frame-sequence (C wrapper for frame-sequence.h) */
#ifndef FRAME_SEQUENCE_C_H
#define FRAME_SEQUENCE_C_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque FrameSequence handle */
typedef void* FrameSequenceHandle;

/* Forward declaration from led-matrix-c.h */
struct RGBLedMatrix;

/* Lifetime */
FrameSequenceHandle frame_sequence_create(int width, int height);
void frame_sequence_destroy(FrameSequenceHandle sequence);

/* Metadata */
int frame_sequence_width(FrameSequenceHandle sequence);
int frame_sequence_height(FrameSequenceHandle sequence);
size_t frame_sequence_frame_count(FrameSequenceHandle sequence);

/* Content mutation */
int frame_sequence_add_frame(FrameSequenceHandle sequence,
                             const uint8_t *rgb24,
                             size_t byte_count,
                             uint32_t hold_time_us);
void frame_sequence_clear(FrameSequenceHandle sequence);

/* File I/O */
int frame_sequence_write_to_file(FrameSequenceHandle sequence, const char *path);
int frame_sequence_read_from_file(FrameSequenceHandle sequence, const char *path);

/*
 * Playback:
 * - interrupt_received: optional pointer; playback stops when *interrupt_received != 0.
 * - brightness_percent: optional pointer; if set, value is polled per frame.
 */
void frame_sequence_play_forever(FrameSequenceHandle sequence,
                                 struct RGBLedMatrix *matrix,
                                 const volatile int *interrupt_received,
                                 const int *brightness_percent);
void frame_sequence_play_count(FrameSequenceHandle sequence,
                               struct RGBLedMatrix *matrix,
                               uint32_t play_count,
                               const volatile int *interrupt_received,
                               const int *brightness_percent);
void frame_sequence_play_duration(FrameSequenceHandle sequence,
                                  struct RGBLedMatrix *matrix,
                                  uint32_t duration_ms,
                                  const volatile int *interrupt_received,
                                  const int *brightness_percent);

/* Legacy compatibility wrapper for older callers. */
void frame_sequence_play(FrameSequenceHandle sequence,
                         struct RGBLedMatrix *matrix,
                         const volatile int *interrupt_received,
                         const int *brightness_percent);

#ifdef __cplusplus
}
#endif

#endif
