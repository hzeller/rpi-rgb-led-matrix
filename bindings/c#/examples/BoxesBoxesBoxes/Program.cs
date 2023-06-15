using RPiRgbLEDMatrix;

// Set this to apropriate settings for your matrix
using var matrix = new RGBLedMatrix(new RGBLedMatrixOptions()
{
    Cols = 64,
    Rows = 64,
    GpioSlowdown = 3,
});

var canvas = matrix.CreateOffscreenCanvas();

Random R = new();
int maxBoxHalfWidth = 22;
int maxBoxHalfHeight = 22;
int borderDiff = 80;
var stopAt = DateTime.Now.AddSeconds(20);

while (DateTime.Now < stopAt)
{
    var nextR = R.Next(0, 220);
    var nextG = R.Next(0, 220);
    var nextB = R.Next(0, 220);

    var nextColorA = new Color(Math.Max(0, nextR - borderDiff), Math.Max(0, nextG - borderDiff), Math.Max(0, nextB - borderDiff));
    var nextColorB = new Color(nextR, nextG, nextG);

    var nextX = R.Next(0, canvas.Width);
    var nextY = R.Next(0, canvas.Height);
    var nextW = R.Next(2, maxBoxHalfWidth);
    var nextH = R.Next(2, maxBoxHalfHeight);

    canvas.SubFill(nextX - nextW, nextY - nextH, nextW * 2, nextH * 2, nextColorA);
    canvas.SubFill(nextX - nextW + 1, nextY - nextH + 1, nextW * 2 - 2, nextH * 2 - 2, nextColorB);

    matrix.SwapOnVsync(canvas);

    canvas = matrix.GetCanvas();
}
