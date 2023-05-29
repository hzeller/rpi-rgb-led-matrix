# This Makefile is intended to be used only by the toplevel Makefile.
# For any other purposes, use .NET SDK build tools directly

# Don't forget to synchronize these variables with the 'RPiRgbLEDMatrix.csproj' file
RGB_LIBDIR=../../lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).so.1

NUGET_VERSION = 1.0.0
NUGET_ID = HZeller.RPiRgbLEDMatrix
NUGET_CONFIG = Release

NUGET_PACKAGE = /bin/$(NUGET_CONFIG)/$(NUGET_ID).$(NUGET_VERSION).nupkg

$(NUGET_PACKAGE): $(RGB_LIBRARY)
	dotnet pack -c $(NUGET_CONFIG) -p:SkipNative=false -p:PackageId=$(NUGET_ID) -p:Version=$(NUGET_VERSION)

# The examples also depend on the 'RPiRgbLEDMatrix.csproj', but this will be handled by 'dotnet'
build: $(RGB_LIBRARY)
	dotnet build examples/FontExample/FontExample.csproj -p:SkipNative=false
	dotnet build examples/MatrixRain/MatrixRain.csproj -p:SkipNative=false
	dotnet build examples/MinimalExample/MinimalExample.csproj -p:SkipNative=false
	dotnet build examples/PulsingBrightness/PulsingBrightness.csproj -p:SkipNative=false
	dotnet build examples/Rotating3DCube/Rotating3DCube.csproj -p:SkipNative=false
	dotnet build examples/PlayGIF/PlayGIF.csproj -p:SkipNative=false

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

# Used by toplevel Makefile
nuget: $(NUGET_PACKAGE)

# Used in 'RPiRgbLEDMatrix.csproj'
library: $(RGB_LIBRARY)
