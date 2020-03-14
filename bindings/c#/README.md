C# bindings for RGB Matrix library
======================================

Building
--------

To build the C# wrapper for the RGB Matrix C library you need to first have mono installed. 

### Install Mono

```shell
$ sudo apt-get update
$ sudo apt-get install mono-complete
```

Then, in the root directory for the matrix library type

```shell
make build-csharp
```

To run the example applications in the c#\examples folder

```shell
sudo mono minimal-example.exe
```

Notes
--------

C# applications look for libraries in the working directory of the application. To use this library for your own projects you will need to ensure you have RGBLedMatrix.dll and librgbmatrix.so in the same folder as your exe.
