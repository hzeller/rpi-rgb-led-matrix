
namespace RPiRgbLEDMatrix
{
    public interface IRGBLedCanvas
    {
        int Height { get; }

        int Width { get; }

        void Clear();

        void DrawCircle(int x, int y, int radius, Color color);

        void DrawLine(int x0, int y0, int x1, int y1, Color color);

        int DrawText(RGBLedFont font, int x, int y, Color color, string text, int spacing = 0, bool vertical = false);

        void Fill(Color color);

        void SetPixel(int x, int y, Color color);

        void SetPixels(int x, int y, int width, int height, Span<Color> colors);

        void SubFill(int x, int y, int width, int height, Color color);
    }
}
