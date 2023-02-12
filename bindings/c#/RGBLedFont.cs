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
    public RGBLedFont(string bdfFontPath) => _font = load_font(bdfFontPath);

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
