using System;
using System.Runtime.InteropServices;

namespace RPiRgbLEDMatrix
{
    internal static class ContentStreamerBindings
    {
        private const string Lib = "librgbmatrix.so.1";

        [DllImport(Lib, EntryPoint = "file_stream_io_create")]
        public static extern IntPtr FileStreamIOCreate([MarshalAs(UnmanagedType.LPStr)] string filename);

        [DllImport(Lib, EntryPoint = "file_stream_io_delete")]
        public static extern void FileStreamIODelete(IntPtr streamIO);

        [DllImport(Lib, EntryPoint = "content_stream_reader_create")]
        public static extern IntPtr ContentStreamReaderCreate(IntPtr streamIO);

        [DllImport(Lib, EntryPoint = "content_stream_reader_destroy")]
        public static extern void ContentStreamReaderDestroy(IntPtr reader);

        [DllImport(Lib, EntryPoint = "content_stream_reader_get_next")]
        public static extern int ContentStreamReaderGetNext(IntPtr reader, IntPtr frameCanvas, ref uint holdTimeUs);

        [DllImport(Lib, EntryPoint = "content_stream_reader_rewind")]
        public static extern void ContentStreamReaderRewind(IntPtr reader);
    }
}
