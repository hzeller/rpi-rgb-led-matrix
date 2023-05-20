CXXFLAGS=-O3 -W -Wall -Wextra -Wno-unused-parameter -D_FILE_OFFSET_BITS=64
OBJECTS=led-image-viewer.o text-scroller.o
BINARIES=led-image-viewer text-scroller

OPTIONAL_OBJECTS=video-viewer.o
OPTIONAL_BINARIES=video-viewer

# Where our library resides. You mostly only need to change the
# RGB_LIB_DISTRIBUTION, this is where the library is checked out.
RGB_LIB_DISTRIBUTION=..
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
RGB_LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

# Imagemagic flags, only needed if actually compiled.
MAGICK_CXXFLAGS?=$(shell GraphicsMagick++-config --cppflags --cxxflags)
MAGICK_LDFLAGS?=$(shell GraphicsMagick++-config --ldflags --libs)
AV_CXXFLAGS=$(shell pkg-config --cflags  libavcodec libavformat libswscale libavutil)
AV_LDFLAGS=$(shell pkg-config --cflags --libs  libavcodec libavformat libswscale libavutil)

simple: $(BINARIES)

all : $(BINARIES) $(OPTIONAL_BINARIES)

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

text-scroller: text-scroller.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) text-scroller.o -o $@ $(LDFLAGS) $(RGB_LDFLAGS)

led-image-viewer: led-image-viewer.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) led-image-viewer.o -o $@ $(LDFLAGS) $(RGB_LDFLAGS) $(MAGICK_LDFLAGS)

video-viewer: video-viewer.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) video-viewer.o -o $@ $(LDFLAGS) $(RGB_LDFLAGS) $(AV_LDFLAGS)

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<

led-image-viewer.o : led-image-viewer.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES) $(OPTIONAL_OBJECTS) $(OPTIONAL_BINARIES)

FORCE:
.PHONY: FORCE
