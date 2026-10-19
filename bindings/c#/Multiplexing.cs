namespace RPiRgbLEDMatrix;

/// <summary>
/// Type of multiplexing.
/// </summary>
public enum Multiplexing : int
{
    /// <summary>
    /// Direct (no multiplexing)
    /// </summary>
    Direct = 0,

    /// <summary>
    /// Stripe multiplexing
    /// </summary>
    Stripe = 1,

    /// <summary>
    /// Checker multiplexing (typical for 32x16 panels)
    /// </summary>
    Checker = 2,

    /// <summary>
    /// Spiral multiplexing
    /// </summary>
    Spiral = 3,

    /// <summary>
    /// Z-Stripe multiplexing (common for P10 panels)
    /// </summary>
    ZStripe = 4,

    /// <summary>
    /// ZnMirrorZStripe multiplexing
    /// </summary>
    ZnMirrorZStripe = 5,

    /// <summary>
    /// Coreman multiplexing
    /// </summary>
    Coreman = 6,

    /// <summary>
    /// Kaler2Scan multiplexing
    /// </summary>
    Kaler2Scan = 7,

    /// <summary>
    /// ZStripeUneven multiplexing
    /// </summary>
    ZStripeUneven = 8,

    /// <summary>
    /// P10-128x4-Z multiplexing (for specific P10 configurations)
    /// </summary>
    P10_128x4_Z = 9,

    /// <summary>
    /// QiangLiQ8 multiplexing
    /// </summary>
    QiangLiQ8 = 10,

    /// <summary>
    /// InversedZStripe multiplexing
    /// </summary>
    InversedZStripe = 11
}
