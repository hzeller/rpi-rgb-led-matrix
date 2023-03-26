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
    def __init__(self, offscreen_canvas, lat, lon):
        self.framerate = 1
        self.offscreen_canvas = offscreen_canvas

        self.api_key = "ba038fd5f64ea1f3741809c0bf36bd1d"
        self.lat = lat
        self.lon = lon
        self.refresh = 5

        self.temp = "--°F"
        self.icon_url = "https://openweathermap.org/img/wn/03n@2x.png"
        self.icon = requests.get(self.icon_url)

        schedule.add_job(self._get_weather_data)
        schedule.add_job(self._get_weather_data, 'interval', minutes=self.refresh)

    def _get_weather_data(self):
        raw = None
        try:
            r = requests.get(f"https://api.openweathermap.org/data/3.0/onecall?lat={self.lat}&lon={self.lon}&exclude=hourly,daily&units=imperial&appid={self.api_key}", timeout=2)
            raw = r.json()['current']

            self.temp = str(round(raw['temp'])) + "°F"
            self.icon_url = "http://openweathermap.org/img/wn/" + raw['weather'][0]['icon'] + "@2x.png"
            self.icon = requests.get(self.icon_url, timeout=2)

            log.info("_get_weather_data: Temp: %s, Icon URL: %s" % (self.temp, self.icon_url))
        except:
            log.warning("_get_weather_data: exception occurred %s" % raw)

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        self.offscreen_canvas = matrix.SwapOnVSync(self.draw())

    def draw(self):
        self.offscreen_canvas.Clear()
        font = graphics.Font()
        font.LoadFont(path + "/../../fonts/5x6.bdf")
        text_font = graphics.Font()
        white = graphics.Color(255, 255, 255)
        black = graphics.Color(0, 0, 0)

        image = Image.open(io.BytesIO(self.icon.content))
        image.thumbnail((18, 18))
        icon = image.convert('RGB')
        self.offscreen_canvas.SetImage(icon, 23, 2)

        width = graphics.DrawText(self.offscreen_canvas, font, 0, 0, black, self.temp)
        graphics.DrawText(self.offscreen_canvas, font, (self.offscreen_canvas.width-width)/2, 24, white, self.temp)

        return self.offscreen_canvas
        