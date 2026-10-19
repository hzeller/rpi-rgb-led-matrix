using System.Runtime.InteropServices;

namespace RPiRgbLEDMatrix;

/// <summary>
/// Managed wrapper for the native FrameSequence API.
/// Frames are packed RGB24 bytes in display order.
/// </summary>
public sealed class FrameSequence : IDisposable
{
    private IntPtr _sequence;
    private bool _disposed;

    public FrameSequence(int width, int height)
    {
        _sequence = frame_sequence_create(width, height);
        if (_sequence == IntPtr.Zero)
            throw new ArgumentException("Failed to create FrameSequence");
    }

    public int Width => frame_sequence_width(_sequence);
    public int Height => frame_sequence_height(_sequence);
    public nuint FrameCount => frame_sequence_frame_count(_sequence);

    public void AddFrame(byte[] rgb24, uint holdTimeUs)
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        ArgumentNullException.ThrowIfNull(rgb24);
        if (!frame_sequence_add_frame(_sequence, rgb24, (nuint)rgb24.Length, holdTimeUs))
            throw new ArgumentException("Failed to add frame. Ensure RGB24 length is width*height*3.");
    }

    public void Clear()
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        frame_sequence_clear(_sequence);
    }

    public void ReadFromFile(string path)
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        if (!frame_sequence_read_from_file(_sequence, path))
            throw new InvalidOperationException($"Failed to read frame sequence from '{path}'.");
    }

    public void WriteToFile(string path)
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        if (!frame_sequence_write_to_file(_sequence, path))
            throw new InvalidOperationException($"Failed to write frame sequence to '{path}'.");
    }

    /// <summary>
    /// Play the sequence forever.
    /// Stop by setting interruptReceived[0] to non-zero.
    /// Adjust brightness dynamically by updating brightnessPercent[0].
    /// </summary>
    public void PlayForever(RGBLedMatrix matrix, int[] interruptReceived, int[] brightnessPercent)
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        ArgumentNullException.ThrowIfNull(matrix);
        ArgumentNullException.ThrowIfNull(interruptReceived);
        ArgumentNullException.ThrowIfNull(brightnessPercent);
        if (interruptReceived.Length < 1)
            throw new ArgumentException("interruptReceived must have at least one element.");
        if (brightnessPercent.Length < 1)
            throw new ArgumentException("brightnessPercent must have at least one element.");

        var stopHandle = GCHandle.Alloc(interruptReceived, GCHandleType.Pinned);
        var brightHandle = GCHandle.Alloc(brightnessPercent, GCHandleType.Pinned);
        try
        {
            frame_sequence_play_forever(_sequence, matrix.NativeHandle,
                                        stopHandle.AddrOfPinnedObject(),
                                        brightHandle.AddrOfPinnedObject());
        }
        finally
        {
            brightHandle.Free();
            stopHandle.Free();
        }
    }

    /// <summary>
    /// Play the sequence a fixed number of times.
    /// Stop by setting interruptReceived[0] to non-zero.
    /// Adjust brightness dynamically by updating brightnessPercent[0].
    /// </summary>
    public void PlayCount(RGBLedMatrix matrix, uint playCount, int[] interruptReceived, int[] brightnessPercent)
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        ArgumentNullException.ThrowIfNull(matrix);
        ArgumentNullException.ThrowIfNull(interruptReceived);
        ArgumentNullException.ThrowIfNull(brightnessPercent);
        if (interruptReceived.Length < 1)
            throw new ArgumentException("interruptReceived must have at least one element.");
        if (brightnessPercent.Length < 1)
            throw new ArgumentException("brightnessPercent must have at least one element.");

        var stopHandle = GCHandle.Alloc(interruptReceived, GCHandleType.Pinned);
        var brightHandle = GCHandle.Alloc(brightnessPercent, GCHandleType.Pinned);
        try
        {
            frame_sequence_play_count(_sequence, matrix.NativeHandle, playCount,
                                      stopHandle.AddrOfPinnedObject(),
                                      brightHandle.AddrOfPinnedObject());
        }
        finally
        {
            brightHandle.Free();
            stopHandle.Free();
        }
    }

    /// <summary>
    /// Play the sequence for a fixed duration in milliseconds.
    /// </summary>
    public void PlayDuration(RGBLedMatrix matrix, uint durationMs, int[] interruptReceived, int[] brightnessPercent)
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        ArgumentNullException.ThrowIfNull(matrix);
        ArgumentNullException.ThrowIfNull(interruptReceived);
        ArgumentNullException.ThrowIfNull(brightnessPercent);
        if (interruptReceived.Length < 1)
            throw new ArgumentException("interruptReceived must have at least one element.");
        if (brightnessPercent.Length < 1)
            throw new ArgumentException("brightnessPercent must have at least one element.");

        var stopHandle = GCHandle.Alloc(interruptReceived, GCHandleType.Pinned);
        var brightHandle = GCHandle.Alloc(brightnessPercent, GCHandleType.Pinned);
        try
        {
            frame_sequence_play_duration(_sequence, matrix.NativeHandle, durationMs,
                                         stopHandle.AddrOfPinnedObject(),
                                         brightHandle.AddrOfPinnedObject());
        }
        finally
        {
            brightHandle.Free();
            stopHandle.Free();
        }
    }

    /// <summary>
    /// Legacy alias for forever playback.
    /// </summary>
    public void PlayForever(RGBLedMatrix matrix)
    {
        ObjectDisposedException.ThrowIf(_sequence == IntPtr.Zero, GetType());
        ArgumentNullException.ThrowIfNull(matrix);
        frame_sequence_play_forever(_sequence, matrix.NativeHandle, IntPtr.Zero, IntPtr.Zero);
    }

    /// <summary>
    /// Legacy alias for forever playback.
    /// </summary>
    public void Play(RGBLedMatrix matrix)
    {
        PlayForever(matrix);
    }

    /// <summary>
    /// Legacy alias for forever playback.
    /// </summary>
    public void Play(RGBLedMatrix matrix, int[] interruptReceived, int[] brightnessPercent)
    {
        PlayForever(matrix, interruptReceived, brightnessPercent);
    }

    public void Dispose()
    {
        if (_disposed) return;
        if (_sequence != IntPtr.Zero)
        {
            frame_sequence_destroy(_sequence);
            _sequence = IntPtr.Zero;
        }
        _disposed = true;
    }
}
