Utilities
=========

This contains useful utilities that might be directly useful without having
to write any code.

### Image Viewer ###

The image viewer reads all kinds of image formats, including animated gifs.
To compile, you first need to install the GraphicsMagick dependencies first:

```
sudo apt-get update
sudo apt-get install libgraphicsmagick++-dev libwebp-dev -y
make led-image-viewer
```

The resulting binary has a couple of flags.
```
usage: ./led-image-viewer [options] <image> [<image> ...]
Options:
        -C                        : Center images.
        -w<seconds>               : If multiple images given: Wait time between in seconds (default: 1.5).
        -f                        : Forever cycle through the list of files on the command line.
        -t<seconds>               : For gif animations: stop after this time.
        -l<loop-count>            : For gif animations: number of loops through a full cycle.
        -L                        : 64x64 large display made out of chain of four 32x32

General LED matrix options:
        --led-rows=<rows>         : Panel rows. 8, 16, 32 or 64. (Default: 32).
        --led-chain=<chained>     : Number of daisy-chained panels. (Default: 1).
        --led-parallel=<parallel> : For A/B+ models or RPi2,3b: parallel chains. range=1..3 (Default: 1).

   # more options for led matrix ...

Switch time between files: -w for static images; -t/-l for animations
Animated gifs: If both -l and -t are given, whatever comes first determines duration.
```

Then, you can run it with any common image format, including animated gifs:

Examples:
```bash
sudo ./led-image-viewer some-image.jpg       # Display an image.
sudo ./led-image-viewer animated.gif         # Show an animated gif
sudo ./led-image-viewer -t5 animated.gif     # Show an animated gif for 5 seconds
sudo ./led-image-viewer -l2 animated.gif     # Show an animated gif for 2 loops

sudo ./led-image-viewer    -w3 foo.jpg bar.png  # show two images, wait 3 seconds between. Stop.
sudo ./led-image-viewer -f -w3 foo.jpg bar.png  # show images, wait 3sec between, go back and loop forever

sudo ./led-image-viewer -f -w3 *.png *.jpg   # Loop forever through a list of images

# Show image.png and animated.gif in a loop. Show the static image for 3 seconds
# while the animation is shown for 5 seconds (-t takes precendence for animated
# images over -w)
sudo ./led-image-viewer -f -w3 -t5 image.png animated.gif
```

It also supports the standard options to specify the connected
displays (e.g. `--led-rows`, `--led-chain`, `--led-parallel` and
[all other led-libray flags](../README.md#changing-parameters-via-command-line-flags))
