namespace RPiRgbLEDMatrix;

/// <summary>
/// Represents an RGB (red, green, blue) color
/// </summary>
public struct Color
{
    /// <summary>
    /// The red component value of this instance.
    /// </summary>
    public byte R;

    /// <summary>
    /// The green component value of this instance.
    /// </summary>
    public byte G;

    /// <summary>
    /// The blue component value of this instance.
    /// </summary>
    public byte B;

    /// <summary>
    /// Creates a new color from the specified color values (red, green, and blue).
    /// </summary>
    /// <param name="r">The red component value.</param>
    /// <param name="g">The green component value.</param>
    /// <param name="b">The blue component value.</param>
    public Color(int r, int g, int b) : this((byte)r, (byte)g, (byte)b) { }

    /// <summary>
    /// Creates a new color from the specified color values (red, green, and blue).
    /// </summary>
    /// <param name="r">The red component value.</param>
    /// <param name="g">The green component value.</param>
    /// <param name="b">The blue component value.</param>
    public Color(byte r, byte g, byte b) => (R, G, B) = (r, g, b);
}
