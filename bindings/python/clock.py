import os
import logging
from datetime import datetime
import zoneinfo
import random
from rgbmatrix import graphics
from secrets import LOCAL_TZ

path = os.path.dirname(__file__)
log = logging.getLogger(__name__)


class BasicClock:
    def __init__(self, offscreen_canvas):
        self.framerate = 1
        self.offscreen_canvas = offscreen_canvas

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        self.offscreen_canvas = matrix.SwapOnVSync(self.draw())

    def draw(self):
        self.offscreen_canvas.Clear()
        font = graphics.Font()
        font.LoadFont(path + "/../../fonts/5x6.bdf")
        white = graphics.Color(255, 255, 255)
        black = graphics.Color(0, 0, 0)

        now = datetime.now().replace(tzinfo=zoneinfo.ZoneInfo(LOCAL_TZ))
        hour = now.strftime("%-I")
        min = now.strftime("%M")
        day = now.strftime("%p")
        time = hour + ":" + min + " " + day

        seconds = int(now.strftime("%-S"))
        offset = graphics.DrawText(self.offscreen_canvas, font, 0, 0, black, hour + ":")
        width = graphics.DrawText(self.offscreen_canvas, font, 0, 0, black, time)
        if seconds % 2:
            graphics.DrawText(
                self.offscreen_canvas,
                font,
                (self.offscreen_canvas.width - width) / 2,
                18,
                white,
                time,
            )
        else:
            graphics.DrawText(
                self.offscreen_canvas,
                font,
                (self.offscreen_canvas.width - width) / 2,
                18,
                white,
                hour,
            )
            graphics.DrawText(
                self.offscreen_canvas,
                font,
                (self.offscreen_canvas.width - width) / 2 + offset,
                18,
                white,
                min + " " + day,
            )

        return self.offscreen_canvas


class BinaryClock:
    def __init__(self, offscreen_canvas):
        self.framerate = 1
        self.offscreen_canvas = offscreen_canvas

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        self.offscreen_canvas = matrix.SwapOnVSync(self.draw())

    def draw(self):
        self.offscreen_canvas.Clear()

        now = datetime.now().replace(tzinfo=zoneinfo.ZoneInfo(LOCAL_TZ))
        if int(now.strftime("%-H")) < 12:
            time = now - now.replace(hour=0, minute=0, second=0, microsecond=0)
        else:
            time = now - now.replace(hour=12, minute=0, second=0, microsecond=0)
        
        start_x = 17
        start_y = 1
        interval = 8
        size = 6
        seconds = bin(time.seconds)[2:].rjust(16, "0")
        for bit in range(0, len(seconds)):
            if seconds[bit] == "1":
                self.draw_square(
                    start_x + bit * interval % 32, 
                    start_y + bit // 4 * interval, 
                    size,
                    (random.randint(0,255),random.randint(0,255),random.randint(0,255))
                )

        return self.offscreen_canvas

    def draw_square(self, x, y, z, color):
        for _x in range(0,z):
            for _y in range(0,z):
                self.offscreen_canvas.SetPixel(
                    x + _x, 
                    y + _y, 
                    color[0], 
                    color[1], 
                    color[2]
                )
