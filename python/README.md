Python bindings for RGB Matrix library
======================================

Building
--------

In the root directory for the matrix library simply type

      $ make build-python
      $ make install-python

Using the library
-----------------

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
