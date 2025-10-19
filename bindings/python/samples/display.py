#!/usr/bin/env python
"""
Display management for RGB LED matrix.

This module provides classes for:
- ScrollableText: Handles smooth text scrolling with configurable styling
- MatrixDisplay: Manages LED matrix operations and rendering
"""

from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from PIL import Image
from config import DisplayConfig


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
            if self.static_delay >= self.static_delay_frames and scroll_counter % DisplayConfig.SCROLL_SPEED_DIVIDER == 0:
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


class MatrixDisplay:
    """Manages the LED matrix display operations."""
    
    def __init__(self, options=None):
        if options is None:
            options = RGBMatrixOptions()
            options.rows = DisplayConfig.MATRIX_ROWS
            options.cols = DisplayConfig.MATRIX_COLS
            options.chain_length = DisplayConfig.CHAIN_LENGTH
            options.parallel = DisplayConfig.PARALLEL
            options.hardware_mapping = DisplayConfig.HARDWARE_MAPPING
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        self.width = self.matrix.width
        self.height = self.matrix.height
        
        # Load fonts
        self._load_fonts()
        
        # Define colors using config
        self.song_color = graphics.Color(*DisplayConfig.SONG_COLOR)
        self.artist_color = graphics.Color(*DisplayConfig.ARTIST_COLOR)
        self.album_color = graphics.Color(*DisplayConfig.ALBUM_COLOR)
        
        # Create scrollable text instances
        self.song_font = graphics.Font()
        self.song_font.LoadFont(DisplayConfig.FONT_PATH)
        self.artist_font = graphics.Font()
        self.artist_font.LoadFont(DisplayConfig.FONT_PATH)
        self.album_font = graphics.Font()
        self.album_font.LoadFont(DisplayConfig.FONT_PATH)
        
        self.song_text = ScrollableText(self.song_font, self.song_color, DisplayConfig.STATIC_DELAY_FRAMES)
        self.artist_text = ScrollableText(self.artist_font, self.artist_color, DisplayConfig.STATIC_DELAY_FRAMES)
        self.album_text = ScrollableText(self.album_font, self.album_color, DisplayConfig.STATIC_DELAY_FRAMES)
        
        # Layout constants from config
        self.padding = DisplayConfig.PADDING
        self.image_size = DisplayConfig.IMAGE_SIZE
    
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


class StockMatrixDisplay(MatrixDisplay):
    """Extended matrix display class for stock-specific functionality."""
    
    def __init__(self, options=None):
        super().__init__(options)
        
        # Load stock-specific fonts
        self.font_large = graphics.Font()
        self.font_large.LoadFont("../../../fonts/5x7.bdf")
        self.font_small = graphics.Font()
        self.font_small.LoadFont("../../../fonts/4x6.bdf")
        
        # Stock-specific colors
        from stock_config import StockConfig
        self.stock_colors = {}
        for name, rgb in StockConfig.STOCK_COLORS.items():
            self.stock_colors[name] = graphics.Color(*rgb)
    
    def draw_stock_info(self, symbol, price, change, change_percent):
        """Draw stock symbol, price, and change information."""
        # Determine color based on change
        is_positive = change >= 0
        left_color = self.stock_colors['neutral']  # White for symbol/price
        right_color = (self.stock_colors['gain_bright'] if is_positive 
                      else self.stock_colors['loss_bright'])
        
        # Left side - Stock symbol and price (white)
        graphics.DrawText(self.canvas, self.font_large, 2, 8, left_color, symbol)
        price_text = f"{price:.2f}"
        graphics.DrawText(self.canvas, self.font_large, 2, 15, left_color, price_text)
        
        # Right side - Change amount and percentage (colored)
        # Right-aligned text
        change_text = f"{change:+.2f}"
        change_width = self._measure_text_width(change_text, self.font_large)
        change_x = self.width - change_width - 2
        graphics.DrawText(self.canvas, self.font_large, change_x, 8, right_color, change_text)
        
        pct_text = f"{change_percent:+.1f}%"
        pct_width = self._measure_text_width(pct_text, self.font_large)
        pct_x = self.width - pct_width - 2
        graphics.DrawText(self.canvas, self.font_large, pct_x, 15, right_color, pct_text)
    
    def _measure_text_width(self, text, font):
        """Measure text width by drawing off-screen."""
        return graphics.DrawText(self.canvas, font, 1000, 8, 
                               self.stock_colors['neutral'], text)
    
    def draw_loading_message(self, symbol=None):
        """Draw a loading message."""
        if symbol:
            graphics.DrawText(self.canvas, self.font_large, 1, 10, 
                            self.stock_colors['neutral'], symbol)
        graphics.DrawText(self.canvas, self.font_large, 1, 22, 
                        self.stock_colors['neutral'], "Loading...")
    
    def get_chart_area(self):
        """Get the chart drawing area coordinates."""
        return {
            'x': 0,
            'y': 16,  # Bottom half of 32px display
            'width': self.width,
            'height': 16
        }