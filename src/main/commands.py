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

        font_prompt = graphics.Font()
        font_prompt.LoadFont("../../fonts/"+ self.args.font)

        font_command = graphics.Font()
        font_command.LoadFont("../../fonts/6x13.bdf") # + self.args.font)
        white_color = graphics.Color(255,255,255)

        times = 0
        blink = 0

        command_char_ar = []
        command_char_ar.extend( command_selected )

        while(times < 5):
            times = times + 1
            blink = 0
            cmd = ""
            while(blink < 10):
                blink = blink + 1

                if(blink <= len(command_char_ar)):
                    cmd = cmd + command_char_ar[blink-1]

                graphics.DrawText(canvas, font_prompt, 2, 19, white_color, "_")
                graphics.DrawText(canvas, font_command, 10, 21, white_color, cmd)
                time.sleep(0.5)
                canvas.Clear()
                graphics.DrawText(canvas, font_command, 10, 21, white_color, cmd)
                time.sleep(0.5)
                graphics.DrawText(canvas, font_prompt, 2, 19, white_color, "_")


