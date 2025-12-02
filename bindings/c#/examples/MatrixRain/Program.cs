using RPiRgbLEDMatrix;

const int MaxHeight = 16;
const int ColorStep = 15;
const int FrameStep = 1;

using var matrix = new RGBLedMatrix(new RGBLedMatrixOptions { ChainLength = 2 });
var canvas = matrix.CreateOffscreenCanvas();

var rnd = new Random();
var points = new List<Point>();
var recycled = new Stack<Point>();
var frame = 0;

var running = true;
Console.CancelKeyPress += (s, e) =>
{
    running = false;
    e.Cancel = true; // don't terminate, we need to dispose
};

// run until user presses Ctrl+C
while (running)
{
    var frameStart = Environment.TickCount64;
    frame++;

    if (frame % FrameStep == 0)
    {
        if (recycled.Count == 0)
            points.Add(new Point(rnd.Next(0, canvas.Width - 1), 0));
        else
        {
            var point = recycled.Pop();
            point.X = rnd.Next(0, canvas.Width - 1);
            point.Y = 0;
            point.Recycled = false;
        }
    }

    canvas.Clear();

    foreach (var point in points)
    {
        if (point.Recycled) continue;
        point.Y++;

        if (point.Y - MaxHeight > canvas.Height)
        {
            point.Recycled = true;
            recycled.Push(point);
        }

        for (var i = 0; i < MaxHeight; i++)
        {
            canvas.SetPixel(point.X, point.Y - i, new Color(0, 255 - i * ColorStep, 0));
        }
    }

    matrix.SwapOnVsync(canvas);

    // force 30 FPS
    var elapsed = Environment.TickCount64 - frameStart;
    if (elapsed < 33) Thread.Sleep(33 - (int)elapsed);
}

class Point
{
    public int X;
    public int Y;
    public bool Recycled = false;

    public Point(int x, int y) => (X, Y) = (x, y);
}
