#!/usr/bin/env python

from rgbmatrix import graphics
import time
import sys
from PIL import Image
import requests
import schedule
from datetime import datetime

import logging
log = logging.getLogger(__name__)

import os
path = os.path.dirname(__file__)

class Weather:
    def __init__(self, matrix, lat, lon):
        self.framerate = 1

        self.api_key = "ba038fd5f64ea1f3741809c0bf36bd1d"
        self.lat = lat
        self.lon = lon

        self.canvas = matrix.CreateFrameCanvas()
        self.offscreen_canvas = matrix.CreateFrameCanvas()
        self._tmp = matrix.CreateFrameCanvas()

        self.draw()
        schedule.every(1).minutes.do(self.draw).tag('weather')
    
    def get_framerate(self):
        return self.framerate

    def show(self):
        return self.canvas

    def draw(self):
        self.offscreen_canvas.Clear()
        font = graphics.Font()
        font.LoadFont(path + "/../../fonts/5x6.bdf")
        text_font = graphics.Font()
        white = graphics.Color(255, 255, 255)

        res = requests.get(f"https://api.openweathermap.org/data/3.0/onecall?lat={self.lat}&lon={self.lon}&exclude=hourly,daily&appid={self.api_key}").json()
        self.time = str(datetime.fromtimestamp(res["current"]["dt"])).split(" ")[1]
        self.temp = str(res["current"]["temp"]) + '°F'

        width = graphics.DrawText(self._tmp, font, 0, 0, white, self.time)
        graphics.DrawText(self.offscreen_canvas, font, (self.canvas.width-width)/2, 13, white, self.time)

        width = graphics.DrawText(self._tmp, font, 0, 0, white, self.temp)
        graphics.DrawText(self.offscreen_canvas, font, (self.canvas.width-width)/2, 23, white, self.temp)
        self.canvas = self.offscreen_canvas
        
