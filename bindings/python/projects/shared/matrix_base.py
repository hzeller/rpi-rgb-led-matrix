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

sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/../../samples'))
from samplebase import SampleBase


class MatrixConfig:
    """Default matrix configuration constants."""
    ROWS = 32
    COLS = 64
    CHAIN_LENGTH = 1
    PARALLEL = 1
    HARDWARE_MAPPING = 'adafruit-hat-pwm'
    BRIGHTNESS = 100


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
        # Set up matrix options with defaults or provided values
        self.options = RGBMatrixOptions()
        self.options.rows = rows or MatrixConfig.ROWS
        self.options.cols = cols or MatrixConfig.COLS
        self.options.chain_length = chain_length or MatrixConfig.CHAIN_LENGTH
        self.options.parallel = parallel or MatrixConfig.PARALLEL
        self.options.hardware_mapping = hardware_mapping or MatrixConfig.HARDWARE_MAPPING
        self.options.brightness = brightness or MatrixConfig.BRIGHTNESS
        
        # Initialize matrix and canvas
        self.matrix = RGBMatrix(options=self.options)
        self.canvas = self.matrix.CreateFrameCanvas()
        
        # Store dimensions for convenience
        self.width = self.matrix.width
        self.height = self.matrix.height
        
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