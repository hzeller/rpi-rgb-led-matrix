#!/usr/bin/env python
"""
ColorPalette - Centralized color definitions and management.

Provides predefined color schemes and utilities for consistent
color usage across all matrix applications.
"""

from rgbmatrix import graphics


class ColorPalette:
    """
    Centralized color definitions and management for RGB LED Matrix applications.
    
    Provides predefined colors, themes, and utility methods for color operations.
    """
    
    # Basic colors
    BLACK = (0, 0, 0)
    WHITE = (255, 255, 255)
    RED = (255, 0, 0)
    GREEN = (0, 255, 0)
    BLUE = (0, 0, 255)
    YELLOW = (255, 255, 0)
    CYAN = (0, 255, 255)
    MAGENTA = (255, 0, 255)
    
    # Grays
    GRAY_DARK = (64, 64, 64)
    GRAY = (128, 128, 128)
    GRAY_LIGHT = (192, 192, 192)
    GRAY_VERY_LIGHT = (224, 224, 224)
    
    # Extended palette
    ORANGE = (255, 165, 0)
    ORANGE_RED = (255, 69, 0)
    PURPLE = (128, 0, 128)
    PINK = (255, 192, 203)
    LIME = (0, 255, 0)
    NAVY = (0, 0, 128)
    TEAL = (0, 128, 128)
    MAROON = (128, 0, 0)
    
    # Status colors
    SUCCESS = (0, 255, 0)        # Green
    WARNING = (255, 165, 0)      # Orange
    ERROR = (255, 0, 0)          # Red
    INFO = (0, 150, 255)         # Light blue
    
    # Financial colors (for stock displays)
    GAIN_BRIGHT = (0, 255, 0)    # Bright green
    GAIN_DIM = (0, 180, 0)       # Dim green
    LOSS_BRIGHT = (255, 0, 0)    # Bright red
    LOSS_DIM = (180, 0, 0)       # Dim red
    
    # Theme collections
    THEMES = {
        'default': {
            'primary': WHITE,
            'secondary': GRAY_LIGHT,
            'accent': BLUE,
            'background': BLACK,
            'success': SUCCESS,
            'warning': WARNING,
            'error': ERROR
        },
        'dark': {
            'primary': WHITE,
            'secondary': GRAY,
            'accent': CYAN,
            'background': BLACK,
            'success': GREEN,
            'warning': ORANGE,
            'error': RED
        },
        'retro': {
            'primary': LIME,
            'secondary': YELLOW,
            'accent': MAGENTA,
            'background': BLACK,
            'success': LIME,
            'warning': YELLOW,
            'error': RED
        },
        'matrix': {
            'primary': (0, 255, 0),      # Matrix green
            'secondary': (0, 180, 0),    # Dimmer green
            'accent': (0, 255, 100),     # Light green
            'background': BLACK,
            'success': (0, 255, 0),
            'warning': (255, 255, 0),
            'error': (255, 0, 0)
        }
    }
    
    def __init__(self, theme='default'):
        """
        Initialize color palette with a specific theme.
        
        Args:
            theme: Theme name from THEMES dictionary
        """
        self.set_theme(theme)
        
    def set_theme(self, theme_name):
        """
        Set the active theme.
        
        Args:
            theme_name: Name of theme from THEMES dictionary
            
        Raises:
            ValueError: If theme_name is not recognized
        """
        if theme_name not in self.THEMES:
            available = ', '.join(self.THEMES.keys())
            raise ValueError(f"Unknown theme '{theme_name}'. Available: {available}")
            
        self.current_theme = theme_name
        theme = self.THEMES[theme_name]
        
        # Set theme colors as attributes
        self.primary = theme['primary']
        self.secondary = theme['secondary']
        self.accent = theme['accent']
        self.background = theme['background']
        self.success = theme['success']
        self.warning = theme['warning']
        self.error = theme['error']
        
    def get_color(self, color_spec):
        """
        Convert various color specifications to graphics.Color object.
        
        Args:
            color_spec: Can be:
                - Tuple of (r, g, b)
                - Attribute name (e.g., 'primary', 'WHITE')
                - graphics.Color object (returned as-is)
                
        Returns:
            graphics.Color object
        """
        if isinstance(color_spec, graphics.Color):
            return color_spec
        elif isinstance(color_spec, (tuple, list)) and len(color_spec) == 3:
            return graphics.Color(*color_spec)
        elif isinstance(color_spec, str):
            # Try theme colors first
            if hasattr(self, color_spec.lower()):
                rgb = getattr(self, color_spec.lower())
                return graphics.Color(*rgb)
            # Try class constants
            elif hasattr(self, color_spec.upper()):
                rgb = getattr(self, color_spec.upper())
                return graphics.Color(*rgb)
            else:
                raise ValueError(f"Unknown color specification: {color_spec}")
        else:
            raise TypeError(f"Invalid color specification type: {type(color_spec)}")
            
    def dim_color(self, color_spec, factor=0.5):
        """
        Create a dimmed version of a color.
        
        Args:
            color_spec: Color specification (see get_color)
            factor: Dimming factor (0.0 = black, 1.0 = original)
            
        Returns:
            graphics.Color object
        """
        if isinstance(color_spec, graphics.Color):
            rgb = (color_spec.red, color_spec.green, color_spec.blue)
        else:
            color = self.get_color(color_spec)
            rgb = (color.red, color.green, color.blue)
            
        dimmed = tuple(int(c * factor) for c in rgb)
        return graphics.Color(*dimmed)
        
    def brighten_color(self, color_spec, factor=1.5):
        """
        Create a brightened version of a color.
        
        Args:
            color_spec: Color specification (see get_color)
            factor: Brightening factor (1.0 = original, >1.0 = brighter)
            
        Returns:
            graphics.Color object
        """
        if isinstance(color_spec, graphics.Color):
            rgb = (color_spec.red, color_spec.green, color_spec.blue)
        else:
            color = self.get_color(color_spec)
            rgb = (color.red, color.green, color.blue)
            
        brightened = tuple(min(255, int(c * factor)) for c in rgb)
        return graphics.Color(*brightened)
        
    def blend_colors(self, color1_spec, color2_spec, ratio=0.5):
        """
        Blend two colors together.
        
        Args:
            color1_spec: First color specification
            color2_spec: Second color specification  
            ratio: Blend ratio (0.0 = color1, 1.0 = color2, 0.5 = equal mix)
            
        Returns:
            graphics.Color object
        """
        color1 = self.get_color(color1_spec)
        color2 = self.get_color(color2_spec)
        
        r = int(color1.red * (1 - ratio) + color2.red * ratio)
        g = int(color1.green * (1 - ratio) + color2.green * ratio)
        b = int(color1.blue * (1 - ratio) + color2.blue * ratio)
        
        return graphics.Color(r, g, b)
        
    def get_theme_colors(self):
        """
        Get all colors in the current theme.
        
        Returns:
            Dictionary of theme colors as graphics.Color objects
        """
        return {
            'primary': self.get_color(self.primary),
            'secondary': self.get_color(self.secondary),
            'accent': self.get_color(self.accent),
            'background': self.get_color(self.background),
            'success': self.get_color(self.success),
            'warning': self.get_color(self.warning),
            'error': self.get_color(self.error)
        }
        
    def get_available_themes(self):
        """
        Get list of available theme names.
        
        Returns:
            List of theme names
        """
        return list(self.THEMES.keys())