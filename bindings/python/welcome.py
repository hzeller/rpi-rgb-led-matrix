import os
import logging
import time
import socket
from rgbmatrix import graphics

path = os.path.dirname(__file__)
log = logging.getLogger(__name__)


class Welcome:
    def __init__(self):
        self.framerate = 1

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        self.ip = s.getsockname()[0]

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        brightness = matrix.brightness

        font = graphics.Font()
        font.LoadFont(path + "/../../fonts/5x6.bdf")
        white = graphics.Color(255, 255, 255)
        black = graphics.Color(0, 0, 0)

        phrase = "hello world"
        matrix.brightness = 0
        width = graphics.DrawText(matrix, font, 0, 0, black, phrase)
        graphics.DrawText(matrix, font, (matrix.width - width) / 2, 18, white, phrase)
        while matrix.brightness < 100:
            matrix.brightness += 1
            graphics.DrawText(
                matrix, font, (matrix.width - width) / 2, 18, white, phrase
            )
            time.sleep(0.01)
        time.sleep(1)
        matrix.brightness = 100
        while matrix.brightness > 0:
            matrix.brightness -= 1
            graphics.DrawText(
                matrix, font, (matrix.width - width) / 2, 18, white, phrase
            )
            time.sleep(0.01)
        matrix.Clear()

        phrase = "i am"
        matrix.brightness = 0
        width = graphics.DrawText(matrix, font, 0, 0, black, phrase)
        graphics.DrawText(matrix, font, (matrix.width - width) / 2, 18, white, phrase)
        while matrix.brightness < 100:
            matrix.brightness += 1
            graphics.DrawText(
                matrix, font, (matrix.width - width) / 2, 18, white, phrase
            )
            time.sleep(0.01)
        time.sleep(1)
        matrix.brightness = 100
        while matrix.brightness > 0:
            matrix.brightness -= 1
            graphics.DrawText(
                matrix, font, (matrix.width - width) / 2, 18, white, phrase
            )
            time.sleep(0.01)
        matrix.Clear()

        matrix.brightness = 0
        width = graphics.DrawText(matrix, font, 0, 0, black, self.ip)
        graphics.DrawText(matrix, font, (matrix.width - width) / 2, 18, white, self.ip)
        while matrix.brightness < 100:
            matrix.brightness += 1
            graphics.DrawText(
                matrix, font, (matrix.width - width) / 2, 18, white, self.ip
            )
            time.sleep(0.01)
        time.sleep(1)
        matrix.brightness = 100
        while matrix.brightness > 0:
            matrix.brightness -= 1
            graphics.DrawText(
                matrix, font, (matrix.width - width) / 2, 18, white, self.ip
            )
            time.sleep(0.01)
        matrix.Clear()

        matrix.brightness = brightness
