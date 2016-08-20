Running some demos
------------------
The demo-main.cc has some testing demos. Via command line flags, you can choose
the display type you have (16x32 or 32x32), and how many you have chained.

```
$ make
$ ./demo
Expected required option -D <demo>
usage: ./demo <options> -D <demo-nr> [optional parameter]
Options:
        -r <rows>     : Panel rows. '16' for 16x32 (1:8 multiplexing),
                        '32' for 32x32 (1:16), '8' for 1:4 multiplexing; Default: 32
        -P <parallel> : For Plus-models or RPi2: parallel chains. 1..3. Default: 1
        -c <chained>  : Daisy-chained boards. Default: 1.
        -L            : 'Large' display, composed out of 4 times 32x32
        -p <pwm-bits> : Bits used for PWM. Something between 1..11
        -l            : Don't do luminance correction (CIE1931)
        -D <demo-nr>  : Always needs to be set
        -d            : run as daemon. Use this when starting in
                        /etc/init.d, but also when running without
                        terminal (e.g. cron).
        -t <seconds>  : Run for these number of seconds, then exit.
                        (if neither -d nor -t are supplied, waits for <RETURN>)
        -b <brightnes>: Sets brightness percent. Default: 100.
        -R <rotation> : Sets the rotation of matrix. Allowed: 0, 90, 180, 270. Default: 0.
Demos, choosen with -D
        0  - some rotating square
        1  - forward scrolling an image (-m <scroll-ms>)
        2  - backward scrolling an image (-m <scroll-ms>)
        3  - test image: a square
        4  - Pulsing color
        5  - Grayscale Block
        6  - Abelian sandpile model (-m <time-step-ms>)
        7  - Conway's game of life (-m <time-step-ms>)
        8  - Langton's ant (-m <time-step-ms>)
        9  - Volume bars (-m <time-step-ms>)
        10 - Evolution of color (-m <time-step-ms>)
        11 - Brightness pulse generator
Example:
        ./demo -t 10 -D 1 runtext.ppm
Scrolls the runtext for 10 seconds
```

To run the actual demos, you need to run this as root so that the
GPIO pins can be accessed.

The most interesting one is probably the demo '1' which requires a ppm (type
raw) with a height of 32 pixel - it is infinitely scrolled over the screen; for
convenience, there is a little runtext.ppm example included:

     $ sudo ./demo -D 1 runtext.ppm

Here is a video of how it looks
[![Runtext][run-vid]](http://youtu.be/OJvEWyvO4ro)

There are also two examples [minimal-example.cc](./minimal-example.cc) and
[text-example.cc](./text-example.cc) that show use of the API.

The text example allows for some interactive output of text (using a bitmap-font
found in the `fonts/` directory). Even though it is just an example, it can
be useful in its own right. For instance, you can connect to its input with a
pipe and simply feed text from a shell-script or other program that wants to
output something. Let's display the time in blue:

     (while :; do date +%T ; sleep 0.2 ; done) | sudo ./text-example -f ../fonts/8x13B.bdf -y8 -c2 -C0,0,255

You could connect this via a pipe to any process that just outputs new
information on standard-output every now and then. The screen is filled with
text until it overflows which then clears it. Or sending an empty line explicitly
clears the screen (if you want to display an empty line, just send a space).

![Time][time]

Using the API
-------------
While there is the demo program, the matrix code can be used independently as
a library. The includes are in `include/`, the library to link is built
in `lib/`. So if you are proficient in C++, then use it in your code.

Due to the wonders of github, it is pretty easy to be up-to-date.
I suggest to add this code as a sub-module in your git repository. That way
you can use that particular version and easily update it if there are changes:

     git submodule add https://github.com/hzeller/rpi-rgb-demo.git matrix

(Read more about how to use [submodules in git][git-submodules])

This will check out the repository in a subdirectory `matrix/`.
The library to build would be in directory `matrix/lib`, so let's hook that
into your toplevel Makefile.
I suggest to set up some variables like this; you only need to change the
location `RGB_LIB_DISTRIBUTION` is pointing to; in the sub-module example, this
was the `matrix` directory:

     RGB_LIB_DISTRIBUTION=matrix
     RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
     RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
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

If you are writing your own Makefile, make sure to pass the `-O3` option to
the compiler to make sure to generate fast code.

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

## Remapping coordinates ##
You might choose a different physical layout than the wiring provides.

Say you have 4 displays with 32x32 and only a single output
like with a Raspberry Pi 1 or the Adafruit HAT -- if we chain
them, we get a display 32 pixel high, (4*32)=128 pixel long. If we arrange
the boards in a square, we get a logical display of 64x64 pixels:

<img src="../img/chained-64x64.jpg" width="400px"> In action:
[![PixelPusher video][pp-vid]](http://youtu.be/ZglGuMaKvpY)

How can we make this 'folded' 128x32 screen behave like a 64x64 screen ?

In the API, there is an interface to implement,
a [`CanvasTransformer`](./include/canvas.h) that allows to program re-arrangements
of pixels in any way. You can plug such a `CanvasTransformer` into the RGBMatrix
to use the new layout (`void RGBMatrix::SetTransformer(CanvasTransformer *transformer)`).

Sometimes you even need this for the panel itself: In newer panels
(often with 1:4 multiplexing) the pixels are often not mapped in
a straight-forward way, but in a snake arrangement for instance. The CanvasTransformer
allows you to work around that (sorry, I have not seen these panels myself so that
I couldn't test that; but if you come accross one, you might want to send a pull-request
with a new CanvasTransformer).

Back to the 64x64 arrangement:

There is a sample implementation `class LargeSquare64x64Transformer` that maps
the 128x32 pixel logical arrangement into the 64x64 arrangement doing
the coordinate mapping. In the demo program and the
[`led-image-viewer`](../utils#image-viewer), you can activate this with
the `-L` option.

[time]: ../img/time-display.jpg
[run-vid]: ../img/running-vid.jpg
[git-submodules]: http://git-scm.com/book/en/Git-Tools-Submodules
[pixelpush]: https://github.com/hzeller/rpi-matrix-pixelpusher
[pp-vid]: ../img/pixelpusher-vid.jpg
