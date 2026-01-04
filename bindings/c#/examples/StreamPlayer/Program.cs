using RPiRgbLEDMatrix;

string path;

// Check if the last command-line argument is provided and is a valid file
if (args.Length > 0 && File.Exists(args[^1]))
{
    path = args[^1];
}
else
{
    path = "";
    var defaultStreamPath = "/root/sample.stream";
    Console.WriteLine("Usage: StreamPlayer [stream_path]\nWhere [stream_path] is the path to a stream file\n");
    Console.WriteLine("Stream files can be created using the 'led-image-viewer' utility with the '-O' option.\n");
    Console.WriteLine("Or type name of stream and hit ENTER");
    Console.WriteLine($"Or just hit ENTER to use the default path: {defaultStreamPath}");
    var enteredPath = Console.ReadLine();
    while (path == "")
    {
        if (enteredPath != "")
        {
            if (File.Exists(enteredPath))
            {
                path = enteredPath;
            }
            else
            {
                Console.WriteLine($"File '{enteredPath}' does not exist. Please enter a valid file path or hit ENTER to use the default path.");
                enteredPath = Console.ReadLine();
            }
        }
        else
        {
            path = defaultStreamPath;
        }
    }
}

// Create an instance of StreamPlayer and play the file
var streamPlayer = new StreamPlayer(MatrixOptions.Load());
streamPlayer.PlayStream(path);
