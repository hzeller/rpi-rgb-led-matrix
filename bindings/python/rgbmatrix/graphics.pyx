# distutils: language = c++

from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t

from . cimport core

cdef class Color:
    def __init__(self, uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0):
        self.__color.r = red
        self.__color.g = green
        self.__color.b = blue

    property red:
        def __get__(self): return self.__color.r
        def __set__(self, uint8_t value): self.__color.r = value

    property green:
        def __get__(self): return self.__color.g
        def __set__(self, uint8_t value): self.__color.g = value

    property blue:
        def __get__(self): return self.__color.b
        def __set__(self, uint8_t value): self.__color.b = value

cdef class Font:
    def CharacterWidth(self, uint32_t char):
        return self.__font.CharacterWidth(char)

    def LoadFont(self, file):
        if (not self.__font.LoadFont(file.encode('utf-8'))):
            raise Exception("Couldn't load font " + file)

    def DrawGlyph(self, core.Canvas c, int x, int y, Color color, uint32_t char):
        return self.__font.DrawGlyph(c._getCanvas(), x, y, color.__color, char)

    property height:
        def __get__(self): return self.__font.height()

    property baseline:
        def __get__(self): return self.__font.baseline()

def DrawText(core.Canvas c, Font f, int x, int y, Color color, text):
    return cppinc.DrawText(c._getCanvas(), f.__font, x, y, color.__color, text.encode('utf-8'))

def DrawCircle(core.Canvas c, int x, int y, int r, Color color):
    cppinc.DrawCircle(c._getCanvas(), x, y, r, color.__color)

def DrawLine(core.Canvas c, int x1, int y1, int x2, int y2, Color color):
    cppinc.DrawLine(c._getCanvas(), x1, y1, x2, y2, color.__color)

# Local Variables:
# mode: python
# End:
