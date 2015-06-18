PCB adapter for Raspberry Pi to Hub75 RGB Matrixes
==================================================

Since hand-wiring can be a little tedious, here are some PCBs that help
with the wiring when using the `rpi-rgb-led-matrix` code.

   * [Passive-3](./active-3) Supports three parallel chains, directly connected
     to the output of the Rapsberry Pi. Works, but usually it is better to buffer
     the outputs using the ...
   * [Active-3](./active-3) Like the active board, but supports three parallel
     chains with level shifting.
   * [Passive-RPi1](./passive-rpi1) adapter board to connect one panel to
     Raspberry Pi 1

![Three Panels connected][three-panels]

[three-panels]: ../img/three-parallel-panels.jpg
