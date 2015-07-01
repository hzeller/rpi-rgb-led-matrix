#!/usr/bin/env python
from samplebase import SampleBase

class PulsingColors(SampleBase):
    def __init__(self, *args, **kwargs):
        super(PulsingColors, self).__init__(*args, **kwargs)

    def Run(self):
        self.offscreenCanvas = self.matrix.CreateFrameCanvas()
        continuum = 0

        while True:
            self.usleep(5 * 1000)
            continuum += 1
            continuum %= 3 * 255

            red = 0
            green = 0
            blue = 0

            if continuum <= 255:
                c = continuum
                blue = 255 - c
                red = c
            elif continuum > 255 and continuum <= 511:
                c = continuum - 256
                red = 255 - c
                green = c
            else:
                c = continuum - 512
                green = 255 - c
                blue = c

            self.offscreenCanvas.Fill(red, green, blue)
            self.offscreenCanvas = self.matrix.SwapOnVSync(self.offscreenCanvas)

# Main function
if __name__ == "__main__":
    parser = PulsingColors()
    if (not parser.process()):
        parser.print_help()