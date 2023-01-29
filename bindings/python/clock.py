#!/usr/bin/env python
import time
from datetime import datetime
import zoneinfo

from rgbmatrix import graphics
from secrets import LOCAL_TZ

import logging
log = logging.getLogger(__name__)

import os
path = os.path.dirname(__file__)

class Clock:
    def __init__(self, matrix):
        self.framerate = 1

        self.matrix = matrix
    
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
    
        now = datetime.now().replace(tzinfo=zoneinfo.ZoneInfo(LOCAL_TZ))
        hour = now.strftime('%-I')
        min = now.strftime('%M')
        day = now.strftime('%p')
        self.time = hour + ':' + min + ' ' + day
        
        seconds = int(now.strftime('%-S'))
        offset = graphics.DrawText(_tmp_canvas, font, 0, 0, white, hour + ':')
        width = graphics.DrawText(_tmp_canvas, font, 0, 0, white, self.time)
        if seconds%2:
            graphics.DrawText(offscreen_canvas, font, (offscreen_canvas.width-width)/2, 18, white, self.time)
        else:
            graphics.DrawText(offscreen_canvas, font, (offscreen_canvas.width-width)/2, 18, white, hour)
            graphics.DrawText(offscreen_canvas, font, (offscreen_canvas.width-width)/2+offset, 18, white, min + ' ' + day)

        return offscreen_canvas
        
