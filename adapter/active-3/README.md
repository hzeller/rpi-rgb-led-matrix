Adapter PCB to support up to 3 panels
======================================

   * Supports up to three panel chains for newer plus models and
     Raspberry Pi 2 that have 40 GPIO pins.
   * Uses HCT245 to level shift signals from 3.3V to 5V and shield
     the Raspberry Pi GPIOs from overloading.
   * Open source KiCAD PCB EDA format.
   * Optional: Pads to power the Pi with 5V, including optional capacitor footprints (use something like 22uF-100uF; might be useful to smooth noisy power supply in particular if you operate the panels from the same supply. I use a 22uF/6.3V ceramic capacitor).
   * Connector for RxD input (literally the only GPIO pin left) in case you want to
     make your panel controlled with a serial interface (3.3V logic level).
   * (not very pretty layout, was just lazy and let the auto-router generate the first pass)
   * BOM:
     - 4x 74HCT245 or 74AHCT245 in 20-SOIC, 7.5mm package which should make
          it easy to hand-solder.
     - 4x 100nF ceramic capacitor (0805 package)
     - 1x 10kOhm resistor (0805 package)
     - 3x 16pin IDC (=2x8) male receptible to connect the panels.
     - 1x 40pin female connector to connect to the RPi.
   * The Gerber FAB files are provided as [active3-rpi-hub75-adapter-fab.zip](./active3-rpi-hub75-adapter-fab.zip)

The board is also [shared on OSH Park][osh-active3] (not affiliated).

![Preview][rendering]
![Real World][real-world]

[rendering]: ../../img/active3-pcb.png
[real-world]: ../../img/three-parallel-panels-soic.jpg
[osh-active3]: https://oshpark.com/shared_projects/QIA9YHZH
