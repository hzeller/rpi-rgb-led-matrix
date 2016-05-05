Adapter PCB to support up to 3 panel chains
===========================================

   * Passive board. Simple, but might need to define `RGB_SLOWDOWN_GPIO` if you see
     glitches. Consider using the [active board](../active-3) in that case.
   * Only really advisable, if the LED panels have 74HCT245 (as opposed to just
     74HC245) in their input stage, because then they can deal properly with
     the 3.3V logic levels coming from the Pi.
   * Supports up to three panel chains for newer plus models and
     Raspberry Pi 2 that have 40 GPIO pins.
   * Open source KiCAD PCB EDA format.
   * (not very pretty layout, was just lazy and let the auto-router do it).
   * The FAB files are provided as [passive3-rpi-hub75-adapter-fab.zip](./passive3-rpi-hub75-adapter-fab.zip)

This board is [shared on OSH Park][osh-passive3] (not affiliated).

![Preview][rendering]

[rendering]: ../../img/passive3-pcb.png
[osh-passive3]: https://oshpark.com/shared_projects/FNAtZUsP
