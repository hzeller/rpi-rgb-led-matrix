cimport cppinc

cdef class FrameCanvas:
    cdef cppinc.FrameCanvas *__canvas
    cdef cppinc.FrameCanvas* __getCanvas(self) except *

cdef class RGBMatrix:
    cdef cppinc.RGBMatrix *__matrix
    cdef cppinc.GPIO *__gpio