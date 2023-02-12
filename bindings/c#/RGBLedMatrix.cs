using System.Buffers;
using System.Runtime.InteropServices;

namespace RPiRgbLEDMatrix;

public class RGBLedMatrix : IDisposable
{
    private IntPtr matrix;
    private bool disposedValue = false;

    public RGBLedMatrix(int rows, int chained, int parallel) =>
        matrix = led_matrix_create(rows, chained, parallel);

    public RGBLedMatrix(RGBLedMatrixOptions options)
    {
        InternalRGBLedMatrixOptions opt = default;
        try
        {
            opt = new(options);
            var args = Environment.GetCommandLineArgs();

            // Because gpio-slowdown is not provided in the options struct,
            // we manually add it.
            // Let's add it first to the command-line we pass to the
            // matrix constructor, so that it can be overridden with the
            // users' commandline.
            // As always, as the _very_ first, we need to provide the
            // program name argv[0].
            var argv = new string[args.Length + 1];
            argv[0] = args[0];
            argv[1] = $"--led-slowdown-gpio={options.GpioSlowdown}";
            Array.Copy(args, 1, argv, 2, args.Length - 1);

            matrix = led_matrix_create_from_options_const_argv(ref opt, argv.Length, argv);
        }
        finally
        {
            if(options.HardwareMapping is not null) Marshal.FreeHGlobal(opt.hardware_mapping);
            if(options.LedRgbSequence is not null) Marshal.FreeHGlobal(opt.led_rgb_sequence);
            if(options.PixelMapperConfig is not null) Marshal.FreeHGlobal(opt.pixel_mapper_config);
            if(options.PanelType is not null) Marshal.FreeHGlobal(opt.panel_type);
        }
    }

    public RGBLedCanvas CreateOffscreenCanvas() => new(led_matrix_create_offscreen_canvas(matrix));

    public RGBLedCanvas GetCanvas() => new(led_matrix_get_canvas(matrix));

    public void SwapOnVsync(RGBLedCanvas canvas) =>
        canvas._canvas = led_matrix_swap_on_vsync(matrix, canvas._canvas);

    public byte Brightness
    {
        get => led_matrix_get_brightness(matrix);
        set => led_matrix_set_brightness(matrix, value);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (disposedValue) return;

        led_matrix_delete(matrix);
        disposedValue = true;
    }

    ~RGBLedMatrix() => Dispose(false);

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }
}
