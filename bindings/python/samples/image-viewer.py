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
        
        # Song text scrolling with proper clipping to text area (x=32 to x=63)
        song_x = song_pos + 32
        
        # Calculate which characters are visible in the text area
        if song_x < 32:
            # Text is partially or fully off the left edge - calculate visible portion
            char_width = 6  # Approximate width of 6x13B font
            chars_off_left = max(0, (32 - song_x + char_width - 1) // char_width)
            
            if chars_off_left < len(song_name):
                # Some characters are still visible - draw the visible portion
                visible_song = song_name[chars_off_left:]
                song_len = graphics.DrawText(canvas, song_font, 32, 16, text_color, visible_song)
                total_song_len = len(song_name) * char_width
            else:
                # All text is off-screen
                total_song_len = len(song_name) * char_width
                song_len = 0
        else:
            # Text starts in visible area - draw normally
            song_len = graphics.DrawText(canvas, song_font, song_x, 16, text_color, song_name)
            total_song_len = song_len
        
        song_pos -= 1
        if song_pos + total_song_len < -32:  # Reset when completely scrolled past
            song_pos = 32  # Start from right edge of text area
        
        # Artist text scrolling with proper clipping to text area (x=32 to x=63)
        artist_x = artist_pos + 32
        
        # Calculate which characters are visible in the text area
        if artist_x < 32:
            # Text is partially or fully off the left edge - calculate visible portion
            char_width = 5  # Approximate width of 5x7 font
            chars_off_left = max(0, (32 - artist_x + char_width - 1) // char_width)
            
            if chars_off_left < len(artist_name):
                # Some characters are still visible - draw the visible portion
                visible_artist = artist_name[chars_off_left:]
                artist_len = graphics.DrawText(canvas, artist_font, 32, canvas.height - 4, text_color, visible_artist)
                total_artist_len = len(artist_name) * char_width
            else:
                # All text is off-screen
                total_artist_len = len(artist_name) * char_width
                artist_len = 0
        else:
            # Text starts in visible area - draw normally
            artist_len = graphics.DrawText(canvas, artist_font, artist_x, canvas.height - 4, text_color, artist_name)
            total_artist_len = artist_len
        
        artist_pos -= 1
        if artist_pos + total_artist_len < -32:  # Reset when completely scrolled past
            artist_pos = 32  # Start from right edge of text area
        
        canvas = matrix.SwapOnVSync(canvas)
        time.sleep(0.05)  # Faster refresh for smooth scrolling
        
except KeyboardInterrupt:
    sys.exit(0)
