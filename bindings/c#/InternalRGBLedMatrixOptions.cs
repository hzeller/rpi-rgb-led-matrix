using System.Runtime.InteropServices;

namespace RPiRgbLEDMatrix;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
internal struct InternalRGBLedMatrixOptions
{
    public IntPtr hardware_mapping;
    public int rows;
    public int cols;
    public int chain_length;
    public int parallel;
    public int pwm_bits;
    public int pwm_lsb_nanoseconds;
    public int pwm_dither_bits;
    public int brightness;
    public int scan_mode;
    public int row_address_type;
    public int multiplexing;
    public IntPtr led_rgb_sequence;
    public IntPtr pixel_mapper_config;
    public IntPtr panel_type;
    public byte disable_hardware_pulsing;
    public byte show_refresh_rate;
    public byte inverse_colors;
    public int limit_refresh_rate_hz;

    public InternalRGBLedMatrixOptions(RGBLedMatrixOptions opt)
    {
        chain_length = opt.ChainLength;
        rows = opt.Rows;
        cols = opt.Cols;
        hardware_mapping = Marshal.StringToHGlobalAnsi(opt.HardwareMapping);
        inverse_colors = (byte)(opt.InverseColors ? 1 : 0);
        led_rgb_sequence = Marshal.StringToHGlobalAnsi(opt.LedRgbSequence);
        pixel_mapper_config = Marshal.StringToHGlobalAnsi(opt.PixelMapperConfig);
        panel_type = Marshal.StringToHGlobalAnsi(opt.PanelType);
        parallel = opt.Parallel;
        multiplexing = (int)opt.Multiplexing;
        pwm_bits = opt.PwmBits;
        pwm_lsb_nanoseconds = opt.PwmLsbNanoseconds;
        pwm_dither_bits = opt.PwmDitherBits;
        scan_mode = (int)opt.ScanMode;
        show_refresh_rate = (byte)(opt.ShowRefreshRate ? 1 : 0);
        limit_refresh_rate_hz = opt.LimitRefreshRateHz;
        brightness = opt.Brightness;
        disable_hardware_pulsing = (byte)(opt.DisableHardwarePulsing ? 1 : 0);
        row_address_type = opt.RowAddressType;
    }
};
