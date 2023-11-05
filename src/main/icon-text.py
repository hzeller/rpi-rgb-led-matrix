#!/usr/bin/env python
import argparse
from common import CommonBase
from rgbmatrix import graphics
import time


class IconText(CommonBase):
    def __init__(self, *args, **kwargs):
        super(IconText, self).__init__(*args, **kwargs)

    def run(self):
        self.args = self.parser.parse_args()
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)

        white = graphics.Color(255, 255, 255)
        word = self.args.text
        if self.args.centered :
            word = word.center(self.args.padding)

        mainModule.log("|" + word + "|")
        mainModule.log("Fade:" + str(self.args.fade))

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
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, white, word)
                time.sleep(0.200)
                y = y - 2
        elif self.args.fade == 'left':
            x = -60
            while(x <= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, white, word)
                time.sleep(0.200)
                x = x + 2
        elif self.args.fade == 'right':
            x = 60
            while(x >= 0):
                canvas.Clear()
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
