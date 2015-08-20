Adapter PCB to support up to 3 panels
======================================

   * Supports up to three panel chains for newer plus models and
     Raspberry Pi 2 that have 40 GPIO pins.
   * Uses HCT245 to level shift signals from 3.3V to 5V and shield
     the Raspberry Pi GPIOs from overloading.
   * Open source KiCAD PCB EDA format.
   * (not very pretty layout, was just lazy and let the auto-router do it).
   * BOM:
     - 4x HCT245 in 20-SOIC (7.5mm package)
     - 4x 100nF ceramic capacitor (0805 package)
     - 1x 10kOhm resistor (0805 package)
     - 3x 16pin IDC (=2x8) male receptible to connect the panels.
     - 1x 40pin female connector to connect to the RPi.
   * The FAB files are provided as [active-3-fab.zip](./active-3-fab.zip)

The board is also [shared on OSH Park][osh-active3].

![Preview][rendering]
![Real World][real-world]

[rendering]: ../../img/active3-pcb.png
[real-world]: ../../img/three-parallel-panels-soic.jpg
[osh-active3]: https://oshpark.com/shared_projects/vIwlNEFd
