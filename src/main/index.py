#!/usr/bin/env python
import random
import asyncio
import time
import sys
import os

from common import Common
from positiveword import PositiveWords
from positivephrase import PositivePhrases
from weather import Weather

from base import Base
from rgbmatrix import graphics
from PIL import Image

class Index(Base):
    def __init__(self, *args, **kwargs):
        super(Index, self).__init__(*args, **kwargs)

    def scale_col(self, val, lo, hi):
        if val < lo:
            return 0
        if val > hi:
            return 255
        return 255 * (val - lo) / (hi - lo)

    def rotate(self, x, y, sin, cos):
        return x * cos - y * sin, x * sin + y * cos

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

    def put_square(self):
        offset_canvas = self.matrix.CreateFrameCanvas()

        #Doble cuadrado
        for x in range(0, self.matrix.width):
            offset_canvas.SetPixel(x, 0, 255, 255, 255)
            offset_canvas.SetPixel(x, 1, 255, 255, 255)

        for x in range(0, self.matrix.width):
            offset_canvas.SetPixel(x, self.matrix.height-1, 255, 255, 255)
            offset_canvas.SetPixel(x, self.matrix.height-2, 255, 255, 255)

        for y in range(0, self.matrix.height):
            offset_canvas.SetPixel(0, y, 255, 255, 255)
            offset_canvas.SetPixel(1, y, 255, 255, 255)

        for y in range(0, self.matrix.height):
            offset_canvas.SetPixel(self.matrix.width-1, y, 255, 255, 255)
            offset_canvas.SetPixel(self.matrix.width-2, y, 255, 255, 255)

        offset_canvas = self.matrix.SwapOnVSync(offset_canvas)

    def show_marquesine(self, phrase: str):

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

    async def run(self):
        self.args = self.parser.parse_args()

        try:
            mainModule.log("Press CTRL-C to stop.")
            moods_count = 4
            randomList=[]
            while(True):

                action = random.randint(1,moods_count)
                mainModule.log("Selected by random: " + str(action))

                if action in randomList:
                    mainModule.log("Already exist: " + str(action))
                    if len(randomList) == moods_count:
                        randomList=[]
                else:
                    randomList.append(action)
                    mainModule.log("Clear selection: " + str(action))
                    mainModule.log(str(action))

                    if action == 1: #Positive Word
                        word_selected = PositiveWords.get(self)
                        word_selected = Common.center_word(self, word_selected)
                        Common.show_text(self, word_selected)
                    if action == 2: #Positive Phrase
                        phrase_selected = PositivePhrases.get(self)
                        mainModule.show_marquesine(phrase_selected)
                    elif action == 3: #Show Clock
                        word_selected = time.strftime('%H:%M')
                        word_selected = Common.center_word(self, word_selected)
                        Common.show_text(self, word_selected)
                    elif action == 4: #Weather
                        await Weather.show_async(self)
                    time.sleep(6)   # show display for 10 seconds before exit

        except IOError as e:
            print(e.strerror)
        except KeyboardInterrupt:
            sys.exit(0)

# Main function
if __name__ == "__main__":
    if os.name == 'nt':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    mainModule = Index()
    if (not asyncio.run(mainModule.process())):
        mainModule.print_help()

