Utilities
=========

This contains useful utilities that might be directly useful without having
to write any code.

### Image Viewer ###

The image viewer reads all kinds of image formats, including animated gifs.

To speed up lengthy loading of image files or animations, you also can also
pre-process images or animations and write them to a 'stream' file that then
later can be loaded very quickly by this viewer (at the expense of disk-space
as these are not compressed). This is in particular useful for large panels
and animations with many frames: less loading time and less RAM used.
See `-O` example below in the example section.

To compile, you first need to install the GraphicsMagick dependencies first:

```
sudo apt-get update
sudo apt-get install libgraphicsmagick++-dev libwebp-dev -y
make led-image-viewer
```

The resulting binary has a couple of flags.
```
usage: ./led-image-viewer [options] <image> [option] [<image> ...]
Options:
        -O<streamfile>            : Output to stream-file instead of matrix (Don't need to be root).
        -C                        : Center images.

These options affect images following them on the command line:
        -w<seconds>               : Regular image: Wait time in seconds before next image is shown (default: 1.5).
        -t<seconds>               : For animations: stop after this time.
        -l<loop-count>            : For animations: number of loops through a full cycle.
        -D<animation-delay-ms>    : For animations: override the delay between frames given in the
                                    gif/stream animation with this value. Use -1 to use default value.

Options affecting display of multiple images:
        -f                        : Forever cycle through the list of files on the command line.
        -s                        : If multiple images are given: shuffle.

Display Options:
        -V<vsync-multiple>        : Expert: Only do frame vsync-swaps on multiples of refresh (default: 1)

General LED matrix options:
        --led-gpio-mapping=<name> : Name of GPIO mapping used. Default "regular"
        --led-rows=<rows>         : Panel rows. Typically 8, 16, 32 or 64. (Default: 32).
        --led-cols=<cols>         : Panel columns. Typically 32 or 64. (Default: 32).
        --led-chain=<chained>     : Number of daisy-chained panels. (Default: 1).
        --led-parallel=<parallel> : Parallel chains. range=1..3 (Default: 1).
        --led-multiplexing=<0..6> : Mux type: 0=direct; 1=Stripe; 2=Checkered; 3=Spiral; 4=ZStripe; 5=ZnMirrorZStripe; 6=coreman (Default: 0)
        --led-pixel-mapper        : Semicolon-separated list of pixel-mappers to arrange pixels.
                                    Optional params after a colon e.g. "U-mapper;Rotate:90"
                                    Available: "Rotate", "U-mapper". Default: ""
        --led-pwm-bits=<1..11>    : PWM bits (Default: 11).
        --led-brightness=<percent>: Brightness in percent (Default: 100).
        --led-scan-mode=<0..1>    : 0 = progressive; 1 = interlaced (Default: 0).
        --led-row-addr-type=<0..2>: 0 = default; 1 = AB-addressed panels; 2 = direct row select(Default: 0).
        --led-show-refresh        : Show refresh rate.
        --led-inverse             : Switch if your matrix has inverse colors on.
        --led-rgb-sequence        : Switch if your matrix has led colors swapped (Default: "RGB")
        --led-pwm-lsb-nanoseconds : PWM Nanoseconds for LSB (Default: 130)
        --led-no-hardware-pulse   : Don't use hardware pin-pulse generation.
        --led-slowdown-gpio=<0..2>: Slowdown GPIO. Needed for faster Pis/slower panels (Default: 1).
        --led-daemon              : Make the process run in the background as daemon.
        --led-no-drop-privs       : Don't drop privileges from 'root' after initializing the hardware.

Switch time between files: -w for static images; -t/-l for animations
Animated gifs: If both -l and -t are given, whatever finishes first determines duration.

The -w, -t and -l options apply to the following images until a new instance of one of these options is seen.
So you can choose different durations for different images.
```

Then, you can run it with any common image format, including animated gifs:

