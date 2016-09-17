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

Note to Old Time Users: Several changes in defines and flags
------------------------------------------------------------
If you have checked out this library before, you might find that some
files are re-organized in different directories (e.g. there is now a separation
for library examples and utilities), and that the flags to binaries are now
long and unified. Also, for the most part, you don't need to tweak paramters
in `lib/Makefile` anymore as they are now exposed via command line flags.

Choosing a different pinout, e.g. for the Adafruit HAT is also a little different.

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

Type  | Scan Multiplexing | Program Option               | Remark
-----:|:-----------------:|:-----------------------------|-------
64x64 |  1:32             | --led-rows=64 --led-chain=2  | For displays with E line.
32x32 |  1:16             | --led-rows=32                |
32x64 |  1:16             | --led-rows=32 --led-chain=2  | internally two chained 32x32
16x32 |  1:8              | --led-rows=16                |
?     |  1:4              | --led-rows=8                 | (not tested myself)

These can be chained by connecting the output of one panel to the input of
the next panel. You can chain quite a few together.

The 64x64 matrixes typically have 5 address lines (A, B, C, D, E). There are
also 64x64 panels out there that only seem to have 1:4 multiplexing (there
is A and B), but I have not had these in my lab yet to test.

Let's do it
------------
This documentation is split into parts that help you through the process

  * <a href="wiring.md"><img src="img/wire-up-icon.png"></a>
    [Wire up the matrix to your Pi](./wiring.md). This document describes what
    goes where. You might also be interested in [breakout boards](./adapter)
    for that. If you have an [Adafruit HAT], you can choose that with
    a command line option [described below](#if-you-have-an-adafruit-hat)
  * Run a demo. You find that in the
     [examples-api-use/](./examples-api-use#running-some-demos) directory:
```
make -C examples-api-use
sudo examples-api-use/demo -D0
```
  * Use the utilities. The [utils](./utils) directory has some ready-made
    useful utilities to show image or text. [Go there](./utils) to see how to
    compile and run these.

### Utilities

The [utils directory](./utils) is meant for ready utilities to show images,
animated gifs, text and video. Read the [README](./utils/README.md) there
for instructions how to compile.

There are external projects that use this library and provide higher level
network protocols, such as the
[FlaschenTaschen implementation](https://github.com/hzeller/flaschen-taschen)
(VLC can send videos to it natively) or the
[PixelPusher implementation](https://github.com/hzeller/rpi-matrix-pixelpusher)
(common in light art installations).

### API

The library comes as an API that you can use for your own utilities and use-cases.

  * The native library is a C++ library (see [include/](./include)).
    Example uses you find in the [examples-api-use/](./examples-api-use)
    directory.
  * If you prefer to program in C, there is also a
    [C API](./include/led-matrix-c.h).
  * In the [python](./python) subdirectory, you find a Python API including a
    couple of [examples](./python/samples) to get started.
  * There are a couple of external bindings, such as this [Nodejs binding]
    by Maxime Journaux.

### Changing parameters via command-line flags

For the programs in this distribution and also automatically in your own
programs using this library, there are a lot of parameters provided as command
line flags, so that you don't have to re-compile your programs to tweak them.
Some might need to be changed for your particular kind of panel.

Here is a little run-down of what these command-line flags do and when you'd
like to change them.

First things first: if you have a different wiring than described in
[wiring](./wiring.md), for instance if you have an Adafruit HAT, you can
choose these here:

```
--led-gpio-mapping=<gpio-mapping>: Name of GPIO mapping used. Default "regular"
```

This can have values such as
  - `--led-gpio-mapping=regular` The standard mapping of this library, described in the [wiring](./wiring.md) page.
  - `--led-gpio-mapping=adafruit-hat` standard Adafruit HAT or
  - `--led-gpio-mapping=adafruit-hat-pwm` Adafruit HAT with the anti-flicker hardware mod [described below](#improving-flicker).

The next most important flags describe the type and number of displays connected

```
--led-rows=<rows>         : Panel rows. 8, 16, 32 or 64. (Default: 32).
--led-chain=<chained>     : Number of daisy-chained panels. (Default: 1).
--led-parallel=<parallel> : For A/B+ models or RPi2,3b: parallel chains. range=1..3 (Default: 1).
```

These are the most important ones: here you choose how many panels you have
connected and how many rows are in each panel. Panels can be chained (each panel
has an input and output connector, see the
[wiring documentation](wiring.md#chains)) -- the `--led-chain` flag tells the
library how many panels are chained together. The newer Raspberry Pi's allow
to connect multiple chains in parallel, the `--led-parallel` flag tells it how
many there are.

This illustrates what each of these parameters mean:

<a href="wiring.md#chaining-parallel-chains-and-coordinate-system"><img src="img/coordinates.png"></a>

```
--led-brightness=<percent>: Brightness in percent (Default: 100).
```

Self explanatory.


```
--led-pwm-bits=<1..11>    : PWM bits (Default: 11).
```

The LEDs can only be switched on or off, so the shaded brightness perception
is achieved via PWM (Pulse Width Modulation). In order to get a good 8 Bit
per color resolution (24Bit RGB), the 11 bits default per color are good
(why ? Because our eyes are actually perceiving brightness logarithmically, so
we need a lot more physical resolution to get 24Bit sRGB).

With this flag, you can change how many bits it should use for this; lowering it
means the lower bits (=more subtle color nuances) are omitted.
Typically you might be mostly interested in the extremes: 1 Bit for situations
that only require 8 colors (e.g. for high contrast text displays) or 11 Bit
for everything else (e.g. showing images or videos). Why would you bother at all ?
Lower number of bits use slightly less CPU and result in a higher refresh rate.

```
--led-show-refresh        : Show refresh rate.
```

This shows the current refresh rate of the LED panel, the time to refresh
a full picture. Typically, you want this number to be pretty high, because the
human eye is pretty sensitive to flicker. Depending on the settings, the
refresh rate with this library are typically in the hundreds of Hertz but
can drop low with very long chains. Humans have different levels of perceiving
flicker - some are fine with 100Hz refresh, others need 250Hz.
So if you are curious, this gives you the number (shown on the terminal).

The refresh rate depends on a lot of factors, from `--led-rows` and `--led-chain`
to `--led-pwm-bits` and `--led-pwm-lsb-nanoseconds`. If you are tweaking these
parameters, showing the refresh rate can be a useful tool.

```
--led-scan-mode=<0..1>    : 0 = progressive; 1 = interlaced (Default: 0).
```

This switches from progressive scan and interlaced scan. The latter might
look be a little nicer when you have a very low refresh rate.

```
--led-pwm-lsb-nanoseconds : PWM Nanoseconds for LSB (Default: 130)
```

This allows to change the base time-unit for the on-time in the lowest
significant bit in nanoseconds.
Lower values will allow higher frame-rate, but will also negatively impact
qualty in some panels (less accurate color or more ghosting).

Good values for full-color display (PWM=11) are somewhere between 100 and 300.

If you you use reduced bit color (e.g. PWM=1) and have sharp contrast
applications, then higher values might be good to minimize ghosting.

How to decide ? Just leave the default if things are fine. But some panels have
trouble with sharp contrasts and short pulses that results
in ghosting. It is particularly apparent in situations such as bright text
on black background. In these cases increase the value until you don't see
this ghosting anymore.

The following example shows how this might look like:

Ghosting with low --led-pwm-lsb-nanoseconds  | No ghosting after tweaking
---------------------------------------------|------------------------------
![](img/text-ghosting.jpg)                   |![](img/text-no-ghosting.jpg)

If you tweak this value, watch the framerate (`--led-show-refresh`) while playing
with this number.

```
--led-slowdown-gpio=<0..2>: Slowdown GPIO. Needed for faster Pis and/or slower panels (Default: 1).
```

The Raspberry Pi 2 and 3 are putting out data too fast for almost all LED panels
I have seen. In this case, you want to slow down writing to GPIO. Zero for this
parameter means 'no slowdown'.

The default 1 (one) typically works fine, but often you have to even go further
by setting it to 2 (two). If you have a Raspberry Pi with a slower processor
(Model A, A+, B+, Zero), then a value of 0 (zero) might work and is desirable.

```
--led-no-hardware-pulse   : Don't use hardware pin-pulse generation.
```

This library uses a hardware subsystem that also is used by the sound. You can't
use them together. If your panel does not work, this might be a good start
to debug if it has something to do with the sound subsystem (see Troubleshooting
section). This is really only recommended for debugging; typically you actually
want the hardware pulses as it results in a much more stable picture.

```
--led-no-drop-privs       : Don't drop privileges from 'root' after initializing the hardware.
```

You need to start programs as root as it needs to access some low-level hardware
at initialization time. After that, it is typically not desirable to stay in this
role, so the library then drops the privileges.

This flag allows to switch off this behavior, so that you stay root.
Not recommended unless you have a specific reason for it.

```
--led-daemon              : Make the process run in the background as daemon.
```

If this is set, the program puts itself into the background (running
as 'daemon').
You might want this if started from an init script at boot-time.

```
--led-inverse             : Switch if your matrix has inverse colors on.
--led-swap-green-blue     : Switch if your matrix has green/blue swapped on.
```

These are if you have a different kind of LED panel in which the logic of the
color bits is reversed (`--led-inverse`) or where the green and blue colors
are swapped (`--led-swap-green-blue`). You know it when you see it.

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
as both need the same internal hardware sub-system (a first test to see if you
are affected is to run the progrem with `--led-no-hardware-pulse` and see if
things work fine then).

If you run `lsmod` and see any modules show up with `snd` in their name,
this could be causing trouble.

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

For GPIO slow-down, add the flag `--led-slowdown-gpio=2` to the invocation of
the binary.

If you have an Adafruit HAT
---------------------------

Generally, if you want to connect RGB panels via an adapter instead of
hand-wiring, I suggest to build one of the adapters whose open-hardware
files you find in the [adapter/](./adapter) subdirectory. It is a fun solder
exercise with large surface mount components.

However, Adafruit [offers an adapter][adafruit-hat] which is already ready-made,
but it only allows for a single chain. If the
ready-made vs. single-chain tradeoff is worthwhile, then you might go for that
(I am not affiliated with Adafruit).

### Switch the Pinout

The Adafruit HAT uses this library but a modified pinout to support other
features on the HAT. So they forked this library and modified the pinout there.
However, that fork is _ancient_, so I strongly suggest to use this original
library instead. You can choose the Adafruit pinout with a command line flag.

Just pass the option `--led-gpio-mapping=adafruit-hat`.

If you want to have this the default whenever you start (or if you are using
the Python library that does not support to set this at runtime yet), add the
following setting in front of your compilation:
```
HARDWARE_DESC=adafruit-hat make
```
(alternatively, you can modify the `lib/Makefile` and change it there directly)
Then re-compile and the new flag default is now `adafruit-hat`, so
no need to set it on the command line.

### Improving flicker

To improve flicker, we need to do a little hardware modification,
but it is very simple: solder a wire between GPIO 4 and 18 as shown in the
following picture (click to enlarge):

<a href="img/adafruit-mod.jpg"><img src="img/adafruit-mod.jpg" height="80px"></a>

Then, start your programs with `--led-gpio-mapping=adafruit-hat-pwm`.

If you want to make this the default setting your program starts with, you can
also manually choose this with
```
HARDWARE_DESC=adafruit-hat-pwm make
```
to get this as default setting.

Now you should have less visible flicker. This essentially
switches on the hardware pulses feature for the Adafruit HAT.

### 64x64 with E-line on Adafruit HAT
There are LED panels that have 64x64 LEDs packed, but they need 5 address lines,
which is 1:32 multiplexing (they have an `E` address-line). The hardware of
the Adafruit HAT is not prepared for this, but it can be done with another
hardware mod.

It is a little more advanced hack, so  is only really for people who are
comfortable with this kind of thing.
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

Also, the output quality is susceptible to other heavy tasks running on that
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
If you are using the Adafruit Hat in the default configuration, then we
can't make use of the PWM hardware (which only outputs
to a particular pin), so you'll see random brightness glitches. I strongly
suggest to do the aforementioned hardware mod.

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
[Nodejs binding]: https://github.com/zeitungen/node-rpi-rgb-led-matrix
