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
export PYTHON=$(which python3)
make build-python
sudo make install-python
```

Using the library
-----------------

Be aware of the fact, that using the RGBMatrix requires root privileges.
Therefore you will need to run all you python scripts as using sudo.

```python
#!/usr/bin/env python
from rgbmatrix import RGBMatrix
import time

rows = 16
chains = 1
parallel = 2
myMatrix = RGBMatrix(rows, chains, parallel)
myMatrix.Fill(255, 0, 0)
time.sleep(5)
myMatrix.Clear()
```
