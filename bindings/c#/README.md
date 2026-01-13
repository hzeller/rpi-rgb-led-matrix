# C# bindings for RGB Matrix library

## Usage:
### Building on the Pi
- Install the .Net SDK
  `sudo apt install dotnet8` should work in most cases.  
   For some old distributions, read [docs](https://learn.microsoft.com/dotnet/core/install/linux)  
- Edit `bindings/c#/MatrixOptions.cs` and put in the options for your matrix  
- Change to an example folder  
  eg `cd bindings/c#/examples/BoxesBoxesBoxes`  
- Build: `dotnet build`
- Run: `sudo dotnet run`

### Build on a Windows PC, run on the Pi
- Only needs .NET runtime on Pi, do not need SDK  
  Or you could modify the `publish current C#` task and add `--self-contained` to deploy with no need for dotnet to be installed on the Pi at all.
- See README in .vscode folder  
  (Set some env variables, install a VSCode extension)  
- Edit `bindings/c#/MatrixOptions.cs` and put in the options for your matrix    
- Make one of the `Program.cs` example files the active file in VSCode  
- Hit F5  
  The example will build on the PC, rsync to the Pi, launch and debug
