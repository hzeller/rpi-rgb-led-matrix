using RPiRgbLEDMatrix;

string path;
if (args.Length > 0 && File.Exists(args[^1]))
{
    path = args[^1];
}
else
{
    path = "";
    const string defaultPath = "/root/sample.fseq";
    Console.WriteLine("Usage: FseqPlayer [fseq_path]");
    Console.WriteLine("Where [fseq_path] points to a .fseq file created by frame-sequence-player -O");
    Console.WriteLine($"Or hit ENTER to use default: {defaultPath}");

    while (path == "")
    {
        var enteredPath = Console.ReadLine();
        if (string.IsNullOrWhiteSpace(enteredPath))
        {
            path = defaultPath;
        }
        else if (File.Exists(enteredPath))
        {
            path = enteredPath;
        }
        else
        {
            Console.WriteLine($"File '{enteredPath}' does not exist. Try again or hit ENTER for default.");
        }
    }
}

var matrixOptions = new RGBLedMatrixOptions
{
    Cols = 128,
    Rows = 64,
    Parallel = 2,
    GpioSlowdown = 4,
    RowAddressType = 5
};

using var player = new FseqPlayer(matrixOptions);

Console.WriteLine("Playing .fseq. Controls: '+'/'-' brightness, 'q' quit.");

var keyThread = new Thread(() =>
{
    while (true)
    {
        var key = Console.ReadKey(intercept: true).KeyChar;
        if (key == '+' || key == '=')
        {
            player.IncreaseBrightness();
            Console.Write($"\rBrightness: {player.BrightnessPercent}   ");
        }
        else if (key == '-' || key == '_')
        {
            player.DecreaseBrightness();
            Console.Write($"\rBrightness: {player.BrightnessPercent}   ");
        }
        else if (key == 'q' || key == 'Q')
        {
            player.Stop();
            return;
        }
    }
});
keyThread.IsBackground = true;
keyThread.Start();

player.PlayForever(path);
Console.WriteLine("\nDone.");
