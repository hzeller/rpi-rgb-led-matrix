#!/usr/bin/env python
import argparse
import time
import sys

from rgbmatrix import RGBMatrix, RGBMatrixOptions
from PIL import Image


class ImageViewer():
    def run(self):
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("--image-path", action="store", help="Image path to show", default="/", type=str)
        print("Trying to draw the image...")

    def process(self):
        self.args = self.parser.parse_args()

        image = Image.open(self.args.image_path)

        # Configuration for the matrix
        options = RGBMatrixOptions()

        options.rows = 32
        options.cols = 64
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
    graphics_test = ImageViewer()
    if (not graphics_test.process()):
        graphics_test.print_help()

