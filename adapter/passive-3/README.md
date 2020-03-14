Adapter PCB to support up to 3 panel chains
===========================================

   * This is a passive board. It is simple, but the logic level will be out of
     spec for the LED matrix (3.3V vs. 5V) which might or might not work.
     Driving long cables with the GPIO pins is also not a good idea.
   * You typically want to consider using the [active board](../active-3).
   * Works for Matrix up to 1:16 multiplexing (32 rows). For 1:32 multiplexing,
     you want to use the [active board](../active-3). You can of also hack
     this board as [suggested in this bugtracker entry](https://github.com/hzeller/rpi-rgb-led-matrix/issues/360#issuecomment-321104348) to make it work with
     64x64 boards.
   * Only really advisable, if the LED panels have 74HCT245 (as opposed to just
     74HC245) in their input stage, because then they can deal properly with
     the 3.3V logic levels coming from the Pi.
   * Supports up to three panel chains for newer plus models and
     Raspberry Pi 2/3 that have 40 GPIO pins.
   * Open source KiCAD PCB EDA format.
   * (not very pretty layout, was just lazy and let the auto-router do it).
   * The FAB files are provided as [passive3-rpi-hub75-adapter-fab.zip](./passive3-rpi-hub75-adapter-fab.zip)

This board is [shared on OSH Park][osh-passive3] (not affiliated).

![Preview][rendering]

[rendering]: ../../img/passive3-pcb.png
[osh-passive3]: https://oshpark.com/shared_projects/FNAtZUsP
