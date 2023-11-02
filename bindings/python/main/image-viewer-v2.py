#!/usr/bin/env python
import argparse
import time
import sys

from rgbmatrix import RGBMatrix, RGBMatrixOptions
from PIL import Image

class ImageViewer(object):
    def __init__(self, *args, **kwargs):
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("--image-path", action="store", help="Image path to show", default="/", type=str)
        self.parser.add_argument("--led-rows", action="store", help="Display rows. 16 for 16x32, 32 for 32x32. Default: 32", default=32, type=int)
        self.parser.add_argument("--led-cols", action="store", help="Panel columns. Typically 32 or 64. (Default: 64)", default=64, type=int)

    def process(self):
        self.args = self.parser.parse_args()
        print("Trying to draw the image from " + self.args.image_path)
        print("Rows: " + str(self.args.led_rows))
        print("Cols " + str(self.args.led_cols))

        image = Image.open(self.args.image_path)

        # Configuration for the matrix
        options = RGBMatrixOptions()

        options.rows = self.args.led_rows
        options.cols = self.args.led_cols
        options.chain_length = 1
        options.parallel = 1
        options.hardware_mapping = 'adafruit-hat'

        matrix = RGBMatrix(options = options)

         # Make image fit our screen.
        image.thumbnail((matrix.width, matrix.height), Image.ANTIALIAS)

        matrix.SetImage(image.convert('RGB'))

        try:
            print("Press CTRL-C to stop.")
            while True:
                time.sleep(100)
        except KeyboardInterrupt:
            sys.exit(0)

# Main function
if __name__ == "__main__":
    viewer = ImageViewer()
    if (not viewer.process()):
        viewer.print_help()

