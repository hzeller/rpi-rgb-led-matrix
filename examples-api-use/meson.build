example_apps = ['minimal-example', 'input-example', 'text-example', 'scrolling-text-example', 'clock', 'ledcat', 'pixel-mover', 'demo-main']

foreach e : example_apps
  # Since these executables will be installed in /usr/bin, prefix them
  executable('rgbmatrix-' + e, e + '.cc',
    dependencies: librgbmatrix_dep,
    install: true)
endforeach
