Python bindings for RGB Matrix library
======================================

Building
--------

In the root directory for the matrix library simply type:

### Python 2

```shell
sudo apt-get update && sudo apt-get install python2.7-dev python-pillow -y
make build-python
sudo make install-python
```

### Python 3
You can also build for Python 3:

```shell
sudo apt-get update && sudo apt-get install python3-dev python3-pillow -y
make build-python PYTHON=$(which python3)
sudo make install-python PYTHON=$(which python3)
```

### PyPy
The cython binding to PyPy seems to be somewhat working but extremely slow (20x
slower even than the regular Python binding, 160x slower than C++), so this is
not recommended. From glancing at the docs, the library might need to
be bound via **[CFFI](https://cffi.readthedocs.io/)** first. If anyone has
experience doing that, please consider preparing a pull request (ideally in
a way that it can be used as well for Python2/3 and does not change the
interface of the current Python binding).

Performance
-----------
The simplicity of scripting comes at a price: Python is slower than C++ of course.
If you have to do a lot of pixel updates in your demo, this can be too slow
depending on what you do. Here are some rough numbers for calling `SetPixel()`
in a tight loop:

  * On a Pi-2 and Pi-3, a Python script will be about 1/8 of the speed compared
    to the corresponding C++ program (pushing ~0.43 Megapixels/s Python
    vs. ~3.5 Megapixels/s C++ on a Pi-3 for instance)
  * On a Pi-1, the difference is even worse: 1/24 of the speed to the
    corresponding C++ program. Given that the Pi-1 is already about 1/10 the
    speed of a Pi-3, this almost makes Python unusable on a Pi-1
    (~0.015 Megapixels/s Python vs. ~0.36 Megapixels/s C++)
  * Also interesting: Python3 is a little bit slower than Python2.7.
    So if you can, stick with Python2.7 for now.
  * The good news is, that this is due to overhead per function call. If you
    can do more per function call, then this is less problematic. For instance
    if you have an image to be displayed with `SetImage()`, that will be faster
    per pixel.
    (Currently, `SetImage()` is still much slower than it could be, because
    it does not send the raw buffer into the C-side but does individual
    SetPixel() calls in the interface layer. If you are interested in improving
    the performance of Python, this would be a good start and welcomed
    pull request).

The ~0.015 Megapixels/s on a Pi-1 means that you can update a 32x32 matrix
at most with ~15Hz. If you have chained 5, then you barely reach 3Hz.
In a Pi-3, you get about 400Hz update rate (85Hz for 5-chain) with a Python
program (while with C++, you can do the same thing with a comfortable 3500Hz
(700Hz for 5)). Keep in mind that this is if all you do is just calling
`SetPixel()`, it does not include any time of what you actually want to do in
your demo - so anything in addition to that will drop your update rate.

Using the library
-----------------

Be aware of the fact, that using the RGBMatrix requires root privileges.
Therefore you will need to run all you python scripts as using sudo.

You find examples in the [samples/](./samples) subdirectory.
The examples all use the [samplebase.py](./samples/samplebase.py) that provides
some utility to all example programs, such as command-line parsing: all
sample-programs accept `--rows`, `--chain` and `--parallel` as options to
adapt to your configuration

```bash
cd samples
sudo ./runtext.py --chain=4
```

Here a complete example how to write an image viewer:
```python
#!/usr/bin/env python
import time
import sys

from rgbmatrix import RGBMatrix
from PIL import Image

if len(sys.argv) < 2:
    sys.exit("Require an image argument")
else:
    image_file = sys.argv[1]

image = Image.open(image_file)

rows = 32
chain = 1
parallel = 1
matrix = RGBMatrix(rows, chain, parallel)

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
