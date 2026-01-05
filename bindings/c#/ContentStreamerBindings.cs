using System.Runtime.InteropServices;

namespace RPiRgbLEDMatrix
{
    internal static class ContentStreamerBindings
    {
        private const string Lib = "librgbmatrix.so.1";

        [DllImport(Lib, EntryPoint = "file_stream_io_create")]
        public static extern IntPtr file_stream_io_create([MarshalAs(UnmanagedType.LPStr)] string filename);

        [DllImport(Lib, EntryPoint = "file_stream_io_delete")]
        public static extern void file_stream_io_delete(IntPtr streamIO);

        [DllImport(Lib, EntryPoint = "content_stream_reader_create")]
        public static extern IntPtr content_stream_reader_create(IntPtr streamIO);

        [DllImport(Lib, EntryPoint = "content_stream_reader_destroy")]
        public static extern void content_stream_reader_destroy(IntPtr reader);

        [DllImport(Lib, EntryPoint = "content_stream_reader_get_next")]
        public static extern int content_stream_reader_get_next(IntPtr reader, IntPtr frameCanvas, out uint holdTimeUs);

        [DllImport(Lib, EntryPoint = "content_stream_reader_rewind")]
        public static extern void content_stream_reader_rewind(IntPtr reader);
    }
}
