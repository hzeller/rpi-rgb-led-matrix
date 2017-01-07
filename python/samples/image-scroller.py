#!/usr/bin/env python
import time
from samplebase import SampleBase
from rgbmatrix import RGBMatrix
from PIL import Image
import numpy as np

class ImageScroller(SampleBase):
    def __init__(self, image_file, *args, **kwargs):
        super(ImageScroller, self).__init__(*args, **kwargs)
        self.image = Image.open(image_file).convert('RGB')

    def Run(self):
        self.image.resize((self.matrix.width, self.matrix.height), Image.ANTIALIAS)

        doubleBuffer = self.matrix.CreateFrameCanvas()
        img_width, img_height = self.image.size

        # let's scroll
        xpos = 0
        while True:
            xpos += 1
            if (xpos > img_width):
                xpos = 0
            doubleBuffer.SetImage(self.image, -xpos)
            doubleBuffer.SetImage(self.image, -xpos + img_width)
        
            doubleBuffer = self.matrix.SwapOnVSync(doubleBuffer)
            time.sleep(0.01)

# Main function
# e.g. call with
#  sudo ./image-scroller.py --chain=4
# if you have a chain of four
if __name__ == "__main__":
    scroller = ImageScroller(image_file = "../../examples-api-use/runtext.ppm")
    if (not scroller.process()):
        scroller.print_help()
