using RPiRgbLEDMatrix;

if (args.Length < 1)
{
    Console.WriteLine("font-example.exe [font_path] <text>");
    return -1;
}
string text = "Hello World!";
if (args.Length > 1)
    text = args[1];

// Set this to apropriate settings for your matrix
var options = new RGBLedMatrixOptions()
{
    Brightness = 80,
    Cols = 64,
    Rows = 64,
    GpioSlowdown = 4
};

using var matrix = new RGBLedMatrix(options);

var canvas = matrix.CreateOffscreenCanvas();
using var font = new RGBLedFont(args[0]);


// The drawing position values may need to change to work for your chosen matrix / font combination.
canvas.DrawText(font, 1, 16, new Color(0, 255, 0), text);
matrix.SwapOnVsync(canvas);

// run until user presses Ctrl+C
var running = true;
Console.CancelKeyPress += (_, e) =>
{
    running = false;
    e.Cancel = true; // do not terminate program with Ctrl+C, we need to dispose.
};
while (running) Thread.Yield();

return 0;
