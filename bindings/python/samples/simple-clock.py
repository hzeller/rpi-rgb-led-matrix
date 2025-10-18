#!/usr/bin/env python3
# Simple digital clock for RGB LED matrix
# Displays current time in a clean, centered format

import time
import sys
import os
from datetime import datetime

# Add the parent directory to Python path to import the rgbmatrix module
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics

class SimpleClock:
    def __init__(self):
        # Configuration for the matrix
        options = RGBMatrixOptions()
        options.rows = 32
        options.cols = 64
        options.chain_length = 1
        options.parallel = 1
        options.hardware_mapping = 'regular'
        options.gpio_slowdown = 4
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        
        # Load font
        self.font = graphics.Font()
        # Try multiple font paths
        font_paths = [
            os.path.join(os.path.dirname(__file__), '..', '..', '..', 'fonts', '7x13.bdf'),
            os.path.join(os.path.dirname(__file__), '..', '..', '..', 'fonts', '6x10.bdf'),
            os.path.join(os.path.dirname(__file__), '..', '..', '..', 'fonts', '5x7.bdf'),
            'fonts/7x13.bdf',
            'fonts/6x10.bdf', 
            'fonts/5x7.bdf',
            '../../../fonts/7x13.bdf',
            '../../../fonts/6x10.bdf',
            '../../../fonts/5x7.bdf'
        ]
        
        font_loaded = False
        for font_path in font_paths:
            if self.font.LoadFont(font_path):
                print(f"Loaded font: {font_path}")
                font_loaded = True
                break
            
        if not font_loaded:
            print("Couldn't load any font! Tried paths:")
            for path in font_paths:
                print(f"  {path}")
            sys.exit(1)
        
        # Colors
        self.time_color = graphics.Color(0, 255, 0)      # Green for time
        self.date_color = graphics.Color(255, 255, 0)    # Yellow for date
        self.seconds_color = graphics.Color(255, 100, 0) # Orange for seconds
        
    def run(self):
        print("Starting simple clock. Press CTRL-C to stop.")
        try:
            while True:
                self.canvas.Clear()
                now = datetime.now()
                
                # Format time and date
                time_str = now.strftime("%H:%M")
                date_str = now.strftime("%m/%d")
                seconds_str = now.strftime(":%S")
                
                # Calculate text positions for centering
                time_width = len(time_str) * 7  # Approximate character width
                date_width = len(date_str) * 7
                seconds_width = len(seconds_str) * 7
                
                # Center the time on the display
                time_x = (64 - time_width) // 2
                time_y = 20
                
                # Center the date below the time
                date_x = (64 - date_width) // 2
                date_y = 30
                
                # Position seconds after the time
                seconds_x = time_x + time_width
                seconds_y = time_y
                
                # Draw the time, date, and seconds
                graphics.DrawText(self.canvas, self.font, time_x, time_y, self.time_color, time_str)
                graphics.DrawText(self.canvas, self.font, seconds_x, seconds_y, self.seconds_color, seconds_str)
                graphics.DrawText(self.canvas, self.font, date_x, date_y, self.date_color, date_str)
                
                # Swap buffers
                self.canvas = self.matrix.SwapOnVSync(self.canvas)
                
                # Update every second
                time.sleep(1)
                
        except KeyboardInterrupt:
            print("\nClock stopped.")
        finally:
            self.matrix.Clear()

if __name__ == "__main__":
    clock = SimpleClock()
    clock.run()