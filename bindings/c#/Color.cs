namespace RPiRgbLEDMatrix;

public struct Color
{
    public byte R;
    public byte G;
    public byte B;

    public Color(int r, int g, int b) : this((byte)r, (byte)g, (byte)b) { }

    public Color(byte r, byte g, byte b) => (R, G, B) = (r, g, b);
}
