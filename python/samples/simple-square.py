#!/usr/bin/env python
from samplebase import SampleBase

class SimpleSquare(SampleBase):
    def __init__(self, *args, **kwargs):
        super(SimpleSquare, self).__init__(*args, **kwargs)

    def Run(self):
        while True:
            for x in range(0, self.matrix.width):
                self.matrix.SetPixel(x, x, 255, 255, 255)
                self.matrix.SetPixel(self.matrix.height - 1 - x, x, 255, 0, 255)

            for x in range(0, self.matrix.width):
                self.matrix.SetPixel(x, 0, 255, 0, 0)
                self.matrix.SetPixel(x, self.matrix.height - 1, 255, 255, 0)

            for y in range(0, self.matrix.height):
                self.matrix.SetPixel(0, y, 0, 0, 255)
                self.matrix.SetPixel(self.matrix.width - 1, y, 0, 255, 0)


# Main function
if __name__ == "__main__":
    parser = SimpleSquare()
    if (not parser.process()):
        parser.print_help()