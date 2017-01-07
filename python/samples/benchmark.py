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
        #self.image.resize((self.matrix.width, self.matrix.height), Image.ANTIALIAS)

        doubleBuffer = self.matrix.CreateFrameCanvas()
        img_width, img_height = self.image.size
      
        nTries = 100
 
        start = time.time() 
        xpos = 0
        tries = 0
        while tries < nTries:
            xpos += 1
            if (xpos > img_width):
                xpos = 0
                tries += 1
            doubleBuffer.SetImage(self.image, -xpos, fastIfPossible=False)
            doubleBuffer.SetImage(self.image, -xpos + img_width, fastIfPossible=False)
            doubleBuffer = self.matrix.SwapOnVSync(doubleBuffer)
        mid = time.time()

        # let's scroll
        xpos = 0
        tries = 0
        while tries < nTries:
            xpos += 1
            if (xpos > img_width):
                xpos = 0
                tries += 1
            doubleBuffer.SetImage(self.image, -xpos, fastIfPossible=True)
            doubleBuffer.SetImage(self.image, -xpos + img_width, fastIfPossible=True)
            doubleBuffer = self.matrix.SwapOnVSync(doubleBuffer)
        end = time.time()	

        print("slow: ", (mid-start)/nTries)
        print("fast: ", (end-mid)/nTries)

# Main function
# e.g. call with
#  sudo ./image-scroller.py --chain=4
# if you have a chain of four
if __name__ == "__main__":
    scroller = ImageScroller(image_file = "../../examples-api-use/runtext16.ppm")
    if (not scroller.process()):
        scroller.print_help()
