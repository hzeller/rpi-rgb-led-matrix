global using static RPiRgbLEDMatrix.Bindings;

using System.Runtime.InteropServices;

namespace RPiRgbLEDMatrix;

/*
Some of the extern methods listed below are marked with [SuppressGCTransition].
This disables some GC checks that may take a long time. But such methods should
be fast and trivial, otherwise the managed code may become unstable (see docs).
Keep this in mind when changing the C/C++ side.

https://learn.microsoft.com/dotnet/api/system.runtime.interopservices.suppressgctransitionattribute
*/
internal static class Bindings
{
    private const string Lib = "librgbmatrix.so.1";

    [DllImport(Lib)]
    public static extern IntPtr led_matrix_create(int rows, int chained, int parallel);

    [DllImport(Lib, CharSet = CharSet.Ansi)]
    public static extern IntPtr led_matrix_create_from_options_const_argv(
        ref InternalRGBLedMatrixOptions options,
        int argc,
        string[] argv);

    [DllImport(Lib)]
    public static extern void led_matrix_delete(IntPtr matrix);

    [DllImport(Lib)]
    public static extern IntPtr led_matrix_create_offscreen_canvas(IntPtr matrix);

    [DllImport(Lib)]
    public static extern IntPtr led_matrix_swap_on_vsync(IntPtr matrix, IntPtr canvas);

    [DllImport(Lib)]
    public static extern IntPtr led_matrix_get_canvas(IntPtr matrix);

    [DllImport(Lib)]
    [SuppressGCTransition]
    public static extern byte led_matrix_get_brightness(IntPtr matrix);

    [DllImport(Lib)]
    [SuppressGCTransition]
    public static extern void led_matrix_set_brightness(IntPtr matrix, byte brightness);

    [DllImport(Lib, CharSet = CharSet.Ansi)]
    public static extern IntPtr load_font(string bdf_font_file);

    [DllImport(Lib, CharSet = CharSet.Ansi)]
    public static extern int draw_text(IntPtr canvas, IntPtr font, int x, int y, byte r, byte g, byte b,
                                       string utf8_text, int extra_spacing);

    [DllImport(Lib, CharSet = CharSet.Ansi)]
    public static extern int vertical_draw_text(IntPtr canvas, IntPtr font, int x, int y, byte r, byte g, byte b,
                                                string utf8_text, int kerning_offset);

    [DllImport(Lib, CharSet = CharSet.Ansi)]
    public static extern void delete_font(IntPtr font);

    [DllImport(Lib)]
    [SuppressGCTransition]
    public static extern void led_canvas_get_size(IntPtr canvas, out int width, out int height);

    [DllImport(Lib)]
    [SuppressGCTransition]
    public static extern void led_canvas_set_pixel(IntPtr canvas, int x, int y, byte r, byte g, byte b);

    [DllImport(Lib)]
    public static extern void led_canvas_set_pixels(IntPtr canvas, int x, int y, int width, int height,
                                                    ref Color colors);

    [DllImport(Lib)]
    public static extern void led_canvas_clear(IntPtr canvas);

    [DllImport(Lib)]
    public static extern void led_canvas_fill(IntPtr canvas, byte r, byte g, byte b);

    [DllImport(Lib)]
    public static extern void draw_circle(IntPtr canvas, int xx, int y, int radius, byte r, byte g, byte b);

    [DllImport(Lib)]
    public static extern void draw_line(IntPtr canvas, int x0, int y0, int x1, int y1, byte r, byte g, byte b);
}
