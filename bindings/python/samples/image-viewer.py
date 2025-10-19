#!/usr/bin/env python
import time
import sys
import requests
import base64
import json
import os
import threading  # Re-enabling threading for seamless updates
import queue  # For thread-safe communication
from io import BytesIO
from dotenv import load_dotenv

from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from PIL import Image, ImageDraw, ImageFont


class ScrollableText:
    """Handles scrolling text display with smooth animations and configurable styling."""
    
    def __init__(self, font, color, static_delay_frames=50):
        self.font = font
        self.color = color
        self.static_delay_frames = static_delay_frames
        self.scroll_pos = 0
        self.static_delay = 0
        
    def reset_scroll(self):
        """Reset scrolling position and delay."""
        self.scroll_pos = 0
        self.static_delay = 0
    
    def _calculate_text_width(self, text):
        """Calculate approximate text width."""
        return len(text) * 4  # Rough estimate: 4 pixels per char for 5x7 font
    
    def _draw_character(self, canvas, char, x, y):
        """Draw a single character and return its width."""
        if char == ' ':
            return 2  # Reduced space width
        else:
            char_width = graphics.DrawText(canvas, self.font, x, y, self.color, char)
            return char_width - 1  # Normal character spacing
    
    def _clear_boundaries(self, canvas, start_x, end_x, y, matrix_height, matrix_width):
        """Clear pixels outside the allowed text boundaries."""
        # Clear left side
        for x in range(0, start_x):
            for clear_y in range(y - 6, y + 2):
                if 0 <= clear_y < matrix_height:
                    canvas.SetPixel(x, clear_y, 0, 0, 0)
        
        # Clear right side
        for x in range(end_x, matrix_width):
            for clear_y in range(y - 6, y + 2):
                if 0 <= clear_y < matrix_height:
                    canvas.SetPixel(x, clear_y, 0, 0, 0)
    
    def draw(self, canvas, text, x, y, max_x, scroll_counter, matrix_width, matrix_height):
        """Draw text with scrolling if needed."""
        available_width = max_x - x
        total_text_width = self._calculate_text_width(text)
        
        if total_text_width > available_width:
            # Scrolling needed
            if self.static_delay < self.static_delay_frames:
                self.static_delay += 1
                scroll_offset = 0
            else:
                scroll_offset = self.scroll_pos
            
            # Draw text with wraparound
            for offset in [0, total_text_width + 10]:
                display_x = x - scroll_offset + offset
                current_x = display_x
                for char in text:
                    if current_x > x - 8 and current_x < matrix_width + 8:
                        char_width = self._draw_character(canvas, char, current_x, y)
                        current_x += char_width
                    else:
                        current_x += 4 if char != ' ' else 2
            
            # Clear boundaries
            self._clear_boundaries(canvas, x, max_x, y, matrix_height, matrix_width)
            
            # Update scroll position
            if self.static_delay >= self.static_delay_frames and scroll_counter % 3 == 0:
                self.scroll_pos += 1
                if self.scroll_pos >= total_text_width + 10:
                    self.scroll_pos = 0
                    self.static_delay = 0
        else:
            # Static display
            current_x = x
            for char in text:
                if current_x + 5 > max_x:
                    break
                char_width = self._draw_character(canvas, char, current_x, y)
                current_x += char_width

