#!/usr/bin/env python
import argparse
from common import CommonBase
from rgbmatrix import graphics
import time
import sys

class Clock(CommonBase):
    def __init__(self, *args, **kwargs):
        super(Clock, self).__init__(*args, **kwargs)
        self.parser.add_argument("--padding", action="store", help="Padding of the word. Default: 32", default=8, type=int)
        self.parser.add_argument("--font", action="store", help="Font of the word. Default: 7x13.bdf", default="7x13.bdf", type=str)

    def run(self):
        self.args = self.parser.parse_args()
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)

        white = graphics.Color(255, 255, 255)

        counter = 0
        while(True):
            counter = counter + 1
            timestamp = time.strftime('%H:%M:%S')
            timestamp = timestamp.center(self.args.padding)
            print("|" + timestamp + "|")
            graphics.DrawText(canvas, font, 0, 21, white, timestamp)
            time.sleep(1)
            canvas.Clear()
            if counter == 10:
                sys.exit(0)


# Main function
if __name__ == "__main__":
    clock = Clock()
    if (not clock.process()):
        clock.print_help()
