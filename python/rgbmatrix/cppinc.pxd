from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t

########################
### External classes ###
########################

cdef extern from "canvas.h" namespace "rgb_matrix":
    cdef cppclass Canvas:
        int width()
        int height()
        void SetPixel(int, int, uint8_t, uint8_t, uint8_t)
        void Clear()
        void Fill(uint8_t, uint8_t, uint8_t)

cdef extern from "gpio.h" namespace "rgb_matrix":
    cdef cppclass GPIO:
        GPIO() except +
        bool Init()
        uint32_t InitOutputs(uint32_t)
        void SetBits(uint32_t)
        void ClearBits(uint32_t)
        void WriteMaskedBits(uint32_t, uint32_t)
        void Write(uint32_t)

cdef extern from "led-matrix.h" namespace "rgb_matrix":
    cdef cppclass RGBMatrix(Canvas):
        RGBMatrix(GPIO*, int, int, int) except +
        void SetGPIO(GPIO*)
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()
        void set_luminance_correct(bool)
        bool luminance_correct()
        void SetBrightness(uint8_t)
        uint8_t brightness()
        FrameCanvas *CreateFrameCanvas()
        FrameCanvas *SwapOnVSync(FrameCanvas*)

    cdef cppclass FrameCanvas(Canvas):
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()

cdef extern from "graphics.h" namespace "rgb_matrix":
    cdef struct Color:
        Color(uint8_t, uint8_t, uint8_t) except +
        uint8_t r
        uint8_t g
        uint8_t b

    cdef cppclass Font:
        Font() except +
        bool LoadFont(const char*)
        int height()
        int baseline()
        int CharacterWidth(uint32_t)
        int DrawGlyph(Canvas*, int, int, const Color, uint32_t);

    cdef int DrawText(Canvas*, const Font, int, int, const Color, const char*)
    cdef void DrawCircle(Canvas*, int, int, int, const Color)
    cdef void DrawLine(Canvas*, int, int, int, int, const Color)