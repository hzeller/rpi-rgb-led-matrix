from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t

########################
### External classes ###
########################

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
    cdef cppclass RGBMatrix:
        RGBMatrix(GPIO*, int, int, int) except +
        void SetGPIO(GPIO*)
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()
        void set_luminance_correct(bool)
        bool luminance_correct()
        int width()
        int height()
        void SetPixel(int, int, uint8_t, uint8_t, uint8_t)
        void Clear()
        void Fill(uint8_t, uint8_t, uint8_t)
        void SetBrightness(uint8_t)
        uint8_t brightness()
        FrameCanvas *CreateFrameCanvas()
        FrameCanvas *SwapOnVSync(FrameCanvas*)

    cdef cppclass FrameCanvas:
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()
        int width()
        int height()
        void SetPixel(int, int, uint8_t, uint8_t, uint8_t)
        void Clear()
        void Fill(uint8_t, uint8_t, uint8_t)