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

        command_arr = bytes(command_selected, 'utf-8')

        while(times < 3):
            times = times + 1
            blink = 0
            cmd = ""
            while(blink < 10):
                blink = blink + 1
                cmd = cmd + str(command_arr[blink], 'UTF-8')
                graphics.DrawText(canvas, font, 2, 19, white_color, "_")
                graphics.DrawText(canvas, font, 12, 21, white_color, cmd)
                time.sleep(1)
                canvas.Clear()
                graphics.DrawText(canvas, font, 12, 21, white_color, cmd)
                time.sleep(1)

            time.sleep(5)


