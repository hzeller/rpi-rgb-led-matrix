import os
import logging
from datetime import datetime
import zoneinfo
from rgbmatrix import graphics
from secrets import LOCAL_TZ

path = os.path.dirname(__file__)
log = logging.getLogger(__name__)


class Clock:
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
        self.time = hour + ":" + min + " " + day

        seconds = int(now.strftime("%-S"))
        offset = graphics.DrawText(self.offscreen_canvas, font, 0, 0, black, hour + ":")
        width = graphics.DrawText(self.offscreen_canvas, font, 0, 0, black, self.time)
        if seconds % 2:
            graphics.DrawText(
                self.offscreen_canvas,
                font,
                (self.offscreen_canvas.width - width) / 2,
                18,
                white,
                self.time,
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
