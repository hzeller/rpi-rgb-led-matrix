# Makefile for the emulator-demo program

# Location of the RGB Matrix Library
RGB_LIB_DISTRIBUTION=..
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib

# Compiler options
CC = g++
CFLAGS = -Wall -O3 -g -Wextra -Wno-unused-parameter
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L$(RGB_LIBDIR) 
LDLIBS = -lrgbmatrix -lrt -lm -lpthread

# Check if ENABLE_EMULATOR is defined
ifdef ENABLE_EMULATOR
CFLAGS += -DENABLE_EMULATOR
LDLIBS += -lSDL2
EMULATOR_TARGETS = emulator-demo
EMULATOR_OBJECTS = emulator-demo.o
EMULATOR_DESC = --led-emulator-scale=12 --led-emulator-refresh=60
else
EMULATOR_TARGETS =
EMULATOR_OBJECTS =
endif

# If you're using a Raspberry Pi, uncomment to use hardware
# Otherwise stick with emulator mode
#HARDWARE_DESC=--led-cols=64 --led-rows=32 --led-chain=1
#HARDWARE_DESC+= --led-parallel=1 --led-slowdown-gpio=4
# Add any other flags you might need

# Default build target
all : $(EMULATOR_TARGETS)

# Only build emulator-demo with SDL2 support when ENABLE_EMULATOR is defined
emulator-demo: emulator-demo.o
	$(CC) $(CXXFLAGS) emulator-demo.o $(LDFLAGS) -o $@ $(LDLIBS)

# Rule for building object files
%.o : %.cc
	$(CC) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<

# Install SDL2 dependency (for Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libsdl2-dev

# Clean up built files
clean:
	rm -f $(EMULATOR_TARGETS) $(EMULATOR_OBJECTS)

# Run the demo using the emulator (only if ENABLE_EMULATOR is defined)
ifdef ENABLE_EMULATOR
run: emulator-demo
	./emulator-demo $(EMULATOR_DESC)
else
run:
	@echo "Emulator not enabled. Build with ENABLE_EMULATOR=1 to use the emulator."
endif

# Run the demo on hardware (if you have the physical matrix)
run-hw: emulator-demo
	./emulator-demo $(HARDWARE_DESC) --led-no-emulator

.PHONY: all clean install-deps run run-hw
