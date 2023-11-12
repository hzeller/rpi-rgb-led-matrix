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

    def show_random2(self):
        main_directory = "../img/fun"
        files = [i for i in glob(f'{main_directory}/*/*') if os.path.isfile(i)]
        random_file = choice(files)
        print(random_file)

        image = Image.open(random_file)

        # Configuration for the matrix
        image.thumbnail((128, 64), Image.ANTIALIAS)
        self.matrix.SetImage(image.convert('RGB'), 10)

        time.sleep(5)

    def show_random(self):
        self.matrix.Clear()
        main_directory = "../img/fun/amimals"
        # files = [i for i in glob(f'{main_directory}/*/*') if os.path.isfile(i)]
        files = [i for i in glob(f'{main_directory}/*') if os.path.isfile(i)]
        random_file = choice(files)
        print(random_file)

        image = Image.open(random_file).convert('RGB')
        img_width, img_height = image.size

        print("self.matrix.width: " + str(self.matrix.width))
        print("self.matrix.height: " + str(self.matrix.height))
        print("img_width original: " + str(img_width))
        print("img_height original: " + str(img_height))

        #if img_height <= self.matrix.height:
        image = image.resize((img_width, self.matrix.height), Image.ANTIALIAS)
        # else:
        #image.thumbnail((self.matrix.width, self.matrix.height), Image.ANTIALIAS)

        double_buffer = self.matrix.CreateFrameCanvas()

        img_width, img_height = image.size
        print("img_width despues: " + str(img_width))
        print("img_height despues: " + str(img_height))

        xpos = 0
        times = 0

        try:
            while times <= 3:
                xpos += 1
                if (xpos > img_width):
                    xpos = 0
                    times = times + 1

                double_buffer.SetImage(image, -xpos)
                double_buffer.SetImage(image, -xpos + img_width)

                double_buffer = self.matrix.SwapOnVSync(double_buffer)
                time.sleep(0.01)
        except Exception as X:
            print("Error in Images: " + str(X))
