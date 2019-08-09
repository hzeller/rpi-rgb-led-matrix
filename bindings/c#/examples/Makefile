CSHARP_LIB=RGBLedMatrix.dll
CSHARP_COMPILER=mcs
CSHARP_LIBDIR=..
CSHARP_LIBRARY=$(CSHARP_LIBDIR)/$(CSHARP_LIB)

RGB_LIBDIR=../../../lib
RGB_LIBRARY_NAME=librgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/$(RGB_LIBRARY_NAME).so.1

all: $(CSHARP_LIB)
	cp $(RGB_LIBRARY) $(RGB_LIBRARY_NAME).so
	cp $(CSHARP_LIBRARY) $(CSHARP_LIB)
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:minimal-example.exe minimal-example.cs
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:matrix-rain.exe matrix-rain.cs
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:font-example.exe font-example.cs
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:pulsing-brightness.exe pulsing-brightness.cs

minimal-example.exe: $(CSHARP_LIB)
	cp $(RGB_LIBRARY) $(RGB_LIBRARY_NAME).so
	cp $(CSHARP_LIBRARY) $(CSHARP_LIB)
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:minimal-example.exe minimal-example.cs

matrix-rain.exe: $(CSHARP_LIB)
	cp $(RGB_LIBRARY) $(RGB_LIBRARY_NAME).so
	cp $(CSHARP_LIBRARY) $(CSHARP_LIB)
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:matrix-rain.exe matrix-rain.cs

font-example.exe: $(CSHARP_LIB)
	cp $(RGB_LIBRARY) $(RGB_LIBRARY_NAME).so
	cp $(CSHARP_LIBRARY) $(CSHARP_LIB)
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:font-example.exe font-example.cs

pulsing-brightness.exe: $(CSHARP_LIB)
	cp $(RGB_LIBRARY) $(RGB_LIBRARY_NAME).so
	cp $(CSHARP_LIBRARY) $(CSHARP_LIB)
	$(CSHARP_COMPILER) -r:$(CSHARP_LIB) -out:pulsing-brightness.exe pulsing-brightness.cs

$(CSHARP_LIB) :
	$(MAKE) -C $(CSHARP_LIBDIR)

.PHONY : all
