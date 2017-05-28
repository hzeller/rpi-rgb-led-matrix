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
        -C                        : Center images.
        -w<seconds>               : If multiple images given: Wait time between in seconds (default: 1.5).
        -f                        : Forever cycle through the list of files on the command line.
        -t<seconds>               : For animations: stop after this time.
        -l<loop-count>            : For animations: number of loops through a full cycle.
        -s                        : If multiple images are given: shuffle.
        -L                        : Large display, in which each chain is 'folded down'
                                    in the middle in an U-arrangement to get more vertical space.
        -R<angle>                 : Rotate output; steps of 90 degrees
        -O<streamfile>            : Output to stream-flie instead of matrix (Don't need to be root).

General LED matrix options:
        --led-gpio-mapping=<name> : Name of GPIO mapping used. Default "regular"
        --led-rows=<rows>         : Panel rows. 8, 16, 32 or 64. (Default: 32).
        --led-chain=<chained>     : Number of daisy-chained panels. (Default: 1).
        --led-parallel=<parallel> : For A/B+ models or RPi2,3b: parallel chains. range=1..3 (Default: 1).
        --led-pwm-bits=<1..11>    : PWM bits (Default: 11).
        --led-brightness=<percent>: Brightness in percent (Default: 100).
        --led-scan-mode=<0..1>    : 0 = progressive; 1 = interlaced (Default: 0).
        --led-show-refresh        : Show refresh rate.
        --led-inverse             : Switch if your matrix has inverse colors on.
        --led-rgb-sequence        : Switch if your matrix has led colors swapped (Default: "RGB")
        --led-pwm-lsb-nanoseconds : PWM Nanoseconds for LSB (Default: 130)
        --led-no-hardware-pulse   : Don't use hardware pin-pulse generation.
        --led-slowdown-gpio=<0..2>: Slowdown GPIO. Needed for faster Pis and/or slower panels (Default: 1).
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
#  o We do have to supply all the options (rows, chain, parallel,
#    hardware-mapping etc), that we would supply to the real viewer later.
#  o We don't need to be root, as we don't write to the matrix
./led-image-viewer --led-rows=32 --led-chain=4 --led-parallel=3 -w0.01666 *.png -Oanimation-out.stream

# Now, play back this animation.
sudo ./led-image-viewer --led-rows=32 --led-chain=4 --led-parallel=3 animation-out.stream
```
