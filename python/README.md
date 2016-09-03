Python bindings for RGB Matrix library
======================================

Building
--------

In the root directory for the matrix library simply type:

```shell
sudo apt-get update && sudo apt-get install python2.7-dev python-pillow -y
make build-python
sudo make install-python
```

You can also build for Python 3:

```shell
sudo apt-get update && sudo apt-get install python3-dev python3-pillow -y
make build-python PYTHON=$(which python3)
sudo make install-python PYTHON=$(which python3)
```

Speed
-----
The simplicity of scripting comes at a price: Python is slower than C++ of course.
If you have to do a lot of pixel updates in your demo, this can be too slow
depending on what you do. Here are some rough numbers:

  * On a Pi-2 and Pi-3, a Python script will be about 1/5 of the speed compared
    to the corresponding C++ program (pushing 0.5 Megapixels/s Python
    vs. 2.5 Megapixels/s C++ on a Pi-3 for instance)
  * On a Pi-1, the difference is even worse: 1/17 of the speed to the
    corresponding C++ program. Given that the Pi-1 is already about 1/10 the
    speed of a Pi-3, this almost makes Python unusable on a Pi-1
    (0.017 Megapixels/s Python vs. 0.3 Megapixels/s C++)
  * Also interesting: Python3 is about 40% slower than Python2.7 (measured on
    a Pi-3). So if you can, stick with Python2.7 for now.

The 0.017 Megapixels/s on a Pi-1 means that you can update a 32x32 matrix
at most with 17Hz. If you have chained 5, then you barely reach 3.5Hz.
In a Pi-3, you get about 450Hz update rate (90Hz for 5-chain) with a Python
program (while with C++, you can do the same thing with a comfortable 2600Hz
(520Hz for 5)). Keep in mind that this is just the time calling `SetPixel()`, it
does not include any time yet of what you actually want to do in your demo - so
anything in addition to that will drop your update rate.

Using the library
-----------------

Be aware of the fact, that using the RGBMatrix requires root privileges.
Therefore you will need to run all you python scripts as using sudo.

You find examples in the [samples/](./samples) subdirectory.

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
