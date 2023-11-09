#!/usr/bin/env python
import time
from glob import glob
from random import choice
import os

from models.commands import get_command
from PIL import Image

class Images:
    def get(self):
        return get_command()

    def show_random(self):
        main_directory = "../img/fun"
        files = [i for i in glob(f'{main_directory}/*/*') if os.path.isfile(i)]
        random_file = choice(files)
        print(random_file)

        image = Image.open(random_file)
        image.thumbnail((self.matrix.width, self.matrix.height), Image.ANTIALIAS)
        self.matrix.SetImage(image.convert('RGB'))

        time.sleep(5)