class SpotifyClient:
    """Handles Spotify API authentication and track information retrieval."""
    
    def __init__(self):
        self.client_id = os.getenv("SPOTIFY_CLIENT_ID")
        self.client_secret = os.getenv("SPOTIFY_CLIENT_SECRET")
        self.refresh_token = os.getenv("SPOTIFY_REFRESH_TOKEN")
        self.last_error_time = None
        
    def _validate_credentials(self):
        """Check if all required Spotify credentials are available."""
        if not all([self.client_id, self.client_secret, self.refresh_token]):
            print("Error: Spotify credentials not found. Please check your .env file.")
            print(f"CLIENT_ID present: {bool(self.client_id)}")
            print(f"CLIENT_SECRET present: {bool(self.client_secret)}")
            print(f"REFRESH_TOKEN present: {bool(self.refresh_token)}")
            return False
        return True
    
    def get_access_token(self):
        """Get access token using refresh token."""
        if not self._validate_credentials():
            return None
            
        auth_str = f"{self.client_id}:{self.client_secret}"
        auth_bytes = auth_str.encode("ascii")
        auth_b64 = base64.b64encode(auth_bytes).decode("ascii")
        
        headers = {
            "Authorization": f"Basic {auth_b64}",
            "Content-Type": "application/x-www-form-urlencoded"
        }
        
        data = {
            "grant_type": "refresh_token",
            "refresh_token": self.refresh_token
        }
        
        try:
            response = requests.post("https://accounts.spotify.com/api/token", headers=headers, data=data)
            print(f"Token request status: {response.status_code}")
            
            if response.status_code == 200:
                return response.json()["access_token"]
            else:
                print(f"Token request failed: {response.text}")
                return None
        except requests.RequestException as e:
            print(f"Network error getting access token: {e}")
            return None
    
    def get_current_playing(self):
        """Get currently playing track from Spotify."""
        access_token = self.get_access_token()
        if not access_token:
            print("Failed to get Spotify access token")
            return None, None, None, None
        
        headers = {"Authorization": f"Bearer {access_token}"}
        
        try:
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
                return self._parse_track_response(response)
                
        except requests.RequestException as e:
            print(f"Network error getting current playing: {e}")
            return None, None, None, None
        
        return None, None, None, None
    
    def _parse_track_response(self, response):
        """Parse the Spotify API response for track information."""
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
                        image_url = album_images[-1]["url"]  # Get the smallest image
                        image = self._fetch_album_image(image_url)
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
        
        return None, None, None, None
    
    def _fetch_album_image(self, image_url):
        """Fetch and return album cover image."""
        try:
            img_response = requests.get(image_url)
            if img_response.status_code == 200:
                return Image.open(BytesIO(img_response.content))
        except requests.RequestException as e:
            print(f"Error fetching album image: {e}")
        return None
    
    def should_skip_update(self):
        """Check if we should skip update due to recent network error."""
        current_time = time.time()
        return (self.last_error_time and 
                current_time - self.last_error_time < 3)
    
    def mark_network_error(self):
        """Mark that a network error occurred."""
        self.last_error_time = time.time()


# Load environment variables from .env file
load_dotenv()


class SpotifyUpdateThread:
    """Handles background Spotify updates using a separate thread."""
    
    def __init__(self, spotify_client, update_queue):
        self.spotify_client = spotify_client
        self.update_queue = update_queue
        self.running = True
    
    def stop(self):
        """Stop the background thread."""
        self.running = False
    
    def run(self):
        """Background thread that continuously updates Spotify data."""
        while self.running:
            try:
                current_song, current_artist, current_album, current_image = self.spotify_client.get_current_playing()
                if current_song and current_artist:
                    # Prepare the update data
                    update_data = {
                        'song_name': current_song,
                        'artist_name': current_artist.upper(),
                        'album_name': current_album if current_album else "Album Name",
                        'image': None
                    }
                    
                    if current_image:
                        # Resize image to 20x20
                        resample_mode = getattr(Image, "Resampling", Image).LANCZOS
                        current_image.thumbnail((20, 20), resample=resample_mode)
                        update_data['image'] = current_image.convert('RGB')
                    
                    # Put update in queue (non-blocking)
                    try:
                        self.update_queue.put_nowait(update_data)
                        print(f"Queued update: {current_song} by {current_artist} from {current_album}")
                    except queue.Full:
                        pass  # Skip if queue is full
                else:
                    # Put default data in queue
                    default_data = {
                        'song_name': "No music playing",
                        'artist_name': "SPOTIFY",
                        'album_name': "Album Name",
                        'image': None
                    }
                    try:
                        self.update_queue.put_nowait(default_data)
                    except queue.Full:
                        pass
                
                time.sleep(0.5)  # Update every 0.5 seconds
            except Exception as e:
                print(f"Background thread error: {e}")
                time.sleep(2)  # Wait on error


