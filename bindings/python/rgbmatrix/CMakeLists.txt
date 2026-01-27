cmake_minimum_required(VERSION 3.22)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CMakePrintHelpers) # so we can print cmake variable values
find_package(Python 3.13 COMPONENTS Interpreter Development.Module Development.Embed REQUIRED)

# Bring in Cython if required
#if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/core.cpp
#    OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/graphics.cpp)
#    message("Need to do a transpile")
    find_package(Cython MODULE REQUIRED VERSION 3.0)
    include(UseCython)
#endif ()

# Transpile core if necessary
#IF (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/core.cpp)
#    message("Should transpile core now.")
    add_custom_command(
        OUTPUT core.cpp
        COMMENT
            "Making ${CMAKE_CURRENT_BINARY_DIR}/core.cpp from ${CMAKE_CURRENT_SOURCE_DIR}/core.pyx"
        COMMAND Python::Interpreter -m cython --cplus
                  "${CMAKE_CURRENT_SOURCE_DIR}/core.pyx" --output-file core.cpp
        DEPENDS core.pyx
        VERBATIM)
#endif ()
# Transpile graphics if necessary
#IF (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/graphics.cpp)
#    message("Should transpile graphics now.")
    add_custom_command(
      OUTPUT graphics.cpp
      COMMENT
        "Making ${CMAKE_CURRENT_BINARY_DIR}/example.c from ${CMAKE_CURRENT_SOURCE_DIR}/graphics.pyx"
      COMMAND Python::Interpreter -m cython --cplus
              "${CMAKE_CURRENT_SOURCE_DIR}/graphics.pyx" --output-file graphics.cpp
      DEPENDS graphics.pyx
      VERBATIM)
#endif ()

python_add_library(core STATIC core.cpp)
set_target_properties(core PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(core
    PRIVATE ../../../include ./shims)
python_add_library(graphics STATIC graphics.cpp)
set_target_properties(graphics PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(graphics PRIVATE ../../../include)
python_add_library(pyrgbmatrix MODULE core graphics WITH_SOABI)

# install(TARGETS pyrgbmatrix DESTINATION ${SKBUILD_PLATLIB_DIR})
# install(TARGETS core graphics DESTINATION . }

# Build core
# Build graphics


# Add core as a module.
#python_add_library(core MODULE WITH_SOABI core.cpp)
# Add graphics as a module
#python_add_library(graphics MODULE WITH_SOABI graphics.cpp)

# install(TARGETS rgbmatrix DESTINATION .)

#file(GLOB rgbmatrix_sources
#    rgbmatrix/*.pyx)

#set(librgbmatrix
#    ../../lib/librgbmatrix.so.1
#)


# Use Cython to compile create the C++ files for core and graphics.
# This custom_command comes from here: https://scikit-build-core.readthedocs.io/en/latest/guide/migration_guide.html
#add_custom_command(
#  OUTPUT core.cpp
#  COMMENT
#    "Making ${CMAKE_CURRENT_BINARY_DIR}/core.cpp from ${CMAKE_CURRENT_SOURCE_DIR}/core.pyx"
#  COMMAND Python::Interpreter -m cython
#          "${CMAKE_CURRENT_SOURCE_DIR}/core.pyx" --output-file core.cpp
#  DEPENDS core.pyx
#  VERBATIM)

#add_custom_command(
#  OUTPUT graphics.cpp
#  COMMENT
#    "Making ${CMAKE_CURRENT_BINARY_DIR}/graphics.cpp from ${CMAKE_CURRENT_SOURCE_DIR}/graphics.pyx"
#  COMMAND Python::Interpreter -m cython
#          "${CMAKE_CURRENT_SOURCE_DIR}/graphics.pyx" --output-file graphics.cpp
#  DEPENDS graphics.pyx
#  VERBATIM)

# add_library(core STATIC ${core})
#python_add_library(core MODULE WITH_SOABI core.cpp)
#python_add_library(graphics MODULE WITH_SOABI graphics.cpp)

#target_link_libraries(rgbmatrix ${linked_module_list})

#add_python_extension(rgbmatrix
#    SOURCES ${rgbmatrix_sources} )
#    # INCLUDE_DIRECTORIES rgbmatrix  )
