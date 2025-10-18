#!/usr/bin/env python
import time
import sys

from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from PIL import Image

if len(sys.argv) < 2:
    sys.exit("Usage: python image-viewer.py <image-file> [song-name] [artist-name]")

image_file = sys.argv[1]
song_name = sys.argv[2] if len(sys.argv) > 2 else "Song Name"
artist_name = sys.argv[3] if len(sys.argv) > 3 else "Artist Name"

# Configuration for the matrix
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'adafruit-hat-pwm'

matrix = RGBMatrix(options = options)

# Load the image and resize it to 32x32 for left placement
image = Image.open(image_file)
resample_mode = getattr(Image, "Resampling", Image).LANCZOS
image.thumbnail((32, 32), resample=resample_mode)

# Load fonts for text - bold for song, regular for artist
song_font = graphics.Font()
song_font.LoadFont("../../../fonts/7x13B.bdf")  # Bold version
artist_font = graphics.Font()
artist_font.LoadFont("../../../fonts/7x13.bdf")  # Regular version
text_color = graphics.Color(255, 255, 255)

# Create canvas for drawing
canvas = matrix.CreateFrameCanvas()

try:
    print("Press CTRL-C to stop.")
    while True:
        canvas.Clear()
        
        # Draw image on the left (32x32)
        canvas.SetImage(image.convert('RGB'), 0, 0)
        
        # Draw song name bold on top right
        graphics.DrawText(canvas, song_font, 34, 12, text_color, song_name)
        
        # Draw artist name regular beneath song name
        graphics.DrawText(canvas, artist_font, 34, 22, text_color, artist_name)
        
        canvas = matrix.SwapOnVSync(canvas)
        time.sleep(0.1)
        
except KeyboardInterrupt:
    sys.exit(0)
