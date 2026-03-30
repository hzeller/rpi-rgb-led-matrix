PCB adapter for Raspberry Pi to Hub75 RGB Matrixes
==================================================

Since hand-wiring can be a little tedious, here are some PCBs that help
with the wiring when using the `rpi-rgb-led-matrix` code.

![Three Panels connected][three-panels]

[three-panels]: ../img/three-parallel-panels-soic.jpg

   * [Passive-3](./passive-3) Supports three parallel chains, directly connected
     to the output of a Raspberry Pi with 40 GPIO pins. Works, but usually it is better to
     buffer the outputs using the ...
   * [Active-3](./active-3) board. Supports three parallel chains with active buffering
     and 3.3V -> 5V level shifting for best reliability. Requires SMD soldering.
     
     As another option you can buy it from these locations not affiliated with this project.
     They are given to help you locate premade boards but no guarantees are given or implied:
     * https://www.electrodragon.com/product/rgb-matrix-panel-drive-board-raspberry-pi/
       ($3/board, but fairly long and/or expensive shipping from HKG). The old board includes 
       support for an optional RTC (real time clock) which had to be disabled for most users 
       who wanted 3 channels instead of an RTC.
     * The new HD board with angled connectors (thinner footprint), is here: https://www.electrodragon.com/product/rgb-matrix-panel-drive-board-for-raspberry-pi-v2/ << **this is the recommended board for most users today**
![399914418-713e2414-04f9-4fc3-86a2-cb576e29e057](https://github.com/user-attachments/assets/2bf3b1b9-aa24-4a0c-a587-842b92fdc5a0)
    
     * If you have an rpi0 2wl, this passive board was made specifically to be as small as possible: https://www.electrodragon.com/product/rpi-passive-3ch-rgb-matrix-drive-board-v0/
       <img width="1000" height="606" alt="image" src="https://github.com/user-attachments/assets/f1d84ac5-a713-49b1-920d-603061b10bdd" />
       <img width="1000" height="670" alt="image" src="https://github.com/user-attachments/assets/61e17396-6948-4510-a6a2-fb0e6da8fb0a" />



   * Another ready-to-use active-3 adapter is available also from Acme Systems ( https://www.acmesystems.it/HAT-A3 )

   * The [Passive-RPi1](./passive-rpi1) adapter board is to connect one panel to
     Raspberry Pi 1 with 26 GPIO pins.

   * For completeness, Adafruit has a single channel active board here: 
     https://www.adafruit.com/product/3211 although it is ultimately inferior to the 
     electrodragon board, but it does ship quicker if you're in the US (note that you will
     need special compile option or command line argument since it uses non standard wiring)

   * As of 2024/12 and this thread: https://rpi-rgb-led-matrix.discourse.group/t/solved-aliexpress-2-string-passive-adapter-not-working-because-rpi-connector-was-soldered-on-the-wrong-side-of-the-board/892/26 , there is also a cheap passive 2 channel board that works great on a small board like rPi0 2W, but all chinese sellers that Marc Merlin surveyed at the time, sold the board soldered wrong and they do not work unless you order the board unsoldered and solder it yourself. Electrodragon is in the process of finishing a low profile 3 channel passive board that will be a proper replacement for the unnamed mis-soldered chinese board.
     
![8a584b59a1da068112905d3a37923521a756353e_2_664x500](https://github.com/user-attachments/assets/38b0163f-bf62-4393-a084-dab432c3ef49)

