Installation:

1. compile main rpi-rgb-led-matrix library 
2. node js bindings expect a local bin folder containing the binaries
   at the moment this is still a manual process so you need
   to create the folder ./bindings/nodejs/bin
   and copy the required files into it.
    a. librgbledmatrix.so      - from core lib
    b. rpi-rgb-led-matrix.dll  - from c# bindings

see ./example for usage