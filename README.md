Controlling a 32 x 32 RGB LED Matrix Panel with a Raspberry Pi and Android / iPhone
===================================================================================

This is mostly experimental code.

The Android / iPhone app has not been finished yet, but you can use the code
with the standard terminal (see Running). I use my Raspberry Pi headless and
if you want to run the code without having Putty always open, use "tmux"
(sudo apt-get install tmux).

This code is based on the great work of Henner Zeller <h.zeller@acm.org>
(https://github.com/hzeller/rpi-rgb-led-matrix and see also
https://github.com/hzeller/rpi-matrix-pixelpusher) and tries to extend
the possibilities of controlling a RGB Matrix. Moreover I got inspired by
Adafruit (https://github.com/adafruit/RGB-matrix-Panel) and Matt Hill
<matt@codebones.com> (https://github.com/mattdh666/rpi-led-matrix-panel).

I want to make a Raspberry Pi version of the "Game Frame: The Art of Pixels"
(https://www.kickstarter.com/projects/jerware/game-frame-the-art-of-pixels)
and the "PIXEL: Interactive LED Art"
(https://www.kickstarter.com/projects/alinke/pixel-interactive-led-art), because
both projects are absolutely amazing. However I think the Raspberry Pi
adds much possibilities to the RGB Matrix and I also favor the RPi.

Overview
--------
I use a 32x32 RGB LED Matrix Panel from Sparkfun, which is similar to the
Adafruit 32x32 RGB LED Matrix Panel, but a lot cheaper.These displays are
also available in 32x16 - they just have one bank.

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
There is a good tutorial on Adafruit:
http://learn.adafruit.com/connecting-a-16x32-rgb-led-matrix-panel-to-a-raspberry-pi

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
   * R1 (Red 1st bank)   : GPIO 17
   * G1 (Green 1st bank) : GPIO 18
   * B1 (Blue 1st bank)  : GPIO 22
   * R2 (Red 2nd bank)   : GPIO 23
   * G2 (Green 2nd bank) : GPIO 24
   * B2 (Blue 2nd bank)  : GPIO 25
   * A, B, C, D (Row address) : GPIO 7, 8, 9, 10
   * OE- (neg. Output enable) : GPIO 2
   * CLK (Serial clock) : GPIO 3
   * STR (Strobe row data) : GPIO 4
   * GND (Ground, '-') to ground of your Raspberry Pi

Running
-------
The main.cc has some testing demos. You need to run this as root so that the
GPIO pins can be accessed:

     $ make
     $ sudo ./led-matrix

The most interesting one is probably the demo '6' which requires a (series of) ppm (type
raw) with a height and width of 32 pixel; there are examples included (rain, Bubbles, glitter, nightdrive, tree, etc):

     $ sudo ./led-matrix 6 tree

You can also set the speed:

     $ sudo ./led-matrix 6 rain 50000

The standard speed is 100000.
Limitations
-----------
There seems to be a limit in how fast the GPIO pins can be controlled. Right
now, I only get about 10Mhz clock speed which ultimately limits the smallest
time constant for the PWM. Thus, only 7 bit PWM looks ok with not too much
flicker.

I tested this with the latest version of Raspbian.
