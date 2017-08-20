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

            var matrix= RGBLedMatrixSharp.led_matrix_create(32, 2, 1);
            if (matrix == null) return 1;

            var canvas = RGBLedMatrixSharp.led_matrix_create_offscreen_canvas(matrix);
            int width;
            int height;
            RGBLedMatrixSharp.led_canvas_get_size(canvas, out width, out height);

            for (var i = 0; i < 1000; ++i)
            {
                for (var y = 0; y < height; ++y)
                {
                    for (var x = 0; x < width; ++x)
                    {
                        RGBLedMatrixSharp.led_canvas_set_pixel(canvas, x, y, (byte)(i & 0xff), (byte)x, (byte)y);
                    }
                }
                canvas = RGBLedMatrixSharp.led_matrix_swap_on_vsync(matrix, canvas);
            }

            RGBLedMatrixSharp.led_matrix_delete(matrix);
            return 0; 
        }
    }
}
