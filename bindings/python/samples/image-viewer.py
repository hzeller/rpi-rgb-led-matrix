#!/usr/bin/env python
import time
import sys
import requests
import base64
import json
import os
import threading
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

def spotify_update_thread():
    """Background thread for updating Spotify data"""
    global current_spotify_data
    while True:
        try:
            current_song, current_artist, current_album, current_image = get_current_playing()
            with spotify_lock:
                if current_song and current_artist:
                    current_spotify_data['song_name'] = current_song
                    current_spotify_data['artist_name'] = current_artist.upper()
                    current_spotify_data['album_name'] = current_album if current_album else "Album Name"
                    if current_image:
                        # Resize image to 20x20 in background thread
                        resample_mode = getattr(Image, "Resampling", Image).LANCZOS
                        current_image.thumbnail((20, 20), resample=resample_mode)
                        current_spotify_data['image'] = current_image.convert('RGB')
                    print(f"Updated: {current_song} by {current_artist} from {current_album}")
                else:
                    current_spotify_data['song_name'] = "No music playing"
                    current_spotify_data['artist_name'] = "SPOTIFY"
                    current_spotify_data['album_name'] = "Album Name"
        except Exception as e:
            print(f"Spotify update error: {e}")
        
        time.sleep(3)  # Update every 3 seconds in background

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
song_font.LoadFont("../../../fonts/5x7.bdf")  # Smaller font for song title
artist_font = graphics.Font()
artist_font.LoadFont("../../../fonts/5x7.bdf")  # Smaller font for artist
album_font = graphics.Font()
album_font.LoadFont("../../../fonts/5x7.bdf")  # Smaller font for album

# Define colors
song_color = graphics.Color(0, 255, 0)      # Green for song title
artist_color = graphics.Color(255, 255, 255) # White for artist name
album_color = graphics.Color(200, 200, 200)  # Light gray for album name

# Text layout variables with scrolling for cut-off text
padding = 2
song_available_width = matrix.width - (padding * 2)  # Song width minus left and right padding
other_available_width = matrix.width - 22 - (padding * 2)  # Other text after image + padding

# Scrolling variables for each text element
song_scroll_pos = 0
artist_scroll_pos = 0
album_scroll_pos = 0
scroll_counter = 0

# Static delay variables (show static for 1 second before scrolling)
song_static_delay = 0
artist_static_delay = 0
album_static_delay = 0
static_delay_frames = 20  # 1 second at 20fps

# Global variables for Spotify data (thread-safe)
current_spotify_data = {
    'song_name': "Loading...",
    'artist_name': "SPOTIFY", 
    'album_name': "Album Name",
    'image': None
}
spotify_lock = threading.Lock()

# Initialize album name
album_name = "Album Name"

# Create canvas for drawing
canvas = matrix.CreateFrameCanvas()

