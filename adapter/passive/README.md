Passive Adapter PCB to support up to 2 panels
=============================================

   * Supports one panel for Raspberry PIs with 26 GPIO pins and two
     panels for newer plus models and Raspberry Pi 2 that have 40 GPIO pins.
   * This board is passive, so it does not need any more components than the
     connectors. If you have trouble with erratic pixels or need to connect
     long cables, then level shifting and buffering might help; check out
     the active boards in this case.
   * This supports two panels and provides convenient breakouts
     for I²C and the serial interface.
   * Open source KiCAD PCB EDA format.

For multiple parallel boards to work and using the I2C and serial interface you have to uncomment

     #DEFINES+=-DSUPPORT_MULTI_PARALLEL   # remove the '#' in the begging

in [lib/Makefile](../../lib/Makefile).

![Preview][rendering]

[rendering]: ../../img/passive2-pcb.png
