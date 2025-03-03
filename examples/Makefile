# If you see that your LED matrix is too dark at the default setting,
# remove the comment mark below for the line containing "BRIGHTNESS"
# and set it to a higher value (upto 100).
#BRIGHTNESS=       -DLED_BRIGHTNESS=60

# If you have a different mapping of your LED colors in your matrix, you can
# pass this via the color-order flag. The normal situation is "RGB", but
# there are matrices out there with
#   "RBG" (yes, this is a thing);
#   "BGR" (Chipsets from China that use this color order);
# Spelling out the color order on the command line is also possible, e.g.
#   ./scrolling-text.py --led-rgb-sequence=BGR
#COLOR_ORDER_MAPPING=-DRGB_MAPPING_OPTIONS="RBG"

CFLAGS=-Wall -O3 -g -Wextra -Wno-unused-parameter
CXXFLAGS=$(CFLAGS) -std=c++17
OBJECTS=demo-main.o minimal-example.o c-example.o
BINARIES=demo minimal-example c-example

# Emulator demo
ENABLE_EMULATOR?=0

ifeq ($(ENABLE_EMULATOR), 1)
  OBJECTS+=emulator-demo.o
  BINARIES+=ledmatrix-emulator
  # SDL2 flags, use sdl2-config tool to find them
  SDL2_CFLAGS=$(shell sdl2-config --cflags)
  SDL2_LDFLAGS=$(shell sdl2-config --libs)
endif

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_INCDIR=../include
RGB_LIBDIR=../lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

# To compile with emulator support when enabled
ifeq ($(ENABLE_EMULATOR), 1)
  CXXFLAGS+=-DENABLE_EMULATOR $(SDL2_CFLAGS)
  LDFLAGS+=$(SDL2_LDFLAGS)
endif

all : $(BINARIES)

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR) ENABLE_EMULATOR=$(ENABLE_EMULATOR)

minimal-example : minimal-example.o $(RGB_LIBRARY)
	$(CXX) $< -o $@ $(LDFLAGS)

c-example : c-example.o $(RGB_LIBRARY)
	$(CC) $< -o $@ $(LDFLAGS)

# Special target for emulator demo when enabled
ifeq ($(ENABLE_EMULATOR), 1)
ledmatrix-emulator : emulator-demo.o $(RGB_LIBRARY)
	$(CXX) $< -o $@ $(LDFLAGS)
endif

# Since the C example uses the C API, which is a simple wrapper via a C++
# method call, we need to link it with the C++ compiler to get the
# C++ dependencies right.
c-example.o : c-example.c
	$(CC) -I$(RGB_INCDIR) $(CFLAGS) -c -o $@ $<

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES)
