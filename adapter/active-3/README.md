Adapter PCB to support up to 3 panels
======================================

   * Supports one panel for Raspberry PIs with 26 GPIO pins up to three
     panel chains for newer plus models and Raspberry Pi 2 that have 40 GPIO pins.
   * Uses HCT245 to level shift signals from 3.3V to 5V and shield
     the Raspberry Pi GPIOs from overloading.
   * Open source KiCAD PCB EDA format.
   * (not very pretty layout, was just lazy and let the auto-router do it).

For multiple parallel boards to work, you have to uncomment

     #DEFINES+=-DSUPPORT_MULTI_PARALLEL   # remove the '#' in the begging

in [lib/Makefile](../../lib/Makefile).

You can [buy the board from OSH Park][osh-active3].

![Preview][rendering]

[rendering]: ../../img/active3-pcb.png
[osh-active3]: https://oshpark.com/shared_projects/D2hXjnYT
