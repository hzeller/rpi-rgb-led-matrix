using RPiRgbLEDMatrix;

using var matrix = new RGBLedMatrix(new RGBLedMatrixOptions { Rows = 32, Cols = 64 });
var canvas = matrix.CreateOffscreenCanvas();

var maxBrightness = matrix.Brightness;
var rnd = new Random();

// run until user presses Ctrl+C
var running = true;
Console.CancelKeyPress += (_, e) =>
{
    running = false;
    e.Cancel = true; // do not terminate program with Ctrl+C, we need to dispose
};

var color = new Color(rnd.Next(0, 256), rnd.Next(0, 256), rnd.Next(0, 256));
while (running)
{
    if (matrix.Brightness < 1)
    {
        matrix.Brightness = maxBrightness;
        color = new Color(rnd.Next(0, 256), rnd.Next(0, 256), rnd.Next(0, 256));
    }
    else
    {
        matrix.Brightness--;
    }

    canvas.Fill(color);
    matrix.SwapOnVsync(canvas);
    Thread.Sleep(20);
}
