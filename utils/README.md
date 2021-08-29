Utilities
=========

This contains useful utilities that might be directly useful without having
to write any code.

Below, the description of the utilities contains a list of commandline flags
they support. Next to the specific flags for their use, they all
have a set of standard options that always come with the LED matrix,
such as choosing the `--led-rows` or `--led-chain`.

For brevity, we don't repeat them below in the synopsis prints of each of
these utilities. You find a description of the standard options in
the [toplevel readme](../README.md#changing-parameters-via-command-line-flags)

<details><summary>Unfold: Standard LED-matrix options present in all utilities</summary>

```
 --led-gpio-mapping=<name> : Name of GPIO mapping used. Default "regular"
 --led-rows=<rows>         : Panel rows. Typically 8, 16, 32 or 64. (Default: 32).
 --led-cols=<cols>         : Panel columns. Typically 32 or 64. (Default: 32).
 --led-chain=<chained>     : Number of daisy-chained panels. (Default: 1).
 --led-parallel=<parallel> : Parallel chains. range=1..3 (Default: 1).
 --led-multiplexing=<0..11> : Mux type: 0=direct; 1=Stripe; 2=Checkered; 3=Spiral; 4=ZStripe; 5=ZnMirrorZStripe; 6=coreman; 7=Kaler2Scan; 8=ZStripeUneven; 9=P10-128x4-Z; 10=QiangLiQ8; 11=InversedZStripe (Default: 0)
 --led-pixel-mapper        : Semicolon-separated list of pixel-mappers to arrange pixels.
                                    Optional params after a colon e.g. "U-mapper;Rotate:90"
                                    Available: "Mirror", "Rotate", "U-mapper". Default: ""
 --led-pwm-bits=<1..11>    : PWM bits (Default: 11).
 --led-brightness=<percent>: Brightness in percent (Default: 100).
 --led-scan-mode=<0..1>    : 0 = progressive; 1 = interlaced (Default: 0).
 --led-row-addr-type=<0..4>: 0 = default; 1 = AB-addressed panels; 2 = direct row select; 3 = ABC-addressed panels; 4 = ABC Shift + DE direct (Default: 0).
 --led-show-refresh        : Show refresh rate.
 --led-inverse             : Switch if your matrix has inverse colors on.
 --led-rgb-sequence        : Switch if your matrix has led colors swapped (Default: "RGB")
 --led-pwm-lsb-nanoseconds : PWM Nanoseconds for LSB (Default: 130)
 --led-pwm-dither-bits=<0..2> : Time dithering of lower bits (Default: 0)
 --led-no-hardware-pulse   : Don't use hardware pin-pulse generation.
 --led-panel-type=<name>   : Needed to initialize special panels. Supported: 'FM6126A'
 --led-slowdown-gpio=<0..4>: Slowdown GPIO. Needed for faster Pis/slower panels (Default: 1).
 --led-daemon              : Make the process run in the background as daemon.
 --led-no-drop-privs       : Don't drop privileges from 'root' after initializing the hardware.
```
</details>

### Image Viewer ###

The image viewer reads all kinds of image formats, including animated gifs.

To speed up lengthy loading of image files or animations, you also can also
pre-process images or animations and write them to a 'stream' file that then
later can be loaded very quickly by this viewer (at the expense of disk-space
as these are not compressed). This is in particular useful for large panels
and animations with many frames: less loading time and less RAM used.
See `-O` example below in the example section.

##### Building

The `led-image-viewer` requires the GraphicsMagick dependency first, then
it can be built with `make led-image-viewer`.

```
sudo apt-get update
sudo apt-get install libgraphicsmagick++-dev libwebp-dev -y
make led-image-viewer
```

##### Usage

The resulting binary has a couple of flags.
```
usage: ./led-image-viewer [options] <image> [option] [<image> ...]
Options:
        -O<streamfile>            : Output to stream-file instead of matrix (Don't need to be root).
        -C                        : Center images.

These options affect images FOLLOWING them on the command line,
so it is possible to have different options for each image
        -w<seconds>               : Regular image: Wait time in seconds before next image is shown (default: 1.5).
        -t<seconds>               : For animations: stop after this time.
        -l<loop-count>            : For animations: number of loops through a full cycle.
        -D<animation-delay-ms>    : For animations: override the delay between frames given in the
                                    gif/stream animation with this value. Use -1 to use default value.
        -V<vsync-multiple>        : For animation (expert): Only do frame vsync-swaps on multiples of refresh (default: 1)
                                    (Tip: use --led-limit-refresh for stable rate)

Options affecting display of multiple images:
        -f                        : Forever cycle through the list of files on the command line.
        -s                        : If multiple images are given: shuffle.

General LED matrix options:
        <... all the --led- options>

Switch time between files: -w for static images; -t/-l for animations
Animated gifs: If both -l and -t are given, whatever finishes first determines duration.

The -w, -t and -l options apply to the following images until a new instance of one of these options is seen.
So you can choose different durations for different images.
```

Then, you can run it with any common image format, including animated gifs:

##### Examples

```bash
sudo ./led-image-viewer some-image.jpg       # Display an image.
sudo ./led-image-viewer animated.gif         # Show an animated gif
sudo ./led-image-viewer -t5 animated.gif     # Show an animated gif for 5 seconds
sudo ./led-image-viewer -l2 animated.gif     # Show an animated gif for 2 loops
sudo ./led-image-viewer -D16 animated.gif    # Play animated gif, use 16ms frame delay

# If you want to have an even frame rate, that is depending on your
# refresh rate, use the following. Note, your refresh rate is dependent on
# factors such as chain length and rows; use --led-show-refresh to get an idea.
# Then fix it with --led-limit-refresh
sudo ./led-image-viewer --led-limit-refresh=200 -D0 -V10 animated.gif # Frame rate = 1/12 refresh rate

sudo ./led-image-viewer    -w3 foo.jpg bar.png  # show two images, wait 3 seconds between. Stop.
sudo ./led-image-viewer    -w3 foo.jpg -w2 bar.png baz.png  # show images, wait 3 seconds after the first, 2 seconds after the second and third. Stop.
sudo ./led-image-viewer -f -w3 foo.jpg bar.png  # show images, wait 3sec between, go back and loop forever

sudo ./led-image-viewer -f -w3 *.png *.jpg   # Loop forever through a list of images

sudo ./led-image-viewer -f -s *.png  # Loop forever but randomize (shuffle) each round.

# Show image.png and animated.gif in a loop. Show the static image for 3 seconds
# while the animation is shown for 5 seconds (-t takes precedence for animated
# images over -w)
sudo ./led-image-viewer -f -w3 -t5 image.png animated.gif

# Create a fast animation from a bunch of *.png files
# with 16.6ms frame time (=60Hz) and write to a raw animation stream
# animation-out.stream (beware, uncompressed, uses lots of disk).
# Note:
#  o We have to supply all the options (rows, chain, parallel, hardware-mapping,
#    rotation etc), that we would supply to the real viewer later.
#  o We don't need to be root, as we don't write to the matrix
./led-image-viewer --led-rows=32 --led-chain=4 --led-parallel=3 -w0.016667 *.png -Oanimation-out.stream

# Now, play back this animation.
sudo ./led-image-viewer --led-rows=32 --led-chain=4 --led-parallel=3 animation-out.stream
```

### Text Scroller ###

The text scroller allows to show some scrolling text.

##### Building
```
make text-scroller
```

##### Usage

```
usage: ./text-scroller [options] <text>
Takes text and scrolls it with speed -s
Options:
        -f <font-file>    : Path to *.bdf-font to be used.
        -s <speed>        : Approximate letters per second.
                            Positive: scroll right to left; Negative: scroll left to right
                            (Zero for no scrolling)
        -l <loop-count>   : Number of loops through the text. -1 for endless (default)
        -b <on-time>,<off-time>  : Blink while scrolling. Keep on and off for these amount of scrolled pixels.
        -x <x-origin>     : Shift X-Origin of displaying text (Default: 0)
        -y <y-origin>     : Shift Y-Origin of displaying text (Default: 0)
        -t <track-spacing>: Spacing pixels between letters (Default: 0)

        -C <r,g,b>        : Text Color. Default 255,255,255 (white)
        -B <r,g,b>        : Background-Color. Default 0,0,0
        -O <r,g,b>        : Outline-Color, e.g. to increase contrast.

General LED matrix options:
        <... all the --led- options>
```

You need to specify a font for the tool to use. We are using BDF-fonts, which are bitmap fonts
nicely suited for low-resolution displays such as ours. A few fonts you find in the
[../fonts](../fonts) directory. The [README.md](../fonts/README.md) there also describes
how to make your own.

##### Examples

```bash
# (use your --led-rows, --led-chain and --led-parallel suited for your setup)

# Red (-C) text on a display with 4 chained displays. Notice you can use UTF-8 characters
# if they are supported by the font.
sudo ./text-scroller -f ../fonts/9x18.bdf -C255,0,0 --led-chain=4 "Hello World ♥"

# .. faster speed; roughly 20 characters per second with option -s.
sudo ./text-scroller -f ../fonts/9x18.bdf -C255,0,0 --led-chain=4 -s20 "The quick brown fox jumps over the lazy dog"

# A speed of zero does just shows the text, no scrolling.
sudo ./text-scroller -f ../fonts/9x18.bdf -C255,0,0 --led-chain=4 -s0 "No Scroll"

# A text might need to be arranged a bit. Let's move it 15 pixels to the right and 5 down:
sudo ./text-scroller -f ../fonts/9x18.bdf -C255,0,0 --led-chain=4 -s0 -x15 -y5 "Shifted"

# Now text in red color on a blue background (-B). We choose an outline (-O)
# of a slightly darker blue for better contrast
sudo ./text-scroller -f ../fonts/9x18.bdf -B0,0,255 -O0,0,100 -C255,0,0 --led-chain=4 "Contrast outline"

# A larger font. This one needs a bit of an y-adjustment
# (move up 11 pixels: a negative y shift) to fit nicely on a panel.
sudo ./text-scroller -f ../fonts/texgyre-27.bdf --led-chain=4 -y-11 "Large Font"
```

### Video Viewer ###

The video viewer allows to play common video formats on the RGB matrix (just
the picture, no sound).

This is currently doing a software decode; if you are familiar with the
av libraries, a pull request that adds hardware decoding is welcome.

Right now, this is CPU intensive and decoding can result in an output that
is not smooth or presents flicker, in particular on older Pis.
If you observe that, it is suggested to
prepare a preprocessed stream that you then later watch with `led-image-viewer`
(see example below). This will use a bit of disk-space, but it will result
in best quality as all the expensive calculation has been done beforehand.

Short of that, if you want to use the video viewer directly (e.g. because the
stream file would be super-large), do the following when you observe flicker:
  - Use the `-T` option to add more decode threads; `-T2` or `-T3` typically.
  - Transcode the video first to the width and height of the final output size
    so that decoding and scaling is much cheaper at runtime.
  - If you use tools such as [youtube-dl] to acquire the video, tell it
    to choose a low resolution version (e.g. for that program use option
    `-f"[height<480]"`).
  - Synchronize output as integer fraction of matrix refresh rate (example
    below).
  - Another route to watch videos is to run a [flaschen-taschen]
    server on your Pi, that provides a network interface to your LED-Matrix.
    Now, you can use [vlc] from some other computer on your network and
    stream the output to your Pi.
    You have to provide the IP address and size of the panel:
    ```
      vlc --vout flaschen --flaschen-display=<IP-address-of-your-pi> \
           --flaschen-width=128 --flaschen-height=64 \
           <video-filename-or-YouTube-URL>
    ```

##### Building

The video-viewer requires some dependencies first, then it can be
built with `make video-viewer`.

```
sudo apt-get update
sudo apt-get install pkg-config libavcodec-dev libavformat-dev libswscale-dev
make video-viewer
```

##### Usage

```
Show one or a sequence of video files on the RGB-Matrix
usage: ./video-viewer [options] <video> [<video>...]
Options:
        -F                 : Full screen without black bars; aspect ratio might suffer
        -O<streamfile>     : Output to stream-file instead of matrix (don't need to be root).
        -s <count>         : Skip these number of frames in the beginning.
        -c <count>         : Only show this number of frames (excluding skipped frames).
        -V<vsync-multiple> : Instead of native video framerate, playback framerate
                             is a fraction of matrix refresh. In particular with a stable refresh,
                             this can result in more smooth playback. Choose multiple for desired framerate.
                             (Tip: use --led-limit-refresh for stable rate)
        -T <threads>       : Number of threads used to decode (default 1, max=4)
        -v                 : verbose; prints video metadata and other info.
        -f                 : Loop forever.

General LED matrix options:
        <... all the --led- options>
```

##### Examples

```bash
# Play video. If you observe that the Pi has trouble to keep up (extensive
# flickering), transcode the video first to the exact size of your display.
sudo ./video-viewer --led-chain=4 --led-parallel=3 -T2 myvideo.webm

# If you observe flicker you can try to synchronize video output with
# the refresh rate of the panel. For that, first figure out with
# --led-show-refresh what the 'natural' refresh rate is of your LED panel.
# Then choose one that is lower and a multiple of the frame-rate of the
# video. Let's say we have a video with a framerate of 25fps and find that
# our panel can refresh with more than 200Hz (after the usual refresh
# tweakings such as with --led-pwm-dither-bits).
# Let's fix the refresh rate to 200 and sync a new frame with every
# 8th refresh to get the desired video fps (200/8 = 25)
sudo ./video-viewer --led-chain=4 --led-parallel=3 --led-limit-refresh=200 -V8 myvideo.webm
```

**Example preparing a preprocessed stream**

```bash
# A way to avoid flicker playback with best possible results even with
# very high framerate: create a preprocessed stream first, then replay it with
# led-image-viewer. This results in best quality (no CPU use at play-time), but
# comes with a caveat: It can use _A LOT_ of disk, as it is not compressed.
# Note:
#  o We don't need to be root, as we don't write to the matrix, just to a file.
#  o We have to supply all the options (rows, chain, parallel, hardware-mapping,
#    rotation etc), that we would supply to the real viewer later as the
#    framebuffer is fully pre-processed to avoid any overhead while playing.
#  o You could even run this on your much faster regular Linux PC (little
#    endian) and create a stream that you then can play on your Pi.
# ----- STEP 1 Preprocessing ------
./video-viewer --led-chain=5 --led-parallel=3 -T4 myvideo.mp4 -O/tmp/vid.stream

#.. now play the resulting stream with the with led-image-viewer. Also try
# using -D or -V to replay with different frame rate.
# Note, you need to give the same options (rows, chain, parallel etc.) as
# you did when creating the stream.
# ----- STEP 2 Actual Playing ------
sudo ./led-image-viewer --led-chain=5 --led-parallel=3 /tmp/vid.stream
```

[youtube-dl]: https://youtube-dl.org/
[flaschen-taschen]: https://github.com/hzeller/flaschen-taschen/tree/master/server#rgb-matrix-panel-display
[vlc]: https://www.videolan.org/vlc
