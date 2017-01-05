#!/usr/bin/env python
import time
from samplebase import SampleBase
from PIL import Image


class ImageScroller(SampleBase):
    def __init__(self, image_file, *args, **kwargs):
        super(ImageScroller, self).__init__(*args, **kwargs)
        self.image = Image.open(image_file)

    def run(self):
        self.image.resize((self.matrix.width, self.matrix.height), Image.ANTIALIAS)

        double_buffer = self.matrix.CreateFrameCanvas()
        img_width, img_height = self.image.size

        # let's scroll
        xpos = 0
        while True:
            xpos += 1
            if (xpos > img_width):
                xpos = 0
                
            double_buffer.SetImage(self.image, -xpos)
            double_buffer.SetImage(self.image, -xpos + img_width)
        
            double_buffer = self.matrix.SwapOnVSync(double_buffer)
            time.sleep(0.01)

# Main function
# e.g. call with
#  sudo ./image-scroller.py --chain=4
# if you have a chain of four
if __name__ == "__main__":
    image_scroller = ImageScroller(image_file="../../examples-api-use/runtext.ppm")
    if (not image_scroller.process()):
        image_scroller.print_help()
