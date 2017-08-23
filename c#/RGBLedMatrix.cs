using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace rpi_rgb_led_matrix_sharp
{
    public class RGBLedMatrix : IDisposable
    {
        #region DLLImports
        [DllImport("librgbmatrix.so")]
        internal static extern IntPtr led_matrix_create(int rows, int chained, int parallel);

        [DllImport("librgbmatrix.so")]
        internal static extern void led_matrix_delete(IntPtr matrix);

        [DllImport("librgbmatrix.so")]
        internal static extern IntPtr led_matrix_create_offscreen_canvas(IntPtr matrix);

        [DllImport("librgbmatrix.so")]
        internal static extern IntPtr led_matrix_swap_on_vsync(IntPtr matrix, IntPtr canvas);

        [DllImport("librgbmatrix.so")]
        internal static extern IntPtr led_matrix_get_canvas(IntPtr matrix);
        #endregion

        public RGBLedMatrix(int rows, int chained, int parallel)
        {
            matrix= led_matrix_create(rows, chained, parallel);            
        }

        private IntPtr matrix;

        public RGBLedCanvas CreateOffscreenCanvas()
        {
            var canvas=led_matrix_create_offscreen_canvas(matrix);
            return new RGBLedCanvas(canvas);
        }

        public RGBLedCanvas GetCanvas()
        {
            var canvas = led_matrix_get_canvas(matrix);
            return new RGBLedCanvas(canvas);
        }

        public RGBLedCanvas SwapOnVsync(RGBLedCanvas canvas)
        {
            canvas._canvas = led_matrix_swap_on_vsync(matrix, canvas._canvas);
            return canvas;
        }
        #region IDisposable Support
        private bool disposedValue = false;

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                led_matrix_delete(matrix);
                disposedValue = true;
            }
        }
        ~RGBLedMatrix() {           
           Dispose(false);
        }
        public void Dispose()
        {            
            Dispose(true);            
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}
