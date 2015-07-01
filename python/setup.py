#!/usr/bin/python
from distutils.core import setup, Extension
 
matrix_ext = Extension(
    name                = 'rgbmatrix',
    sources             = ['rgbmatrix.cpp'],
    include_dirs        = ['../include'],
    library_dirs        = ['../lib'],
    libraries           = ['rgbmatrix'],
    language            = 'c++'
)

setup(
    name                = 'RGBMatrix Python Library',
    version             = '0.0.1',
    author              = 'Christoph Friedrich',
    author_email        = 'christoph.friedrich@vonaffenfels.de',
    classifiers         = ['Development Status :: 3 - Alpha'],
    ext_modules         = [
        matrix_ext
    ],
)