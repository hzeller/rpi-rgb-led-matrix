// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "content-streamer.h"
#include "led-matrix.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>

namespace rgb_matrix {

namespace {
// We write magic values as integers to automatically detect endian issues.
// Streams are stored in little-endian. This is the ARM default (running
// the Raspberry Pi, but also x86; so it is possible to create streams easily
// on a different x86 Linux PC.
static const uint32_t kFileMagicValue = 0xED0C5A48;
struct FileHeader {
  uint32_t magic;  // kFileMagicValue
  uint32_t buf_size;
  uint32_t width;
  uint32_t height;
  uint64_t future_use1;
  uint64_t future_use2;
};

static const uint32_t kFrameMagicValue = 0x12345678;
struct FrameHeader {
  uint32_t magic;  // kFrameMagic
  uint32_t size;
  uint32_t hold_time_us;  // How long this frame lasts in usec.
  uint32_t future_use1;
  uint64_t future_use2;
  uint64_t future_use3;
};
}

FileStreamIO::FileStreamIO(int fd) : fd_(fd) {}
FileStreamIO::~FileStreamIO() { close(fd_); }

void FileStreamIO::Rewind() { lseek(fd_, 0, SEEK_SET); }

ssize_t FileStreamIO::Read(void *buf, const size_t count) {
  return read(fd_, buf, count);
}

ssize_t FileStreamIO::Append(const void *buf, const size_t count) {
  return write(fd_, buf, count);
}

void MemStreamIO::Rewind() { pos_ = 0; }
ssize_t MemStreamIO::Read(void *buf, size_t count) {
  const size_t amount = std::min(count, buffer_.size() - pos_);
  memcpy(buf, buffer_.data() + pos_, amount);
  pos_ += amount;
  return amount;
}
ssize_t MemStreamIO::Append(const void *buf, size_t count) {
  buffer_.append((const char*)buf, count);
  return count;
}

static ssize_t FullRead(StreamIO *io, void *buf, const size_t count) {
  int remaining = count;
  char *char_buffer = (char*)buf;
  while (remaining > 0) {
    int r = io->Read(char_buffer, remaining);
    if (r < 0) return r;
    if (r == 0) break;  // EOF.
    char_buffer += r; remaining -= r;
  }
  return count - remaining;
}

static ssize_t FullAppend(StreamIO *io, const void *buf, const size_t count) {
  int remaining = count;
  const char *char_buffer = (const char*) buf;
  while (remaining > 0) {
    int w = io->Append(char_buffer, remaining);
    if (w < 0) return w;
    char_buffer += w; remaining -= w;
  }
  return count;
}

StreamWriter::StreamWriter(StreamIO *io) : io_(io), header_written_(false) {}
bool StreamWriter::Stream(const FrameCanvas &frame, uint32_t hold_time_us) {
  const char *data;
  size_t len;
  frame.Serialize(&data, &len);

  if (!header_written_) {
    WriteFileHeader(frame, len);
  }
  FrameHeader h = {};
  h.magic = kFrameMagicValue;
  h.size = len;
  h.hold_time_us = hold_time_us;
  FullAppend(io_, &h, sizeof(h));
  return FullAppend(io_, data, len) == (ssize_t)len;
}

void StreamWriter::WriteFileHeader(const FrameCanvas &frame, size_t len) {
  FileHeader header = {};
  header.magic = kFileMagicValue;
  header.width = frame.width();
  header.height = frame.height();
  header.buf_size = len;
  FullAppend(io_, &header, sizeof(header));
  header_written_ = true;
}

StreamReader::StreamReader(StreamIO *io)
  : io_(io), state_(STREAM_AT_BEGIN), buffer_(NULL) {
  io_->Rewind();
}
StreamReader::~StreamReader() { delete [] buffer_; }

void StreamReader::Rewind() {
  io_->Rewind();
  state_ = STREAM_AT_BEGIN;
}

bool StreamReader::GetNext(FrameCanvas *frame, uint32_t* hold_time_us) {
  if (state_ == STREAM_AT_BEGIN && !ReadFileHeader(*frame)) return false;
  if (state_ != STREAM_READING) return false;
  FrameHeader h;
  if (FullRead(io_, &h, sizeof(h)) != sizeof(h)) return false;

  // TODO: we might allow for this to be a kFileMagicValue, to allow people
  // to just concatenate streams. In that case, we just would need to read
  // ahead past this header (both headers are designed to be same size)
  if (h.magic != kFrameMagicValue) {
    state_ = STREAM_ERROR;
    return false;
  }
  // In the future, we might allow larger buffers (audio?), but never smaller.
  if (h.size < buf_size_)
    return false;
  if (hold_time_us) *hold_time_us = h.hold_time_us;
  if (FullRead(io_, buffer_, buf_size_) != (ssize_t)buf_size_) return false;
  return frame->Deserialize(buffer_, buf_size_);
}

bool StreamReader::ReadFileHeader(const FrameCanvas &frame) {
  FileHeader header;
  FullRead(io_, &header, sizeof(header));
  if (header.magic != kFileMagicValue) {
    state_ = STREAM_ERROR;
    return false;
  }
  if ((int)header.width != frame.width()
      || (int)header.height != frame.height()) {
    fprintf(stderr, "This stream is for %dx%d, can't play on %dx%d. "
            "Please use the same settings for record/replay\n",
            header.width, header.height, frame.width(), frame.height());
    state_ = STREAM_ERROR;
    return false;
  }
  state_ = STREAM_READING;
  buf_size_ = header.buf_size;
  if (!buffer_) buffer_ = new char [ header.buf_size ];
  return true;
}
}  // namespace rgb_matrix
