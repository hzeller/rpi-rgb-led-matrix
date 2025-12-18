using RPiRgbLEDMatrix;
using System.Runtime.InteropServices;
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
    Console.WriteLine("Invalid or missing file path. Please enter the path to a valid GIF file:");
    path = Console.ReadLine()!;

    // Keep prompting until a valid file is provided
    while (!File.Exists(path))
    {
        Console.WriteLine("File does not exist. Please enter a valid file path:");
        path = Console.ReadLine()!;
    }
}

Configuration.Default.PreferContiguousImageBuffers = true;
using var image = Image.Load<Rgb24>(path);

//using var matrix = new RGBLedMatrix(32, 2, 1);
UserLogger.LogUser("Before maxtrix initialization:");
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
