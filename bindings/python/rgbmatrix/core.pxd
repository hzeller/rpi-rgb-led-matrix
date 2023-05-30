cimport cppinc
from libcpp.stack cimport stack

cdef class Canvas:
    cdef cppinc.Canvas* __getCanvas(self) except NULL

cdef class FrameCanvas(Canvas):
    cdef RGBMatrix __owner
    cdef cppinc.FrameCanvas *__canvas
    cdef cppinc.Canvas* __getCanvas(self) except NULL
    @staticmethod
    cdef FrameCanvas __create(RGBMatrix owner, cppinc.FrameCanvas *canvas)

cdef class RGBMatrix(Canvas):
    cdef cppinc.RGBMatrix *__matrix
    cdef stack[cppinc.FrameCanvas *] __recycled_fc         # created FrameCanvases that can be recycled
    cdef readonly FrameCanvas frame_displayed              # FrameCanvas currently showing

    cdef cppinc.Canvas* __getCanvas(self) except NULL
    cdef void __recycle_put(self, cppinc.FrameCanvas *canvas)
    cdef cppinc.FrameCanvas* __recycle_get(self)

cdef class RGBMatrixOptions:
    cdef cppinc.Options __options                           # cython will handle object lifetime
    cdef cppinc.RuntimeOptions __runtime_options
    # Must keep a reference to the encoded bytes for the strings,
    # otherwise, when the Options struct is used, it will be garbage collected
    cdef bytes __py_encoded_hardware_mapping
    cdef bytes __py_encoded_led_rgb_sequence
    cdef bytes __py_encoded_pixel_mapper_config
    cdef bytes __py_encoded_panel_type

# Local Variables:
# mode: python
# End:
