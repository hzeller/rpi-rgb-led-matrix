using RPiRgbLEDMatrix;

public sealed class FseqPlayer : IDisposable
{
    private readonly RGBLedMatrix _matrix;
    private readonly int _width;
    private readonly int _height;
    private readonly int[] _stop = new[] { 0 };
    private readonly int[] _brightness;
    private bool _disposed;

    /// <summary>
    /// Initializes a new instance of the <see cref="FseqPlayer"/> class.
    /// </summary>
    /// <param name="options">The matrix options.</param>
    public FseqPlayer(RGBLedMatrixOptions options)
    {
        _matrix = new RGBLedMatrix(options);
        _width = options.Cols * options.ChainLength;
        _height = options.Rows * options.Parallel;
        _brightness = new[] { (int)_matrix.Brightness };
    }

    public int BrightnessPercent => _brightness[0];

    /// <summary>
    /// Plays an fseq file on the LED matrix.
    /// </summary>
    /// <param name="sequencePath">The path to the .fseq file.</param>
    public void PlayForever(string sequencePath)
    {
        ObjectDisposedException.ThrowIf(_disposed, GetType());
        using var sequence = new FrameSequence(_width, _height);
        sequence.ReadFromFile(sequencePath);
        _stop[0] = 0;
        sequence.PlayForever(_matrix, _stop, _brightness);
    }

    public void Stop()
    {
        _stop[0] = 1;
    }

    public void SetBrightness(int percent)
    {
        _brightness[0] = ClampBrightness(percent);
    }

    public void IncreaseBrightness(int step = 5)
    {
        _brightness[0] = ClampBrightness(_brightness[0] + step);
    }

    public void DecreaseBrightness(int step = 5)
    {
        _brightness[0] = ClampBrightness(_brightness[0] - step);
    }

    private static int ClampBrightness(int percent)
    {
        return Math.Max(1, Math.Min(100, percent));
    }

    public void Dispose()
    {
        if (_disposed)
            return;

        _matrix.Dispose();
        _disposed = true;
    }
}
