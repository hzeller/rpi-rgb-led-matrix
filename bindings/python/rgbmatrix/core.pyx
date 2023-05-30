# distutils: language = c++

from libcpp cimport bool
from libc.stdint cimport uint8_t, uint32_t, uintptr_t
from PIL import Image
import cython
from cpython cimport PyBUF_WRITABLE

cdef object init_key = object()

cdef class Canvas:
    def __init__(self):
        raise TypeError("This class cannot be instantiated directly.")

    cdef cppinc.Canvas* __getCanvas(self) except NULL:
        raise NotImplementedError()

    @property
    def height(self):
        return self.__getCanvas().height()

    @property
    def width(self):
        return self.__getCanvas().width()

    def SetPixel(self, int x, int y, uint8_t red, uint8_t green, uint8_t blue):
        self.__getCanvas().SetPixel(x, y, red, green, blue)

    def Clear(self):
        self.__getCanvas().Clear()

    def Fill(self, uint8_t red, uint8_t green, uint8_t blue):
        self.__getCanvas().Fill(red, green, blue)

    def SetImage(self, image, int offset_x = 0, int offset_y = 0, unsafe=True):
        if (image.mode != "RGB"):
            raise Exception("Currently, only RGB mode is supported for SetImage(). Please create images with mode 'RGB' or convert first with image = image.convert('RGB'). Pull requests to support more modes natively are also welcome :)")

        if unsafe:
            #In unsafe mode we directly access the underlying PIL image array
            #in cython, which is considered unsafe pointer accecss,
            #however it's super fast and seems to work fine
            #https://groups.google.com/forum/#!topic/cython-users/Dc1ft5W6KM4
            img_width, img_height = image.size
            self.SetPixelsPillow(offset_x, offset_y, img_width, img_height, image)
        else:
            # First implementation of a SetImage(). OPTIMIZE_ME: A more native
            # implementation that directly reads the buffer and calls the underlying
            # C functions can certainly be faster.
            img_width, img_height = image.size
            pixels = image.load()
            for x in range(max(0, -offset_x), min(img_width, self.width - offset_x)):
                for y in range(max(0, -offset_y), min(img_height, self.height - offset_y)):
                    (r, g, b) = pixels[x, y]
                    self.SetPixel(x + offset_x, y + offset_y, r, g, b)

    @cython.boundscheck(False)
    @cython.wraparound(False)
    def SetPixelsPillow(self, int xstart, int ystart, int width, int height, image):
        cdef cppinc.Canvas* canvas = self.__getCanvas()
        cdef int frame_width = canvas.width()
        cdef int frame_height = canvas.height()
        cdef int row, col
        cdef uint8_t r, g, b
        cdef uint32_t **image_ptr
        cdef uint32_t pixel
        image.load()
        ptr_tmp = dict(image.im.unsafe_ptrs)['image32']
        image_ptr = (<uint32_t **>(<uintptr_t>ptr_tmp))

        for col in range(max(0, -xstart), min(width, frame_width - xstart)):
            for row in range(max(0, -ystart), min(height, frame_height - ystart)):
                pixel = image_ptr[row][col]
                r = (pixel ) & 0xFF
                g = (pixel >> 8) & 0xFF
                b = (pixel >> 16) & 0xFF
                canvas.SetPixel(xstart+col, ystart+row, r, g, b)

# class with buffer interface, used to publish framebuffer to python
cdef class FrameData:
    cdef FrameCanvas __owner                      # FrameData needs to keep FrameCanvas alive
    cdef const char* __data
    cdef readonly size_t size                     # size is available to python
    def __cinit__(self, key=None, FrameCanvas owner=None):
        if key is not init_key:
            raise TypeError("This class cannot be instantiated directly.");
        self.__owner = owner

    @staticmethod
    cdef FrameData __create(FrameCanvas owner, const char* data, size_t size):
        cdef FrameData fd = FrameData(init_key, owner)
        fd.__data = data
        fd.size = size
        return fd

    def __getbuffer__(self, Py_buffer *buffer, int flags):
        if flags & PyBUF_WRITABLE:
            raise BufferError("FrameData is read-only")
        buffer.buf = <void *>self.__data
        buffer.obj = self
        buffer.len = self.size                # size in bytes
        buffer.readonly = True                  # Technically, it may be possible to overwrite internal buffer, but it is dangerous
        buffer.itemsize = 1                     # opaque buffer TODO
        buffer.format = NULL                    # implicit B (bytes)
        buffer.ndim = 0                         # single item
        buffer.shape = NULL
        buffer.strides = NULL
        buffer.suboffsets = NULL
        buffer.internal = NULL

    def __releasebuffer__(self, Py_buffer *buffer):
        pass

    def __repr__(self):
        return f"<FrameData owned by {self.__owner} data={<size_t>self.__data:#x}, len={self.size}"

