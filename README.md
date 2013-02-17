Controlling RGB LED display with Raspberry Pi GPIO
==================================================

This is mostly experimental code.

Code is (c) Henner Zeller <h.zeller@acm.org>, and I grant you the permission
to do whatever you want with it :)

Overview
--------
The 32x32 RGB LED matrix panels can be scored at AdaFruit or eBay. If you are
in China, I'd try to get them directly from some manufacturer or Taobao which
seems to be a cheaper choice.
They all seem to have the same standard interface, essentially controlling
two banks of 16 rows. There are two rows that are controlled in parallel
(There are also displays available that are 32x16 - they just have one bank).

The data for each row needs to be clocked in serially using one bit for red,
green and blue for both rows that are controlled in parallel) (= 6 bits), then
a positive clock pulse to shift them in. With 'strobe', the data is transferred
to the output buffers. There are four lines that select the current row to be
displayed. Also, there is an 'output enable' which switches if LEDs are on
at all.
Since LEDs can either be on or off, we have to do our own PWM.


Connection
----------
The RPi has 3.6V logic output level, but the 5v operated Matrix seems to digest
just fine (you can as well just operate the matrix at 4V, these guys are
pretty bright already).

You need 13 IO pins. It doesn't really matter how these IO bins are connected,
if you use a different layout, change in the IoPins union in led-matrix.cc
if necessary (This was done with a Model B,
older versions have some different IOs on the header; check
<http://elinux.org/RPi_Low-level_peripherals> )

LED-Panel to GPIO with this code:
   * R1 (Red 1st bank)   : GPIO 17
   * G1 (Green 1st bank) : GPIO 18
   * B1 (Blue 1st bank)  : GPIO 22
   * R2 (Red 2nd bank)   : GPIO 23
   * G2 (Green 2nd bank) : GPIO 24
   * B2 (Blue 2nd bank)  : GPIO 25
   * A, B, C, D (Row address) : GPIO 7, 8, 9, 10
   * OE (neg. Output enable) : GPIO 2
   * CLK (Serial clock) : GPIO 3
   * STR (Strobe row data) : GPIO 4

Running
-------
The main.cc has some testing demos. You need to run this as root so that the
GPIO pins can be accessed:

     $ make
     $ sudo ./led-matrix

The most interesting one is probably the demo '1' which requires a ppm (type
raw) with a height of 32 pixel - it is infinitely scrolled over the screen; for
convenience, there is a little runtext.ppm example included:

     $ sudo ./led-matrix 1 runtext.ppm

Here is a video of how it looks:
<http://www.youtube.com/watch?v=OJvEWyvO4ro>

Limitations
-----------
There seems to be a limit in how fast the GPIO pins can be controlled. Right
now, I only get about 10Mhz clock speed which ultimately limits the smallest
time constant for the PWM. Thus, only 7 bit PWM looks ok with not too much
flicker.

The output should be luminance ('gamma') corrected, but isn't currently
in the code (this can be simply done in SetPixel(), but ideally, we have more
PWM output resolution, such as 10 bits).

Right now, I tested this with the default Linux distribution ("wheezy"). Because
this does not have any realtime patches, the PWM can look a bit uneven under
load. If you test this with realtime extensions, let me know how it works.
