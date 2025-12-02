C# bindings for RGB Matrix library
======================================

Building
--------

To build the C# wrapper for the RGB Matrix C library you need to first have __.NET SDK__ installed. 

### Install .NET SDK

`sudo apt install dotnet6` should work in most cases.  
For some old distributions, read [docs](https://learn.microsoft.com/dotnet/core/install/linux)

Then, in the `bindings/c#` directory type: `dotnet build`

To run the example applications in the c#\examples\EXAMPLE folder: `sudo dotnet run`
