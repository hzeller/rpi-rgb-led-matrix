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

### Working on the C++
- Make your changes to the code
- The following tasks are available:
  - `Sync C++`: Syncs the local folder with the remote (Does not sync the `bindings` folder)
  - `Build C++`: Builds the C++ library (Auto-runs `Sync C++`)
  - `Deploy C++`: Deploys `librgbmatrix.so.1` to `/usr/lib` (Auto-runs `Build C++`)

## C#
### Initial Setup
- Do all the setup steps for C++
- Use the `Deploy C++` task to make sure that the linked library is in place
- Install this extension: https://marketplace.visualstudio.com/items?itemName=rioj7.command-variable  
  This allows us to get variables in the task in a linux (/ separator) format  
  ie normally, `${relativeFileDirname}` in tasks would resolve to like `bindings\c#\examples\BoxesBoxesBoxes`  
  This extensions allows us to do `${command:extension.commandvariable.file.relativeFileDirnamePosix}`...  
  Which will resolve to `bindings/c#/examples/BoxesBoxesBoxes`  
  This is very useful as it allows us to build rsync commands etc using the folder name of the example in linux format
- Ensure VSDBG is set up on the Pi  
  Currently, the launch configuration is set up for debugger path `~/vsdbg/vsdbg`

### Working on the C# bindings
- Make your changes to the code  
- Run an example to test it  
  Note that if you also have to make changes to the C library, you will need to Deploy the C++ too

### Working on C# examples
- To debug an example:
  - Select the `Debug Current C#` configuration
  - Ensure `Program.cs` (Or any file in the same folder as it) for the example is the current tab in VS Code  
  - Hit F5
