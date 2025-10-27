#!/usr/bin/env python

# 2025-10, updated to import from setuptools rather than distutils.core which was removed in Python 3.12.
from setuptools import setup, Extension

core_ext = Extension(
    name                = 'core',
    sources             = ['rgbmatrix/core.cpp', 'rgbmatrix/shims/pillow.c'],
    include_dirs        = ['../../include', 'rgbmatrix/shims'],
    library_dirs        = ['../../lib'],
    libraries           = ['rgbmatrix'],
    extra_compile_args  = ["-O3", "-Wall"],
    language            = 'c++'
)

graphics_ext = Extension(
    name                = 'graphics',
    sources             = ['rgbmatrix/graphics.cpp'],
    include_dirs        = ['../../include'],
    library_dirs        = ['../../lib'],
    libraries           = ['rgbmatrix'],
    extra_compile_args  = ["-O3", "-Wall"],
    language            = 'c++'
)

setup(
    name                = 'rgbmatrix',
    version             = '0.0.1',
    author              = 'Christoph Friedrich',
    author_email        = 'christoph.friedrich@vonaffenfels.de',
    classifiers         = ['Development Status :: 3 - Alpha'],
    ext_package         = 'rgbmatrix',
    ext_modules         = [core_ext, graphics_ext],
    packages            = ['rgbmatrix']
)
