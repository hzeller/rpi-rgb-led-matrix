# Creating RGB matrix library
# When you link this library with your binary, you need to add -lrt -lm -lpthread
# So
#   -lrgbmatrix
##
OBJECTS=gpio.o led-matrix.o framebuffer.o thread.o bdf-font.o graphics.o
TARGET=librgbmatrix.a

# Uncomment to allow to use up to three parallel boards.
# This will only work on the plus models or RPi2 with 40 GPIO pins.
#
# This is useful in any case, _unless_ you have an older installation with
# classic cabeling of previous versions of this library or a
# Raspberry Pi 1, Revision 1 (very old).
#
# The reason why this is disabled by default is that it is not compatible with
# GPIO cabelings of previous versions of this software and thus is already
# wired up 'in the field'. Also it will drop support for the very first
# Raspberry Pi, Revision 1.
# Hence we leave this off for now for the least amount of surprise.
# If you connect 3 chains in parallel, switch this on. The default
# will work up to 2 chains just fine, but you won't be able to use I²C.
#DEFINES+=-DSUPPORT_MULTI_PARALLEL

# If you see that your display is inverse, you might have a matrix variant
# has uses inverse logic for the RGB bits. In that case: uncomment this.
#DEFINES+=-DINVERSE_RGB_DISPLAY_COLORS

DEFINES+=-DADAFRUIT_RGBMATRIX_HAT

INCDIR=../include
CXXFLAGS=-Wall -O3 -g $(DEFINES)

$(TARGET) : $(OBJECTS)
	ar rcs $@ $^

led-matrix.o: led-matrix.cc $(INCDIR)/led-matrix.h
thread.o : thread.cc $(INCDIR)/thread.h

%.o : %.cc
	$(CXX) -I$(INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
