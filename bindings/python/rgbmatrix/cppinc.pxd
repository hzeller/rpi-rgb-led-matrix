from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t

########################
### External classes ###
########################

cdef extern from "canvas.h" namespace "rgb_matrix":
    cdef cppclass Canvas:
        int width()
        int height()
        void SetPixel(int, int, uint8_t, uint8_t, uint8_t) nogil
        void Clear() nogil
        void Fill(uint8_t, uint8_t, uint8_t) nogil

cdef extern from "led-matrix.h" namespace "rgb_matrix":
    cdef cppclass RGBMatrix(Canvas):
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()
        void set_luminance_correct(bool)
        bool luminance_correct()
        void SetBrightness(uint8_t)
        uint8_t brightness()
        FrameCanvas *CreateFrameCanvas()
        FrameCanvas *SwapOnVSync(FrameCanvas*, uint8_t)

    cdef cppclass FrameCanvas(Canvas):
        bool SetPWMBits(uint8_t)
        uint8_t pwmbits()
        void SetBrightness(uint8_t)
        uint8_t brightness()

    struct RuntimeOptions:
      RuntimeOptions() except +
      int gpio_slowdown
      int daemon
      int drop_privileges
      const char *drop_priv_user
      const char *drop_priv_group


    RGBMatrix *CreateMatrixFromOptions(Options &options, RuntimeOptions runtime_options)



cdef extern from "led-matrix.h" namespace "rgb_matrix::RGBMatrix":
    cdef struct Options:
        Options() except +

        const char *hardware_mapping

        int rows
        int cols
        int chain_length
        int parallel
        int pwm_bits
        int pwm_lsb_nanoseconds
        int brightness
        int scan_mode
        int row_address_type
        int multiplexing
        int pwm_dither_bits
        int limit_refresh_rate_hz

        bool disable_hardware_pulsing
        bool show_refresh_rate
        bool inverse_colors

        const char *led_rgb_sequence
        const char *pixel_mapper_config
        const char *panel_type

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
