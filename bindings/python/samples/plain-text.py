#!/usr/bin/env python
import argparse
from samplebase import SampleBase
from rgbmatrix import graphics
import time


class GraphicsTest(SampleBase):
    def __init__(self, *args, **kwargs):
        super(GraphicsTest, self).__init__(*args, **kwargs)
        self.parser.add_argument("--text", action="store", help="Text to show", default="AMOR", type=str)
        self.parser.add_argument("--padding", action="store", help="Padding of the word. Default: 32", default=10, type=int)
        self.parser.add_argument("--font", action="store", help="Font of the word. Default: 7x13.bdf", default="7x13.bdf", type=str)
        self.parser.add_argument("--centered", action="store", help="Center the word. Default: true", default="True", type=bool)

    def run(self):
        self.args = self.parser.parse_args()
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)

        # red = graphics.Color(255, 0, 0)
        # graphics.DrawLine(canvas, 5, 5, 22, 13, red)

        # green = graphics.Color(0, 255, 0)
        # graphics.DrawCircle(canvas, 15, 15, 10, green)

        #blue = graphics.Color(0, 0, 255)
        white = graphics.Color(255, 255, 255)
        word = self.args.text
        if self.args.centered :
            word = word.center(self.args.padding)

        print("|" + word + "|")
        graphics.DrawText(canvas, font, 0, 20, white, word)

        time.sleep(20)   # show display for 10 seconds before exit


# Main function
if __name__ == "__main__":
    graphics_test = GraphicsTest()
    if (not graphics_test.process()):
        graphics_test.print_help()
