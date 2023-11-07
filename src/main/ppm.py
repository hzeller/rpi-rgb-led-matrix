#!/usr/bin/env python

import os
import random
import python_weather
import time

from common import Common
from models.cities import Cities
from PIL import Image
from rgbmatrix import graphics

class PPM():
    def show(self):
        self.image = Image.open("../../../examples-api-use/runtext.ppm").convert('RGB')
        self.image.resize((self.matrix.width-10, self.matrix.height-10), Image.ANTIALIAS)

        double_buffer = self.matrix.CreateFrameCanvas()
        img_width, img_height = self.image.size

        # let's scroll
        xpos = 0
        while xpos < 470:
            xpos += 1

            if (xpos > img_width):
                xpos = 0

            double_buffer.SetImage(self.image, -xpos)
            double_buffer.SetImage(self.image, -xpos + img_width)

            double_buffer = self.matrix.SwapOnVSync(double_buffer)
            time.sleep(0.01)

        self.matrix.Clear()
