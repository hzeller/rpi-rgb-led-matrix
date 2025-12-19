using RPiRgbLEDMatrix;
using System.Runtime.InteropServices;
using System.IO;
using Color = RPiRgbLEDMatrix.Color;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;
using SixLabors.ImageSharp.Processing;

string path;

// Check if the last command-line argument is provided and is a valid file
if (args.Length > 0 && File.Exists(args[^1]))
{
    path = args[^1];
}
else
{
    do
    {
        Console.WriteLine("Enter a path to a media file (GIF, PNG etc), or a stream file:");
        path = Console.ReadLine()!;
    } while (!File.Exists(path));
}

// Check if the file is a stream
bool isStream = false;
using (var fileStream = new FileStream(path, FileMode.Open, FileAccess.Read))
{
    byte[] magicBytes = new byte[4];
    fileStream.Read(magicBytes, 0, 4);
    isStream = BitConverter.ToUInt32(magicBytes, 0) == 0xED0C5A48; // kFileMagicValue
}

if (isStream)
{
    Console.WriteLine("Playing stream file.");
    PlayStream(path);
}
else
{
    Console.WriteLine("Detected regular file.");
    PlayMedia(path);
}

void PlayMedia(string mediaPath)
{
    Configuration.Default.PreferContiguousImageBuffers = true;
    using var image = Image.Load<Rgb24>(mediaPath);

    UserLogger.LogUser("Before matrix initialization:");
    using var matrix = new RGBLedMatrix(new RGBLedMatrixOptions()
    {
        Cols = 128,
        Rows = 64,
        Parallel = 2,
        GpioSlowdown = 2,
        RowAddressType = 5
    });
    var canvas = matrix.CreateOffscreenCanvas();

    image.Mutate(o => o.Resize(canvas.Width, canvas.Height));

    var running = true;
    Console.CancelKeyPress += (s, e) =>
    {
        running = false;
        e.Cancel = true; // don't terminate, we need to dispose
    };

    var frame = -1;
    // preprocess frames to get delays and pixel buffers
    var frames = image.Frames
        .Select(f => (
            Pixels: f.DangerousTryGetSinglePixelMemory(out var memory) ? memory : throw new("Could not get pixel buffer"),
            Delay: f.Metadata.GetGifMetadata().FrameDelay * 10
        )).ToArray();

    // run until user presses Ctrl+C
    while (running)
    {
        frame = (frame + 1) % frames.Length;

        var data = MemoryMarshal.Cast<Rgb24, Color>(frames[frame].Pixels.Span);
        canvas.SetPixels(0, 0, canvas.Width, canvas.Height, data);

        matrix.SwapOnVsync(canvas);
        Thread.Sleep(frames[frame].Delay);
    }
}

void PlayStream(string streamPath)
{
    using var fileStream = new FileStream(streamPath, FileMode.Open, FileAccess.Read);
    int fd = (int)fileStream.SafeFileHandle.DangerousGetHandle(); // Explicit cast to int
    IntPtr io = Bindings.file_stream_io_create(fd);
    if (io == IntPtr.Zero)
        throw new InvalidOperationException("Failed to create FileStreamIO.");

    using var reader = new StreamReaderWrapper(io);
    using var matrix = new RGBLedMatrix(new RGBLedMatrixOptions()
    {
        Cols = 128,
        Rows = 64,
        Parallel = 2,
        GpioSlowdown = 2,
        RowAddressType = 5
    });
    var canvas = matrix.CreateOffscreenCanvas();

    var running = true;
    Console.CancelKeyPress += (s, e) =>
    {
        running = false;
        e.Cancel = true; // don't terminate, we need to dispose
    };

    while (running)
    {
        uint delay;
        if (!reader.GetNext(canvas, out delay))
        {
            reader.Rewind();
            continue;
        }

        matrix.SwapOnVsync(canvas);
        Thread.Sleep((int)(delay / 1000)); // Convert microseconds to milliseconds
    }

    // Clean up the FileStreamIO instance
    Bindings.file_stream_io_delete(io);
}

// UID/EUID logging helpers
public static class UserLogger
{
    [DllImport("libc")]
    public static extern uint getuid();
    [DllImport("libc")]
    public static extern uint geteuid();
    public static void LogUser(string label)
    {
        Console.WriteLine($"{label}: User={Environment.UserName}, UID={getuid()}, EUID={geteuid()}");
    }
}
