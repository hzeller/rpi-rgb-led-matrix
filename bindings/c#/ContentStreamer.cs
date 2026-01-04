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

        /// <summary>
        /// Clears the given canvas (sets all pixels to black).
        /// </summary>
        /// <param name="frameCanvas">The canvas to clear.</param>
        public static void ClearCanvas(IntPtr frameCanvas)
        {
            ContentStreamerBindings.led_canvas_clear(frameCanvas);
        }

        /// <summary>
        /// Rewinds the stream to the beginning.
        /// </summary>
        public void Rewind()
        {
            ContentStreamerBindings.content_stream_reader_rewind(_reader);
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
            int result = ContentStreamerBindings.content_stream_reader_get_next(_reader, frameCanvas, ref holdTimeUs);
            return result != 0;
        }

        /// <summary>
        /// Checks if the stream file has a valid header.
        /// </summary>
        /// <param name="frameCanvas">The canvas to use for checking the header.</param>
        /// <returns>True if the header is valid; otherwise, false.</returns>
        public bool CheckFileHeader(IntPtr frameCanvas)
        {
            int result = ContentStreamerBindings.content_stream_reader_check_file_header(_reader, frameCanvas);
            return result != 0;
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