class MatrixDisplay:
    """Manages the LED matrix display operations."""
    
    def __init__(self, options=None):
        if options is None:
            options = RGBMatrixOptions()
            options.rows = 32
            options.cols = 64
            options.chain_length = 1
            options.parallel = 1
            options.hardware_mapping = 'adafruit-hat-pwm'
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        self.width = self.matrix.width
        self.height = self.matrix.height
        
        # Load fonts
        self._load_fonts()
        
        # Define colors
        self.song_color = graphics.Color(0, 255, 0)      # Green for song title
        self.artist_color = graphics.Color(255, 255, 255) # White for artist name
        self.album_color = graphics.Color(200, 200, 200)  # Light gray for album name
        
        # Create scrollable text instances
        self.song_font = graphics.Font()
        self.song_font.LoadFont("../../../fonts/5x7.bdf")
        self.artist_font = graphics.Font()
        self.artist_font.LoadFont("../../../fonts/5x7.bdf")
        self.album_font = graphics.Font()
        self.album_font.LoadFont("../../../fonts/5x7.bdf")
        
        self.song_text = ScrollableText(self.song_font, self.song_color)
        self.artist_text = ScrollableText(self.artist_font, self.artist_color)
        self.album_text = ScrollableText(self.album_font, self.album_color)
        
        # Layout constants
        self.padding = 2
        self.image_size = 20
    
    def _load_fonts(self):
        """Load fonts for text display."""
        pass  # Fonts are loaded in __init__ for now
    
    def clear(self):
        """Clear the display canvas."""
        self.canvas.Clear()
    
    def draw_image(self, image, x=None, y=None):
        """Draw an image on the canvas."""
        if x is None:
            x = self.padding
        if y is None:
            y = self.height - image.height - self.padding
        
        self.canvas.SetImage(image, x, y)
        return x, y
    
    def draw_song_title(self, song_name, scroll_counter):
        """Draw the song title with scrolling if needed."""
        song_x = self.padding
        song_y = self.padding + 6
        max_x = self.width - self.padding
        
        self.song_text.draw(self.canvas, song_name.upper(), song_x, song_y, 
                           max_x, scroll_counter, self.width, self.height)
    
    def draw_artist_name(self, artist_name, scroll_counter):
        """Draw the artist name with scrolling if needed."""
        artist_x = self.padding + self.image_size + self.padding
        artist_y = self.height // 2 + 2
        max_x = self.width - self.padding
        
        self.artist_text.draw(self.canvas, artist_name, artist_x, artist_y,
                             max_x, scroll_counter, self.width, self.height)
    
    def draw_album_name(self, album_name, scroll_counter):
        """Draw the album name with scrolling if needed."""
        album_x = self.padding + self.image_size + self.padding
        album_y = self.height - self.padding - 2
        max_x = self.width - self.padding
        
        self.album_text.draw(self.canvas, album_name, album_x, album_y,
                            max_x, scroll_counter, self.width, self.height)
    
    def swap_canvas(self):
        """Swap the canvas to display the updated content."""
        self.canvas = self.matrix.SwapOnVSync(self.canvas)


class DisplayConfig:
    """Configuration constants for the display application."""
    
    # Matrix configuration
    MATRIX_ROWS = 32
    MATRIX_COLS = 64
    CHAIN_LENGTH = 1
    PARALLEL = 1
    HARDWARE_MAPPING = 'adafruit-hat-pwm'
    
    # Display settings
    PADDING = 2
    IMAGE_SIZE = 20
    REFRESH_RATE = 0.03  # 33fps
    
    # Font paths
    FONT_PATH = "../../../fonts/5x7.bdf"
    
    # Spotify update settings
    SPOTIFY_UPDATE_INTERVAL = 0.5
    SPOTIFY_ERROR_RETRY_DELAY = 2
    SPOTIFY_NETWORK_ERROR_DELAY = 3
    
    # Scrolling settings
    STATIC_DELAY_FRAMES = 50  # 1.5 seconds at 33fps
    SCROLL_SPEED_DIVIDER = 3  # Every 3rd frame


