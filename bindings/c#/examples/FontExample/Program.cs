using RPiRgbLEDMatrix;

if (args.Length < 1)
{
    Console.WriteLine("font-example.exe [font_path] <text>");
    return -1;
}
string text = "Hello World!";
if (args.Length > 1)
    text = args[1];


using var matrix = new RGBLedMatrix(32, 2, 1);
var canvas = matrix.CreateOffscreenCanvas();
using var font = new RGBLedFont(args[0]);

canvas.DrawText(font, 1, 6, new Color(0, 255, 0), text);
matrix.SwapOnVsync(canvas);

// run until user presses Ctrl+C
var running = true;
Console.CancelKeyPress += (_, e) =>
{
    running = false;
    e.Cancel = true; // do not terminate program with Ctrl+C, we need to dispose
};
while (running) Thread.Yield();

return 0;
