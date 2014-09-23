Controlling RGB LED display with Raspberry Pi GPIO
==================================================

A library to control commonly available 32x32 or 16x32 RGB LED panels with the
Raspberry Pi. Can support PWM up to 11Bit per channel, providing true 24bbp
color with CIE1931 profile (but: see Limitations below).

The LED-matrix library is (c) Henner Zeller <h.zeller@acm.org> with
GNU General Public License Version 2.0 <http://www.gnu.org/licenses/gpl-2.0.txt>

The example code using this library is released in the public domain.

Overview
--------
The 32x32 or 16x32 RGB LED matrix panels can be scored at [Sparkfun][sparkfun],
[AdaFruit][ada] or eBay. If you are in China, I'd try to get them directly
from some manufacturer or Taobao.

The `RGBMatrix` class provided in `include/led-matrix.h` does what is needed
to control these. You can use this as a library in your own projects or just
use the demo binary provided here which provides some useful examples.

Connection
----------
The RPi has 3.3V logic output level, but a display operated at 5V digests these
logic levels just fine (also, the display will work well with 4V; watch out,
they easily can sink 2 Amps if all LEDs are on). Since we only need output
pins, we don't need to worry about level conversion back.

We need 13 IO pins. It doesn't really matter to which GPIO pins these are
connected (but the code assumes right now that the row address are adjacent
bits) - if you use a different layout, change in the `IoBits` union in
framebuffer-internal.h if necessary. Check
<http://elinux.org/RPi_Low-level_peripherals> for details of available GPIOs
and pin-header)

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
   * OE- (neg. Output enable) : GPIO 2 (Rev 2 RPi) or GPIO 0 (Rev 1 RPi)
   * CLK (Serial clock) : GPIO 3 (Rev 2 RPi) or GPIO 1 (Rev 1 RPi)
   * STR (Strobe row data) : GPIO 4

Here a typical pinout on these LED panels, found on the circuit board:
![Hub 75 interface][hub75]

Running
-------
The main.cc has some testing demos. Via command line flags, you can choose
the display type you have (16x32 or 32x32), and how many you have chained.
(Previous versions of this software required to do modifications in the source,
that is now all dynamically configurable).

     $ make
     $ ./led-matrix
     usage: ./led-matrix <options> -D <demo-nr> [optional parameter]
     Options:
         -r <rows>     : Display rows. 16 for 16x32, 32 for 32x32. Default: 32
         -c <chained>  : Daisy-chained boards. Default: 1.
         -L            : 'Large' display, composed out of 4 times 32x32
         -p <pwm-bits> : Bits used for PWM. Something between 1..11
         -l            : Don't do luminance correction (CIE1931)
         -D <demo-nr>  : Always needs to be set
         -d            : run as daemon. Use this when starting in
                         /etc/init.d, but also when running without
                         terminal (e.g. cron)
         -t <seconds>  : Run for these number of seconds, then exit.
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

