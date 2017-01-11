cimport cppinc

cdef class Canvas:
    cdef cppinc.Canvas *__getCanvas(self) except +

cdef class FrameCanvas(Canvas):
    cdef cppinc.FrameCanvas *__canvas

cdef class RGBMatrix(Canvas):
    cdef cppinc.RGBMatrix *__matrix

cdef class RGBMatrixOptions:
    cdef cppinc.Options __options
    cdef cppinc.RuntimeOptions __runtime_options
    # Must keep a reference to the encoded bytes for hardware_mapping string
    # otherwise, when the Options struct is used, it will be garbage collected
    cdef bytes __py_encoded_hardware_mapping


# Local Variables:
# mode: python
# End:
