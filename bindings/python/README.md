Python bindings for RGB Matrix library
======================================

Building
--------

If you have a different than the standard wiring (for instance if you have an
Adafruit HAT), you can edit the [../../lib/Makefile](../../lib/Makefile#L26) first to choose
the hardware in question (see below for setting it via command line argument).

Then, in the root directory for the matrix library simply type:

### Python 3

```shell
sudo apt-get update && sudo apt-get install python3-dev cython3 -y
make build-python 
sudo make install-python 
```

### PyPy
The cython binding to PyPy seems to be somewhat working but extremely slow (20x
slower even than the regular Python binding, 160x slower than C++), so this is
not recommended.

So Cython is not good together with PyPy which works best with a
[CFFI](https://cffi.readthedocs.io/) binding. @Duality4Y did an experiment here
https://github.com/Duality4Y/rgb-matrix-cffi which works well with PyPy and is
about twice as fast as running Python3+cython (but Python3+cffi is slower than
Python3+cython, so we can't just replace everything with cffi).

Of course, it would be nice to have the fastest possible binding to all kinds
of Python interpreters. If anyone wants to work on that, this would certainly
be a welcome pull request.

Performance
-----------
The simplicity of scripting comes at a price: Python is slower than C++ of
course.
If you have to do a lot of pixel updates in your demo, this can be too slow
depending on what you do. Here are some rough numbers for calling `SetPixel()`
in a tight loop:

  * On a Pi-2 and Pi-3, a Python script will be about 1/8 of the speed compared
    to the corresponding C++ program (pushing ~0.43 Megapixels/s Python
    vs. ~3.5 Megapixels/s C++ on a Pi-3 for instance)
  * On a Pi-1/Pi Zero, the difference is even worse: 1/24 of the speed to the
    corresponding C++ program. Given that this Pi is already about 1/10 the
    speed of a Pi-3, this almost makes Python unusable on a Pi-1
    (~0.015 Megapixels/s Python vs. ~0.36 Megapixels/s C++)
  * Also interesting: Python3 is a little bit slower than Python2.7.
    So if you can, stick with Python2.7 for now.
  * The good news is, that this is due to overhead per function call. If you
    can do more per function call, then this is less problematic. For instance
    if you have an image to be displayed with `SetImage()`, that will much
    faster per pixel (internally this then copies the pixels natively).

The ~0.015 Megapixels/s on a Pi-1 means that you can update a 32x32 matrix
at most with ~15fps. If you have chained 5, then you barely reach 3fps.
In a Pi-3, you get about 400fps update rate (85fps for 5-chain) with a Python
program (while with C++, you can do the same thing with a comfortable 3500fps
(700fps for 5)). Keep in mind that this is if all you do is just calling
`SetPixel()`, it does not include any time of what you actually want to do in
your demo - so anything in addition to that will drop your update rate.

If you can prepare the animation you want to show, then you can either prepare
images and then use the much faster call to `SetImage()`, or can fill
entire offscreen-frames (create with `CreateFrameCanvas()`) and then
swap with `SwapOnVSync()` (this is the fastest method).

Using the library
-----------------

Be aware of the fact that using the full performance of the RGBMatrix requires root privileges.
Therefore you should run all you python scripts as using `sudo`.

You may find examples in the [samples/](./samples) subdirectory.
The examples all use the [samplebase.py](./samples/samplebase.py) that provides
some basic capabilities to all example programs, such as command-line parsing: all
sample-programs accept `--led-rows`, `--led-chain` and `--led-parallel` as
command line options to adapt to your configuration

```bash
cd samples
sudo ./runtext.py --led-chain=4
```

To use different wiring without recompiling the library to change the default,
you can use `--led-gpio-mapping` (or `-m`). For example, to use Adafruit HAT:
```bash
sudo ./runtext.py --led-gpio-mapping=adafruit-hat
```

Here is a complete example showing how to write an image viewer:
```python
#!/usr/bin/env python
import time
import sys

from rgbmatrix import RGBMatrix, RGBMatrixOptions
from PIL import Image

if len(sys.argv) < 2:
    sys.exit("Require an image argument")
else:
    image_file = sys.argv[1]

image = Image.open(image_file)

# Configuration for the matrix
options = RGBMatrixOptions()
options.rows = 32
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'regular'  # If you have an Adafruit HAT: 'adafruit-hat'

matrix = RGBMatrix(options = options)

# Make image fit our screen.
image.thumbnail((matrix.width, matrix.height), Image.ANTIALIAS)

matrix.SetImage(image.convert('RGB'))

try:
    print("Press CTRL-C to stop.")
    while True:
        time.sleep(100)
except KeyboardInterrupt:
    sys.exit(0)
```

## API

The source of truth for what is available in the Python bindings may be found [here](rgbmatrix/core.pyx) (RGBMatrix, FrameCanvas, RGBMatrixOptions) and [here](rgbmatrix/graphics.pyx) (graphics).  The underlying implementation's ground truth documentation may be found [here](../../include), specifically for [RGBMatrix, RGBMatrixOptions, and FrameCanvas](../../include/led-matrix.h), [Canvas](../../include/canvas.h) (base class of RGBMatrix), and [graphics methods and Font](../../include/graphics.h).

### User

As noted in the Performance section above, Python programs not run as `root` will not be as high-performance as those run as `root`.  When running as `root`, be aware of a potentially-unexpected behavior: to reduce the security attack surface, initializing an RGBMatrix as `root` changes the user from `root` to `daemon` (see [#1170](https://github.com/hzeller/rpi-rgb-led-matrix/issues/1170) for more information) by default.  This means, for instance, that some file operations possible before initializing the RGBMatrix will not be possible after initialization.  To disable this behavior, set `drop_privileges=False` in RGBMatrixOptions, but be aware that doing so will reduce security.