cdef class FrameCanvas(Canvas):
    def __cinit__(self, key=None, RGBMatrix owner=None):
        if key is not init_key:                                 # prevent even construction using __new__
            raise TypeError("Use RGBMatrix.CreateFrameCanvas() to create FrameCanvas");
        self.__owner = owner

    def __del__(self):                          # use del if available - guarantees that owner is not deleted first (may not work before Python 3.4)
        self.__dealloc__()

    def __dealloc__(self):
        if self.__owner is not None and self.__canvas != NULL:    # C++ FrameCanvas is owned by RGBMatrix, recycle it
            self.__owner.__recycle_put(self.__canvas)
            self.__canvas = NULL

    @staticmethod
    cdef FrameCanvas __create(RGBMatrix owner, cppinc.FrameCanvas *canvas):
        if canvas == NULL:
            raise Exception("Can't create canvas from NULL")
        cdef FrameCanvas fc = FrameCanvas.__new__(FrameCanvas, init_key, owner)  # skip __init__ here
        fc.__canvas = canvas
        return fc

    cdef cppinc.Canvas* __getCanvas(self) except NULL:
        assert self.__canvas != NULL
        return self.__canvas

    @property
    def framebuffer(self):
        cdef char *data
        cdef size_t len
        # get pointer and length
        self.__canvas.Serialize(<const char**>&data, &len)
        return FrameData.__create(self, data, len)

    @property
    def pwmBits(self): return self.__canvas.pwmbits()
    @pwmBits.setter
    def pwmBits(self, pwmBits): self.__canvas.SetPWMBits(pwmBits)

    @property
    def brightness(self): return self.__canvas.brightness()
    @brightness.setter
    def brightness(self, val): self.__canvas.SetBrightness(val)

cdef charp_to_python(const char *s):
    return <bytes>(s) if s != NULL else None

cdef python_to_bytes(s, where):
    if isinstance(s, str):
        return s.encode('utf-8')
    if isinstance(s, bytes) or s is None:
        return s
    if isinstance(s, bytearray):
       return bytes(s)
    raise TypeError(f"TypeError: {where} must be [str, bytes, bytearray, None], not {type(s).__name__}")

cdef const char* bytes_to_charp(bytes s):
    return <const char*>s if s is not None else NULL

cdef class RGBMatrixOptions:
    # RGBMatrix::Options properties
    property hardware_mapping:
        def __get__(self): return charp_to_python(self.__options.hardware_mapping)
        def __set__(self, value):
            self.__py_encoded_hardware_mapping = python_to_bytes(value, "hardware_mapping")
            self.__options.hardware_mapping = bytes_to_charp(self.__py_encoded_hardware_mapping)

    property rows:
        def __get__(self): return self.__options.rows
        def __set__(self, uint8_t value): self.__options.rows = value

    property cols:
        def __get__(self): return self.__options.cols
        def __set__(self, uint32_t value): self.__options.cols = value

    property chain_length:
        def __get__(self): return self.__options.chain_length
        def __set__(self, uint8_t value): self.__options.chain_length = value

    property parallel:
        def __get__(self): return self.__options.parallel
        def __set__(self, uint8_t value): self.__options.parallel = value

    property pwm_bits:
        def __get__(self): return self.__options.pwm_bits
        def __set__(self, uint8_t value): self.__options.pwm_bits = value

    property pwm_lsb_nanoseconds:
        def __get__(self): return self.__options.pwm_lsb_nanoseconds
        def __set__(self, uint32_t value): self.__options.pwm_lsb_nanoseconds = value

    property brightness:
        def __get__(self): return self.__options.brightness
        def __set__(self, uint8_t value): self.__options.brightness = value

    property scan_mode:
        def __get__(self): return self.__options.scan_mode
        def __set__(self, uint8_t value): self.__options.scan_mode = value

    property multiplexing:
        def __get__(self): return self.__options.multiplexing
        def __set__(self, uint8_t value): self.__options.multiplexing = value

    property row_address_type:
        def __get__(self): return self.__options.row_address_type
        def __set__(self, uint8_t value): self.__options.row_address_type = value

    property disable_hardware_pulsing:
        def __get__(self): return self.__options.disable_hardware_pulsing
        def __set__(self, value): self.__options.disable_hardware_pulsing = value

    property show_refresh_rate:
        def __get__(self): return self.__options.show_refresh_rate
        def __set__(self, value): self.__options.show_refresh_rate = value

    property inverse_colors:
        def __get__(self): return self.__options.inverse_colors
        def __set__(self, value): self.__options.inverse_colors = value

    property led_rgb_sequence:
        def __get__(self): return charp_to_python(self.__options.led_rgb_sequence)
        def __set__(self, value):
            self.__py_encoded_led_rgb_sequence = python_to_bytes(value, "led_rgb_sequence")
            self.__options.led_rgb_sequence = bytes_to_charp(self.__py_encoded_led_rgb_sequence)

    property pixel_mapper_config:
        def __get__(self): return charp_to_python(self.__options.pixel_mapper_config)
        def __set__(self, value):
            self.__py_encoded_pixel_mapper_config = python_to_bytes(value, "pixel_mapper_config")
            self.__options.pixel_mapper_config = bytes_to_charp(self.__py_encoded_pixel_mapper_config)

    property panel_type:
        def __get__(self): return charp_to_python(self.__options.panel_type)
        def __set__(self, value):
            self.__py_encoded_panel_type = python_to_bytes(value, "panel_type")
            self.__options.panel_type = bytes_to_charp(self.__py_encoded_panel_type)

    property pwm_dither_bits:
        def __get__(self): return self.__options.pwm_dither_bits
        def __set__(self, uint8_t value): self.__options.pwm_dither_bits = value

    property limit_refresh_rate_hz:
        def __get__(self): return self.__options.limit_refresh_rate_hz
        def __set__(self, value): self.__options.limit_refresh_rate_hz = value


    # RuntimeOptions properties

    property gpio_slowdown:
        def __get__(self): return self.__runtime_options.gpio_slowdown
        def __set__(self, uint8_t value): self.__runtime_options.gpio_slowdown = value

    property daemon:
        def __get__(self): return self.__runtime_options.daemon
        def __set__(self, uint8_t value): self.__runtime_options.daemon = value

    property drop_privileges:
        def __get__(self): return self.__runtime_options.drop_privileges
        def __set__(self, uint8_t value): self.__runtime_options.drop_privileges = value

    @property
    def do_gpio_init(self):
        return self.__runtime_options.do_gpio_init
    @do_gpio_init.setter
    def do_gpio_init(self, uint8_t value):
        self.__runtime_options.do_gpio_init = value

