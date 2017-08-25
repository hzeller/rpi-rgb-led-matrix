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

        [DllImport("librgbmatrix.so", CallingConvention= CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr led_matrix_create_from_options(
            ref InternalRGBLedMatrixOptions options,  
            IntPtr argc,
            out IntPtr argv);

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

        public RGBLedMatrix(RGBLedMatrixOptions options)
        {
            var opt = new InternalRGBLedMatrixOptions();

            try {      
                // pass in options to interal data structure           
                opt.chain_length = options.ChainLength;
                opt.rows = options.Rows;
                opt.hardware_mapping = options.HardwareMapping != null ? Marshal.StringToHGlobalUni(options.HardwareMapping) : IntPtr.Zero;
                opt.inverse_colors = (uint)(options.InverseColors ? 0 : 1);  
                opt.led_rgb_sequence = options.LedRgbSequence != null ? Marshal.StringToHGlobalUni(options.LedRgbSequence) : IntPtr.Zero;
                opt.parallel = options.Parallel;
                opt.pwm_bits = options.PwmBits;
                opt.pwm_lsb_nanoseconds = options.PwmLsbNanoseconds;
                opt.scan_mode = options.ScanMode;
                opt.show_refresh_rate = (uint)(options.ShowRefreshRate ? 0 : 1);
                opt.brightness = options.Brightness;
                opt.disable_hardware_pulsing = (uint)(options.DisableHardwarePulsing ? 0 : 1);

                // dont care about these
                var argc = IntPtr.Zero;
                var argv = IntPtr.Zero;
                matrix = led_matrix_create_from_options(ref opt, argc, out argv);
            }
            finally
            {
                if (options.HardwareMapping != null) Marshal.FreeHGlobal(opt.hardware_mapping);
                if (options.LedRgbSequence != null) Marshal.FreeHGlobal(opt.led_rgb_sequence);
            }
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

        #region RGBLedMatrixOptions struct
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        internal struct InternalRGBLedMatrixOptions
        {
            public IntPtr hardware_mapping;
            public int rows;
            public int chain_length;
            public int parallel;
            public int pwm_bits;
            public int pwm_lsb_nanoseconds;
            public int brightness;
            public int scan_mode;
            public IntPtr led_rgb_sequence;
            public uint disable_hardware_pulsing;
            public uint show_refresh_rate;
            public uint inverse_colors;
        };
        #endregion
    }

    public struct RGBLedMatrixOptions
    {
        /// <summary>
        /// Name of the hardware mapping used. If passed NULL here, the default is used. 
        /// </summary>       
        public string HardwareMapping;

        /// <summary>
        /// The "rows" are the number of rows supported by the display, so 32 or 16.
        /// Default: 32.
        /// </summary>
        public int Rows;

        /// <summary>
        /// The chain_length is the number of displays daisy-chained together
        /// (output of one connected to input of next). Default: 1
        /// </summary>
        public int ChainLength;

        /// <summary>
        /// The number of parallel chains connected to the Pi; in old Pis with 26 
        /// GPIO pins, that is 1, in newer Pis with 40 interfaces pins, that can also 
        /// be 2 or 3. The effective number of pixels in vertical direction is then 
        /// thus rows * parallel. Default: 1 
        /// </summary>
        public int Parallel;

        /// <summary>
        /// Set PWM bits used for output. Default is 11, but if you only deal with limited 
        /// comic-colors, 1 might be sufficient. Lower require less CPU and increases refresh-rate.
        /// </summary>
        public int PwmBits;

        /// <summary>
        /// Change the base time-unit for the on-time in the lowest significant bit in 
        /// nanoseconds. Higher numbers provide better quality (more accurate color, less 
        /// ghosting), but have a negative impact on the frame rate.
        /// </summary>
        public int PwmLsbNanoseconds;

        /// <summary>
        /// The initial brightness of the panel in percent. Valid range is 1..100
        /// </summary>
        public int Brightness;

        /// <summary>
        /// Scan mode: 0=progressive, 1=interlaced
        /// </summary>
        public int ScanMode;

        /// <summary>
        /// In case the internal sequence of mapping is not "RGB", this contains the real mapping. Some panels mix up these colors.
        /// </summary>         
        public string LedRgbSequence;

        /// <summary>
        /// Allow to use the hardware subsystem to create pulses. This won't do anything if output enable is not connected to GPIO 18.
        /// </summary>
        public bool DisableHardwarePulsing;
        public bool ShowRefreshRate;
        public bool InverseColors;
    };
}
