# CMake for rpi-rgb-led-matrix
# Derived from https://github.com/ofloveandhate/scikit-build_boost-python/
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(rgbmatrix)
include(CMakePrintHelpers) # so we can print cmake variable values
include(GNUInstallDirs)  # so that `CMAKE_INSTALL_LIBDIR` is not blank. eg
find_package(Python 3.13 REQUIRED COMPONENTS Interpreter)
# include(UseCython)

# Build the rgbmatrix library and the demo.
add_subdirectory(lib) #add_library(librgbmatrix.so SHARED lib)
# Build the Python Bindings
add_subdirectory(bindings/python)
#python_add_library(rgbmatrix MODULE
#    bindings/python/rgbmatrix/core.py
#    WITH_SOABI )