cdef class RGBMatrix(Canvas):
    def __cinit__(self, rows = None, chains = None, parallel = None,
                  RGBMatrixOptions options = None):

        # If RGBMatrixOptions not provided, create defaults and set any optional
        # parameters supplied
        if options is None:
            options = RGBMatrixOptions()

        if rows is not None:
            options.rows = rows
        if chains is not None:
            options.chain_length = chains
        if parallel is not None:
            options.parallel = parallel

        self.__matrix = cppinc.CreateMatrixFromOptions(options.__options,
                                                       options.__runtime_options)

        if self.__matrix == NULL:
            raise Exception("RGBMatrix creation failed")

    def __init__(self, *args, **kwargs):          # no call to Canvas.__init__
        pass

    def __dealloc__(self):
        if self.__matrix != NULL:
            self.__matrix.Clear()
            del self.__matrix

    cdef cppinc.Canvas* __getCanvas(self) except NULL:
        assert self.__matrix != NULL
        return self.__matrix

    cdef void __recycle_put(self, cppinc.FrameCanvas *canvas):
        self.__recycled_fc.push(canvas)

    cdef cppinc.FrameCanvas* __recycle_get(self):
        if self.__recycled_fc.empty():
            return NULL
        cdef cppinc.FrameCanvas* fc = self.__recycled_fc.top()
        self.__recycled_fc.pop()
        return fc

    def CreateFrameCanvas(self):
        cdef cppinc.FrameCanvas* newCanvas = self.__recycle_get()
        if newCanvas == NULL:
            newCanvas = self.__matrix.CreateFrameCanvas()
        if newCanvas == NULL:
            raise Exception("C++ CreateFrameCanvas failed")
        return FrameCanvas.__create(self, newCanvas)

    # The optional "framerate_fraction" parameter allows to choose which
    # multiple of the global frame-count to use. So it slows down your animation
    # to an exact integer fraction of the refresh rate.
    # Default is 1, so immediately next available frame.
    # (Say you have 140Hz refresh rate, then a value of 5 would give you an
    # 28Hz animation, nicely locked to the refresh-rate).
    # If you combine this with RGBMatrixOptions.limit_refresh_rate_hz you can create
    # time-correct animations.
    # You can pass None as newFrame to get synchronization without swapping frame
    def SwapOnVSync(self, FrameCanvas nextFrame, uint8_t framerate_fraction = 1):
        cdef cppinc.FrameCanvas* next = nextFrame.__canvas if nextFrame is not None else NULL
        cdef cppinc.FrameCanvas* prev = self.__matrix.SwapOnVSync(next, framerate_fraction)
        if prev == NULL:
            raise Exception("C++ SwapOnVSync failed")
        if self.frame_displayed is None:           ## first call, create from internal frame
            self.frame_displayed = FrameCanvas.__create(self, prev)

        assert self.frame_displayed.__canvas == prev
        cdef FrameCanvas prevFrame = self.frame_displayed
        if nextFrame is not None:
            self.frame_displayed = nextFrame
        return prevFrame

    @property
    def luminanceCorrect(self):
        return self.__matrix.luminance_correct()
    @luminanceCorrect.setter
    def luminanceCorrect(self, luminanceCorrect):
        self.__matrix.set_luminance_correct(luminanceCorrect)

    @property
    def pwmBits(self):
        return self.__matrix.pwmbits()
    @pwmBits.setter
    def pwmBits(self, pwmBits):
        self.__matrix.SetPWMBits(pwmBits)

    @property
    def brightness(self):
        return self.__matrix.brightness()
    @brightness.setter
    def brightness(self, brightness):
        self.__matrix.SetBrightness(brightness)



# Local Variables:
# mode: python
# End:
