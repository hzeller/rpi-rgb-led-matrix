# rgbmatrix Python bindings
cmake_minimum_required(VERSION 3.22)
project(${SKBUILD_PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CMakePrintHelpers) # so we can print cmake variable values
find_package(Python 3.13 REQUIRED COMPONENTS Interpreter Development.Module REQUIRED)
add_subdirectory(rgbmatrix)
