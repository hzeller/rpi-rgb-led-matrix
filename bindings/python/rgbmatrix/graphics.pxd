cimport cppinc

cdef class Color:
    cdef cppinc.Color __color

cdef class Font:
    cdef cppinc.Font __font

# Local Variables:
# mode: python
# End:
