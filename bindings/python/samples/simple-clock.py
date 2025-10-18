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
        # Configuration for the matrix - use same settings as image-viewer.py
        options = RGBMatrixOptions()
        options.rows = 32
        options.cols = 64
        options.chain_length = 1
        options.parallel = 1
        options.hardware_mapping = 'adafruit-hat-pwm'  # Same as image-viewer
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        
        # Load font - use a bigger, bolder font for classic alarm clock look
        self.font = graphics.Font()
        self.font.LoadFont("../../../fonts/9x18B.bdf")  # Bold 9x18 font
        
        # Colors - classic alarm clock style
        self.time_color = graphics.Color(255, 255, 255)  # Bright white for time
        self.colon_color = graphics.Color(255, 255, 255)  # White for colons too
        
    def run(self):
        print("Starting classic alarm clock. Press CTRL-C to stop.")
        print(f"Matrix size: {self.matrix.width}x{self.matrix.height}")
        print(f"Font height: {self.font.height}")
        
        try:
            while True:
                self.canvas.Clear()
                now = datetime.now()
                
                # Format time in classic 12-hour format with AM/PM
                time_str = now.strftime("%I:%M")
                if time_str.startswith("0"):
                    time_str = time_str[1:]  # Remove leading zero (e.g., "01:30" -> "1:30")
                ampm_str = now.strftime("%p")
                
                print(f"Time: {time_str} {ampm_str}")
                
                # Calculate text positions for perfect centering
                # 9x18B font is about 9px wide per character
                time_width = len(time_str) * 9
                ampm_width = len(ampm_str) * 9
                
                # Center the time perfectly in the display
                time_x = (64 - time_width) // 2
                time_y = 20  # Vertically centered in 32px height
                
                # Position AM/PM below and centered
                ampm_x = (64 - ampm_width) // 2
                ampm_y = time_y + 10  # Below the time
                
                print(f"Positions - Time: ({time_x},{time_y}), AM/PM: ({ampm_x},{ampm_y})")
                
                # Draw the time and AM/PM in classic white
                graphics.DrawText(self.canvas, self.font, time_x, time_y, self.time_color, time_str)
                graphics.DrawText(self.canvas, self.font, ampm_x, ampm_y, self.time_color, ampm_str)
                
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