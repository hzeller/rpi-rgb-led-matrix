# distutils: language = c++

from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t

########################
### External classes ###
########################

cdef extern from "gpio.h" namespace "rgb_matrix":
    cdef cppclass CPPGPIO "rgb_matrix::GPIO":
        CPPGPIO() except +
        bool Init()
        uint32_t InitOutputs(uint32_t)
        void SetBits(uint32_t)
        void ClearBits(uint32_t)

cdef extern from "led-matrix.h" namespace "rgb_matrix":
    cdef cppclass CPPRGBMatrix "rgb_matrix::RGBMatrix":
        CPPRGBMatrix(CPPGPIO*, int, int, int) except +
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()
        void set_luminance_correct(bool)
        bool luminance_correct()
        int width()
        int height()
        void SetPixel(int, int, uint8_t, uint8_t, uint8_t)
        void Clear()
        void Fill(uint8_t, uint8_t, uint8_t)
        CPPFrameCanvas *CreateFrameCanvas()
        CPPFrameCanvas *SwapOnVSync(CPPFrameCanvas*)

    cdef cppclass CPPFrameCanvas "rgb_matrix::FrameCanvas":
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()
        int width()
        int height()
        void SetPixel(int, int, uint8_t, uint8_t, uint8_t)
        void Clear()
        void Fill(uint8_t, uint8_t, uint8_t)

######################
### Module Classes ###
######################

cdef class FrameCanvas:
    cdef CPPFrameCanvas *__canvas

    def __dealloc__(self):
        if <void*>self.__canvas != NULL:
            self.__canvas = NULL

    @staticmethod
    cdef FrameCanvas __createInternal(CPPFrameCanvas* newCanvas):
        canvas = FrameCanvas()
        canvas.__canvas = newCanvas
        return canvas

    cdef CPPFrameCanvas* __getCanvas(self) except *:
        if <void*>self.__canvas != NULL:
            return self.__canvas
        raise Exception("FrameCanvas was destroyed or not initialized, you cannot use this object anymore")

    def Fill(self, uint8_t red, uint8_t green, uint8_t blue):
        self.__getCanvas().Fill(red, green, blue)

    def Clear(self):
        self.__getCanvas().Clear()

    def SetPixel(self, int x, int y, uint8_t red, uint8_t green, uint8_t blue):
        self.__getCanvas().SetPixel(x, y, red, green, blue)

    property width:
        def __get__(self): return self.__getCanvas().width()

    property height:
        def __get__(self): return self.__getCanvas().height()

    property pwmBits:
        def __get__(self): return self.__getCanvas().pwmbits()
        def __set__(self, pwmBits): self.__getCanvas().SetPWMBits(pwmBits)

cdef class RGBMatrix:
    cdef CPPRGBMatrix *__matrix
    cdef CPPGPIO *__gpio

    def __cinit__(self, int rows, int chains = 1, int parallel = 1):
        self.__gpio = new CPPGPIO()
        if not self.__gpio.Init():
            raise Exception("Error initializing GPIOs")

        self.__matrix = new CPPRGBMatrix(self.__gpio, rows, chains, parallel)

    def __dealloc__(self):
        self.__matrix.Clear()
        del self.__matrix
        del self.__gpio

    def Fill(self, uint8_t red, uint8_t green, uint8_t blue):
        self.__matrix.Fill(red, green, blue)

    def SetPixel(self, int x, int y, uint8_t red, uint8_t green, uint8_t blue):
        self.__matrix.SetPixel(x, y, red, green, blue)

    def Clear(self):
        self.__matrix.Clear()

    def CreateFrameCanvas(self):
        return FrameCanvas.__createInternal(self.__matrix.CreateFrameCanvas())

    def SwapOnVSync(self, FrameCanvas newFrame):
        return FrameCanvas.__createInternal(self.__matrix.SwapOnVSync(newFrame.__canvas))

    property luminanceCorrect:
        def __get__(self): return self.__matrix.luminance_correct()
        def __set__(self, luminanceCorrect): self.__matrix.set_luminance_correct(luminanceCorrect)

    property pwmBits:
        def __get__(self): return self.__matrix.pwmbits()
        def __set__(self, pwmBits): self.__matrix.SetPWMBits(pwmBits)

    property height:
        def __get__(self): return self.__matrix.height()

    property width:
        def __get__(self): return self.__matrix.width()
