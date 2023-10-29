#!/usr/bin/env python
import argparse
from samplebase import SampleBase
from rgbmatrix import graphics
import time


class PlainText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(PlainText, self).__init__(*args, **kwargs)
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
        print("Fade:" + str(self.args.fade))

        x = 0
        y = 0

        if self.args.fade == 'top':
            y = -21
            while(y <= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, white, word)
                time.sleep(0.200)
                y = y + 2
        elif self.args.fade == 'bottom':
            y = 41
            while(y >= 21):
                print("y:" + str(y))
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, white, word)
                time.sleep(0.200)

                y = y - 2
        elif self.args.fade == 'left':
            x = -60
            while(x <= 0):
                canvas.Clear()
                print("x:" + str(x))
                graphics.DrawText(canvas, font, x, 21, white, word)
                time.sleep(0.200)
                x = x + 2
        elif self.args.fade == 'right':
            x = 60
            while(x >= 0):
                canvas.Clear()
                print("x:" + str(x))
                graphics.DrawText(canvas, font, x, 21, white, word)
                time.sleep(0.200)
                x = x - 2
        else:
            graphics.DrawText(canvas, font, 0, 21, white, word)

        time.sleep(10)   # show display for 10 seconds before exit


# Main function
if __name__ == "__main__":
    mainModule = PlainText()
    if (not mainModule.process()):
        mainModule.print_help()
