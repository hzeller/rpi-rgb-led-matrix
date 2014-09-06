CXXFLAGS=-Wall -O3 -g
OBJECTS=demo-main.o
BINARIES=led-matrix

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_INCDIR=include
RGB_LIBDIR=lib
RGB_LIBRARY_NAME=rgbmatrix

LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
# To link, we also need some other libraries, librt, libm and libpthread
LDFLAGS=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

all : $(BINARIES)

$(LIBRARY):
	$(MAKE) -C lib/

led-matrix : $(OBJECTS) $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES)
	$(MAKE) -C lib clean
