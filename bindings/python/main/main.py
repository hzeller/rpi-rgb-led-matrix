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

    def center_word(self, word: str):
        if self.args.centered :
            word = word.center(self.args.padding)
        return word

    def show_text(self, word: str):
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)
        random_color = graphics.Color(random.randint(0,255), random.randint(0,255), random.randint(0,255))

        x = 0
        y = 0

        action = random.randint(1,4)

        if action == 1: #top
            y = -21
            while(y <= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, random_color, word)
                time.sleep(0.200)
                y = y + 2
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

    def show_text_weather(self, first_line: str, second_line: str, kind: str):

        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/7x14B.bdf")

        kind = str(kind)
        kind = kind.upper().replace(" ", "_")
        print(kind)
        image_weather_path = "../img/weather/" + kind + ".PNG"

        if os.path.exists(image_weather_path):
            print('The file ' + image_weather_path + ' exists!')
            image_weather = Image.open(image_weather_path)
            image_weather.thumbnail((15, 15), Image.ANTIALIAS)
        else:
            print('The file ' + image_weather_path + ' does not exist.')
            image_weather_path = "../img/weather/none.png"

        random_color_first_line = graphics.Color(random.randint(0,255), random.randint(0,255), random.randint(0,255))
        random_color_second_line = graphics.Color(random.randint(0,255), random.randint(0,255), random.randint(0,255))

        x = 0
        y = 0

        action = random.randint(1,4)

        action = 3
        first_line_gap = 15
        #temperature = mainModule.center_word(temperature)
        max_top_second_line = 26

        if action == 1: #top
            y = max_top_second_line * -1
            while(y <= max_top_second_line):
                canvas.Clear()
                y_first_line = y - first_line_gap
                graphics.DrawText(canvas, font, 1, y_first_line, random_color_first_line, first_line)
                graphics.DrawText(canvas, font, 25, y, random_color_second_line, second_line)
                if os.path.exists(image_weather_path):
                    canvas.SetImage(image_weather.convert('RGB'), 5, y_first_line + 3)
                time.sleep(0.150)
                y = y + 2
        elif action == 2:  #'bottom'
            y = 41
            while(y >= max_top_second_line):
                canvas.Clear()
                y_first_line = y - first_line_gap
                graphics.DrawText(canvas, font, 1, y_first_line, random_color_first_line, first_line)
                graphics.DrawText(canvas, font, 25, y, random_color_second_line, second_line)
                if os.path.exists(image_weather_path):
                    canvas.SetImage(image_weather.convert('RGB'), 5, y_first_line + 3)
                time.sleep(0.150)
                y = y - 2
        elif action == 3: # 'left'
            x = -60
            y_first_line = max_top_second_line - first_line_gap
            while(x <= 0):
                canvas.Clear()
                x_first_line = x - 24
                graphics.DrawText(canvas, font, x_first_line, y_first_line, random_color_first_line, first_line)
                graphics.DrawText(canvas, font, x, max_top_second_line, random_color_second_line, second_line)
                # if os.path.exists(image_weather_path):
                #     canvas.SetImage(image_weather.convert('RGB'), 5, y_first_line + 3)
                time.sleep(0.150)
                x = x + 2
        elif action == 4: #'right':
            x = 60
            while(x >= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, max_top_second_line, random_color_second_line, second_line)
                time.sleep(0.150)
                x = x - 2
        else:
            graphics.DrawText(canvas, font, 0, max_top_second_line, random_color_second_line, second_line)

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
        font.LoadFont("../../../fonts/" + self.args.font)
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

    async def show_weather_async(self):
        # declare the client. the measuring unit used defaults to the metric system (celcius, km/h, etc.)
        async with python_weather.Client(unit=python_weather.METRIC) as client:
            city = "Barcelona"
            weather = await client.get(city)

            temperature = str(weather.current.temperature) + "ºC"
            city = mainModule.center_word(city)
            mainModule.show_text_weather(city, temperature, weather.current.kind)

    async def run(self):
        self.args = self.parser.parse_args()

        try:
            mainModule.log("Press CTRL-C to stop.")

            randomList=[]
            while(True):

                action = 5
                #action = random.randint(1,5)
                mainModule.log("Selected by random: " + str(action))

                if action in randomList:
                    mainModule.log("Already exist: " + str(action))
                    if len(randomList) == 5:
                        randomList=[]
                else:
                    randomList.append(action)
                    mainModule.log("Clearn selection: " + str(action))
                    mainModule.log(str(action))

                    if action == 1: #Positive Word
                        word_selected = get_positive_word()
                        word_selected = mainModule.center_word(word_selected)
                        mainModule.show_text(word_selected)
                    if action == 2: #Positive Phrase
                        phrase_selected = get_positive_phrase()
                        mainModule.show_marquesine(phrase_selected)
                    elif action == 3: #Show Clock
                        word_selected = time.strftime('%H:%M')
                        word_selected = mainModule.center_word(word_selected)
                        mainModule.show_text(word_selected)
                    elif action == 4: #ppm
                        mainModule.show_ppm()
                    elif action == 5: #Weather
                        await mainModule.show_weather_async()
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
    if (not asyncio.run(mainModule.process())):
        mainModule.print_help()
