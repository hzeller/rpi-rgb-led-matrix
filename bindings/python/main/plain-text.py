#!/usr/bin/env python
import argparse
from common import CommonBase
from rgbmatrix import graphics
import time


class PlainText(CommonBase):
    def __init__(self, *args, **kwargs):
        super(PlainText, self).__init__(*args, **kwargs)

    def getTextToShow(self):
        text = ""
        if self.args.text == True:
            text = self.args.text
            if self.args.centered :
                text = self.args.text.center(self.args.padding)
        elif self.args.clock == True:
            text = time.strftime('%H:%M:%S')
            if self.args.centered :
                text = text.center(self.args.padding)

        return text
    def run(self):
        self.args = self.parser.parse_args()
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)
        white = graphics.Color(255, 255, 255)
        mainModule.log("Fade:" + str(self.args.fade))

        x = 0
        y = 0

        if self.args.fade == 'top':
            y = -21
            while(y <= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, white, mainModule.getTextToShow())
                time.sleep(0.200)
                y = y + 2
        elif self.args.fade == 'bottom':
            y = 41
            while(y >= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, white, mainModule.getTextToShow())
                time.sleep(0.200)
                y = y - 2
        elif self.args.fade == 'left':
            x = -60
            while(x <= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, white, mainModule.getTextToShow())
                time.sleep(0.200)
                x = x + 2
        elif self.args.fade == 'right':
            x = 60
            while(x >= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, white, mainModule.getTextToShow())
                time.sleep(0.200)
                x = x - 2
        else:
            graphics.DrawText(canvas, font, 0, 21, white, mainModule.getTextToShow())

        time.sleep(10)   # show display for 10 seconds before exit


# Main function
if __name__ == "__main__":
    mainModule = PlainText()
    if (not mainModule.process()):
        mainModule.print_help()
