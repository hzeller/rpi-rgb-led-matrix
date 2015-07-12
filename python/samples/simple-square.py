#!/usr/bin/env python
from samplebase import SampleBase

class SimpleSquare(SampleBase):
    def __init__(self, *args, **kwargs):
        super(SimpleSquare, self).__init__(*args, **kwargs)

    def Run(self):
        offsetCanvas = self.matrix.CreateFrameCanvas()
        while True:
            for x in range(0, self.matrix.width):
                offsetCanvas.SetPixel(x, x, 255, 255, 255)
                offsetCanvas.SetPixel(offsetCanvas.height - 1 - x, x, 255, 0, 255)

            for x in range(0, offsetCanvas.width):
                offsetCanvas.SetPixel(x, 0, 255, 0, 0)
                offsetCanvas.SetPixel(x, offsetCanvas.height - 1, 255, 255, 0)

            for y in range(0, offsetCanvas.height):
                offsetCanvas.SetPixel(0, y, 0, 0, 255)
                offsetCanvas.SetPixel(offsetCanvas.width - 1, y, 0, 255, 0)
            offsetCanvas = self.matrix.SwapOnVSync(offsetCanvas)


# Main function
if __name__ == "__main__":
    parser = SimpleSquare()
    if (not parser.process()):
        parser.print_help()