Examples:
```bash
sudo ./led-image-viewer some-image.jpg       # Display an image.
sudo ./led-image-viewer animated.gif         # Show an animated gif
sudo ./led-image-viewer -t5 animated.gif     # Show an animated gif for 5 seconds
sudo ./led-image-viewer -l2 animated.gif     # Show an animated gif for 2 loops
sudo ./led-image-viewer -D16 animated.gif    # Play animated gif, use 16ms frame delay

# If you want to have an even frame rate, that is depending on your
# refresh rate, use the following. Note, your refresh rate is dependent on
# factors such as chain length and rows; use --led-show-refresh to get an idea.
sudo ./led-image-viewer -D0 -V12 animated.gif # Frame rate = 1/12 refresh rate

sudo ./led-image-viewer    -w3 foo.jpg bar.png  # show two images, wait 3 seconds between. Stop.
sudo ./led-image-viewer    -w3 foo.jpg -w2 bar.png baz.png  # show images, wait 3 seconds after the first, 2 seconds after the second and third. Stop.
sudo ./led-image-viewer -f -w3 foo.jpg bar.png  # show images, wait 3sec between, go back and loop forever

sudo ./led-image-viewer -f -w3 *.png *.jpg   # Loop forever through a list of images

sudo ./led-image-viewer -f -s *.png  # Loop forever but randomize (shuffle) each round.

# Show image.png and animated.gif in a loop. Show the static image for 3 seconds
# while the animation is shown for 5 seconds (-t takes precendence for animated
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

### Video Viewer ###

The video viewer allows to play common video formats on the RGB matrix (just
the picture, no sound).

Note, this is CPU intensive and decoding can result in an output that is not
smooth. If you observe that, it is suggested to do one of these:

  - Transcode the video first to the width and height of the final output size.
  - Prepare an animation stream that you then later watch with led-image-viewer
    (see example below).

```
sudo apt-get update
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev
make video-viewer
```

```
usage: ./video-viewer [options] <video>
Options:
        -O<streamfile>     : Output to stream-file instead of matrix (don't need to be root).
        -v                 : verbose.

General LED matrix options:
        --led-gpio-mapping=<name> : Name of GPIO mapping used. Default "regular"
        --led-rows=<rows>         : Panel rows. Typically 8, 16, 32 or 64. (Default: 32).
        --led-cols=<cols>         : Panel columns. Typically 32 or 64. (Default: 32).
        --led-chain=<chained>     : Number of daisy-chained panels. (Default: 1).
        --led-parallel=<parallel> : Parallel chains. range=1..3 (Default: 1).
        --led-multiplexing=<0..6> : Mux type: 0=direct; 1=Stripe; 2=Checkered; 3=Spiral; 4=ZStripe; 5=ZnMirrorZStripe; 6=coreman (Default: 0)
        --led-pixel-mapper        : Semicolon-separated list of pixel-mappers to arrange pixels.
                                    Optional params after a colon e.g. "U-mapper;Rotate:90"
                                    Available: "Rotate", "U-mapper". Default: ""
        --led-pwm-bits=<1..11>    : PWM bits (Default: 11).
        --led-brightness=<percent>: Brightness in percent (Default: 100).
        --led-scan-mode=<0..1>    : 0 = progressive; 1 = interlaced (Default: 0).
        --led-row-addr-type=<0..2>: 0 = default; 1 = AB-addressed panels; 2 = direct row select(Default: 0).
        --led-show-refresh        : Show refresh rate.
        --led-inverse             : Switch if your matrix has inverse colors on.
        --led-rgb-sequence        : Switch if your matrix has led colors swapped (Default: "RGB")
        --led-pwm-lsb-nanoseconds : PWM Nanoseconds for LSB (Default: 130)
        --led-no-hardware-pulse   : Don't use hardware pin-pulse generation.
        --led-slowdown-gpio=<0..2>: Slowdown GPIO. Needed for faster Pis/slower panels (Default: 1).
        --led-daemon              : Make the process run in the background as daemon.
        --led-no-drop-privs       : Don't drop privileges from 'root' after initializing the hardware.
```

Examples:
```bash
# Play video. If you observe that the Pi has trouble to keep up (extensive
# flickering), transcode the video first to the exact size of your display.
sudo ./video-viewer --led-chain=4 --led-parallel=3 myvideo.webm

# Another way to avoid flicker playback with best possible results even with
# very high framerate: create a preprocessed stream first, then replay it with
# led-image-viewer. This results in best quality (no CPU use at play-time), but
# comes with a caveat: It can use _A LOT_ of disk, as it is not compressed.
# Note:
#  o We have to supply all the options (rows, chain, parallel, hardware-mapping,
#    rotation etc), that we would supply to the real viewer later.
#  o We don't need to be root, as we don't write to the matrix
./video-viewer --led-chain=5 --led-parallel=3 myvideo.webm -O/tmp/vid.stream

#.. now play it with led-image-viewer. Also try using -D or -V to replay with
# different frame rate.
sudo ./led-image-viewer --led-chain=5 --led-parallel=3 /tmp/vid.stream
```