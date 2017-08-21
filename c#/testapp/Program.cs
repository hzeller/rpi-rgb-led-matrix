using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using rpi_rgb_led_matrix_sharp;

namespace testapp
{
    class Program
    {
        static int Main(string[] args)
        {

            var matrix= new RGBLedMatrix(32, 2, 1);           
            var canvas = matrix.CreateOffscreenCanvas();

            for (var i = 0; i < 1000; ++i)
            {
                for (var y = 0; y < canvas.Height; ++y)
                {
                    for (var x = 0; x < canvas.Width; ++x)
                    {
                        canvas.SetPixel(x, y, new Color(i & 0xff, x, y));
                    }
                }
                canvas = matrix.SwapOnVsync(canvas);
            }

            return 0; 
        }
    }
}
