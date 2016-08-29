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
