using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace rpi_rgb_led_matrix_sharp
{
    public class RGBLedFont
    {
        [DllImport("librgbmatrix.so", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr load_font(string bdf_font_file);

        [DllImport("librgbmatrix.so", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern int draw_text(IntPtr canvas, IntPtr font, int x, int y, byte r, byte g, byte b, string utf8_text, int extra_spacing);

        [DllImport("librgbmatrix.so", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern int vertical_draw_text(IntPtr canvas, IntPtr font, int x, int y, byte r, byte g, byte b, string utf8_text, int kerning_offset);
        
        public RGBLedFont(string bdf_font_file_path)
        {
            _font = load_font(bdf_font_file_path);
        }
        internal IntPtr _font;

        internal int DrawText(IntPtr canvas, int x, int y, Color color, string text, int spacing=0, bool vertical=false)
        {
            if (!vertical)
                return draw_text(canvas, _font, x, y, color.R, color.G, color.B, text, spacing);
            else
                return vertical_draw_text(canvas, _font, x, y, color.R, color.G, color.B, text, spacing);
        }

    }
}
