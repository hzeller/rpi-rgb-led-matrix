using rpi_rgb_led_matrix_sharp;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace matrix_rain
{
    class Program
    {
        const int MAX_HEIGHT = 16;
        const int COLOR_STEP = 15;
        const int FRAME_STEP = 1;

        static int Main(string[] args)
        {

            var matrix = new RGBLedMatrix(new RGBLedMatrixOptions { ChainLength = 2 });
            var canvas = matrix.CreateOffscreenCanvas();
            var rnd = new Random();
            var points = new List<Point>();
            var recycled = new Stack<Point>();
            int frame = 0;
            var stopwatch = new Stopwatch();

            while (!Console.KeyAvailable) {
                stopwatch.Restart();

                frame++;

                if (frame % FRAME_STEP == 0)
                {
                    if (recycled.Count == 0)
                        points.Add(new Point(rnd.Next(0, canvas.Width - 1), 0));
                    else
                    {
                        var point = recycled.Pop();
                        point.x = rnd.Next(0, canvas.Width - 1);
                        point.y = 0;
                        point.recycled = false;
                    }
                }

                canvas.Clear();

                foreach (var point in points)
                {
                    if (!point.recycled)
                    {
                        point.y++;

                        if (point.y - MAX_HEIGHT > canvas.Height)
                        {
                            point.recycled = true;
                            recycled.Push(point);
                        }

                        for (var i=0; i< MAX_HEIGHT; i++)
                        {
                            canvas.SetPixel(point.x, point.y - i, new Color(0, 255 - i * COLOR_STEP, 0));
                        }
                    }
                }
                
                canvas = matrix.SwapOnVsync(canvas);

                // force 30 FPS
                var elapsed= stopwatch.ElapsedMilliseconds;
                if (elapsed < 33)
                {
                    Thread.Sleep(33 - (int)elapsed);
                }
            }

            return 0;
        }

        class Point
        {
            public Point(int x, int y)
            {
                this.x = x;
                this.y = y;
            }
            public int x;
            public int y;
            public bool recycled;
        }
    }
}
