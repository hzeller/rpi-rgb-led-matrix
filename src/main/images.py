#!/usr/bin/env python
import time
from glob import glob
from random import choice

from rgbmatrix import graphics
from models.commands import get_command

class Images:
    def get(self):
        return get_command()

    def show_random(self):
        main_directory = "../img/fun"
        random_file = choice(glob(f'{main_directory}/**/*.gif'))
        print(random_file)
        time.sleep(3)
