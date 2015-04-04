Adapter PCB to support up to 2 panels
======================================

   * Supports one panel for Raspberry PIs with 26 GPIO pins and two
     panels for newer plus models and Raspberry Pi 2 that have 40 GPIO pins.
   * Uses HCT245 to level shift signals from 3.3V to 5V and shield
     the Raspberry Pi GPIOs from overloading.
   * This supports two panels and provides convenient breakouts
     for I²C and the serial interface.
   * Open source KiCAD PCB EDA format.
   * If you are interested in connecting three RGB panels, check out the
     [Active-3](../active-3/) board (it won't support the serial intefaces
     though).

For multiple parallel boards to work and using the I2C and serial interface you have to uncomment

     #DEFINES+=-DSUPPORT_MULTI_PARALLEL   # remove the '#' in the begging

in [lib/Makefile](../../lib/Makefile).

![Preview][rendering]

[rendering]: ../../img/active2-pcb.png
