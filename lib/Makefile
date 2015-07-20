# Creating RGB matrix library
# When you link this library with your binary, you need to add -lrt -lm -lpthread
# So
#   -lrgbmatrix
##
OBJECTS=gpio.o led-matrix.o framebuffer.o thread.o bdf-font.o graphics.o
TARGET=librgbmatrix.a

###
# After you change any of the following DEFINES, make sure to 'make clean'
# and make again
###

# If you see that your display is inverse, you might have a matrix variant
# has uses inverse logic for the RGB bits. In that case: uncomment this.
#DEFINES+=-DINVERSE_RGB_DISPLAY_COLORS

# For curiosity reasons: uncomment to see refresh rate in terminal.
#DEFINES+=-DSHOW_REFRESH_RATE

# The hardeare PWM timer creates very stable timing so that the display
# is pretty good in displaying a stable output. However, at this point
# the brighness is a bit lower. In that case, you can disable the use of
# the PWM timer here. Expect more random flicker.
#DEFINES+=-DUSE_HARDWARE_PWM_TIMER=0

# The signal can be too fast for some LED panels, in particular with newer
# (faster) Raspberry Pi 2s.
# In these cases, you want to make sure that
#   - The cables are short enough from the GPIO header to the panel.
#   - Maybe use an active level shifter and bus driver, for instance by building
#     up one of the adapter-PCBs in the ../adapter/ directory or use
#     the Adafruit HAT. The output drivers of the GPIO are not really good
#     in driving long cables - this will improve the situation.
#
# If the above fails or you can't implement them, try uncommenting the
# following line and recompile; it will slow down GPIO, but will as well reduce
# the frame-rate.
#DEFINES+=-DRGB_SLOWDOWN_GPIO

# ------------ Pinout options; usually no change needed here --------------

# Uncomment the following line for Adafruit Matrix HAT gpio mappings
#DEFINES+=-DADAFRUIT_RGBMATRIX_HAT

# If you have used this library before July 2015 and have wired it up, then
# you need to define RGB_CLASSIC_PINOUT for your panel to work. The standard
# pinout changed so that we can make use of the PWM pin for much more stable
# timing. So even if you have the classic pinout, you might consider wiring
# up the new one (sorry for the inconvenience).
#DEFINES+=-DRGB_CLASSIC_PINOUT

# If you use RGB_CLASSIC_PINOUT and use Raspberry Pi 1, Revision A,
# this might be useful.
#DEFINES+=-DONLY_SINGLE_CHAIN

INCDIR=../include
CXXFLAGS=-Wall -O3 -g -fPIC $(DEFINES)

$(TARGET) : $(OBJECTS)
	ar rcs $@ $^

led-matrix.o: led-matrix.cc $(INCDIR)/led-matrix.h
thread.o : thread.cc $(INCDIR)/thread.h

%.o : %.cc
	$(CXX) -I$(INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
