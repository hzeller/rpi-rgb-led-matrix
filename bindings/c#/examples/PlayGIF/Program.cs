using RPiRgbLEDMatrix;
using System.Runtime.InteropServices;
using Color = RPiRgbLEDMatrix.Color;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;
using SixLabors.ImageSharp.Processing;

Console.Write("GIF path: ");
var path = Console.ReadLine()!;

using var matrix = new RGBLedMatrix(32, 2, 1);
var canvas = matrix.CreateOffscreenCanvas();

Configuration.Default.PreferContiguousImageBuffers = true;
using var image = Image.Load<Rgb24>(path);
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
