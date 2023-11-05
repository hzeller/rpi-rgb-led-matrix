#!/usr/bin/env python
import argparse
from base import Base
from rgbmatrix import graphics
import time
import sys
from PIL import Image

class Clock(Base):
    def __init__(self, *args, **kwargs):
        super(Clock, self).__init__(*args, **kwargs)

    def run(self):
        self.args = self.parser.parse_args()
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../fonts/" + self.args.font)

        white = graphics.Color(255, 255, 255)

        counter = 0
        cur_frame = 0
        while(True):
            counter = counter + 1
            timestamp = time.strftime('%H:%M:%S')
            timestamp = timestamp.center(self.args.padding)
            mainModule.log("|" + timestamp + "|")
            graphics.DrawText(canvas, font, 5, 21, white, timestamp)
            time.sleep(1)
            canvas.Clear()
            if counter == 10:
                 sys.exit(0)


# Main function
if __name__ == "__main__":
    mainModule = Clock()
    if (not mainModule.process()):
        mainModule.print_help()
