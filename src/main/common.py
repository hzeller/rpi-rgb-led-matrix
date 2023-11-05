#!/usr/bin/env python
import random
import time

from rgbmatrix import graphics

class Common():
    def center_word(self, word: str):
        if self.args.centered :
            word = word.center(self.args.padding)
        return word

    def show_text(self, word: str):
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../fonts/" + self.args.font)
        random_color = graphics.Color(random.randint(0,255), random.randint(0,255), random.randint(0,255))

        x = 0
        y = 0

        action = random.randint(1,4)

        if action == 1: #top
            y = -21
            while(y <= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, random_color, word)
                time.sleep(0.100)
                y = y + 1
        elif action == 2:  #'bottom'
            y = 41
            while(y >= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, random_color, word)
                time.sleep(0.200)
                y = y - 2
        elif action == 3: # 'left'
            x = -60
            while(x <= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, random_color, word)
                time.sleep(0.200)
                x = x + 2
        elif action == 4: #'right':
            x = 60
            while(x >= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, random_color, word)
                time.sleep(0.200)
                x = x - 2
        else:
            graphics.DrawText(canvas, font, 0, 21, random_color, word)
