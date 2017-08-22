using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace rpi_rgb_led_matrix_sharp
{
    public class RGBLedCanvas
    {
        #region DLLImports
        [DllImport("librgbmatrix.so")]
        internal static extern void led_canvas_get_size(IntPtr canvas, out int width, out int height);

        [DllImport("librgbmatrix.so")]
        internal static extern void led_canvas_set_pixel(IntPtr canvas, int x, int y, byte r, byte g, byte b);

        [DllImport("librgbmatrix.so")]
        internal static extern void led_canvas_clear(IntPtr canvas);

        [DllImport("librgbmatrix.so")]
        internal static extern void led_canvas_fill(IntPtr canvas, byte r, byte g, byte b);       
        #endregion

        // This is a wrapper for canvas no need to implement IDisposable here 
        // because RGBLedMatrix has ownership and takes care of disposing canvases
        internal IntPtr _canvas;

        // this is not called directly by the consumer code,
        // consumer uses factory methods in RGBLedMatrix
        internal RGBLedCanvas(IntPtr canvas)
        {
            _canvas = canvas;
            int width;
            int height;
            led_canvas_get_size(_canvas, out width, out height);
            Width = width;
            Height = height;
        }

        public int Width {get; private set; }
        public int Height { get; private set; }

        public void SetPixel(int x, int y, Color color)
        {
            led_canvas_set_pixel(_canvas, x, y, color.R, color.G, color.B);
        }

        public void Fill(Color color)
        {
            led_canvas_fill(_canvas, color.R, color.G, color.B);
        }

        public void Clear()
        {
            led_canvas_clear(_canvas);
        }

        public void DrawCircle(int x0, int y0, int radius, Color color)
        {
            int x = radius, y = 0;
            int radiusError = 1 - x;

            while (y <= x)
            {
                SetPixel(x + x0, y + y0, color);
                SetPixel(y + x0, x + y0, color);
                SetPixel(-x + x0, y + y0, color);
                SetPixel(-y + x0, x + y0, color);
                SetPixel(-x + x0, -y + y0, color);
                SetPixel(-y + x0, -x + y0, color);
                SetPixel(x + x0, -y + y0, color);
                SetPixel(y + x0, -x + y0, color);
                y++;
                if (radiusError < 0)
                {
                    radiusError += 2 * y + 1;
                }
                else
                {
                    x--;
                    radiusError += 2 * (y - x + 1);
                }
            }
        }

        public void DrawLine (int x0, int y0, int x1, int y1, Color color)
        {
            int dy = y1 - y0, dx = x1 - x0, gradient, x, y, shift = 0x10;

            if (Math.Abs(dx) > Math.Abs(dy))
            {
                // x variation is bigger than y variation
                if (x1 < x0)
                {
                    swap(ref x0, ref x1);
                    swap(ref y0, ref y1);
                }
                gradient = (dy << shift) / dx;

                for (x = x0, y = 0x8000 + (y0 << shift); x <= x1; ++x, y += gradient)
                {
                    SetPixel(x, y >> shift, color);
                }
            }
            else if (dy != 0)
            {
                // y variation is bigger than x variation
                if (y1 < y0)
                {
                    swap(ref x0, ref x1);
                    swap(ref y0, ref y1);
                }
                gradient = (dx << shift) / dy;
                for (y = y0, x = 0x8000 + (x0 << shift); y <= y1; ++y, x += gradient)
                {
                    SetPixel(x >> shift, y, color);
                }
            }
            else
            {
                SetPixel(x0, y0, color);
            }
        }

        private void swap(ref int a, ref int b)
        {
            int tmp = a;
            a = b;
            b = tmp;
        }        
    }

    public struct Color
    {
        public Color (int r, int g, int b)
        {
            R = (byte)r;
            G = (byte)g;
            B = (byte)b;
        }
        public Color(byte r, byte g, byte b)
        {
            R = r;
            G = g;
            B = b;
        }
        public byte R;
        public byte G;
        public byte B;
    }
}
