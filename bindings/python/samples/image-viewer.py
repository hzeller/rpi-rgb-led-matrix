#!/usr/bin/env python
import time
import sys

from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from PIL import Image, ImageDraw, ImageFont

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
song_font.LoadFont("../../../fonts/6x13B.bdf")  # Bold version, smaller than 7x13B
artist_font = graphics.Font()
artist_font.LoadFont("../../../fonts/5x7.bdf")  # Smaller font

# Text scrolling variables
available_width = matrix.width - 32 - 2  # Width minus image minus small margin
song_pos = available_width  # Start from right edge
artist_pos = available_width  # Start from right edge
text_color = graphics.Color(255, 255, 255)

# Create canvas for drawing
canvas = matrix.CreateFrameCanvas()

try:
    print(f"Available width: {available_width}")
    print("Press CTRL-C to stop.")
    while True:
        canvas.Clear()
        
        # Draw image on the left (32x32)
        canvas.SetImage(image.convert('RGB'), 0, 0)
        
        # Song text scrolling - draw at actual position, let matrix clip naturally
        song_x = song_pos + 32
        song_len = graphics.DrawText(canvas, song_font, song_x, 16, text_color, song_name)
        
        song_pos -= 1
        if song_pos + song_len < -32:  # Reset when completely scrolled past
            song_pos = 32  # Start from right edge of text area
        
        # Artist text scrolling - draw at actual position, let matrix clip naturally  
        artist_x = artist_pos + 32
        artist_len = graphics.DrawText(canvas, artist_font, artist_x, canvas.height - 4, text_color, artist_name)
        
        artist_pos -= 1
        if artist_pos + artist_len < -32:  # Reset when completely scrolled past
            artist_pos = 32  # Start from right edge of text area
        
        canvas = matrix.SwapOnVSync(canvas)
        time.sleep(0.05)  # Faster refresh for smooth scrolling
        
except KeyboardInterrupt:
    sys.exit(0)
