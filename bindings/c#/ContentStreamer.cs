namespace RPiRgbLEDMatrix
{
    public class ContentStreamer : IDisposable
    {
        private IntPtr _streamIO;
        private IntPtr _reader;
        private bool _disposed;

        /// <summary>
        /// Initializes a new instance of the <see cref="ContentStreamer"/> class.
        /// </summary>
        /// <param name="filename">The path to the stream file.</param>
        public ContentStreamer(string filename)
        {
            _streamIO = Bindings.file_stream_io_create(filename);
            if (_streamIO == IntPtr.Zero)
                throw new InvalidOperationException($"Failed to open stream file: {filename}");
            _reader = Bindings.content_stream_reader_create(_streamIO);
            if (_reader == IntPtr.Zero)
            {
                Bindings.file_stream_io_delete(_streamIO);
                throw new InvalidOperationException("Failed to create stream reader");
            }
        }

        /// <summary>
        /// Rewinds the stream to the beginning.
        /// </summary>
        public void Rewind()
        {
            Bindings.content_stream_reader_rewind(_reader);
        }

        /// <summary>
        /// Gets the next frame from the stream.
        /// </summary>
        /// <param name="frameCanvas">The canvas to render the frame onto.</param>
        /// <param name="holdTimeUs">The time to hold the frame in microseconds.</param>
        /// <returns>True if a frame was retrieved; otherwise, false.</returns
        public bool GetNext(IntPtr frameCanvas, out uint holdTimeUs)
        {
            holdTimeUs = 0;
            return Bindings.content_stream_reader_get_next(_reader, frameCanvas, out holdTimeUs);
        }


        /// <summary>
        /// Disposes the resources used by the <see cref="ContentStreamer"/> instance.
        /// </summary>
        public void Dispose()
        {
            if (!_disposed)
            {
                if (_reader != IntPtr.Zero)
                {
                    Bindings.content_stream_reader_destroy(_reader);
                    _reader = IntPtr.Zero;
                }
                if (_streamIO != IntPtr.Zero)
                {
                    Bindings.file_stream_io_delete(_streamIO);
                    _streamIO = IntPtr.Zero;
                }
                _disposed = true;
            }
        }
    }
}
