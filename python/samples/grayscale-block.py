#!/usr/bin/env python
from samplebase import SampleBase
import time


class GrayscaleBlock(SampleBase):
    def __init__(self, *args, **kwargs):
        super(GrayscaleBlock, self).__init__(*args, **kwargs)

    def run(self):
        sub_blocks = 16
        width = self.matrix.width
        height = self.matrix.height
        x_step = max(1, width / sub_blocks)
        y_step = max(1, height / sub_blocks)
        count = 0

        while True:
            for y in range(0, height):
                for x in range(0, width):
                    c = sub_blocks * int(y / y_step) + int(x / x_step)
                    if count % 4 == 0:
                        self.matrix.SetPixel(x, y, c, c, c)
                    elif count % 4 == 1:
                        self.matrix.SetPixel(x, y, c, 0, 0)
                    elif count % 4 == 2:
                        self.matrix.SetPixel(x, y, 0, c, 0)
                    elif count % 4 == 3:
                        self.matrix.SetPixel(x, y, 0, 0, c)

            count += 1
            time.sleep(2)


# Main function
if __name__ == "__main__":
    grayscale_block = GrayscaleBlock()
    if (not grayscale_block.process()):
        grayscale_block.print_help()
