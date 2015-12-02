# Creating RGB matrix library
# When you link this library with your binary, you need to add -lrt -lm -lpthread
# So
#   -lrgbmatrix
##
OBJECTS=gpio.o led-matrix.o framebuffer.o thread.o bdf-font.o graphics.o transformer.o
TARGET=librgbmatrix.a

###
# After you change any of the following DEFINES, make sure to 'make clean'
# and make again
###

# If you see that your display is inverse, you might have a matrix variant
# has uses inverse logic for the RGB bits. In that case: uncomment this.
#DEFINES+=-DINVERSE_RGB_DISPLAY_COLORS

# Some panels out there seem to have Green and Blue swapped (even though the
# writing on connector looks correct). This option will swap these colors if
# you happen to have gotten such panel.
#DEFINES+=-DRGB_SWAP_GREEN_BLUE

# For curiosity reasons: uncomment to see refresh rate in terminal.
#DEFINES+=-DSHOW_REFRESH_RATE

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
# Sometimes, you even have to give RGB_SLOWDOWN_GPIO=2 for particularly slow
# panels or bad signal cable situations.
DEFINES+=-DRGB_SLOWDOWN_GPIO=1

# ------------ Pinout options; usually no change needed here --------------

# Uncomment the following line for Adafruit Matrix HAT gpio mappings.
# If you have an Adafruit HAT ( https://www.adafruit.com/products/2345 ),
# you need to use this option as the HAT swaps pins around that are not
# compatible with the default mapping.
#DEFINES+=-DADAFRUIT_RGBMATRIX_HAT

# Uncomment if you want to use the Adafruit HAT with stable PWM timings.
# The newer version of this library allows for much more stable (less flicker)
# output, but it does not work with the Adafruit HAT unless you do a
# simple hardware hack on them:
# connect GPIO 4 (old OE) with 18 (the new OE); there are
# convenient solder holes labeled 4 and 18 on the Adafruit HAT, pretty
# close together.
# Then uncomment the following define and recompile.
#DEFINES+=-DADAFRUIT_RGBMATRIX_HAT_PWM

# Compatibility option, usually not needed.
# If you have used this library before July 2015 and have wired it up for that,
# then you need to define RGB_CLASSIC_PINOUT for your panel to work. The
# standard pinout changed so that we can make use of the PWM pin for much
# more stable timing. So even if you have the classic pinout, you might
# consider wiring up the new one (sorry for the inconvenience).
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
framebuffer.o: framebuffer.cc framebuffer-internal.h
graphics.o: graphics.cc utf8-internal.h

%.o : %.cc compiler-flags
	$(CXX) -I$(INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

compiler-flags: FORCE
	@echo '$(CXXFLAGS)' | cmp -s - $@ || echo '$(CXXFLAGS)' > $@

.PHONY: FORCE
