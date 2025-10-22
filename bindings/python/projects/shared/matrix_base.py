#!/usr/bin/env python
"""
MatrixBase - Base class for RGB LED Matrix applications.

Provides common functionality for matrix setup, canvas management,
and basic display operations that all projects can inherit from.
"""

import sys
import os
from abc import ABC, abstractmethod

# Add paths for rgbmatrix and samplebase
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/../..'))
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics

# Import shared components
from config_manager import ConfigManager

sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/../../samples'))
from samplebase import SampleBase


class MatrixConfig:
    """Default matrix configuration constants."""
    ROWS = 32
    COLS = 64
    CHAIN_LENGTH = 1
    PARALLEL = 1
    HARDWARE_MAPPING = 'adafruit-hat-pwm'
    BRIGHTNESS = 60  # More reasonable default (60% instead of 100%)


class MatrixBase(ABC):
    """
    Base class for RGB LED Matrix applications.
    
    Provides common functionality for matrix initialization, canvas management,
    and basic display operations. Applications should inherit from this class
    and implement the run() method.
    """
    
    def __init__(self, rows=None, cols=None, chain_length=None, parallel=None, 
                 hardware_mapping=None, brightness=None):
        """
        Initialize the matrix with optional configuration overrides.
        
        Args:
            rows: Number of LED rows (default: MatrixConfig.ROWS)
            cols: Number of LED columns (default: MatrixConfig.COLS)
            chain_length: Number of chained panels (default: MatrixConfig.CHAIN_LENGTH)
            parallel: Number of parallel chains (default: MatrixConfig.PARALLEL)
            hardware_mapping: Hardware mapping type (default: MatrixConfig.HARDWARE_MAPPING)
            brightness: Display brightness 1-100 (default: MatrixConfig.BRIGHTNESS)
        """
        # Initialize config manager for settings
        config_manager = ConfigManager()
        matrix_config = config_manager.get_matrix_config()
        brightness_config = config_manager.get_brightness_config()
        
        # Set up matrix options with defaults or provided values
        self.options = RGBMatrixOptions()
        self.options.rows = rows or matrix_config['rows']
        self.options.cols = cols or matrix_config['cols']
        self.options.chain_length = chain_length or matrix_config['chain_length']
        self.options.parallel = parallel or matrix_config['parallel']
        self.options.hardware_mapping = hardware_mapping or matrix_config['hardware_mapping']
        self.options.brightness = brightness or matrix_config['brightness']
        
        # Initialize matrix and canvas
        self.matrix = RGBMatrix(options=self.options)
        self.canvas = self.matrix.CreateFrameCanvas()
        
        # Store dimensions and brightness config for convenience
        self.width = self.matrix.width
        self.height = self.matrix.height
        self.brightness_config = brightness_config
        
    def clear(self):
        """Clear the display canvas."""
        self.canvas.Clear()
        
    def swap(self):
        """Swap the canvas to display the updated content."""
        self.canvas = self.matrix.SwapOnVSync(self.canvas)
        
    def set_pixel(self, x, y, color):
        """
        Set a single pixel to the specified color.
        
        Args:
            x: X coordinate
            y: Y coordinate
            color: graphics.Color object
        """
        self.canvas.SetPixel(x, y, color.red, color.green, color.blue)
        
    def set_brightness(self, brightness):
        """
        Set the display brightness dynamically.
        
        Args:
            brightness: Brightness level from 1-100
        """
        min_bright = self.brightness_config['min_brightness']
        max_bright = self.brightness_config['max_brightness']
        
        if min_bright <= brightness <= max_bright:
            self.matrix.brightness = brightness
            print(f"🔆 Brightness set to {brightness}%")
        else:
            print(f"⚠️ Brightness must be between {min_bright}-{max_bright}, got {brightness}")
            
    def get_brightness(self):
        """
        Get the current display brightness.
        
        Returns:
            Current brightness level (1-100)
        """
        return self.matrix.brightness
        
    def dim_display(self, factor=0.5):
        """
        Dim the display by a factor of current brightness.
        
        Args:
            factor: Dimming factor (0.1 = 10% of current, 0.5 = 50% of current)
        """
        current = self.get_brightness()
        new_brightness = max(1, int(current * factor))
        self.set_brightness(new_brightness)
        
    def save_brightness_preference(self, brightness):
        """
        Save brightness as user preference for future sessions.
        
        Args:
            brightness: Brightness level to save as default
        """
        env_file = os.path.join(os.path.dirname(__file__), '..', '.env')
        
        # Read existing .env content
        env_lines = []
        if os.path.exists(env_file):
            with open(env_file, 'r') as f:
                env_lines = f.readlines()
        
        # Update or add brightness setting
        brightness_line = f"BRIGHTNESS_DEFAULT={brightness}\n"
        updated = False
        
        for i, line in enumerate(env_lines):
            if line.startswith('BRIGHTNESS_DEFAULT='):
                env_lines[i] = brightness_line
                updated = True
                break
                
        if not updated:
            env_lines.append(brightness_line)
            
        # Write back to .env file
        with open(env_file, 'w') as f:
            f.writelines(env_lines)
            
        print(f"💾 Saved brightness preference: {brightness}%")
        
    def draw_text(self, font, x, y, color, text, kerning=0):
        """
        Draw text on the canvas.
        
        Args:
            font: graphics.Font object
            x: X position
            y: Y position (baseline)
            color: graphics.Color object
            text: Text to draw
            kerning: Additional character spacing (handled manually if needed)
            
        Returns:
            Width of the drawn text
        """
        return graphics.DrawText(self.canvas, font, x, y, color, text)
        
    def draw_text_centered(self, font, y, color, text, kerning=0):
        """
        Draw text centered horizontally on the canvas.
        
        Args:
            font: graphics.Font object
            y: Y position (baseline)
            color: graphics.Color object
            text: Text to draw
            kerning: Additional character spacing
            
        Returns:
            X position where text was drawn
        """
        text_width = sum(font.CharacterWidth(ord(c)) + kerning for c in text)
        x = (self.width - text_width) // 2
        graphics.DrawText(self.canvas, font, x, y, color, text)
        return x
        
    def draw_text_right_aligned(self, font, x_right, y, color, text, kerning=0):
        """
        Draw text right-aligned at the specified position.
        
        Args:
            font: graphics.Font object
            x_right: Right edge X position
            y: Y position (baseline)
            color: graphics.Color object
            text: Text to draw
            kerning: Additional character spacing
            
        Returns:
            X position where text was drawn
        """
        text_width = sum(font.CharacterWidth(ord(c)) + kerning for c in text)
        x = x_right - text_width
        graphics.DrawText(self.canvas, font, x, y, color, text)
        return x
        
    def draw_line(self, x1, y1, x2, y2, color):
        """
        Draw a line between two points.
        
        Args:
            x1, y1: Start coordinates
            x2, y2: End coordinates
            color: graphics.Color object
        """
        graphics.DrawLine(self.canvas, x1, y1, x2, y2, color)
        
    def draw_circle(self, x, y, radius, color):
        """
        Draw a circle outline.
        
        Args:
            x, y: Center coordinates
            radius: Circle radius
            color: graphics.Color object
        """
        graphics.DrawCircle(self.canvas, x, y, radius, color)
        
    def get_text_width(self, font, text, kerning=0):
        """
        Calculate the width of text when rendered with the given font.
        
        Args:
            font: graphics.Font object
            text: Text to measure
            kerning: Additional character spacing
            
        Returns:
            Width in pixels
        """
        return sum(font.CharacterWidth(ord(c)) + kerning for c in text)
        
    @abstractmethod
    def run(self):
        """
        Main application logic. Must be implemented by subclasses.
        """
        pass


class SampleMatrixBase(MatrixBase, SampleBase):
    """
    Matrix base class that also inherits from SampleBase for command-line argument parsing.
    
    This class combines MatrixBase functionality with SampleBase argument parsing,
    useful for applications that need command-line configuration.
    """
    
    def __init__(self):
        """Initialize both MatrixBase and SampleBase."""
        SampleBase.__init__(self)
        
    def process(self):
        """Override SampleBase.process() to use our matrix configuration."""
        # Parse arguments first
        self.args = self.parser.parse_args()
        
        # Initialize matrix with parsed arguments
        MatrixBase.__init__(
            self,
            rows=self.args.led_rows,
            cols=self.args.led_cols,
            chain_length=self.args.led_chain,
            parallel=self.args.led_parallel,
            hardware_mapping=self.args.led_gpio_mapping,
            brightness=self.args.led_brightness
        )
        
        # Call the main run method
        try:
            self.run()
            return True
        except KeyboardInterrupt:
            print("\nApplication interrupted by user")
            return True
        except Exception as e:
            print(f"Error: {e}")
            return False