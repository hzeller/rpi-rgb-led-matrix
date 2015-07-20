# distutils: language = c++

from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t

cdef class Canvas:
    cdef cppinc.Canvas* __getCanvas(self) except +:
        raise Exception("Not implemented")

cdef class FrameCanvas(Canvas):
    def __dealloc__(self):
        if <void*>self.__canvas != NULL:
            self.__canvas = NULL

    cdef cppinc.Canvas* __getCanvas(self) except *:
        if <void*>self.__canvas != NULL:
            return self.__canvas
        raise Exception("Canvas was destroyed or not initialized, you cannot use this object anymore")

    def Fill(self, uint8_t red, uint8_t green, uint8_t blue):
        (<cppinc.FrameCanvas*>self.__getCanvas()).Fill(red, green, blue)

    def Clear(self):
        (<cppinc.FrameCanvas*>self.__getCanvas()).Clear()

    def SetPixel(self, int x, int y, uint8_t red, uint8_t green, uint8_t blue):
        (<cppinc.FrameCanvas*>self.__getCanvas()).SetPixel(x, y, red, green, blue)

    property width:
        def __get__(self): return (<cppinc.FrameCanvas*>self.__getCanvas()).width()

    property height:
        def __get__(self): return (<cppinc.FrameCanvas*>self.__getCanvas()).height()

    property pwmBits:
        def __get__(self): return (<cppinc.FrameCanvas*>self.__getCanvas()).pwmbits()
        def __set__(self, pwmBits): (<cppinc.FrameCanvas*>self.__getCanvas()).SetPWMBits(pwmBits)


cdef class RGBMatrix(Canvas):
    def __cinit__(self, int rows, int chains = 1, int parallel = 1):
        self.__gpio = new cppinc.GPIO()
        if not self.__gpio.Init():
            raise Exception("Error initializing GPIOs")

        self.__matrix = new cppinc.RGBMatrix(self.__gpio, rows, chains, parallel)

    def __dealloc__(self):
        self.__matrix.Clear()
        del self.__matrix
        del self.__gpio

    cdef cppinc.Canvas* __getCanvas(self) except *:
        if <void*>self.__matrix != NULL:
            return self.__matrix
        raise Exception("Canvas was destroyed or not initialized, you cannot use this object anymore")

    def Fill(self, uint8_t red, uint8_t green, uint8_t blue):
        self.__matrix.Fill(red, green, blue)

    def SetPixel(self, int x, int y, uint8_t red, uint8_t green, uint8_t blue):
        self.__matrix.SetPixel(x, y, red, green, blue)

    def Clear(self):
        self.__matrix.Clear()

    def CreateFrameCanvas(self):
        return __createFrameCanvas(self.__matrix.CreateFrameCanvas())

    def SwapOnVSync(self, FrameCanvas newFrame):
        return __createFrameCanvas(self.__matrix.SwapOnVSync(newFrame.__canvas))

    property luminanceCorrect:
        def __get__(self): return self.__matrix.luminance_correct()
        def __set__(self, luminanceCorrect): self.__matrix.set_luminance_correct(luminanceCorrect)

    property pwmBits:
        def __get__(self): return self.__matrix.pwmbits()
        def __set__(self, pwmBits): self.__matrix.SetPWMBits(pwmBits)

    property brightness:
        def __get__(self): return self.__matrix.brightness()
        def __set__(self, brightness): self.__matrix.SetBrightness(brightness)

    property height:
        def __get__(self): return self.__matrix.height()

    property width:
        def __get__(self): return self.__matrix.width()

cdef __createFrameCanvas(cppinc.FrameCanvas* newCanvas):
    canvas = FrameCanvas()
    canvas.__canvas = newCanvas
    return canvas