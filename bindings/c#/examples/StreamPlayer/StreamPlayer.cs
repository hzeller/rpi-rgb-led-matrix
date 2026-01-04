using RPiRgbLEDMatrix;

public class StreamPlayer
{
    private readonly RGBLedMatrix _matrix;

    public StreamPlayer(RGBLedMatrixOptions options)
    {
        _matrix = new RGBLedMatrix(options);
    }


    private bool IsStream(string path)
    {
        // ToDo: Should this not be moved into ContentStreamer?
        // At the very least, use kFileMagicValue from content-streamer.cc
        // There appears to be code in there (ReadFileHeader) that does this already...
        // ... but it appears to log to stderr on failure, which may not be desired.
        using var fileStream = new FileStream(path, FileMode.Open, FileAccess.Read);
        byte[] magicBytes = new byte[4];
        fileStream.Read(magicBytes, 0, 4);
        return BitConverter.ToUInt32(magicBytes, 0) == 0xED0C5A48; // kFileMagicValue
    }

    public void PlayStream(string streamPath)
    {
        if (!IsStream(streamPath))
        {
            throw new NotSupportedException("Only stream files are supported in this example.");
        }
        // Replace manual FileStreamIO and Bindings usage with ContentStreamer
        using var contentStreamer = new ContentStreamer(streamPath);
        var canvas = _matrix.CreateOffscreenCanvas();

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
