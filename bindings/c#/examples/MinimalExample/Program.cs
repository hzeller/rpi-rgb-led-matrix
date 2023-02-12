using RPiRgbLEDMatrix;

using var matrix = new RGBLedMatrix(32, 2, 1);
var canvas = matrix.CreateOffscreenCanvas();

var centerX = canvas.Width / 2;
var centerY = canvas.Height / 2;
for (var i = 0; i < 1000; ++i)
{
    for (var y = 0; y < canvas.Height; ++y)
        for (var x = 0; x < canvas.Width; ++x)
            canvas.SetPixel(x, y, new Color(i & 0xFF, x, y));

    canvas.DrawCircle(centerX, centerY, 6, new Color(0, 0, 255));
    canvas.DrawLine(centerX - 3, centerY - 3, centerX + 3, centerY + 3, new Color(0, 0, 255));
    canvas.DrawLine(centerX - 3, centerY + 3, centerX + 3, centerY - 3, new Color(0, 0, 255));

    matrix.SwapOnVsync(canvas);
}
