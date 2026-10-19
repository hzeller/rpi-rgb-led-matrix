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
    public static extern void led_canvas_subfill(IntPtr canvas, int x, int y, int width, int height, byte r, byte g, byte b);

    [DllImport(Lib)]
    public static extern void draw_circle(IntPtr canvas, int xx, int y, int radius, byte r, byte g, byte b);

    [DllImport(Lib)]
    public static extern void draw_line(IntPtr canvas, int x0, int y0, int x1, int y1, byte r, byte g, byte b);

    /* Content-streamer bindings (merged) */
    [DllImport(Lib, EntryPoint = "file_stream_io_create", CharSet = CharSet.Ansi)]
    public static extern IntPtr file_stream_io_create(string filename);

    [DllImport(Lib, EntryPoint = "file_stream_io_delete")]
    public static extern void file_stream_io_delete(IntPtr streamIO);
    [DllImport(Lib, EntryPoint = "file_stream_io_is_compatible_with_canvas")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool file_stream_io_is_compatible_with_canvas(IntPtr streamIO, IntPtr frameCanvas);

    [DllImport(Lib, EntryPoint = "content_stream_reader_create")]
    public static extern IntPtr content_stream_reader_create(IntPtr streamIO);

    [DllImport(Lib, EntryPoint = "content_stream_reader_destroy")]
    public static extern void content_stream_reader_destroy(IntPtr reader);

    [DllImport(Lib, EntryPoint = "content_stream_reader_get_next")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool content_stream_reader_get_next(IntPtr reader, IntPtr frameCanvas, out uint holdTimeUs);

    [DllImport(Lib, EntryPoint = "content_stream_reader_rewind")]
    public static extern void content_stream_reader_rewind(IntPtr reader);

    /* Frame-sequence bindings */
    [DllImport(Lib, EntryPoint = "frame_sequence_create")]
    public static extern IntPtr frame_sequence_create(int width, int height);

    [DllImport(Lib, EntryPoint = "frame_sequence_destroy")]
    public static extern void frame_sequence_destroy(IntPtr sequence);

    [DllImport(Lib, EntryPoint = "frame_sequence_width")]
    public static extern int frame_sequence_width(IntPtr sequence);

    [DllImport(Lib, EntryPoint = "frame_sequence_height")]
    public static extern int frame_sequence_height(IntPtr sequence);

    [DllImport(Lib, EntryPoint = "frame_sequence_frame_count")]
    public static extern nuint frame_sequence_frame_count(IntPtr sequence);

    [DllImport(Lib, EntryPoint = "frame_sequence_add_frame")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool frame_sequence_add_frame(IntPtr sequence,
                                                       byte[] rgb24,
                                                       nuint byteCount,
                                                       uint holdTimeUs);

    [DllImport(Lib, EntryPoint = "frame_sequence_clear")]
    public static extern void frame_sequence_clear(IntPtr sequence);

    [DllImport(Lib, EntryPoint = "frame_sequence_write_to_file", CharSet = CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool frame_sequence_write_to_file(IntPtr sequence, string path);

    [DllImport(Lib, EntryPoint = "frame_sequence_read_from_file", CharSet = CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool frame_sequence_read_from_file(IntPtr sequence, string path);

    [DllImport(Lib, EntryPoint = "frame_sequence_play_forever")]
    public static extern void frame_sequence_play_forever(IntPtr sequence,
                                                           IntPtr matrix,
                                                           IntPtr interruptReceived,
                                                           IntPtr brightnessPercent);

    [DllImport(Lib, EntryPoint = "frame_sequence_play_count")]
    public static extern void frame_sequence_play_count(IntPtr sequence,
                                                        IntPtr matrix,
                                                        uint playCount,
                                                        IntPtr interruptReceived,
                                                        IntPtr brightnessPercent);

    [DllImport(Lib, EntryPoint = "frame_sequence_play_duration")]
    public static extern void frame_sequence_play_duration(IntPtr sequence,
                                                           IntPtr matrix,
                                                           uint durationMs,
                                                           IntPtr interruptReceived,
                                                           IntPtr brightnessPercent);

    [DllImport(Lib, EntryPoint = "frame_sequence_play")]
    public static extern void frame_sequence_play(IntPtr sequence,
                                                  IntPtr matrix,
                                                  IntPtr interruptReceived,
                                                  IntPtr brightnessPercent);
}
