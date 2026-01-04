using RPiRgbLEDMatrix;

var options = new RGBLedMatrixOptions()
{
    Cols = 128,
    Rows = 64,
    Parallel = 2,
    GpioSlowdown = 2,
    RowAddressType = 5
};

string path;

// Check if the last command-line argument is provided and is a valid file
if (args.Length > 0 && File.Exists(args[^1]))
{
    path = args[^1];
}
else
{
    path = "/root/sample.stream"; // default path
}

// Create an instance of StreamPlayer and play the file
var streamPlayer = new StreamPlayer(options);
streamPlayer.PlayStream(path);