class ImageViewer:
    """Main application class that orchestrates the image viewer functionality."""
    
    def __init__(self, use_spotify=True, image_file=None, song_name=None, 
                 artist_name=None, album_name=None):
        self.use_spotify = use_spotify
        self.running = True
        self.scroll_counter = 0
        
        # Initialize display
        self.display = MatrixDisplay()
        
        # Initialize Spotify if needed
        if self.use_spotify:
            self.spotify_client = SpotifyClient()
            self.spotify_update_queue = queue.Queue(maxsize=1)
            self.spotify_update_thread = SpotifyUpdateThread(
                self.spotify_client, self.spotify_update_queue
            )
            self.current_spotify_data = {
                'song_name': "Loading...",
                'artist_name': "SPOTIFY", 
                'album_name': "Album Name",
                'image': None
            }
        else:
            # Use provided data
            self.current_spotify_data = {
                'song_name': song_name or "Song Name",
                'artist_name': (artist_name or "Artist Name").upper(),
                'album_name': album_name or "Album Name",
                'image': self._load_image(image_file) if image_file else None
            }
    
    def _load_image(self, image_file):
        """Load and resize an image file."""
        try:
            image = Image.open(image_file)
            resample_mode = getattr(Image, "Resampling", Image).LANCZOS
            image.thumbnail((DisplayConfig.IMAGE_SIZE, DisplayConfig.IMAGE_SIZE), 
                           resample=resample_mode)
            return image.convert('RGB')
        except Exception as e:
            print(f"Error loading image {image_file}: {e}")
            return Image.new('RGB', (DisplayConfig.IMAGE_SIZE, DisplayConfig.IMAGE_SIZE), (50, 50, 50))
    
    def start(self):
        """Start the image viewer application."""
        try:
            if self.use_spotify:
                print("Using Spotify integration - starting background update thread")
                thread = threading.Thread(target=self.spotify_update_thread.run, daemon=True)
                thread.start()
            
            print("Press CTRL-C to stop.")
            self._main_loop()
            
        except KeyboardInterrupt:
            print("\nShutting down...")
            self.stop()
    
    def stop(self):
        """Stop the application and cleanup resources."""
        self.running = False
        if self.use_spotify:
            self.spotify_update_thread.stop()
            print("Stopping background thread...")
            time.sleep(0.1)
    
    def _update_spotify_data(self):
        """Update Spotify data from the queue if available."""
        if not self.use_spotify:
            return
        
        try:
            update_data = self.spotify_update_queue.get_nowait()
            self.current_spotify_data.update(update_data)
            self.spotify_update_queue.task_done()
        except queue.Empty:
            pass  # No update available
    
    def _main_loop(self):
        """Main display loop."""
        # Prepare initial image
        current_image = (self.current_spotify_data['image'] or 
                        Image.new('RGB', (DisplayConfig.IMAGE_SIZE, DisplayConfig.IMAGE_SIZE), (50, 50, 50)))
        
        while self.running:
            # Check for Spotify updates
            self._update_spotify_data()
            
            # Get current data
            song_name = self.current_spotify_data['song_name']
            artist_name = self.current_spotify_data['artist_name']
            album_name = self.current_spotify_data['album_name']
            
            if self.current_spotify_data['image'] is not None:
                current_image = self.current_spotify_data['image']
            
            # Clear and draw
            self.display.clear()
            
            # Draw album cover
            image_x, image_y = self.display.draw_image(current_image)
            
            # Draw text elements
            self.display.draw_song_title(song_name, self.scroll_counter)
            self.display.draw_artist_name(artist_name, self.scroll_counter)
            self.display.draw_album_name(album_name, self.scroll_counter)
            
            # Redraw album cover to ensure it's always visible
            self.display.draw_image(current_image, image_x, image_y)
            
            # Update display
            self.display.swap_canvas()
            
            # Increment scroll counter and sleep
            self.scroll_counter += 1
            time.sleep(DisplayConfig.REFRESH_RATE)



def main():
    """Main entry point for the application."""
    if len(sys.argv) > 1:
        # Use provided image file
        image_file = sys.argv[1]
        song_name = sys.argv[2] if len(sys.argv) > 2 else "Song Name"
        artist_name = sys.argv[3] if len(sys.argv) > 3 else "Artist Name"
        album_name = sys.argv[4] if len(sys.argv) > 4 else "Album Name"
        
        viewer = ImageViewer(
            use_spotify=False,
            image_file=image_file,
            song_name=song_name,
            artist_name=artist_name,
            album_name=album_name
        )
    else:
        # Use Spotify integration
        viewer = ImageViewer(use_spotify=True)
    
    viewer.start()


if __name__ == "__main__":
    main()
