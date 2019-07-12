Adapter PCB to support up to 3 panel chains
===========================================

   * Supports up to three panel chains for newer plus models and
     Raspberry Pi 2 that have 40 GPIO pins.
   * Uses HCT245 to level shift signals from 3.3V to 5V and shield
     the Raspberry Pi GPIOs from overloading.
   * Open source KiCAD PCB EDA format.
   * Optional: Pads to power the Pi with 5V, including optional capacitor footprints.
   * Connector for RxD input (literally the only GPIO pin left) in case you want to
     make your panel controlled with a serial interface (3.3V logic level).
   * Provides a way to choose the pinout for different kinds of 64x64 matrixes.
   * (not very pretty layout, was just lazy and let the auto-router generate the first pass)
   * BOM:
     - 4x 74HCT245 or 74AHCT245 in 20-SOIC, 7.5mm package which should make
     it easy to hand-solder. Make sure to get the variant with the **T**: HC**T** or AHC**T**
     (there are also HC or AHC, don't use these).
     - 4x 100nF ceramic capacitor (0805 package)
     - 1x 10kOhm resistor (0805 package). Not critcial, just a pullup (2.2k .. 15k probably ok).
     - 3x 16pin IDC (=2x8) male receptible to connect the panels.
     - 1x 40pin female connector to connect to the RPi.
     - 1x (optional) 22μF .. 100μF capacitor for 5V rail (either 1206 SMD or
       radial electrolytic with 2.5mm pitch/6.3mm diameter)
   * The Gerber FAB files are provided as [active3-rpi-hub75-adapter-fab.zip](./active3-rpi-hub75-adapter-fab.zip)

The board is also [shared on OSH Park][osh-active3] (not affiliated).

![Preview][rendering]
![Real World][real-world]

Essentially, this is connecting the output pins through level shifting buffers (they
are operated at 5V, but the HCT series chips accept 3.3V input levels from the Pi). The
strobe, OE and clock signals are separately buffered for each connector.

![Schematic][schematic]

## Optional

### Power in

The area on the left has 5V/GND input pads, that allow you to power your Raspberry Pi from
a 5V source ... which you are likely to have as you are powering the LED Matrix. This is often
more convenient than using the USB connector to power the Pi.

If you do that, there are pads to add a capacitor to smooth the supply - two footprints are
provided: C5 and C6 for through-hole or surface mount components. The value is not critical;
I usually use a 22μF/6.3V ceramic capacitor on the C6 pads.

### Choose E-Line for 64x64 panels with 1:32 multiplexing

If you have a 64x64 matrix with 1:32 multiplexing, you need to supply an `E`-address line to it.
While the Address lines `A` to `D` have fixed positions on the Hub75 connector, there
seem to be two different ways to connect the `E` address line: it is either on pin 4 or pin 8
of the connector. So this adapter board provides the flexibility to choose the right pin for
your matrix.

Look at the back of the matrix or the documentation to find out for your specific board where
to connect E.

Once you know that, the jumper area in the bottom left of this adapter board allows to choose
to which pin to connect the E-address line to. The corresponding other pin should be connected
to GND. Simply solder a wire bridge as indicated below (or use a jumper that you can change later).

If you are not using such 64x64 matrix, you can connect both these pins to GND.

Here are the typical configurations:

No 1:32 64x64: to GND    | E-Line on Pin 4   | E-Line on Pin 8|
-------------------------|-------------------|----------------
![][config-default]      |![][config-pin4]   |![][config-pin8]

### Input for serial RxD

If you are not using a 64x64 display that occupies the E-Line, you can use the RxD serial input -
this might come in handy if you are using the display to be controlled by a serial line. Be aware
that the input requires 3.3V level, so if you have a RS232, make sure to first adapt the levels.

[rendering]: ../../img/active3-pcb.png
[config-default]: ../../img/active3-pcb-config-default.png
[config-pin4]: ../../img/active3-pcb-config-pin4.png
[config-pin8]: ../../img/active3-pcb-config-pin8.png
[schematic]: ../../img/active3-schematic.png
[real-world]: ../../img/three-parallel-panels-soic.jpg
[osh-active3]: https://oshpark.com/shared_projects/6xAD1VXr
