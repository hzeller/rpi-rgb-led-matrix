using rpi_rgb_led_matrix_sharp;
using System;
using System.Threading;

namespace font_example
{
    class Program
    {
        static int Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("font-example.exe [font_path] <text>");
                return -1;
            }
            string text = "Hello World!";
            if (args.Length > 1)            
                text = args[1];
            

            var matrix = new RGBLedMatrix(32, 2, 1);
            var canvas = matrix.CreateOffscreenCanvas();
            var font = new RGBLedFont(args[0]);

            canvas.DrawText(font, 1, 6, new Color(0, 255, 0), text);
            matrix.SwapOnVsync(canvas);

            while (!Console.KeyAvailable)
            {
                Thread.Sleep(250);
            }

            return 0;
        }
    }
}
