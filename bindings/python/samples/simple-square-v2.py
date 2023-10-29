#!/usr/bin/env python
import time
import sys
from samplebase import SampleBase


class SimpleSquare(SampleBase):
    def __init__(self, *args, **kwargs):
        super(SimpleSquare, self).__init__(*args, **kwargs)

    def run(self):
        offset_canvas = self.matrix.CreateFrameCanvas()
        #while True:
        print ("matrix.width: " + str(self.matrix.width))
        print ("matrix.height: " + str(self.matrix.height))

        #Doble cuadrado
        for x in range(0, self.matrix.width):
            offset_canvas.SetPixel(x, 0, 255, 255, 255)
            offset_canvas.SetPixel(x, 1, 255, 255, 255)

        for x in range(0, self.matrix.width):
            offset_canvas.SetPixel(x, self.matrix.height-1, 255, 255, 255)
            offset_canvas.SetPixel(x, self.matrix.height-2, 255, 255, 255)

        for y in range(0, self.matrix.height):
            offset_canvas.SetPixel(0, y, 255, 255, 255)
            offset_canvas.SetPixel(1, y, 255, 255, 255)

        for y in range(0, self.matrix.height):
            offset_canvas.SetPixel(self.matrix.width-1, y, 255, 255, 255)
            offset_canvas.SetPixel(self.matrix.width-2, y, 255, 255, 255)

        offset_canvas = self.matrix.SwapOnVSync(offset_canvas)

        # while True:
        #     for x in range(0, self.matrix.width):
        #         offset_canvas.SetPixel(x, x, 255, 255, 255)
        #         offset_canvas.SetPixel(offset_canvas.height - 1 - x, x, 255, 0, 255)

        #     for x in range(0, offset_canvas.width):
        #         offset_canvas.SetPixel(x, 0, 255, 0, 0)
        #         offset_canvas.SetPixel(x, offset_canvas.height - 1, 255, 255, 0)

        #     for y in range(0, offset_canvas.height):
        #         offset_canvas.SetPixel(0, y, 0, 0, 255)
        #         offset_canvas.SetPixel(offset_canvas.width - 1, y, 0, 255, 0)
        #     offset_canvas = self.matrix.SwapOnVSync(offset_canvas)

        try:
            print("Press CTRL-C to stop.")
            while True:
                time.sleep(100)
        except KeyboardInterrupt:
            sys.exit(0)

# Main function
if __name__ == "__main__":
    simple_square = SimpleSquare()
    if (not simple_square.process()):
        simple_square.print_help()