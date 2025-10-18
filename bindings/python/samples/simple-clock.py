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
        
        # Load font - use same approach as image-viewer.py
        self.font = graphics.Font()
        self.font.LoadFont("../../../fonts/7x13.bdf")
        
        # Colors
        self.time_color = graphics.Color(0, 255, 0)      # Green for time
        self.date_color = graphics.Color(255, 255, 0)    # Yellow for date
        self.seconds_color = graphics.Color(255, 100, 0) # Orange for seconds
        
    def run(self):
        print("Starting simple clock. Press CTRL-C to stop.")
        print(f"Matrix size: {self.matrix.width}x{self.matrix.height}")
        print(f"Font height: {self.font.height}")
        
        try:
            while True:
                self.canvas.Clear()
                now = datetime.now()
                
                # Format time and date
                time_str = now.strftime("%H:%M")
                date_str = now.strftime("%m/%d")
                seconds_str = now.strftime(":%S")
                
                print(f"Time: {time_str}{seconds_str}, Date: {date_str}")
                
                # Calculate text positions for centering
                # Use actual font metrics instead of approximation
                time_width = len(time_str) * 6  # 7x13 font is about 6px wide per char
                date_width = len(date_str) * 6
                seconds_width = len(seconds_str) * 6
                
                # Center the time on the display
                time_x = max(2, (64 - time_width) // 2)
                time_y = self.font.height + 2  # Use font height + margin
                
                # Center the date below the time
                date_x = max(2, (64 - date_width) // 2)
                date_y = time_y + self.font.height + 2
                
                # Position seconds after the time
                seconds_x = time_x + time_width
                seconds_y = time_y
                
                print(f"Positions - Time: ({time_x},{time_y}), Seconds: ({seconds_x},{seconds_y}), Date: ({date_x},{date_y})")
                
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