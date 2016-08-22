### Image Viewer ###

The image viewer reads all kinds of image formats, including animated gifs.
It is not compiled by default, as you need to install the GraphicsMagick
dependencies first:

     sudo apt-get update
     sudo apt-get install libgraphicsmagick++-dev libwebp-dev
     make led-image-viewer

Then, you can run it with any common image format, including animated gifs:

    sudo ./led-image-viewer myimage.gif

It also supports the standard options to specify the connected
displays (e.g. `--led-rows`, `--led-chain`, `--led-parallel` but also many more tweak
flags).
