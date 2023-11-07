#!/usr/bin/env python
import time

from rgbmatrix import graphics
from models.commands import get_command

class Command:
    def get(self):
        return get_command()

    def show(self):
        command_selected = Command.get(self)

        print("command selected: " + command_selected)

        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../fonts/" + self.args.font)
        white_color = graphics.Color(255,255,255)

        times = 0
        blink = 0

        while(times < 3):
            times = times + 1
            blink = 0
            while(blink < 10):
                blink = blink + 1
                graphics.DrawText(canvas, font, 2, 21, white_color, "_")
                time.sleep(1)
                canvas.Clear()
                time.sleep(1)

            time.sleep(5)


