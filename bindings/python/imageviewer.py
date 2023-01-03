#!/usr/bin/env python
import time
import sys
from PIL import Image

import logging
log = logging.getLogger(__name__)

class ImageViewer:
    def __init__(self, matrix, path):
        self.framerate = 100

        image = Image.open(path)
        image.thumbnail((matrix.width, matrix.height), Image.ANTIALIAS)
        self.image = image.convert('RGB')
        self.canvas = matrix.CreateFrameCanvas()
        self.canvas.SetImage(self.image)
    
    def get_framerate(self):
        return self.framerate

    def show(self):
        return self.canvas
