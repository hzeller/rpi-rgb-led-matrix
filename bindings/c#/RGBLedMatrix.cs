using System.Buffers;
using System.Runtime.InteropServices;

namespace RPiRgbLEDMatrix;

/// <summary>
/// Represents a RGB matrix.
/// </summary>
public class RGBLedMatrix : IDisposable
{
    private IntPtr matrix;
    private bool disposedValue = false;

    /// <summary>
    /// Initializes a new matrix.
    /// </summary>
    /// <param name="rows">Size of a single module. Can be 32, 16 or 8.</param>
    /// <param name="chained">How many modules are connected in a chain.</param>
    /// <param name="parallel">How many modules are connected in a parallel.</param>
    public RGBLedMatrix(int rows, int chained, int parallel)
    {
        matrix = led_matrix_create(rows, chained, parallel);
        if (matrix == (IntPtr)0)
            throw new ArgumentException("Could not initialize a new matrix");
    }

    /// <summary>
    /// Initializes a new matrix.
    /// </summary>
    /// <param name="options">A configuration of a matrix.</param>
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
            if (matrix == (IntPtr)0)
                throw new ArgumentException("Could not initialize a new matrix");
        }
        finally
        {
            if(options.HardwareMapping is not null) Marshal.FreeHGlobal(opt.hardware_mapping);
            if(options.LedRgbSequence is not null) Marshal.FreeHGlobal(opt.led_rgb_sequence);
            if(options.PixelMapperConfig is not null) Marshal.FreeHGlobal(opt.pixel_mapper_config);
            if(options.PanelType is not null) Marshal.FreeHGlobal(opt.panel_type);
        }
    }

    /// <summary>
    /// Creates a new backbuffer canvas for drawing on.
    /// </summary>
    /// <returns>An instance of <see cref="RGBLedCanvas"/> representing the canvas.</returns>
    public RGBLedCanvas CreateOffscreenCanvas() => new(led_matrix_create_offscreen_canvas(matrix));

    /// <summary>
    /// Returns a canvas representing the current frame buffer.
    /// </summary>
    /// <returns>An instance of <see cref="RGBLedCanvas"/> representing the canvas.</returns>
    /// <remarks>Consider using <see cref="CreateOffscreenCanvas"/> instead.</remarks>
    public RGBLedCanvas GetCanvas() => new(led_matrix_get_canvas(matrix));

    /// <summary>
    /// Swaps this canvas with the currently active canvas. The active canvas
    /// becomes a backbuffer and is mapped to <paramref name="canvas"/> instance.
    /// <br/>
    /// This operation guarantees vertical synchronization.
    /// </summary>
    /// <param name="canvas">Backbuffer canvas to swap.</param>
    public void SwapOnVsync(RGBLedCanvas canvas) =>
        canvas._canvas = led_matrix_swap_on_vsync(matrix, canvas._canvas);

    /// <summary>
    /// The general brightness of the matrix.
    /// </summary>
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

    /// <inheritdoc/>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }
}
