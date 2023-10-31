#!/usr/bin/env python
import random
import sys
import time
import asyncio
import os
import python_weather

from words import get_positive_word
from phrases import get_positive_phrase

from common import CommonBase
from rgbmatrix import graphics
from PIL import Image

class PlainText(CommonBase):
    def __init__(self, *args, **kwargs):
        super(PlainText, self).__init__(*args, **kwargs)

    def scale_col(self, val, lo, hi):
        if val < lo:
            return 0
        if val > hi:
            return 255
        return 255 * (val - lo) / (hi - lo)

    def rotate(self, x, y, sin, cos):
        return x * cos - y * sin, x * sin + y * cos

    def prepare_word(self, word):
        if self.args.centered :
            word = word.center(self.args.padding)
        return word

    def show_text(self, word):
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)
        random_color = graphics.Color(random.randint(0,255), random.randint(0,255), random.randint(0,255))

        x = 0
        y = 0

        if self.args.fade == 'top':
            y = -21
            while(y <= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, random_color, word)
                time.sleep(0.200)
                y = y + 2
        elif self.args.fade == 'bottom':
            y = 41
            while(y >= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, random_color, word)
                time.sleep(0.200)
                y = y - 2
        elif self.args.fade == 'left':
            x = -60
            while(x <= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, random_color, word)
                time.sleep(0.200)
                x = x + 2
        elif self.args.fade == 'right':
            x = 60
            while(x >= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, random_color, word)
                time.sleep(0.200)
                x = x - 2
        else:
            graphics.DrawText(canvas, font, 0, 21, random_color, word)

    def show_ppm(self):
        self.image = Image.open("../../../examples-api-use/runtext.ppm").convert('RGB')
        self.image.resize((self.matrix.width-10, self.matrix.height-10), Image.ANTIALIAS)

        double_buffer = self.matrix.CreateFrameCanvas()
        img_width, img_height = self.image.size

        # let's scroll
        xpos = 0
        while xpos < 470:
            xpos += 1

            if (xpos > img_width):
                xpos = 0

            double_buffer.SetImage(self.image, -xpos)
            double_buffer.SetImage(self.image, -xpos + img_width)

            double_buffer = self.matrix.SwapOnVSync(double_buffer)
            time.sleep(0.01)

        self.matrix.Clear()

    def show_marquesine(self, phrase):

        offscreen_canvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)
        random_color = graphics.Color(random.randint(0,255), random.randint(0,255), random.randint(0,255))
        pos = offscreen_canvas.width

        while True:
            offscreen_canvas.Clear()
            len_word = graphics.DrawText(offscreen_canvas, font, pos, 21, random_color, phrase)
            pos -= 1
            if (pos + len_word < 0):
                break

            time.sleep(0.05)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)

    async def show_weather(self):
        # declare the client. the measuring unit used defaults to the metric system (celcius, km/h, etc.)
        async with python_weather.Client(unit=python_weather.METRIC) as client:
            weather = await client.get('Barcelona')

            # returns the current day's forecast temperature (int)
            print(weather.current.temperature)

    def run(self):
        self.args = self.parser.parse_args()

        try:
            mainModule.log("Press CTRL-C to stop.")


            while(True):

                action = 0
                action = random.randint(1,5)
                mainModule.log(str(action))

                action = 5

                if action == 1: #Positive Word
                    word_selected = get_positive_word()
                    word_selected = mainModule.prepare_word(word_selected)
                    mainModule.show_text(word_selected)
                if action == 2: #Positive Phrase
                    phrase_selected = get_positive_phrase()
                    mainModule.show_marquesine(phrase_selected)
                elif action == 3: #Show Clock
                    word_selected = time.strftime('%H:%M')
                    word_selected = mainModule.prepare_word(word_selected)
                    mainModule.show_text(word_selected)
                elif action == 4: #ppm
                    mainModule.show_ppm()
                elif action == 5: #Weather
                    mainModule.show_weather()

                time.sleep(6)   # show display for 10 seconds before exit

        except IOError as e:
            print(e.strerror)
        except KeyboardInterrupt:
            sys.exit(0)

# Main function
if __name__ == "__main__":
    if os.name == 'nt':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    mainModule = PlainText()
    if (not mainModule.process()):
        mainModule.print_help()
