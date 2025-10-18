#!/usr/bin/env python
import time
import sys
import requests
import base64
import json
import os
from io import BytesIO
from dotenv import load_dotenv

from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from PIL import Image, ImageDraw, ImageFont

# Load environment variables from .env file
load_dotenv()

# Spotify API credentials from environment variables
SPOTIFY_CLIENT_ID = os.getenv("SPOTIFY_CLIENT_ID")
SPOTIFY_CLIENT_SECRET = os.getenv("SPOTIFY_CLIENT_SECRET")
SPOTIFY_REFRESH_TOKEN = os.getenv("SPOTIFY_REFRESH_TOKEN")

def get_spotify_access_token():
    """Get access token using refresh token"""
    if not SPOTIFY_CLIENT_ID or not SPOTIFY_CLIENT_SECRET or not SPOTIFY_REFRESH_TOKEN:
        print("Error: Spotify credentials not found. Please check your .env file.")
        return None
        
    auth_str = f"{SPOTIFY_CLIENT_ID}:{SPOTIFY_CLIENT_SECRET}"
    auth_bytes = auth_str.encode("ascii")
    auth_b64 = base64.b64encode(auth_bytes).decode("ascii")
    
    headers = {
        "Authorization": f"Basic {auth_b64}",
        "Content-Type": "application/x-www-form-urlencoded"
    }
    
    data = {
        "grant_type": "refresh_token",
        "refresh_token": SPOTIFY_REFRESH_TOKEN
    }
    
    response = requests.post("https://accounts.spotify.com/api/token", headers=headers, data=data)
    if response.status_code == 200:
        return response.json()["access_token"]
    return None

def get_current_playing():
    """Get currently playing track from Spotify"""
    access_token = get_spotify_access_token()
    if not access_token:
        return None, None, None
    
    headers = {"Authorization": f"Bearer {access_token}"}
    response = requests.get("https://api.spotify.com/v1/me/player/currently-playing", headers=headers)
    
    if response.status_code == 200 and response.text:
        data = response.json()
        if data and data.get("is_playing"):
            track = data["item"]
            song_name = track["name"]
            artist_name = ", ".join([artist["name"] for artist in track["artists"]])
            
            # Get album cover
            album_images = track["album"]["images"]
            if album_images:
                # Get the smallest image (usually 64x64)
                image_url = album_images[-1]["url"]
                img_response = requests.get(image_url)
                if img_response.status_code == 200:
                    image = Image.open(BytesIO(img_response.content))
                    return song_name, artist_name, image
            
            return song_name, artist_name, None
    
    return None, None, None

if len(sys.argv) > 1:
    # Use provided image file
    image_file = sys.argv[1]
    song_name = sys.argv[2] if len(sys.argv) > 2 else "Song Name"
    artist_name = sys.argv[3] if len(sys.argv) > 3 else "Artist Name"
    image = Image.open(image_file)
    use_spotify = False
else:
    # Use Spotify integration
    use_spotify = True
    song_name = "Loading..."
    artist_name = "Spotify"
    # Create a default image
    image = Image.new('RGB', (32, 32), (50, 50, 50))

# Configuration for the matrix
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'adafruit-hat-pwm'

matrix = RGBMatrix(options = options)

# Resize image to 32x32 for left placement  
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
    if use_spotify:
        print("Using Spotify integration - fetching current track every 10 seconds")
    print("Press CTRL-C to stop.")
    
    # Convert image to RGB for pixel access
    image_rgb = image.convert('RGB')
    
    last_spotify_check = 0
    spotify_check_interval = 10  # Check Spotify every 10 seconds
    
    while True:
        # Update from Spotify if using integration
        if use_spotify and time.time() - last_spotify_check > spotify_check_interval:
            current_song, current_artist, current_image = get_current_playing()
            if current_song and current_artist:
                song_name = current_song
                artist_name = current_artist
                if current_image:
                    image = current_image
                    # Resize image to 32x32
                    resample_mode = getattr(Image, "Resampling", Image).LANCZOS
                    image.thumbnail((32, 32), resample=resample_mode)
                    image_rgb = image.convert('RGB')
                print(f"Now playing: {song_name} by {artist_name}")
            else:
                song_name = "No music playing"
                artist_name = "Spotify"
            last_spotify_check = time.time()
        
        canvas.Clear()
        
        # Draw image on the left (32x32)
        canvas.SetImage(image_rgb, 0, 0)
        
        # Song text scrolling - draw at actual position
        song_x = song_pos + 32
        song_len = graphics.DrawText(canvas, song_font, song_x, 16, text_color, song_name)
        
        # Restore image pixels where text would overlap (x < 32)
        if song_x < 32:
            for y in range(4, 17):  # Song font area
                for x in range(max(0, song_x), 32):
                    if x < image_rgb.width and y < image_rgb.height:
                        pixel = image_rgb.getpixel((x, y))
                        canvas.SetPixel(x, y, pixel[0], pixel[1], pixel[2])
        
        song_pos -= 1
        if song_pos + 32 + song_len <= 32:  # Reset when the rightmost text reaches x=32
            song_pos = 32  # Start from right edge of text area
        
        # Artist text scrolling - draw at actual position
        artist_x = artist_pos + 32
        artist_len = graphics.DrawText(canvas, artist_font, artist_x, canvas.height - 4, text_color, artist_name)
        
        # Restore image pixels where text would overlap (x < 32)
        if artist_x < 32:
            for y in range(canvas.height - 11, canvas.height):  # Artist font area
                for x in range(max(0, artist_x), 32):
                    if x < image_rgb.width and y < image_rgb.height:
                        pixel = image_rgb.getpixel((x, y))
                        canvas.SetPixel(x, y, pixel[0], pixel[1], pixel[2])
        
        artist_pos -= 1
        if artist_pos + 32 + artist_len <= 32:  # Reset when the rightmost text reaches x=32
            artist_pos = 32  # Start from right edge of text area
        
        canvas = matrix.SwapOnVSync(canvas)
        time.sleep(0.05)  # Faster refresh for smooth scrolling
        
except KeyboardInterrupt:
    sys.exit(0)
