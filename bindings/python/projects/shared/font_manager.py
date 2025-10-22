#!/usr/bin/env python
"""
FontManager - Centralized font loading and management.

Provides a clean interface for loading and managing fonts with automatic
path resolution and caching to avoid redundant font loading.
"""

import os
from rgbmatrix import graphics


class FontManager:
    """
    Manages font loading and caching for RGB LED Matrix applications.
    
    Provides predefined font sizes and automatic path resolution.
    Caches loaded fonts to improve performance.
    """
    
    # Font size definitions with common use cases
    FONTS = {
        'tiny': '4x6.bdf',           # Very small text, status indicators
        'small': '5x7.bdf',         # Small details, secondary text
        'medium': '6x10.bdf',       # Medium text, labels
        'medium_bold': '6x13B.bdf', # Bold medium text, emphasized labels
        'large': '7x13.bdf',        # Large text, main content
        'large_bold': '7x13B.bdf',  # Bold large text, headings
        'xlarge': '8x13B.bdf',      # Extra large text, timers
        'xxlarge': '9x18B.bdf',     # Very large text, prominent displays
        'tom_thumb': 'tom-thumb.bdf' # Extremely small, special use
    }
    
    def __init__(self):
        """Initialize the font manager with empty cache."""
        self._font_cache = {}
        self._font_base_path = self._get_font_path()
        
    def _get_font_path(self):
        """Get the absolute path to the fonts directory."""
        # Get the path relative to this file
        current_dir = os.path.dirname(__file__)
        font_path = os.path.abspath(os.path.join(current_dir, '../../../../fonts'))
        return font_path
        
    def get_font(self, size_name):
        """
        Get a font by size name.
        
        Args:
            size_name: Font size name from FontManager.FONTS
            
        Returns:
            graphics.Font object
            
        Raises:
            ValueError: If size_name is not recognized
            FileNotFoundError: If font file cannot be found
        """
        if size_name not in self.FONTS:
            available = ', '.join(self.FONTS.keys())
            raise ValueError(f"Unknown font size '{size_name}'. Available: {available}")
            
        # Check cache first
        if size_name in self._font_cache:
            return self._font_cache[size_name]
            
        # Load font
        font_filename = self.FONTS[size_name]
        font_path = os.path.join(self._font_base_path, font_filename)
        
        if not os.path.exists(font_path):
            raise FileNotFoundError(f"Font file not found: {font_path}")
            
        font = graphics.Font()
        try:
            font.LoadFont(font_path)
        except Exception as e:
            raise RuntimeError(f"Failed to load font {font_path}: {e}")
            
        # Cache and return
        self._font_cache[size_name] = font
        return font
        
    def load_custom_font(self, font_path, cache_key=None):
        """
        Load a custom font from a specific path.
        
        Args:
            font_path: Absolute or relative path to font file
            cache_key: Optional cache key (defaults to font_path)
            
        Returns:
            graphics.Font object
        """
        cache_key = cache_key or font_path
        
        # Check cache first
        if cache_key in self._font_cache:
            return self._font_cache[cache_key]
            
        # Resolve relative paths
        if not os.path.isabs(font_path):
            font_path = os.path.join(self._font_base_path, font_path)
            
        if not os.path.exists(font_path):
            raise FileNotFoundError(f"Font file not found: {font_path}")
            
        font = graphics.Font()
        try:
            font.LoadFont(font_path)
        except Exception as e:
            raise RuntimeError(f"Failed to load font {font_path}: {e}")
            
        # Cache and return
        self._font_cache[cache_key] = font
        return font
        
    def get_available_fonts(self):
        """
        Get a list of available predefined font sizes.
        
        Returns:
            List of font size names
        """
        return list(self.FONTS.keys())
        
    def clear_cache(self):
        """Clear the font cache to free memory."""
        self._font_cache.clear()
        
    def get_text_dimensions(self, font, text, kerning=0):
        """
        Calculate text dimensions for layout planning.
        
        Args:
            font: Font object (from get_font or load_custom_font)
            text: Text to measure
            kerning: Additional character spacing
            
        Returns:
            Tuple of (width, height)
        """
        width = sum(font.CharacterWidth(ord(c)) + kerning for c in text)
        height = font.height
        return width, height
        
    def get_font_info(self, font):
        """
        Get information about a font.
        
        Args:
            font: Font object
            
        Returns:
            Dict with 'height' and 'baseline' keys
        """
        return {
            'height': font.height,
            'baseline': font.baseline
        }