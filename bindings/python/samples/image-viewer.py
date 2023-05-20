#!/usr/bin/env python
import time
import sys
import random
import math

from rgbmatrix import RGBMatrix, RGBMatrixOptions
from PIL import Image

# if len(sys.argv) < 2:
#     sys.exit("Require an image argument")
# else:
#     image_file = sys.argv[1]

# image = Image.open(image_file)

# Configuration for the matrix
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'adafruit-hat-pwm'  # If you have an Adafruit HAT: 'adafruit-hat'

matrix = RGBMatrix(options = options)
canvas = matrix.CreateFrameCanvas()

# Make image fit our screen.
# image.thumbnail((matrix.width, matrix.height), Image.ANTIALIAS)

as_bitmap =[[0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,0],
            [0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1],
            [0,0,0,0,0,1,1,0,0,1,1,0,0,0,1,1],
            [0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,0],
            [1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,0],
            [1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,1],
            [1,1,0,0,0,1,1,0,0,1,1,0,0,0,1,1],
            [1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0],
            [0,1,1,1,1,0,1,0,0,0,1,1,1,1,1,0]]

as_buffer =[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]

def wheel(pos):
    if pos < 85:
        return (pos * 3) , (255 - pos * 3) , 0
    elif pos < 170:
        pos -= 85
        return (255 - pos * 3) , 0, (pos * 3)
    else:
        pos -= 170
        return 0 , (pos * 3) , (255 - pos * 3)

count = 0
def draw_wheel():
    global count

    x_offset = (matrix.width - len(as_bitmap[0])) / 2
    y_offset = (matrix.height - len(as_bitmap)) / 2
    for x in range(0,len(as_bitmap[0])):
        count = 0 if count >= 1020 else count+1
        r,g,b = wheel(count/4)
        for y in range(0,len(as_bitmap)):
            if as_bitmap[y][x]:
                canvas.SetPixel(x+x_offset,y+y_offset,r,g,b)

fade_in = True
def draw_pulsing():
    global count, fade_in

    if fade_in:
        if count < 255:
            count += 1 
        else:
            fade_in = False
    else:
        if count > 0:
            count -= 1 
        else:
            fade_in = True

    x_offset = (matrix.width - len(as_bitmap[0])) / 2
    y_offset = (matrix.height - len(as_bitmap)) / 2
    for x in range(0,len(as_bitmap[0])):
        for y in range(0,len(as_bitmap)):
            if as_bitmap[y][x]:
                canvas.SetPixel(x+x_offset,y+y_offset,count,count,count)

order = list(range(0,len(as_bitmap[0])*len(as_bitmap)))
random.shuffle(order)
def draw_bits():
    global count, fade_in
    count += 1

    if fade_in:
        if count < len(order):
            count += 1 
        else:
            fade_in = False
            random.shuffle(order)
    else:
        if count > 0:
            count -= 1 
        else:
            fade_in = True
            random.shuffle(order)

    x_offset = (matrix.width - len(as_bitmap[0])) / 2
    y_offset = (matrix.height - len(as_bitmap)) / 2
    
    index = order[count]
    found = False
    while not found:
        x = index % len(as_buffer[0])
        y = math.floor(index / len(as_buffer))
        print(x,y)
        if as_buffer[y][x]:
            canvas.SetPixel(x+x_offset,index / len(as_buffer)+y_offset,255,255,255)
            found = True
        else:
            draw_bits()

try:
    print("Press CTRL-C to stop.")
    while True:
        draw_bits()
        matrix.SwapOnVSync(canvas)
        time.sleep(0.01)
except KeyboardInterrupt:
    sys.exit(0)
