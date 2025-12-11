namespace RPiRgbLEDMatrix
{
    public interface IRGBLedMatrix
    {
        IRGBLedCanvas CreateOffscreenCanvas();

        void Dispose();

        IRGBLedCanvas GetCanvas();

        void SwapOnVsync(IRGBLedCanvas canvas);
    }
}
