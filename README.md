Controlling RGB LED display with Raspberry Pi GPIO
==================================================

A library to control commonly available 32x32 or 16x32 RGB LED panels with the
Raspberry Pi. Can support PWM up to 11Bit per channel, providing true 24bpp
color with CIE1931 profile.

Supports 3 chains with many 32x32-panels each.
On a Raspberry Pi 2, you can easily chain 12 panels in that chain (so 36 panels total),
but you can stretch that to up to 96-ish panels (32 chain length) and still reach
around 100Hz refresh rate with full 24Bit color (theoretical - never tested this;
there might likely be timing problems with the panels that will creep up then).
With fewer colors you can control even more, faster.

The LED-matrix library is (c) Henner Zeller <h.zeller@acm.org>, licensed with
[GNU General Public License Version 2.0](http://www.gnu.org/licenses/gpl-2.0.txt)
(which means, if you use it in a product somewhere, you need to make the
source and all your modifications available to the receiver of such product so
that they have the freedom to adapt and improve).

Overview
--------
The 32x32 or 16x32 RGB LED matrix panels can be scored at [Sparkfun][sparkfun],
[AdaFruit][ada] or eBay and Aliexpress. If you are in China, I'd try to get
them directly from some manufacturer, Taobao or Alibaba.

The `RGBMatrix` class provided in `include/led-matrix.h` does what is needed
to control these. You can use this as a library in your own projects or just
use the demo binary provided here which provides some useful examples.

Check out [utils/ directory for some ready-made tools](./utils) to get started
using the library, or the [example-api-use/](./example-api-use) directory if
you want to get started programming your own utils.

All Raspberry Pi versions supported
-----------------------------------

This supports the old Raspberry Pi's Version 1 with 26 pin header and also the
B+ models, the Pi Zero, as well as the Raspberry Pi 2 and 3 with 40 pins.
The 26 pin models can drive one chain of RGB panels, the 40 pin models
**up to three** chains in parallel (each chain 12 or more panels long).

The Raspberry Pi 2 and 3 are faster than older models (and the Pi Zero) and
sometimes the cabeling can't keep up with the speed; check out
this [troubleshooting section](#help-some-pixels-are-not-displayed-properly)
what to do.

The [Raspbian Lite][raspbian-lite] distribution is recommended.

Types of Displays
-----------------
There are various types of displays that come all with the same Hub75 connector.
They vary in the way the multiplexing is happening.

Type  | Scan Multiplexing | Program Option  | Remark
-----:|:-----------------:|:----------------|-------
64x64 |  1:32             | -r 64 -c 2      | For displays with E line.
32x32 |  1:16             | -r 32           |
32x64 |  1:16             | -r 32 -c 2      | internally two chained 32x32
16x32 |  1:8              | -r 16           |
?     |  1:4              | -r 8            | (not tested myself)

These can be chained by connecting the output of one panel to the input of
the next panel. You can chain quite a few together.

The 64x64 matrixes typically have 5 address lines (A, B, C, D, E). There are
also 64x64 panels out there that only seem to have 1:4 multiplexing (there
is A and B), but I have not had these in my lab yet to test.

Let's do it
------------
This documentation is split into parts that help you through the process

  1. [Wire up the matrix to your Pi](./wiring.md). This document describes what
     goes where. You might also be interested in [breakout boards](./adapter)
     for that. If you have an [Adafruit HAT], necessary steps are
     [described below](#if-you-have-an-adafruit-hat)
  2. Run a demo. You find that in the
     [examples-api-use/](./examples-api-use#running-some-demos) directory.
  3. Use the utilities. The [utils](./utils) directory has some ready-made
     useful utilities to show image or text.

Chaining panels
---------------

We might only have a limited amount of GPIOs on the Raspberry Pi, but luckily,
the RGB matrices can be chained. The display panels have an input connector,
and also have an output port, that you can connect to the next display in a
daisy-chain manner. There is the flag `-c` in the demo program to give number
of displays that are chained.
You end up with a very wide display (chain * 32 pixels).

The [wiring.md](./wiring.md#chaining-parallel-chains-and-coordinate-system)
document explains the details.

<a href="wiring.md#chaining-parallel-chains-and-coordinate-system"><img src="img/coordinates.png"></a>
<a href="adapter/"><img src="img/three-parallel-panels-soic.jpg" width="300px"></a>


Troubleshooting
---------------
Here are some tips in case things don't work as expected.

### Use minimal Raspbian distribution
In general, run a minimal configuration on your Pi. There were some
unconfirmed reports of problems with Pis running GUI systems. Even though the
Raspberry Pi foundation makes you believe that you can do that: don't. Using it
with a GUI is a frustratingly slow use of an otherwise perfectly good
embedded device.

Everything seems to work well with a **[Raspbian Lite][raspbian-lite]**
distribution.

### Bad interaction with Sound
If sound is enabled on your Pi, this will not work together with the LED matrix,
as both need the same internal hardware sub-system. So if you run `lsmod` and
see any modules show up with `snd` in their name, this could be causing trouble.

In that case, you should create a kernel module blacklist file like the
following on your system and update your initramfs:

```
cat <<EOF | sudo tee /etc/modprobe.d/blacklist-rgb-matrix.conf
blacklist snd_bcm2835
blacklist snd_pcm
blacklist snd_timer
blacklist snd_pcsp
blacklist snd
EOF

sudo update-initramfs -u
```

Reboot and confirm that no 'snd' module is running.

### Logic level voltage not sufficient
Some panels don't interpret the 3.3V logic level well, or the RPi output drivers
have trouble driving longer cables, in particular with
faster Raspberry Pis Version 2. This results in artifacts like randomly
showing up pixels, color fringes, or parts of the panel showing 'static'.

If you encounter this, try these things

   - Make sure to have as short as possible flat-cables connecting your
     Raspberry Pi with the LED panel.

   - In particular if the chips close to the input of the LED panel
     read 74HC245 instead of 74HCT245 or 74AHCT245, then this board will not
     work properly with 3.3V inputs coming from the Pi.
     Use an [adapter board](./adapter/active-3) with a bus-driver that acts as
     level shifter between 3.3V and 5V.
     (In any case, it is always a good idea to use the level shifters).

   - A temporary hack to make HC245 inputs work with the 3.3V levels is to
     supply only like 4V to the LED panel. But the colors will be off, so not
     really useable as long-term solution.

   - If you can't implement the above things, or still have problems, you can
     slow down the GPIO writing a bit. This will of course reduce the
     frame-rate, so it comes at a cost.

For GPIO slow-down, the following line in the [lib/Makefile](lib/Makefile)
is interesting:

     DEFINES+=-DRGB_SLOWDOWN_GPIO=1

The default value is 1, if you still have problems, try the value 2. If you
know that your display is fast enough, try to comment out that line.

Then `make` again.

### Ghosting
Some panels have trouble with sharp contrasts and short pulses that results
in ghosting. It is particularly apparent with very sharp contrasts, such as
bright text on black background. This can be improved by tweaking the `LSB_PWM_NANOSECONDS`
parameter in [lib/Makefile](./lib/Makefile). See description there for details.

The following example is a little exaggerated:

Ghosting with low LSB_PWM_NANOSECONDS  | No ghosting after tweaking
---------------------------------------|------------------------------
![](img/text-ghosting.jpg)             |![](img/text-no-ghosting.jpg)

### Inverted Colors ?

There are some displays out there that use inverse logic for the colors. You
notice that your image looks like a 'negative'. The parameter to tweak is
`INVERSE_RGB_DISPLAY_COLORS` in [lib/Makefile](./lib/Makefile).

### Check configuration in lib/Makefile

There are lots of parameters in [lib/Makefile](./lib/Makefile) that you might
be interested in tweaking.

If you have an Adafruit HAT
---------------------------

Generally, if you want to connect RGB panels via an adapter instead of
hand-wiring, I suggest to build one of the adapters whose open-hardware
files you find in the [adapter/](./adapter) subdirectory.

However, Adafruit [offers an adapter][adafruit-hat] which is already ready-made,
but it only allows for a single chain. If the
ready-made vs. single-chain tradeoff is worthwhile, then you might go for that
(I am not affiliated with Adafruit).

### Switch the Pinout

The Adafruit HAT uses a modified pinout, so they forked this library and
modified the pinout there. However, that fork is _ancient_, so I strongly
suggest to use this original library instead.

In this library here, you have to uncomment the following line in
the [lib/Makefile](./lib/Makefile)

```
#DEFINES+=-DADAFRUIT_RGBMATRIX_HAT
```
Uncommenting means: remove the `#` in front of that line.

Then re-compile and a display connected to the HAT should work.

### Improving flicker

To improve flicker, we need to do a little hardware modification,
but it is very simple: solder a wire between GPIO 4 and 18 as shown in the
following picture (click to enlarge):

<a href="img/adafruit-mod.jpg"><img src="img/adafruit-mod.jpg" height="80px"></a>

Then, uncomment the following line in the Makefile and recompile.

```
#DEFINES+=-DADAFRUIT_RGBMATRIX_HAT_PWM
```

Reboot the Pi and you now should have less visible flicker.

### 64x64 with E-line on Adafruit HAT
There is another hardware mod needed for 1:32 multiplexing 64x64
panels that require an E-channel. It is a little more advanced hack, so this
is only really for people who are comfortable with this kind of thing.
First, you have to figure out which is the input of the E-Line on your matrix
(they seem to be either on Pin 4 or Pin 8 of the IDC connector).
You need to disconnect that Pin from the ground plane (e.g. with an Exacto
knife) and connect GPIO 24 to it. The following images illustrate the case for
IDC Pin 4.

<a href="img/adafruit-64x64-front.jpg"><img src="img/adafruit-64x64-front.jpg" height="80px"></a>
<a href="img/adafruit-64x64-back.jpg"><img src="img/adafruit-64x64-back.jpg" height="80px"></a>

If the direct connection does not work, you need to send it through a free
level converter of the Adafruit HAT. Since all unused inputs are grounded
with traces under the chip, this involves lifting a leg from the
HCT245 (figure out a free bus driver from the schematic). If all of the
above makes sense to you, you have the Ninja level to do it!

It might be more convienent at this point to consider the [Active3 adapter](./adapter/active-3)
that has that covered already.

CPU use
-------

These displays need to be updated constantly to show an image with PWMed
LEDs. This is dependent on the length of the chain: for each chain element,
about 1'000'000 write operations have to happen every second!
(chain_length * 32 pixel long * 16 rows * 11 bit planes * 180 Hz refresh rate).

We can't use hardware support for writing these as DMA is too slow,
thus the constant CPU use on an RPi is roughly 30-40% of one core.
Keep that in mind if you plan to run other things on this computer (This
is less noticable on Raspberry Pi, Version 2 or 3 that has more cores).

Also, the output quality is suceptible to other heavy tasks running on that
computer - there might be changes in the overall brigthness when this affects
the referesh rate.

If you have a loaded system and one of the newer Pis with 4 cores, you can
reserve one core just for the refresh of the display:

```
isolcpus=3
```

.. at the end of the line of `/boot/cmdline.txt`. This will use the last core
only to refresh the display then, but it also means, that no other process can
utilize it then. Still, I'd typically recommend it.

Limitations
-----------
If you are using the RGB_CLASSIC_PINOUT, or Adafruit Hat in the default
configuration, then we can't make use of the PWM hardware (which only outputs
to a particular pin), so you'll see random brightness glitches. I strongly
suggest to change the pinout.

The system needs constant CPU to update the display. Using the DMA controller
was considered but after extensive experiments
( https://github.com/hzeller/rpi-gpio-dma-demo )
dropped due to its slow speed..

There is an upper limit in how fast the GPIO pins can be controlled, which
limits the frame-rate. Raspberry Pi 2's and newer are generally faster.

Fun
---
I am always happy to see users successfully using the software for wonderful
things, like this installation by Dirk in Scharbeutz, Germany:

![](./img/user-action-shot.jpg)

[matrix64]: ./img/chained-64x64.jpg
[sparkfun]: https://www.sparkfun.com/products/12584
[ada]: http://www.adafruit.com/product/1484
[rt-paper]: https://www.osadl.org/fileadmin/dam/rtlws/12/Brown.pdf
[adafruit-hat]: https://www.adafruit.com/products/2345
[raspbian-lite]: https://downloads.raspberrypi.org/raspbian_lite_latest
[Adafruit HAT]: https://www.adafruit.com/products/2345