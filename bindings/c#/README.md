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
- See README in .vscode folder  
  (Set some env variables, install a VSCode extension)  
- Edit `bindings/c#/MatrixOptions.cs` and put in the options for your matrix    
- Make one of the `Program.cs` example files the active file in VSCode  
- Hit F5  
