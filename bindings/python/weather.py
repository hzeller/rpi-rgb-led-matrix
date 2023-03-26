#!/usr/bin/env python

from rgbmatrix import graphics
import time
import sys
import io
from PIL import Image
import requests
import schedule
from datetime import datetime
import zoneinfo
from apscheduler.schedulers.background import BackgroundScheduler

from secrets import LOCAL_TZ

import logging
log = logging.getLogger(__name__)

import os
path = os.path.dirname(__file__)

schedule = BackgroundScheduler(daemon=True)
schedule.start()


class Weather:
    def __init__(self, matrix, lat, lon):
        self.framerate = 1

        self.api_key = "ba038fd5f64ea1f3741809c0bf36bd1d"
        self.lat = lat
        self.lon = lon
        self.refresh = 5

        self.temp = "--°F"
        self.icon_url = "https://openweathermap.org/img/wn/03n@2x.png"
        self.icon = requests.get(self.icon_url)

        self.matrix = matrix

        schedule.add_job(self._get_weather_data)
        schedule.add_job(self._get_weather_data, 'interval', minutes=self.refresh)

    def _get_weather_data(self):
        try:
            r = requests.get(f"https://api.openweathermap.org/data/3.0/onecall?lat={self.lat}&lon={self.lon}&exclude=hourly,daily&units=imperial&appid={self.api_key}", timeout=2)
            raw = r.json()['current']

            self.temp = str(round(raw['temp'])) + "°F"
            self.icon_url = "http://openweathermap.org/img/wn/" + raw['weather'][0]['icon'] + "@2x.png"
            self.icon = requests.get(self.icon_url, timeout=2)

            log.info("_get_weather_data: Temp: %s, Icon URL: %s" % (self.temp, self.icon_url))
        except:
            log.warning("_get_weather_data: exception occurred")

    def get_framerate(self):
        return self.framerate

    def show(self):
        return self.draw()

    def draw(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        _tmp_canvas = self.matrix.CreateFrameCanvas()

        font = graphics.Font()
        font.LoadFont(path + "/../../fonts/5x6.bdf")
        text_font = graphics.Font()
        white = graphics.Color(255, 255, 255)

        image = Image.open(io.BytesIO(self.icon.content))
        image.thumbnail((18, 18))
        icon = image.convert('RGB')
        offscreen_canvas.SetImage(icon, (offscreen_canvas.width-18)/2, 2)

        width = graphics.DrawText(_tmp_canvas, font, 0, 0, white, self.temp)
        graphics.DrawText(offscreen_canvas, font, (offscreen_canvas.width-width)/2, 24, white, self.temp)

        return offscreen_canvas
        