try:
    print(f"Song width: {song_available_width}, Other width: {other_available_width}")
    if use_spotify:
        print("Using Spotify integration - background updates every 3 seconds")
        # Start background thread for Spotify updates
        spotify_thread = threading.Thread(target=spotify_update_thread, daemon=True)
        spotify_thread.start()
    print("Press CTRL-C to stop.")
    
    # Convert image to RGB for pixel access
    image_rgb = image.convert('RGB')
    
    while True:
        # Get current Spotify data from background thread (no API calls in main loop)
        if use_spotify:
            with spotify_lock:
                song_name = current_spotify_data['song_name']
                artist_name = current_spotify_data['artist_name']
                album_name = current_spotify_data['album_name']
                if current_spotify_data['image'] is not None:
                    image_rgb = current_spotify_data['image']
        
        canvas.Clear()
        
        # Draw album cover in lower left (20x20) with 2px padding
        image_x = padding  # 2px from left edge
        image_y = canvas.height - image_rgb.height - padding  # 2px from bottom edge
        canvas.SetImage(image_rgb, image_x, image_y)
        
        # Song title spans width at top with padding (scrolls if cut off)
        song_x = padding  # Start 2px from left edge
        song_y = padding + 6   # 2px padding + 6px for font baseline (moved down 1px)
        song_display = song_name.upper()  # Convert to all caps
        max_x = canvas.width - padding  # Maximum x position (2px from right edge)
        
        # Calculate total text width
        total_song_width = 0
        for char in song_display:
            char_width = len(char) * 4  # Rough estimate: 4 pixels per char for 5x7 font
            total_song_width += char_width
        
        # Check if scrolling is needed
        if total_song_width > song_available_width:
            # Check if we need to start or continue the static delay
            if song_static_delay < static_delay_frames:
                # Show static text for 1 second before scrolling
                song_static_delay += 1
                scroll_offset = 0  # Static position
            else:
                # Start scrolling after delay
                scroll_offset = song_scroll_pos
            
            # Draw text with wraparound - draw it twice to create seamless loop
            for offset in [0, total_song_width + 20]:  # Draw original and wrapped version
                display_x = song_x - scroll_offset + offset
                current_x = display_x
                for char in song_display:
                    if char == ' ':
                        # Space between words - make it smaller
                        current_x += 2  # Reduced space width
                    else:
                        # Only draw if within visible area (with some buffer)
                        if current_x > -8 and current_x < matrix.width + 8:
                            char_width = graphics.DrawText(canvas, song_font, current_x, song_y, song_color, char)
                            current_x += char_width - 1  # Normal character spacing
                        else:
                            current_x += 4  # Estimated width when not drawing
            
            # Clear pixels outside the allowed boundaries (clipping effect)
            # Clear left side (before song area)
            for x in range(0, song_x):
                for y in range(song_y - 6, song_y + 2):
                    if 0 <= y < matrix.height:
                        canvas.SetPixel(x, y, 0, 0, 0)
            
            # Clear right side (after song area)
            for x in range(max_x, matrix.width):
                for y in range(song_y - 6, song_y + 2):
                    if 0 <= y < matrix.height:
                        canvas.SetPixel(x, y, 0, 0, 0)
            
            # Update scroll position only after static delay
            if song_static_delay >= static_delay_frames and scroll_counter % 4 == 0:
                song_scroll_pos += 1
                if song_scroll_pos >= total_song_width + 20:  # Reset when first copy is off-screen
                    song_scroll_pos = 0
                    song_static_delay = 0  # Reset static delay for next cycle
        else:
            # Static display if text fits
            current_x = song_x
            for char in song_display:
                if current_x + 5 > max_x:
                    break
                if char == ' ':
                    # Space between words - make it smaller
                    current_x += 2  # Reduced space width
                else:
                    char_width = graphics.DrawText(canvas, song_font, current_x, song_y, song_color, char)
                    current_x += char_width - 1  # Normal character spacing
        
        # Artist name in middle right in all caps (scrolls if cut off)
        artist_x = padding + 20 + padding  # 2px + 20px album + 2px spacing = x=24
        artist_y = canvas.height // 2 + 2  # Middle of screen + 2px down
        max_x = canvas.width - padding  # Maximum x position (2px from right edge)
        artist_available_width = max_x - artist_x  # Available width for artist text
        
        # Calculate total text width
        total_artist_width = 0
        for char in artist_name:
            total_artist_width += 4  # Rough estimate: 4 pixels per char
        
        # Check if scrolling is needed
        if total_artist_width > artist_available_width:
            # Check if we need to start or continue the static delay
            if artist_static_delay < static_delay_frames:
                # Show static text for 1 second before scrolling
                artist_static_delay += 1
                scroll_offset = 0  # Static position
            else:
                # Start scrolling after delay
                scroll_offset = artist_scroll_pos
            
            # Draw text with wraparound - draw it twice to create seamless loop
            for offset in [0, total_artist_width + 20]:  # Draw original and wrapped version
                display_x = artist_x - scroll_offset + offset
                current_x = display_x
                for char in artist_name:
                    if char == ' ':
                        # Space between words - make it smaller
                        current_x += 2  # Reduced space width
                    else:
                        # Only draw if within visible area (with some buffer)
                        if current_x > artist_x - 8 and current_x < matrix.width + 8:
                            char_width = graphics.DrawText(canvas, artist_font, current_x, artist_y, artist_color, char)
                            current_x += char_width - 1  # Normal character spacing
                        else:
                            current_x += 4  # Estimated width when not drawing
            
            # Clear pixels outside the allowed boundaries (clipping effect)
            # Clear everything to the left of artist area (including album cover area for this text row)
            for x in range(0, artist_x):
                for y in range(artist_y - 6, artist_y + 2):
                    if 0 <= y < matrix.height:
                        canvas.SetPixel(x, y, 0, 0, 0)
            
            # Clear right side (after artist area)
            for x in range(max_x, matrix.width):
                for y in range(artist_y - 6, artist_y + 2):
                    if 0 <= y < matrix.height:
                        canvas.SetPixel(x, y, 0, 0, 0)
            
            # Update scroll position only after static delay
            if artist_static_delay >= static_delay_frames and scroll_counter % 4 == 0:
                artist_scroll_pos += 1
                if artist_scroll_pos >= total_artist_width + 20:  # Reset when first copy is off-screen
                    artist_scroll_pos = 0
                    artist_static_delay = 0  # Reset static delay for next cycle
        else:
            # Static display if text fits
            current_x = artist_x
            for char in artist_name:
                if current_x + 5 > max_x:
                    break
                if char == ' ':
                    # Space between words - make it smaller
                    current_x += 2  # Reduced space width
                else:
                    char_width = graphics.DrawText(canvas, artist_font, current_x, artist_y, artist_color, char)
                    current_x += char_width - 1  # Normal character spacing
        
        # Album name in lower right in normal case (scrolls if cut off)
        album_x = padding + 20 + padding  # 2px + 20px album + 2px spacing = x=24
        album_y = canvas.height - padding - 2  # 2px from bottom edge (proper padding)
        max_x = canvas.width - padding  # Maximum x position (2px from right edge)
        album_available_width = max_x - album_x  # Available width for album text
        
        # Calculate total text width
        total_album_width = 0
        for char in album_name:
            total_album_width += 4  # Rough estimate: 4 pixels per char
        
        # Check if scrolling is needed
        if total_album_width > album_available_width:
            # Check if we need to start or continue the static delay
            if album_static_delay < static_delay_frames:
                # Show static text for 1 second before scrolling
                album_static_delay += 1
                scroll_offset = 0  # Static position
            else:
                # Start scrolling after delay
                scroll_offset = album_scroll_pos
            
            # Draw text with wraparound - draw it twice to create seamless loop
            for offset in [0, total_album_width + 20]:  # Draw original and wrapped version
                display_x = album_x - scroll_offset + offset
                current_x = display_x
                for char in album_name:
                    if char == ' ':
                        # Space between words - make it smaller
                        current_x += 2  # Reduced space width
                    else:
                        # Only draw if within visible area (with some buffer)
                        if current_x > album_x - 8 and current_x < matrix.width + 8:
                            char_width = graphics.DrawText(canvas, album_font, current_x, album_y, album_color, char)
                            current_x += char_width - 1  # Normal character spacing
                        else:
                            current_x += 4  # Estimated width when not drawing
            
            # Clear pixels outside the allowed boundaries (clipping effect)
            # Clear everything to the left of album area (including album cover area for this text row)
            for x in range(0, album_x):
                for y in range(album_y - 6, album_y + 2):
                    if 0 <= y < matrix.height:
                        canvas.SetPixel(x, y, 0, 0, 0)
            
            # Clear right side (after album area)
            for x in range(max_x, matrix.width):
                for y in range(album_y - 6, album_y + 2):
                    if 0 <= y < matrix.height:
                        canvas.SetPixel(x, y, 0, 0, 0)
            
            # Update scroll position only after static delay
            if album_static_delay >= static_delay_frames and scroll_counter % 4 == 0:
                album_scroll_pos += 1
                if album_scroll_pos >= total_album_width + 20:  # Reset when first copy is off-screen
                    album_scroll_pos = 0
                    album_static_delay = 0  # Reset static delay for next cycle
        else:
            # Static display if text fits
            current_x = album_x
            for char in album_name:
                if current_x + 5 > max_x:
                    break
                if char == ' ':
                    # Space between words - make it smaller
                    current_x += 2  # Reduced space width
                else:
                    char_width = graphics.DrawText(canvas, album_font, current_x, album_y, album_color, char)
                    current_x += char_width - 1  # Normal character spacing
        
        # Redraw album cover after text clipping to ensure it's always visible
        # (Text clipping may have cleared parts of the album cover)
        canvas.SetImage(image_rgb, image_x, image_y)
        
        # Increment scroll counter for timing
        scroll_counter += 1
        
        canvas = matrix.SwapOnVSync(canvas)
        time.sleep(0.05)  # Faster refresh for smooth scrolling
        
except KeyboardInterrupt:
    sys.exit(0)
