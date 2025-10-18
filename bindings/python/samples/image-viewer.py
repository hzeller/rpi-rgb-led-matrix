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
        print(f"CLIENT_ID present: {bool(SPOTIFY_CLIENT_ID)}")
        print(f"CLIENT_SECRET present: {bool(SPOTIFY_CLIENT_SECRET)}")
        print(f"REFRESH_TOKEN present: {bool(SPOTIFY_REFRESH_TOKEN)}")
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
    print(f"Token request status: {response.status_code}")
    
    if response.status_code == 200:
        return response.json()["access_token"]
    else:
        print(f"Token request failed: {response.text}")
        return None

def get_current_playing():
    """Get currently playing track from Spotify"""
    access_token = get_spotify_access_token()
    if not access_token:
        print("Failed to get Spotify access token")
        return None, None, None, None
    
    headers = {"Authorization": f"Bearer {access_token}"}
    response = requests.get("https://api.spotify.com/v1/me/player/currently-playing", headers=headers)
    
    print(f"Spotify API response status: {response.status_code}")
    
    if response.status_code == 401:
        print("Unauthorized - check your Spotify credentials and refresh token")
        return None, None, None, None
    elif response.status_code == 403:
        print("Forbidden - check your Spotify app permissions")
        return None, None, None, None
    elif response.status_code == 429:
        print("Rate limited - too many requests to Spotify API")
        return None, None, None, None
    elif response.status_code == 204:
        print("No content - no music currently playing")
        return None, None, None, None
    
    if response.status_code == 200 and response.text:
        try:
            data = response.json()
            print(f"Spotify response data keys: {data.keys() if data else 'No data'}")
            
            if data and data.get("is_playing"):
                track = data["item"]
                if track:
                    song_name = track["name"]
                    artist_name = ", ".join([artist["name"] for artist in track["artists"]])
                    album_name = track["album"]["name"]
                    print(f"Found track: {song_name} by {artist_name} from {album_name}")
                    
                    # Get album cover
                    album_images = track["album"]["images"]
                    if album_images:
                        # Get the smallest image (usually 64x64)
                        image_url = album_images[-1]["url"]
                        img_response = requests.get(image_url)
                        if img_response.status_code == 200:
                            image = Image.open(BytesIO(img_response.content))
                            return song_name, artist_name, album_name, image
                    
                    return song_name, artist_name, album_name, None
                else:
                    print("Track item is None")
            else:
                print(f"Not playing or no data. is_playing: {data.get('is_playing') if data else 'No data'}")
        except json.JSONDecodeError as e:
            print(f"JSON decode error: {e}")
        except Exception as e:
            print(f"Error parsing Spotify response: {e}")
    else:
        print(f"Unexpected response: status {response.status_code}, content: {response.text[:200] if response.text else 'No content'}")
    
    return None, None, None, None

if len(sys.argv) > 1:
    # Use provided image file
    image_file = sys.argv[1]
    song_name = sys.argv[2] if len(sys.argv) > 2 else "Song Name"
    artist_name = (sys.argv[3] if len(sys.argv) > 3 else "Artist Name").upper()
    album_name = sys.argv[4] if len(sys.argv) > 4 else "Album Name"
    image = Image.open(image_file)
    use_spotify = False
else:
    # Use Spotify integration
    use_spotify = True
    song_name = "Loading..."
    artist_name = "SPOTIFY"
    album_name = "Album Name"
    # Create a default image
    image = Image.new('RGB', (20, 20), (50, 50, 50))

# Configuration for the matrix
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'adafruit-hat-pwm'

matrix = RGBMatrix(options = options)

# Resize image to 20x20 for lower left corner
resample_mode = getattr(Image, "Resampling", Image).LANCZOS
image.thumbnail((20, 20), resample=resample_mode)

# Load fonts for different elements
song_font = graphics.Font()
song_font.LoadFont("../../../fonts/6x10.bdf")  # Medium font for song title (made smaller)
artist_font = graphics.Font()
artist_font.LoadFont("../../../fonts/6x10.bdf")  # Medium font for artist
album_font = graphics.Font()
album_font.LoadFont("../../../fonts/5x7.bdf")  # Smaller font for album

# Define colors
song_color = graphics.Color(0, 255, 0)      # Green for song title
artist_color = graphics.Color(255, 255, 255) # White for artist name
album_color = graphics.Color(200, 200, 200)  # Light gray for album name

# Text scrolling variables - new layout
song_available_width = matrix.width        # Song spans entire width at top
other_available_width = matrix.width - 22  # Other text after 20px image + margin
artist_pos = 22                           # Artist starts after image
album_pos = 22                            # Album starts after image

# Initialize album name
album_name = "Album Name"

# Create canvas for drawing
canvas = matrix.CreateFrameCanvas()

try:
    print(f"Song width: {song_available_width}, Other width: {other_available_width}")
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
            current_song, current_artist, current_album, current_image = get_current_playing()
            if current_song and current_artist:
                song_name = current_song
                artist_name = current_artist.upper()  # Convert artist to all caps
                album_name = current_album if current_album else "Album Name"
                if current_image:
                    image = current_image
                    # Resize image to 20x20
                    resample_mode = getattr(Image, "Resampling", Image).LANCZOS
                    image.thumbnail((20, 20), resample=resample_mode)
                    image_rgb = image.convert('RGB')
                print(f"Now playing: {song_name} by {artist_name} from {album_name}")
            else:
                song_name = "No music playing"
                artist_name = "SPOTIFY"
                album_name = "Album Name"
            last_spotify_check = time.time()
        
        canvas.Clear()
        
        # Draw album cover in lower left (20x20)
        image_y = canvas.height - image_rgb.height  # Bottom of screen
        canvas.SetImage(image_rgb, 0, image_y)
        
        # Song title spans entire width at top (static, no scrolling)
        song_x = 0   # Start at left edge
        song_y = 8   # Top of screen with 1px padding above (6x10 font height + 1px = 8px from top)
        song_len = graphics.DrawText(canvas, song_font, song_x, song_y, song_color, song_name)
        
        # If song is too long, truncate it to fit available width
        if song_len > song_available_width:
            # Calculate how many characters we can fit
            char_width = 6  # Approximate width of 6x10 font
            max_chars = song_available_width // char_width
            truncated_song = song_name[:max_chars-1] + "..." if len(song_name) > max_chars else song_name
            graphics.DrawText(canvas, song_font, song_x, song_y, song_color, truncated_song)
        
        # Artist name in middle right in all caps (white)
        artist_x = artist_pos
        artist_y = canvas.height // 2  # Middle of screen
        artist_len = graphics.DrawText(canvas, artist_font, artist_x, artist_y, artist_color, artist_name)
        
        # Only scroll artist if it doesn't fit
        if artist_len > other_available_width:
            artist_pos -= 1
            if artist_pos + artist_len <= 22:  # Reset when fully scrolled past image
                artist_pos = matrix.width
        
        # Album name in lower right in normal case (light gray)
        album_x = album_pos
        album_y = canvas.height - 4  # Bottom of screen
        album_len = graphics.DrawText(canvas, album_font, album_x, album_y, album_color, album_name)
        
        # Only scroll album if it doesn't fit
        if album_len > other_available_width:
            album_pos -= 1
            if album_pos + album_len <= 22:  # Reset when fully scrolled past image
                album_pos = matrix.width
        
        canvas = matrix.SwapOnVSync(canvas)
        time.sleep(0.05)  # Faster refresh for smooth scrolling
        
except KeyboardInterrupt:
    sys.exit(0)
