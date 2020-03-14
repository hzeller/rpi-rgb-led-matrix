PCB adapter for Raspberry Pi to Hub75 RGB Matrixes
==================================================

Since hand-wiring can be a little tedious, here are some PCBs that help
with the wiring when using the `rpi-rgb-led-matrix` code.

   * [Passive-3](./passive-3) Supports three parallel chains, directly connected
     to the output of a Rapsberry Pi with 40 GPIO pins. Works, but usually it is better to
     buffer the outputs using the ...
   * [Active-3](./active-3) board. Supports three parallel chains with active buffering
     and 3.3V -> 5V level shifting for best reliability. Requires SMD soldering.
     
     As another option you can buy it from these locations not affiliated with this project.
     They are given to help you locate premade boards but no guarantees are given or implied:
     * https://www.electrodragon.com/product/rgb-matrix-panel-drive-board-raspberry-pi/
       ($3/board, but fairly long and/or expensive shipping from HKG)
     * Seller #2 (fill me)
   * The [Passive-RPi1](./passive-rpi1) adapter board is to connect one panel to
     Raspberry Pi 1 with 26 GPIO pins.

![Three Panels connected][three-panels]

[three-panels]: ../img/three-parallel-panels-soic.jpg
