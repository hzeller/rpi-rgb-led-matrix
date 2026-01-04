using System;

namespace RPiRgbLEDMatrix
{
    public class ContentStreamer : IDisposable
    {
        private IntPtr _streamIO;
        private IntPtr _reader;
        private bool _disposed;

        public ContentStreamer(string filename)
        {
            _streamIO = ContentStreamerBindings.file_stream_io_create(filename);
            if (_streamIO == IntPtr.Zero)
                throw new InvalidOperationException($"Failed to open stream file: {filename}");
            _reader = ContentStreamerBindings.content_stream_reader_create(_streamIO);
            if (_reader == IntPtr.Zero)
            {
                ContentStreamerBindings.file_stream_io_delete(_streamIO);
                throw new InvalidOperationException("Failed to create stream reader");
            }
        }

        public void Rewind()
        {
            ContentStreamerBindings.content_stream_reader_rewind(_reader);
        }

        public bool GetNext(IntPtr frameCanvas, out uint holdTimeUs)
        {
            holdTimeUs = 0;
            int result = ContentStreamerBindings.content_stream_reader_get_next(_reader, frameCanvas, ref holdTimeUs);
            return result != 0;
        }

        public void Dispose()
        {
            if (!_disposed)
            {
                if (_reader != IntPtr.Zero)
                {
                    ContentStreamerBindings.content_stream_reader_destroy(_reader);
                    _reader = IntPtr.Zero;
                }
                if (_streamIO != IntPtr.Zero)
                {
                    ContentStreamerBindings.file_stream_io_delete(_streamIO);
                    _streamIO = IntPtr.Zero;
                }
                _disposed = true;
            }
        }
    }
}
