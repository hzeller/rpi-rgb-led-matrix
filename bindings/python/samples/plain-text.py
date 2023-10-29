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
        self.parser.add_argument("--fade", action="store", help="Animation Fade. Default: top", default="top", type=str)

    def run(self):
        self.args = self.parser.parse_args()
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)
        white = graphics.Color(255, 255, 255)
        word = self.args.text
        if self.args.centered :
            word = word.center(self.args.padding)

        print("|" + word + "|")

        x = 0
        y = 0

        if self.args.padding == 'top':
            y = -21
            while(y <= 21):
                graphics.DrawText(canvas, font, 0, y, white, word)
                time.sleep(0.200)
                canvas.Clear()
                y = y+1
        elif self.args.padding == 'bottom':
            y = 41
            while(y >= 21):
                print("y:" + str(y))
                graphics.DrawText(canvas, font, 0, y, white, word)
                time.sleep(0.200)
                canvas.Clear()
                y = y-1
        elif self.args.padding == 'left':
            x = -10
            while(x <= 0):
                print("x:" + str(x))
                graphics.DrawText(canvas, font, x, 21, white, word)
                time.sleep(0.200)
                canvas.Clear()
                x = x + 1
        elif self.args.padding == 'right':
            x = 20
            while(x >= 0):
                print("x:" + str(x))
                graphics.DrawText(canvas, font, x, 21, white, word)
                time.sleep(0.200)
                canvas.Clear()
                x = x - 1
        else:
            graphics.DrawText(canvas, font, 0, 21, white, word)

        time.sleep(20)   # show display for 10 seconds before exit


# Main function
if __name__ == "__main__":
    graphics_test = GraphicsTest()
    if (not graphics_test.process()):
        graphics_test.print_help()
