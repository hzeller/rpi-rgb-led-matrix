# Creating RGB matrix library
# When you link this library with your binary, you need to add -lrt -lm -lpthread
# So
#   -lrgbmatrix
##
OBJECTS=gpio.o led-matrix.o framebuffer.o thread.o bdf-font.o graphics.o
TARGET=librgbmatrix.a

INCDIR=../include
CXXFLAGS=-Wall -O3 -g

$(TARGET) : $(OBJECTS)
	ar rcs $@ $^

led-matrix.o: led-matrix.cc $(INCDIR)/led-matrix.h
thread.o : thread.cc $(INCDIR)/thread.h

%.o : %.cc
	$(CXX) -I$(INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
