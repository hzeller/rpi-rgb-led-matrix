#!/usr/bin/env python

import os
import random
import python_weather
import time

from common import Common
from models.cities import Cities
from PIL import Image
from rgbmatrix import graphics

class Weather():
    async def show_async(self):
        #def show(self, first_line: str, second_line: str, kind: str):

        async with python_weather.Client(unit=python_weather.METRIC) as client:

            city = Cities.get_random_one(self)
            weather = await client.get(city)

            temperature = str(weather.current.temperature) + "ºC"
            city = Common.center_word(self, city)

            canvas = self.matrix
            font = graphics.Font()
            font.LoadFont("../../fonts/7x14B.bdf")

            kind = str(weather.current.kind)
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
            first_line_gap = 15
            max_top_second_line = 26

            if action == 1: #top
                y = max_top_second_line * -1
                while(y <= max_top_second_line):
                    canvas.Clear()
                    y_first_line = y - first_line_gap
                    graphics.DrawText(canvas, font, 1, y_first_line, random_color_first_line, city)
                    graphics.DrawText(canvas, font, 25, y, random_color_second_line, temperature)
                    if os.path.exists(image_weather_path):
                        canvas.SetImage(image_weather.convert('RGB'), 5, y_first_line + 3)
                    time.sleep(0.150)
                    y = y + 2
            elif action == 2:  #'bottom'
                y = 41
                while(y >= max_top_second_line):
                    canvas.Clear()
                    y_first_line = y - first_line_gap
                    graphics.DrawText(canvas, font, 1, y_first_line, random_color_first_line, city)
                    graphics.DrawText(canvas, font, 25, y, random_color_second_line, temperature)
                    if os.path.exists(image_weather_path):
                        canvas.SetImage(image_weather.convert('RGB'), 5, y_first_line + 3)
                    time.sleep(0.150)
                    y = y - 2
            elif action == 3: # 'left'
                x = -60
                y_first_line = max_top_second_line - first_line_gap
                while(x <= 0):
                    canvas.Clear()
                    graphics.DrawText(canvas, font, x, y_first_line, random_color_first_line, city)
                    graphics.DrawText(canvas, font, x + 25, max_top_second_line, random_color_second_line, temperature)
                    if os.path.exists(image_weather_path):
                        canvas.SetImage(image_weather.convert('RGB'), x + 5, max_top_second_line - 13)
                    time.sleep(0.150)
                    x = x + 2
            elif action == 4: #'right':
                x = 60
                y_first_line = max_top_second_line - first_line_gap
                while(x >= 0):
                    canvas.Clear()
                    graphics.DrawText(canvas, font, x, y_first_line, random_color_first_line, first_line)
                    graphics.DrawText(canvas, font, x + 25, max_top_second_line, random_color_second_line, second_line)
                    if os.path.exists(image_weather_path):
                        canvas.SetImage(image_weather.convert('RGB'), x + 5, max_top_second_line - 13)
                    time.sleep(0.150)
                    x = x - 2
            else:
                graphics.DrawText(canvas, font, 0, max_top_second_line, random_color_second_line, second_line)