Here is a video of how it looks
[![Runtext][run-vid]](http://youtu.be/OJvEWyvO4ro)

There are also two examples `minimal-example.cc` and `text-example.cc` that
show use of the API. The text example allows for some interactive output of
text (using a bitmap-font found in the `fonts/` directory), but it could also
be used to connct via a pipe, say you have an program or shell-script
that wants to output something; let's display the time in blue:

     (while :; do date +%T ; sleep 0.2 ; done) | sudo ./text-example -f fonts/8x13B.bdf -y8 -c2 -C0,0,255

You could connect this via a pipe to any process that just outputs new
information on standard-output every now and then. The screen is filled with
text until it overflows which then clears it. Or sending an empty line explicitly
clears the screen (if you want to display an empty line, just send a space).

![Time][time]


**CPU use**

These displays need to be updated constantly to show an image with PWMed
LEDs. For one 32x32 display, every second about 500'000 pixels have to be
updated. We can't use any hardware support to do that - thus the constant
CPU use on an RPi is roughly 30%. Keep that in mind if you plan to run other
things on this computer.

Also, the output quality is suceptible to other heavy tasks running on that
computer as the precise timing needed might be slipping. Even if the system is
otherwise idle, you might see occasional brightness variations in the darker
areas of your picture.
Ideally, this would run on a system with hard realtime guarantees
(There are Linux extensions for that, but haven't tried that yet).

Chaining
--------

Displays also have an output port, that you can connect to the next display
in a daisy-chain manner. There is a parameter in the demo program to give
number of displays that are chained. You end up with a very wide
display (chain * 32 pixels).

You can as well chain multiple boards together and then arrange them in a
different layout. Say you have 4 displays with 32x32 -- if we chain
them, we get a display 32 pixel high, (4*32)=128 pixel long. If we arrange
the boards in a square, we get a logical display of 64x64 pixels.

For convenience, we should only deal with the logical coordinates of
64x64 pixels in our program: implement a `Canvas`
interface to do the coordinate mapping. Have a look at
`class LargeSquare64x64Canvas` for an example and see how it is delegating to
the underlying RGBMatrix with changed coordinates.

Here is how the wiring would look like:

<img src="img/chained-64x64.jpg" width="400px"> In action:
[![PixelPusher video][pp-vid]](http://youtu.be/ZglGuMaKvpY)

Using the API
-------------
While there is a demo program, the matrix code can be used independently as
a library. The includes are in `include/`, the library to link is built
in `lib/`. So if you are proficient in C++, then use it in your code.

Due to the wonders of github, it is pretty easy to be up-to-date.
I suggest to add this code as a sub-module in your git repository. That way
you can use that particular version and easily update it if there are changes:

     git submodule add https://github.com/hzeller/rpi-rgb-led-matrix.git matrix

(Read more about how to use [submodules in git][git-submodules])

This will check out the repository in a subdirectory `matrix/`.
The library to build would be in directory `matrix/lib`, so let's hook that
into your toplevel Makefile.
I suggest to set up some variables like this:

     RGB_INCDIR=matrix/include
     RGB_LIBDIR=matrix/lib
     RGB_LIBRARY_NAME=rgbmatrix
     RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
     LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

Also, you want to add a target to build the libary in your sub-module

	 # (FYI: Make sure, there is a TAB-character in front of the $(MAKE))
     $(RGB_LIBRARY):
		 $(MAKE) -C $(RGB_LIBDIR)

Now, your final binary needs to depend on your objects and also the
`$(RGB_LIBRARY)`

     my-binary : $(OBJECTS) $(RGB_LIBRARY)
	     $(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

As an example, see the [PixelPusher implementation][pixelpush] which is using
this library in a git sub-module.

Note, all the types provided are in the `rgb_matrix` namespace. That way, they
won't clash with other types you might use in your code; in particular pretty
common names such as `GPIO` or `Canvas` might run into clashing trouble.

Anyway, for convenience you just might add using-declarations in your
code:

     // Types exported by the RGB-Matrix library.
     using rgb_matrix::Canvas;
     using rgb_matrix::GPIO;
     using rgb_matrix::RGBMatrix;
     using rgb_matrix::ThreadedCanvasManipulator;

Or, if you are lazy, just import the whole namespace:

     using namespace rgb_matrix;

Read the [`minimal-example.cc`](./minimal-example.cc) to get started, then
have a look into [`demo-main.cc`](./demo-main.cc).

A word about power
------------------

These displays suck a lot of current. At 5V, when all LEDs are on (full white),
my LED panel draws about 3.4A. That means, you need a beefy power supply to
drive these panels; a 2A USB charger or similar is not enough for a
32x32 panel; it might be for a 16x32.

If you connect multiple boards together, you needs a power supply that can
keep up with 3.5A / panel. Good are PC power supplies that often provide > 20A
on the 5V rail.

The current draw is pretty spiky. Due to the PWM of the LEDs, there are very
short peaks of about 4 microseconds to about 1ms of full current draw.
Often, the power cable can't support these very short spikes due to inherent
inductance. This can results in 'noisy' outputs, with random pixels not behaving
as they should. A low ESR capacitor close to the input is good in these cases.

On some displays, the quality of the output quickly gets erratic
when voltage drops below 4.5V. Some even need a little bit higher voltage around
5.5V to work reliably.

When you connect these boards to a power source, the following are good
guidelines:
   - Have fairly thick cables connecting the power to the board.
     Plan to not loose more than 50mV from the source to the LED matrix.
     So that would be 50mV / 3.5A = 14 mΩ. For both supply wires, so 7mΩ
     each trace.
     A 1mm² copper cable has about 17.5mΩ/meter, so you'd need a **2.5mm²
     copper cable per meter and panel**. Multiply by meter and number of
     panels to get the needed cross-section.
     (For Americans: that would be ~13 gauge wire for 3 ft and one panel)

   - You might consider using aluminum mounting brackets or bars as part of
     your power trace solution. With 1mm²aluminums' specific resistivity of
     about 28mΩ/meter, you'd need a cross sectional area of about 4mm² per panel
     and meter.

   - These are the minimum values to not drop more than 50mV. As engineer, you'd
     like to aim for less than that :)

   - Often these boards come with connectors that have cables crimped on.
     These cables are typically too thin; you might want to clip them close to
     the connector wire your proper, thick cable to it.

   - It is good to buffer the current spikes directly at the panel. The most
     spikes happen while PWM-ing a single line.
     So let's say we want to buffer the energy to power a single line without
     dropping more than 50mV. We use 3.5A which is 3.5Joule/second. We do
     about 140Hz refresh rate and divide that in 16 lines, so we need
     3.5 Joule/140/16 = ~1.6mJoule in the time period to display one line.
     We want to get the energy out of the voltage drop of 50mV; so with
     W = 1/2*C*U², we can calculate the capacitance needed:
       C = 2 * 1.6mJoule / ((5V)² - (5V - 50mV)²) = ~6400µF.
     So, **2 x 3300µF** low-ESR capacitors in parallel directly
     at the board are a good choice (two, because lower parallel ESR; also
     fits easier under board).
     (In reality, we need of course less, as the highest ripple comes with
      50% duty cyle thus half the current; also the input is recharching all
      the time. But: as engineer plan for maximum and then some).

   - If you still see noise, increase the voltage sligthly to 5.5V. This is
     typically only a symptom of too thin traces.

Technical details
-----------------

The matrix modules available on the market all seem to have the same
standard interface, essentially controlling
two banks of 16 rows (0..15 and 16..31) There are always two rows (n and n+16),
that are controlled in parallel
(These displays are also available in 16x32; in that case, it is two banks of 8).

The data for each row needs to be clocked in serially using one bit for red,
green and blue for both rows that are controlled in parallel (= 6 bits), then
a positive clock edge to shift them in - 32 pixels for one row are clocked in
like this (or more: you can chain these displays).
With 'strobe', the data is transferred to the output buffers for the row.
There are four bits that select the current row(-pair) to be displayed.
Also, there is an 'output enable' which switches if LEDs are on at all.

Since LEDs can only be on or off, we have to do our own PWM by constantly
clocking in pixels.

Limitations
-----------
If using higher resolution color (This code supports up to 24bpp @3x11 bit PWM),
you will see dynamic glitches - lines that flicker and randomly look a bit
brighter. At lower bit PWM between <= 4, this is typically not visible.

This is due to the fact that we have to do the PWM ourselves and for
high-resolution PWM, the smallest time-period is around 200ns. We would need
hard real-time requirements of the operating system of << 200ns.
Even for realtime environments, that is pretty tough.
We're running this on a general purpose computer with no dedicated
realtime hardware (such as dedicated, separate realtime core(s) we could use
on the BeagleBone Black). Linux does provide some support for realtime
applications, but the latency goals here are in the tens of microseconds at
best. Even with realtime-patches applied (I tried the
[RPi wheezy image provided by Emlid][emlid-rt]), this does not make much of
a dent.

According to the paper [How fast is fast enough? Choosing between Xenomai and
Linux for real-time applications][rt-paper], it might pay off to move the display
update part to the kernel. Future TODO.

(One experiment already done was to use the DMA controller of the RPi to make
use of dedicated hardware. However, it turns out that the DMA controller was
slower writing data than using GPIO directly. But maybe it might be worthwile if
it turns out to have more stable realtime properties.)

There seems to be a limit in how fast the GPIO pins can be controlled.
We get about 10Mhz clock speed out of GPIO clocking. Do do things correctly,
we would have to take the time it takes to clock a row in as essentially the
lowest PWM time (~3.4µs).
However, we just ignore this 'black' time, and switch the row on and off after
the clocking with the needed time-period; that way we get down to 200ns.

[hub75]: ./img/hub75.jpg
[matrix64]: ./img/chained-64x64.jpg
[time]: ./img/time-display.jpg
[pp-vid]: ./img/pixelpusher-vid.jpg
[run-vid]: ./img/running-vid.jpg
[pixelpush]: https://github.com/hzeller/rpi-matrix-pixelpusher
[sparkfun]: https://www.sparkfun.com/products/12584
[ada]: http://www.adafruit.com/product/1484
[git-submodules]: http://git-scm.com/book/en/Git-Tools-Submodules
[emlid-rt]: http://www.emlid.com/raspberry-pi-real-time-kernel-available-for-download/
[rt-paper]: https://www.osadl.org/fileadmin/dam/rtlws/12/Brown.pdf
