# Instructions for remote developent on Windows using VSCode  
These instructions are primarily aimed at those wishing to work on the C# bindings, but they are probably relevant if you wish to work on the C++ too.  

## C++
### Initial setup
- Set the environment variables on your local PC:
  - `RGBLEDMATRIX_REMOTE_MACHINE`: Username and hostname of your Pi, eg `root@DietPi`
  - `RGBLEDMATRIX_REMOTE_DIR`: The remote folder on your Pi where you wish to store the repo, eg `/root/dev/rpi-rgb-led-matrix`  
    Ensure this folder exists
- Ensure Passwordless SSH to the Pi is set up for the account you specified in the environment variable
- WSL must be installed
- RSync must be installed in WSL

If you wish to build the C++ locally, you must install the required packages in WSL:
```
sudo apt update
sudo apt install -y make build-essential rsync ssh \
  crossbuild-essential-arm64
```

### Working on the C++
#### Remote building
The following tasks are available:
- `Sync C++`: Syncs the local folder with the remote (Does not sync the `bindings` folder)
- `Build C++`: Builds the C++ library (Auto-runs `Sync C++`)
- `Deploy C++`: Deploys `librgbmatrix.so.1` to `/usr/lib` (Auto-runs `Build C++`)

#### Local building
The following tasks are available:
- `WSL: Cross-build native (arm64)`: Builds in WSL

## C#

### Initial Setup
- Install this extension: https://marketplace.visualstudio.com/items?itemName=rioj7.command-variable  
  This allows us to get variables in the task in a linux (/ separator) format  
  ie normally, `${relativeFileDirname}` in tasks would resolve to like `bindings\c#\examples\BoxesBoxesBoxes`  
  This extensions allows us to do `${command:extension.commandvariable.file.relativeFileDirnamePosix}`...  
  Which will resolve to `bindings/c#/examples/BoxesBoxesBoxes`  
  This is very useful as it allows us to build rsync commands etc using the folder name of the example in linux format
- Ensure VSDBG is set up on the Pi  
  Currently, the launch configuration is set up for debugger path `~/vsdbg/vsdbg`

### Running / Debugging the examples
#### If using Local C++ building
- Ensure `Program.cs` (Or any file in the same folder as it) for the example is the current tab in VS Code
- Select the `Debug Current C# (Local C++ build)` configuration  
- Hit F5
  - C++ will be built locally
  - The currently selected C# example will be built locally
  - `librgbmatrix.so.1` will then be copied into the publish folder for the example
  - The example will be rsynced to the Pi
  - The debugger will be attached

#### If using Remote C++ building
- Do all the setup steps for C++
- Use the `Deploy C++` task to make sure that the linked library is in place
- Ensure `Program.cs` (Or any file in the same folder as it) for the example is the current tab in VS Code
- Select the `Debug Current C# (No C++ build)` configuration  
- Hit F5
  - The currently selected C# example will be built locally
  - The example will be rsynced to the Pi
  - The debugger will be attached 

