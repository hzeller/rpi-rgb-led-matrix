using System;

namespace RPiRgbLEDMatrix
{
    public class StreamReaderWrapper : IDisposable
    {
        private IntPtr _streamIO;
        private IntPtr _reader;
        private bool _disposed;

        public StreamReaderWrapper(string filename)
        {
            _streamIO = ContentStreamerBindings.FileStreamIOCreate(filename);
            if (_streamIO == IntPtr.Zero)
                throw new InvalidOperationException($"Failed to open stream file: {filename}");
            _reader = ContentStreamerBindings.ContentStreamReaderCreate(_streamIO);
            if (_reader == IntPtr.Zero)
            {
                ContentStreamerBindings.FileStreamIODelete(_streamIO);
                throw new InvalidOperationException("Failed to create stream reader");
            }
        }

        public void Rewind()
        {
            ContentStreamerBindings.ContentStreamReaderRewind(_reader);
        }

        public bool GetNext(IntPtr frameCanvas, out uint holdTimeUs)
        {
            holdTimeUs = 0;
            int result = ContentStreamerBindings.ContentStreamReaderGetNext(_reader, frameCanvas, ref holdTimeUs);
            return result != 0;
        }

        public void Dispose()
        {
            if (!_disposed)
            {
                if (_reader != IntPtr.Zero)
                {
                    ContentStreamerBindings.ContentStreamReaderDestroy(_reader);
                    _reader = IntPtr.Zero;
                }
                if (_streamIO != IntPtr.Zero)
                {
                    ContentStreamerBindings.FileStreamIODelete(_streamIO);
                    _streamIO = IntPtr.Zero;
                }
                _disposed = true;
            }
        }
    }
}
