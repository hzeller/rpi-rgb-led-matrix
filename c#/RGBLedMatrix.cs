using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace rpi_rgb_led_matrix_sharp
{
    public class RGBLedMatrixSharp
    {
        [DllImport("librgbmatrix.so")]
        public static extern IntPtr led_matrix_create(int rows, int chained, int parallel);

        [DllImport("librgbmatrix.so")]
        public static extern void led_matrix_delete(IntPtr matrix);

        [DllImport("librgbmatrix.so")]
        public static extern void led_canvas_get_size(IntPtr canvas, out int width, out int height);

        [DllImport("librgbmatrix.so")]
        public static extern void led_canvas_set_pixel(IntPtr canvas, int x, int y, byte r, byte g, byte b);

        [DllImport("librgbmatrix.so")]
        public static extern void led_canvas_clear(IntPtr canvas);

        [DllImport("librgbmatrix.so")]
        public static extern void led_canvas_fill(IntPtr canvas, byte r, byte g, byte b);

        [DllImport("librgbmatrix.so")]
        public static extern IntPtr led_matrix_create_offscreen_canvas(IntPtr matrix);

        [DllImport("librgbmatrix.so")]
        public static extern IntPtr led_matrix_swap_on_vsync(IntPtr matrix, IntPtr canvas);
    }
}
