namespace RPiRgbLEDMatrix;

public class RGBLedFont : IDisposable
{
    public RGBLedFont(string bdf_font_file_path)
    {
        _font = load_font(bdf_font_file_path);
    }
    internal IntPtr _font;

    internal int DrawText(IntPtr canvas, int x, int y, Color color, string text, int spacing=0, bool vertical=false)
    {
        if (!vertical)
            return draw_text(canvas, _font, x, y, color.R, color.G, color.B, text, spacing);
        else
            return vertical_draw_text(canvas, _font, x, y, color.R, color.G, color.B, text, spacing);
    }

    #region IDisposable Support
    private bool disposedValue = false;

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            delete_font(_font);
            disposedValue = true;
        }
    }
    ~RGBLedFont()
    {
        Dispose(false);
    }
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }
    #endregion
}
