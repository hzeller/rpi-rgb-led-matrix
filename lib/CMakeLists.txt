# this is the cmake file for the rgbmatrix library

cmake_minimum_required(VERSION 3.22)
project(
    rgbmatrix
    VERSION "0.0.0"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CMakePrintHelpers) # so we can print cmake variable values

# In order to find conda, run 'conda activate' and then use 'cmake .. -DCMAKE_PREFIX_PATH=$CONDA_PREFIX' when cmaking

# All source files to be compiled
# We can either explicitly list all files or use glob,
# we chose to explicitly list files and not to glob
#file(GLOB SOURCES src/*.cpp)

cmake_print_variables(CMAKE_CURRENT_SOURCE_DIR)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

include_directories("../include")

#############
#
# Specify how to build the library
#
#####################

set(RGBMATRIX_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/canvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/content-streamer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/graphics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/led-matrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/led-matrix-c.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/pixel-mapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/thread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/threaded-canvas-manipulator.h
)
cmake_print_variables(RGBMATRIX_HEADERS)

set(RGBMATRIX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bdf-font.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/content-streamer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/gpio.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/hardware-mapping.c
    ${CMAKE_CURRENT_SOURCE_DIR}/led-matrix.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/led-matrix-c.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/multiplex-mappers.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/options-initialize.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel-mapper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.cc
)
cmake_print_variables(RGBMATRIX_SOURCES)

add_library(rgbmatrix SHARED ${RGBMATRIX_SOURCES})

# see https://stackoverflow.com/questions/25676277/cmake-target-include-directories-prints-an-error-when-i-try-to-add-the-source
# for more on these generator expressions, $<INSTALL_INTERFACE, etc.

target_include_directories(rgbmatrix PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
    )

target_include_directories(rgbmatrix PUBLIC $<INSTALL_INTERFACE:include>)

install(
    TARGETS rgbmatrix
    EXPORT rgbmatrix_targets  # this name should match the name below where we generate the cmake files for finding the package
    # ARCHIVE DESTINATION "lib"
    # LIBRARY DESTINATION "lib"
    # COMPONENT library
    FILE_SET HEADERS
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(
    FILES ${RGBMATRIX_CORE_HEADERS}
    DESTINATION "../include"
)

############
#
#  cmake install files, so other packages can find the core
#
####

# see https://github.com/scikit-build/scikit-build-sample-projects/blob/main/projects/hello-cmake-package/CMakeLists.txt
# for my sources for doing this part

# also, see https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html.
# and https://iamsorush.com/posts/cpp-cmake-config/

# this cmake include gives us access to CMAKE_INSTALL_INCLUDEDIR
# include(GNUInstallDirs)  # https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html

#set(EXAMPLE_CMAKE_PACKAGE_INSTALL_SUBDIR "share/example/cmake")
#set(SYSCONFIG_INSTALL_DIR ${CMAKE_INSTALL_SYSCONFDIR}
#    CACHE PATH "Location of configuration files" )

#install(
#  EXPORT exampleTargets  # this name before Targets should match the name of the variable set above in a previous install call
#  NAMESPACE example::
#  DESTINATION ${EXAMPLE_CMAKE_PACKAGE_INSTALL_SUBDIR})

#include(CMakePackageConfigHelpers)

#write_basic_package_version_file(
#  exampleConfigVersion.cmake
#  VERSION ${PROJECT_VERSION}
#  COMPATIBILITY SameMajorVersion)

#set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})  #set so we can use in configure_package_config_file()
#set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}) # ditto

#configure_package_config_file(
#  "${PROJECT_SOURCE_DIR}/../cmake/exampleConfig.cmake.in" exampleConfig.cmake
#  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR LIB_INSTALL_DIR
#  INSTALL_DESTINATION ${EXAMPLE_CMAKE_PACKAGE_INSTALL_SUBDIR}
#  )

#install(FILES "${PROJECT_BINARY_DIR}/exampleConfig.cmake"
#              "${PROJECT_BINARY_DIR}/exampleConfigVersion.cmake"
#        DESTINATION ${EXAMPLE_CMAKE_PACKAGE_INSTALL_SUBDIR})

##############################
#
# now stuff for the executable
#
##############################

#set(EXAMPLE_EXECUTABLE_SOURCES
#    src/executable.cpp
#)

#set(EXAMPLE_EXECUTABLE_HEADERS
#    include/example/executable.hpp
#)


#if(PROJECT_IS_TOP_LEVEL)
#    add_executable(example_core_exe)
#endif()

#target_sources(example_core_exe PRIVATE ${EXAMPLE_EXECUTABLE_SOURCES} ${EXAMPLE_EXECUTABLE_HEADERS})

#target_link_libraries(example_core_exe example_core_lib)

#install(
#    TARGETS example_core_exe
#    ARCHIVE DESTINATION "bin"
#    COMPONENT executable
#)

###########################################
#
# set up some things for unit testing, which uses Boost.Unittest
#
##################################

# TODO make this conditional on wanting them actually built.
# see https://github.com/bertiniteam/b2/issues/202

#set(EXAMPLE_CORE_TEST_SOURCES
#    test/test_core.cpp
#)
#
#add_executable(test_core ${EXAMPLE_CORE_TEST_SOURCES})
#target_include_directories(test_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/test)

#target_link_libraries(test_core ${Boost_LIBRARIES} example_core_lib)
#add_test(NAME test_core COMMAND ${CMAKE_BINARY_DIR}/test_core)

#enable_testing()
