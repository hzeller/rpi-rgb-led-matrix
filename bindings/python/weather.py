#!/usr/bin/env python

from rgbmatrix import graphics
import time
import sys
from PIL import Image
import requests

import logging
log = logging.getLogger(__name__)

import os
path = os.path.dirname(__file__)

class Weather:
    def __init__(self, matrix, loc):
        self.framerate = 1

        self.api_key = "0450c03c3a5647f886b232044230301"
        self.location = loc

        self.canvas = matrix.CreateFrameCanvas()
        self._tmp = matrix.CreateFrameCanvas()
    
    def get_framerate(self):
        return self.framerate

    def show(self):
        self.draw()
        return self.canvas

    def draw(self):
        font = graphics.Font()
        font.LoadFont(path + "/../../fonts/5x6.bdf")
        text_font = graphics.Font()
        white = graphics.Color(255, 255, 255)

        res = requests.get("https://api.weatherapi.com/v1/current.json?key=" + self.api_key + "&q=" + self.location + "&aqi=no").json()
        print(res)
        self.time = res["location"]["localtime"].split(" ")[1]
        self.temp = str(res["current"]["temp_f"]) + ' F'

        width = graphics.DrawText(self._tmp, font, 0, 0, white, self.time)
        graphics.DrawText(self.canvas, font, (self.canvas.width-width)/2, 13, white, self.time)

        width = graphics.DrawText(self._tmp, font, 0, 0, white, self.temp)
        graphics.DrawText(self.canvas, font, (self.canvas.width-width)/2, 23, white, self.temp)
        
