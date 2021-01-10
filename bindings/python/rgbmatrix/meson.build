py3_mod = import('python')
py3_dep = dependency('python3')
py3_inst = py3_mod.find_installation('python3')

cython = find_program('cython3')


core_pyx_cpp = custom_target('core_pyx',
  output : 'core.cpp',
  input : 'core.pyx',
  command : [cython, '@INPUT@', '--cplus', '-o', '@OUTPUT@'],
)

rgbmatrix_pylib = py3_inst.extension_module('core',
  core_pyx_cpp,
  dependencies: [py3_dep, librgbmatrix_dep],
  install: true,
  subdir: py3_inst.get_path('purelib') + '/rgbmatrix',
)

graphics_pyx_cpp = custom_target('graphics_pyx',
  output : 'graphics.cpp',
  input : 'graphics.pyx',
  command : [cython, '@INPUT@', '--cplus', '-o', '@OUTPUT@'],
)

rgbmatrix_pylib = py3_inst.extension_module('graphics',
  graphics_pyx_cpp,
  dependencies: [py3_dep, librgbmatrix_dep],
  install: true,
  subdir: py3_inst.get_path('purelib') + '/rgbmatrix',
)

