# Creating RGB matrix library
# When you link this library with your binary, you need to add -lrt -lm -lpthread
# So
#   -lrgbmatrix
##
OBJECTS=gpio.o led-matrix.o options-initialize.o framebuffer.o \
        thread.o bdf-font.o graphics.o transformer.o led-matrix-c.o
TARGET=librgbmatrix

###
# After you change any of the following DEFINES, make sure to 'make' again.
#
# Note, most of these options now can be set programmatically and via command
# line options as well. So be prepared for these to be removed.
###

# If you see that your display is inverse, you might have a matrix variant
# has uses inverse logic for the RGB bits. In that case: uncomment this.
#DEFINES+=-DINVERSE_RGB_DISPLAY_COLORS

# Some panels out there seem to have Green and Blue swapped (even though the
# writing on connector looks correct). This option will swap these colors if
# you happen to have gotten such panel.
#DEFINES+=-DRGB_SWAP_GREEN_BLUE

# For curiosity reasons and while tweaking values for LSB_PWM_NANOSECONDS,
# uncomment to see refresh rate in terminal.
#DEFINES+=-DSHOW_REFRESH_RATE

# For low refresh rates below 100Hz (e.g. a lot of panels), the eye will notice
# some flicker. With this option enabled, the refreshed lines are interleaved,
# so it is less noticeable. But looks less pleasant with fast eye movements.
#DEFINES+=-DRGB_SCAN_INTERLACED=1

# The signal can be too fast for some LED panels, in particular with newer
# (faster) Raspberry Pi 2s - in that case, the LED matrix only shows garbage.
# This allows to slow down the GPIO for these cases.
#
# Set to 1 for RPi2 or RPi3 (default below), because they are typically
# faster than the panels can digest.
#
# Set to 0 (or comment out) for RPi1, that are slow enough.
#
# Sometimes, you even have to give RGB_SLOWDOWN_GPIO=2 or even 3 for
# particularly slow panels or bad signal cable situations. If that happens, you
# typically should double check cables and add TTL level converter if you
# haven't.
#DEFINES+=-DRGB_SLOWDOWN_GPIO=1

# This allows to change the base time-unit for the on-time in the lowest
# significant bit in nanoseconds.
# Higher numbers provide better quality (more accurate color, less ghosting),
# but have a negative impact on the frame rate.
#
# For the same frame-rate, displays with higher multiplexing (e.g. 1:16 or 1:32)
# require lower values.
#
# Good values for full-color display (PWM=11) are somewhere between 100 and 300.
#
# If you you use reduced bit color (e.g. PWM=1 for 8 colors like for text),
# then higher values might be good to minimize ghosting (and you can afford
# that, because lower PWM values result in higher frame-rates).
#
# How to decide ? Just leave the default if things are fine. If you see
# ghosting in high-contrast applications (e.g. text), increase the value.
# If you want to tweak, watch the framerate (-DSHOW_FRAME_RATE) while playing
# with this number and the PWM values.
#DEFINES+=-DLSB_PWM_NANOSECONDS=130

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

# Typically, a Hub75 panel is split in two half displays, so that a 1:16
# multiplexing actually multiplexes over two half displays and gives 32 lines.
# There are some other displays out there that you might experiment with
# that are internally wired to only have one sub-panel. In that case you might
# want to try this define to get a more reasonable canvas mapping.
# This option is typically _not_ needed, only use when you attempt to connect
# some oddball old (typically one-colored) display, such as Hub12.
#DEFINES+=-DONLY_SINGLE_SUB_PANEL

# This is to debug problems with the hardware pulse generation. The PWM hardware
# module is also used by Raspberry Pi sound system, so there might be
# interference. Note, you typically don't want the hardware pulses disabled, as
# the image will have visible brightness glitches; but for debugging, this is
# a good choice.
#DEFINES+=-DDISABLE_HARDWARE_PULSES

# Experimental. Only use this if you do 1 bit PWM.
#DEFINES+=-DEXPERIMENTAL_HIGH_BRIGHTNESS

# If someone gives additional values on the make commandline e.g.
# make USER_DEFINES="-DSHOW_REFRESH_RATE"
DEFINES+=$(USER_DEFINES)

INCDIR=../include
CXXFLAGS=-Wall -O3 -g -fPIC $(DEFINES)

all : $(TARGET).a $(TARGET).so.1

$(TARGET).a : $(OBJECTS)
	ar rcs $@ $^

$(TARGET).so.1 : $(OBJECTS)
	g++ -shared -Wl,-soname,$@ -o $@ $^ -lpthread  -lrt -lm -lpthread

led-matrix.o: led-matrix.cc $(INCDIR)/led-matrix.h
thread.o : thread.cc $(INCDIR)/thread.h
framebuffer.o: framebuffer.cc framebuffer-internal.h
graphics.o: graphics.cc utf8-internal.h

%.o : %.cc compiler-flags
	$(CXX) -I$(INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET).a $(TARGET).so.1

compiler-flags: FORCE
	@echo '$(CXX) $(CXXFLAGS)' | cmp -s - $@ || echo '$(CXX) $(CXXFLAGS)' > $@

.PHONY: FORCE
