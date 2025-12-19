using System;

namespace RPiRgbLEDMatrix;

/// <summary>
/// Managed wrapper for the native StreamReader class.
/// </summary>
public class StreamReaderWrapper : IDisposable
{
    private IntPtr reader;
    private bool disposedValue = false;

    /// <summary>
    /// Initializes a new instance of the StreamReaderWrapper class.
    /// </summary>
    /// <param name="io">The StreamIO instance to read from.</param>
    public StreamReaderWrapper(IntPtr io)
    {
        reader = Bindings.stream_reader_create(io);
        if (reader == IntPtr.Zero)
            throw new InvalidOperationException("Failed to create StreamReader.");
    }

    /// <summary>
    /// Rewinds the stream to the beginning.
    /// </summary>
    public void Rewind()
    {
        Bindings.stream_reader_rewind(reader);
    }

    /// <summary>
    /// Gets the next frame and its delay.
    /// </summary>
    /// <param name="frame">The frame canvas to populate.</param>
    /// <param name="holdTimeUs">The delay for the frame in microseconds.</param>
    /// <returns>True if a frame was retrieved; otherwise, false.</returns>
    public bool GetNext(RGBLedCanvas frame, out uint holdTimeUs)
    {
        return Bindings.stream_reader_get_next(reader, frame._canvas, out holdTimeUs);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            if (reader != IntPtr.Zero)
            {
                Bindings.stream_reader_delete(reader);
                reader = IntPtr.Zero;
            }
            disposedValue = true;
        }
    }

    ~StreamReaderWrapper()
    {
        Dispose(disposing: false);
    }

    /// <inheritdoc/>
    public void Dispose()
    {
        Dispose(disposing: true);
        GC.SuppressFinalize(this);
    }
}
