#!/usr/bin/env python
"""
Configuration constants for the music display application.

This module contains all configurable parameters for:
- Matrix hardware settings
- Display layout and styling
- Timing and refresh rates
- Font and color configurations
"""


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
    FONT_PATH = "../../../../fonts/5x7.bdf"
    
    # Spotify update settings
    SPOTIFY_UPDATE_INTERVAL = 0.5
    SPOTIFY_ERROR_RETRY_DELAY = 2
    SPOTIFY_NETWORK_ERROR_DELAY = 3
    
    # Scrolling settings
    STATIC_DELAY_FRAMES = 50  # 1.5 seconds at 33fps
    SCROLL_SPEED_DIVIDER = 3  # Every 3rd frame
    
    # Colors (RGB values)
    SONG_COLOR = (0, 255, 0)      # Green for song title
    ARTIST_COLOR = (255, 255, 255) # White for artist name
    ALBUM_COLOR = (200, 200, 200)  # Light gray for album name
    DEFAULT_IMAGE_COLOR = (50, 50, 50)  # Default image background