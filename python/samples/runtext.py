#!/usr/bin/env python
# Display a runtext with double-buffering.
from samplebase import SampleBase
from rgbmatrix import graphics
import time

class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)

    def Run(self):
        offscreenCanvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        if (not font.LoadFont("../../fonts/7x13.bdf")):
            raise("Couldn't load font. Typo in path ?")
        textColor = graphics.Color(255, 255, 0)
        pos = offscreenCanvas.width
        myText = "Hello World!"

        while True:
            offscreenCanvas.Clear()
            len = graphics.DrawText(offscreenCanvas, font, pos, 10, textColor, myText)
            pos -= 1
            if (pos + len < 0):
                pos = offscreenCanvas.width

            time.sleep(0.05)
            offscreenCanvas = self.matrix.SwapOnVSync(offscreenCanvas)


# Main function
if __name__ == "__main__":
    parser = RunText()
    if (not parser.process()):
        parser.print_help()
