#!/usr/bin/env python
import time
import socket

from rgbmatrix import graphics

import logging
log = logging.getLogger(__name__)

import os
path = os.path.dirname(__file__)

class Welcome:
    def __init__(self, matrix):
        self.framerate = 1

        self.matrix = matrix

        self.hostname = socket.gethostname()
        self.ip = socket.gethostbyname(self.hostname)

        self.show()
    
    def get_framerate(self):
        return self.framerate

    def show(self):
        _tmp_canvas = self.matrix.CreateFrameCanvas()

        font = graphics.Font()
        font.LoadFont(path + "/../../fonts/5x6.bdf")
        text_font = graphics.Font()
        white = graphics.Color(255, 255, 255)

        phrase = "hello world"
        self.matrix.brightness = 0
        width = graphics.DrawText(_tmp_canvas, font, 0, 0, white, phrase)
        graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, phrase)
        while self.matrix.brightness < 100:
            self.matrix.brightness += 1
            graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, phrase)
            time.sleep(0.01)
        time.sleep(1)
        self.matrix.brightness = 100
        while self.matrix.brightness > 0:
            self.matrix.brightness -= 1
            graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, phrase)
            time.sleep(0.01)
        self.matrix.Clear()

        phrase = "i am"
        self.matrix.brightness = 0
        width = graphics.DrawText(_tmp_canvas, font, 0, 0, white, phrase)
        graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, phrase)
        while self.matrix.brightness < 100:
            self.matrix.brightness += 1
            graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, phrase)
            time.sleep(0.01)
        time.sleep(1)
        self.matrix.brightness = 100
        while self.matrix.brightness > 0:
            self.matrix.brightness -= 1
            graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, phrase)
            time.sleep(0.01)
        self.matrix.Clear()
        
        self.matrix.brightness = 0
        width = graphics.DrawText(_tmp_canvas, font, 0, 0, white, self.ip)
        graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, self.ip)
        while self.matrix.brightness < 100:
            self.matrix.brightness += 1
            graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, self.ip)
            time.sleep(0.01)
        time.sleep(1)
        self.matrix.brightness = 100
        while self.matrix.brightness > 0:
            self.matrix.brightness -= 1
            graphics.DrawText(self.matrix, font, (self.matrix.width-width)/2, 18, white, self.ip)
            time.sleep(0.01)
        self.matrix.Clear()

        self.matrix.brightness = 100