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
                
                # Format time in classic 12-hour format with AM/PM on same line
                time_str = now.strftime("%I:%M")
                if time_str.startswith("0"):
                    time_str = time_str[1:]  # Remove leading zero (e.g., "01:30" -> "1:30")
                ampm_str = now.strftime("%p")
                full_time_str = f"{time_str} {ampm_str}"
                
                print(f"Time: {full_time_str}")
                
                # Calculate text positions for perfect centering on one line
                # 9x18B font is about 9px wide per character
                full_time_width = len(full_time_str) * 9
                
                # Center the entire time string perfectly in the display
                time_x = (64 - full_time_width) // 2
                time_y = 16  # Vertically centered in 32px height (font baseline)
                
                print(f"Position - Full time: ({time_x},{time_y})")
                
                # Draw the complete time string in classic white
                graphics.DrawText(self.canvas, self.font, time_x, time_y, self.time_color, full_time_str)
                
                # Swap buffers
                self.canvas = self.matrix.SwapOnVSync(self.canvas)
                
                # Wait until the next second boundary for accurate timing
                current_time = time.time()
                next_second = int(current_time) + 1
                sleep_time = next_second - current_time
                time.sleep(sleep_time)
                
        except KeyboardInterrupt:
            print("\nClock stopped.")
        finally:
            self.matrix.Clear()

if __name__ == "__main__":
    clock = SimpleClock()
    clock.run()