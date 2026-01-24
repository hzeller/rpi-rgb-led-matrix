namespace RPiRgbLEDMatrix;

/// <summary>
/// Represents a <c>.BDF</c> font.
/// </summary>
public class RGBLedFont : IDisposable
{
    internal IntPtr _font;
    private bool disposedValue = false;

    /// <summary>
    /// Loads the BDF font from the specified file.
    /// </summary>
    /// <param name="bdfFontPath">The path to the BDF file to load.</param>
    /// <exception cref="ArgumentException" >Thrown when <paramref name="bdfFontPath"/> is null or empty.</exception>
    /// <exception cref="FileNotFoundException">Thrown when the specified file does not exist.</exception>"
    public RGBLedFont(string bdfFontPath)
    {
        if (!File.Exists(bdfFontPath))
            throw new FileNotFoundException("The specified font file was not found.", bdfFontPath);

        _font = load_font(bdfFontPath);
    }

    internal int DrawText(IntPtr canvas, int x, int y, Color color, string text, int spacing = 0, bool vertical = false)
    {
        if (!vertical)
            return draw_text(canvas, _font, x, y, color.R, color.G, color.B, text, spacing);
        else
            return vertical_draw_text(canvas, _font, x, y, color.R, color.G, color.B, text, spacing);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (disposedValue) return;
        delete_font(_font);
        disposedValue = true;
    }

    ~RGBLedFont() => Dispose(false);

    /// <inheritdoc/>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }
}
