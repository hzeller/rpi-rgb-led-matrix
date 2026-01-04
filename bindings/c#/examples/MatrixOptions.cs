/*
This file contains matrix options for use in all examples
*/
namespace RPiRgbLEDMatrix
{
	public static class MatrixOptions
	{
		public static RGBLedMatrixOptions Load() => new RGBLedMatrixOptions
		{
			Cols = 128,
			Rows = 64,
			Parallel = 2,
			GpioSlowdown = 2,
			RowAddressType = 5
		};
	}
}
