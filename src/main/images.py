#!/usr/bin/env python
import time
import os

from random import choice
from rgbmatrix import RGBMatrix, RGBMatrixOptions
from models.commands import get_command
from PIL import Image
from glob import glob

class Images:
    def get(self):
        return get_command()

    def show_random(self):
        main_directory = "../img/fun"
        files = [i for i in glob(f'{main_directory}/*/*') if os.path.isfile(i)]
        random_file = choice(files)
        print(random_file)

        image = Image.open(random_file)

        # Configuration for the matrix
        image.thumbnail((128, 64), Image.ANTIALIAS)
        self.matrix.SetImage(image.convert('RGB'))

        time.sleep(5)
