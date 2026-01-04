using RPiRgbLEDMatrix;

public class StreamPlayer
{
    private readonly RGBLedMatrix _matrix;

    /// <summary>
    /// Initializes a new instance of the <see cref="StreamPlayer"/> class.
    /// </summary>
    /// <param name="options">The matrix options.</param>
    public StreamPlayer(RGBLedMatrixOptions options)
    {
        _matrix = new RGBLedMatrix(options);
    }

    /// <summary>
    /// Plays a stream file on the LED matrix.
    /// </summary>
    /// <param name="streamPath">The path to the stream file.</param>
    /// <exception cref="InvalidOperationException"></exception>
    public void PlayStream(string streamPath)
    {
        using var contentStreamer = new ContentStreamer(streamPath);
        var canvas = _matrix.CreateOffscreenCanvas();
        if (!contentStreamer.CheckFileHeader(canvas.Handle))
        {
            throw new InvalidOperationException("Invalid stream file header.");
        }
        var running = true;
        Console.CancelKeyPress += (s, e) =>
        {
            running = false;
            e.Cancel = true; // don't terminate, we need to dispose
        };

        while (running)
        {
            uint delay;
            if (!contentStreamer.GetNext(canvas.Handle, out delay))
            {
                contentStreamer.Rewind();
                continue;
            }

            _matrix.SwapOnVsync(canvas);
            Thread.Sleep((int)(delay / 1000)); // Convert microseconds to milliseconds
        }
    }
}
