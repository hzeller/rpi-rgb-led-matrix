# distutils: language = c++

from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t
from PIL import Image
haveNumpy = False
try:
    import numpy as np
    cimport numpy as np
    haveNumpy = True
except ImportError:
    haveNumpy = False

cdef class Canvas:
    cdef cppinc.Canvas* __getCanvas(self) except +:
        raise Exception("Not implemented")

    # First implementation of a SetImage(). OPTIMIZE_ME: A more native
    # implementation that directly reads the buffer and calls the underlying
    # C functions can certainly be faster.
    def SetImage(self, image, int offset_x = 0, int offset_y = 0, fastIfPossible=True):
        if (image.mode != "RGB"):
            raise Exception("Currently, only RGB mode is supported for SetImage(). Please create images with mode 'RGB' or convert first with image = image.convert('RGB'). Pull requests to support more modes natively are also welcome :)")

        if haveNumpy and fastIfPossible:
            self._fastSetImage(image, offset_x, offset_y)
        else:
            img_width, img_height = image.size
            pixels = image.load()
            for x in range(max(0, -offset_x), min(img_width, self.width - offset_x)):
                for y in range(max(0, -offset_y), min(img_height, self.height - offset_y)):
                    (r, g, b) = pixels[x, y]
                    self.SetPixel(x + offset_x, y + offset_y, r, g, b)

    def _fastSetImage(self, image, int offset_x, int offset_y):
        cdef np.ndarray[np.uint8_t, mode='c', ndim=3] pixels = np.asarray(image, dtype=np.uint8, order='C')
        img_width, img_height = image.size
        print("Running fastSetImage")
        self.SetPixels3D(offset_x, offset_y, img_width, img_height, &pixels[0,0,0])

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

    def SetPixels(self, int x, int y, int width, int height,
                  const uint8_t *red, const uint8_t *green, const uint8_t *blue):
        (<cppinc.FrameCanvas*>self.__getCanvas()).SetPixels(x, y, width, height, red, green, blue)

    def SetPixels3D(self, int x, int y, int width, int height, const uint8_t *pixels):
        (<cppinc.FrameCanvas*>self.__getCanvas()).SetPixels3D(x, y, width, height, pixels)

    property width:
        def __get__(self): return (<cppinc.FrameCanvas*>self.__getCanvas()).width()

    property height:
        def __get__(self): return (<cppinc.FrameCanvas*>self.__getCanvas()).height()

    property pwmBits:
        def __get__(self): return (<cppinc.FrameCanvas*>self.__getCanvas()).pwmbits()
        def __set__(self, pwmBits): (<cppinc.FrameCanvas*>self.__getCanvas()).SetPWMBits(pwmBits)


cdef class RGBMatrix(Canvas):
    def __cinit__(self, int rows, int chains = 1, int parallel = 1):
        # TODO(Saij): this should initialize an RGBMatrix::Options and
        # RuntimeOptions, then call CreateMatrixFromOptions() instead of the
        # cppinc.RGBMatrix() constructor directly. No __gpio needed anymore.
        # The options allow to set more things, so they should probably be
        # available as named parameters in Python ?
        self.__gpio = new cppinc.GPIO()
        if not self.__gpio.Init():
            raise Exception("Error initializing GPIOs")  # will segfault?!
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

    def SetPixels(self, int x, int y, int width, int height,
                  const uint8_t *red, const uint8_t *green, const uint8_t *blue):
        self.__matrix.SetPixels(x, y, width, height, red, green, blue)

    def SetPixels3D(self, int x, int y, int width, int height, const uint8_t *pixels):
        self.__matrix.SetPixels3D(x, y, width, height, pixels)

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

# Local Variables:
# mode: python
# End:
