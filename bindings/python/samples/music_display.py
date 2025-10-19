#!/usr/bin/env python
"""
Music Display Application

A Raspberry Pi RGB LED matrix display that shows currently playing music
information from Spotify or static content. Features smooth scrolling text,
album art display, and real-time updates.

Usage:
    # Spotify mode (requires .env file with Spotify credentials)
    python music_display.py
    
    # Static mode with image and text
    python music_display.py <image_file> [song_name] [artist_name] [album_name]
"""

import sys
import time
import queue
import threading
from PIL import Image
from dotenv import load_dotenv

from spotify_client import SpotifyClient, SpotifyUpdateThread
from display import MatrixDisplay
from config import DisplayConfig


class MusicDisplay:
    """Main application class that orchestrates the music display functionality."""
    
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
            self.current_data = {
                'song_name': "Loading...",
                'artist_name': "SPOTIFY", 
                'album_name': "Album Name",
                'image': None
            }
        else:
            # Use provided data
            self.current_data = {
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
            return Image.new('RGB', 
                           (DisplayConfig.IMAGE_SIZE, DisplayConfig.IMAGE_SIZE), 
                           DisplayConfig.DEFAULT_IMAGE_COLOR)
    
    def start(self):
        """Start the music display application."""
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
            self.current_data.update(update_data)
            self.spotify_update_queue.task_done()
        except queue.Empty:
            pass  # No update available
    
    def _main_loop(self):
        """Main display loop."""
        # Prepare initial image
        current_image = (self.current_data['image'] or 
                        Image.new('RGB', 
                                 (DisplayConfig.IMAGE_SIZE, DisplayConfig.IMAGE_SIZE), 
                                 DisplayConfig.DEFAULT_IMAGE_COLOR))
        
        while self.running:
            # Check for Spotify updates
            self._update_spotify_data()
            
            # Get current data
            song_name = self.current_data['song_name']
            artist_name = self.current_data['artist_name']
            album_name = self.current_data['album_name']
            
            if self.current_data['image'] is not None:
                current_image = self.current_data['image']
            
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
    # Load environment variables
    load_dotenv()
    
    if len(sys.argv) > 1:
        # Use provided image file
        image_file = sys.argv[1]
        song_name = sys.argv[2] if len(sys.argv) > 2 else "Song Name"
        artist_name = sys.argv[3] if len(sys.argv) > 3 else "Artist Name"
        album_name = sys.argv[4] if len(sys.argv) > 4 else "Album Name"
        
        display = MusicDisplay(
            use_spotify=False,
            image_file=image_file,
            song_name=song_name,
            artist_name=artist_name,
            album_name=album_name
        )
    else:
        # Use Spotify integration
        display = MusicDisplay(use_spotify=True)
    
    display.start()


if __name__ == "__main__":
    main()