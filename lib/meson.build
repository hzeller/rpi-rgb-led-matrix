lib_sources = [
  'gpio.cc',
  'led-matrix.cc',
  'options-initialize.cc',
  'framebuffer.cc',
  'thread.cc',
  'bdf-font.cc',
  'graphics.cc',
  'led-matrix-c.cc',
  'hardware-mapping.c',
  'pixel-mapper.cc',
  'multiplex-mappers.cc',
  'content-streamer.cc'
]

thread_dep = dependency('threads')

librgbmatrix = library('rgbmatrix',
  lib_sources,
  include_directories: incdir,
  dependencies: thread_dep,
  cpp_args: [
    '-fno-exceptions',
    ],
  install: true,
  soversion: '1',
  version: '1.2.3'
)

librgbmatrix_dep = declare_dependency(
  include_directories: incdir,
  link_with: librgbmatrix
)
