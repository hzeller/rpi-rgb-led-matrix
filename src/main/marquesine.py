#!/usr/bin/env python
import random
import time

from rgbmatrix import graphics

class Marquesine():
    def show(self, phrase: str):
        try:
            offscreen_canvas = self.matrix.CreateFrameCanvas()
            font = graphics.Font()
            font.LoadFont("../../fonts/" + self.args.font)
            random_color = graphics.Color(random.randint(0,255), random.randint(0,255), random.randint(0,255))
            pos = offscreen_canvas.width

            while True:
                offscreen_canvas.Clear()
                len_word = graphics.DrawText(offscreen_canvas, font, pos, 21, random_color, phrase)
                pos -= 1
                if (pos + len_word < 0):
                    break

                time.sleep(0.03)
                offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)
        except Exception as ex:
            print("Error in MArquesine: " + str(ex))
            return
