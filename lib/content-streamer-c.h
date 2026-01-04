#ifndef RPI_CONTENT_STREAMER_C_H
#define RPI_CONTENT_STREAMER_C_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// Opaque type for StreamReader
typedef void* ContentStreamReaderHandle;

// Forward declaration for FrameCanvas and StreamIO
struct FrameCanvas;
struct StreamIO;

ContentStreamReaderHandle content_stream_reader_create(StreamIO* io);
void content_stream_reader_destroy(ContentStreamReaderHandle reader);
int content_stream_reader_get_next(ContentStreamReaderHandle reader, struct FrameCanvas* frame, uint32_t* hold_time_us);
void content_stream_reader_rewind(ContentStreamReaderHandle reader);
int content_stream_reader_check_file_header(ContentStreamReaderHandle reader, struct FrameCanvas* frame);

// FileStreamIO management
StreamIO* file_stream_io_create(const char* filename);
void file_stream_io_delete(StreamIO* io);

#ifdef __cplusplus
}
#endif

#endif // RPI_CONTENT_STREAMER_C_H
