Controlling RGB LED display with Raspberry Pi GPIO
==================================================

This is mostly experimental code.

Code is (c) Henner Zeller <h.zeller@acm.org>, and I grant you the permission
to do whatever you want with it :)

Overview
--------
The 32x32 or 16x32 RGB LED matrix panels can be scored at AdaFruit or eBay.
If you are in China, I'd try to get them directly from some manufacturer or
Taobao.
They all seem to have the same standard interface, essentially controlling
two banks of 16 rows (0..15 and 16..31) There are always two rows (n and n+16),
that are controlled in parallel
(These displays are also available in 16x32 - they just have one bank of 16 or
two banks of 8)

The data for each row needs to be clocked in serially using one bit for red,
green and blue for both rows that are controlled in parallel (= 6 bits), then
a positive clock edge to shift them in - 32 pixels for one row are clocked in
like this (or more: you can chain these displays).
With 'strobe', the data is transferred to the output buffers for the row.
There are four bits that select the current row(-pair) to be displayed.
Also, there is an 'output enable' which switches if LEDs are on at all.

Since LEDs can only be on or off, we have to do our own PWM. The RGBMatrix
class in led-matrix.h does that.

Connection
----------
The RPi has 3.3V logic output level, but a display operated at 5V digests these
logic levels just fine (also, the display will work well with 4V; watch out,
they easily can sink 2 Amps if all LEDs are on). Since we only need output
pins, we don't need to worry about level conversion back.

We need 13 IO pins. It doesn't really matter to which GPIO pins these are
connected (but the code assumes right now that the row address are adjacent
bits) - if you use a different layout, change in the `IoBits` union in
led-matrix.h if necessary (This was done with a Model B,
older versions have some different IOs on the header; check
<http://elinux.org/RPi_Low-level_peripherals> )

LED-Panel to GPIO with this code:
   * GND (Ground, '-') to ground of your Raspberry Pi
   * R1 (Red 1st bank)   : GPIO 17
   * G1 (Green 1st bank) : GPIO 18
   * B1 (Blue 1st bank)  : GPIO 22
   * R2 (Red 2nd bank)   : GPIO 23
   * G2 (Green 2nd bank) : GPIO 24
   * B2 (Blue 2nd bank)  : GPIO 25
   * A, B, C, D (Row address) : GPIO 7, 8, 9, 10 (There is no `D` needed if you
	   have a display with 16 rows with 1:8 multiplexing)
   * OE- (neg. Output enable) : GPIO 2
   * CLK (Serial clock) : GPIO 3
   * STR (Strobe row data) : GPIO 4

This is the typical pinout on these LED panels
![Hub 75 interface][hub75]

Configuration
-------------
The default settings are for a single 32x32 display. If you have different
settings, such as a 16x32 display or multiple displays chained, you can change
these constants in `led-matrix.h`

     // Displays typically come in (rows x columns) = 16x32 or 32x32
     // configuration. Set number of rows in your display here, so 16 or 32.
     kDisplayRows = 32

     // You can chain the output of one board to the input of the next board.
     // That increases the total number of columns you can display. With only
     // one board, this is 1.
     kChainedBoards = 1

     // Maximum PWM resolution. The number of gray-values you can display per
     // color is 2^kPWMBits, so 4 -> 16 gray values (=> 16*16*16 = 4096 colors )
     // Higher values up to 7 are possible, but things get sluggish with slower
     // refresh rate.
     kPWMBits = 4

After changing these settings, you need to recompile.

Running
-------
The main.cc has some testing demos.


     $ make
     $ ./led-matrix
     usage: ./led-matrix <options> -D <demo-nr> [optional parameter]
     Options:
             -D <demo-nr>  : Always needs to be set
             -d            : run as daemon. Use this when starting in
                             /etc/init.d, but also when running without
                             terminal.
             -t <seconds>  : Run for these number of seconds, then exit
                    (if neither -d nor -t are supplied, waits for <RETURN>)
     Demos, choosen with -D
             0  - some rotating square
             1  - forward scrolling an image
             2  - backward scrolling an image
             3  - test image: a square
             4  - Pulsing color
     Example:
             ./led-matrix -d -t 10 -D 1 runtext.ppm
     Scrolls the runtext for 10 seconds

To run the actual demos, you need to run this as root so that the
GPIO pins can be accessed.

The most interesting one is probably the demo '1' which requires a ppm (type
raw) with a height of 32 pixel - it is infinitely scrolled over the screen; for
convenience, there is a little runtext.ppm example included:

     $ sudo ./led-matrix -D 1 runtext.ppm

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

One experiment was to use the DMA controller of the RPi to circumvent the
realtime problems. However, it turns out that the DMA controller slower writing
data to the GPIO pins than doing it directly. So even if offloading this
task to the DMA controller would improve the realtime-ness, it is too slow for
any meaningful display.

[hub75]: https://github.com/hzeller/rpi-rgb-led-matrix/raw/master/img/hub75.jpg

