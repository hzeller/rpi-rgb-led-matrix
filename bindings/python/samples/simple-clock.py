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
        
        # Load smaller font for date
        self.date_font = graphics.Font()
        self.date_font.LoadFont("../../../fonts/5x7.bdf")  # Smaller font for date
        
        # Colors - classic alarm clock style
        self.time_color = graphics.Color(255, 255, 255)  # Bright white for time
        self.date_color = graphics.Color(200, 200, 200)  # Slightly dimmer white for date
        
    def get_ordinal_suffix(self, day):
        """Get the ordinal suffix for a day (1st, 2nd, 3rd, 4th, etc.)"""
        if 10 <= day % 100 <= 20:
            suffix = 'th'
        else:
            suffix = {1: 'st', 2: 'nd', 3: 'rd'}.get(day % 10, 'th')
        return suffix

    def run(self):
        print("Starting classic alarm clock. Press CTRL-C to stop.")
        print(f"Matrix size: {self.matrix.width}x{self.matrix.height}")
        print(f"Font height: {self.font.height}")
        print(f"Date font height: {self.date_font.height}")
        
        try:
            while True:
                self.canvas.Clear()
                now = datetime.now()
                
                # Format date in the requested format: "Saturday, October 18th"
                day_name = now.strftime("%A")
                month_name = now.strftime("%B")
                day_num = now.day
                ordinal_suffix = self.get_ordinal_suffix(day_num)
                date_str = f"{day_name}, {month_name} {day_num}{ordinal_suffix}"
                
                # Format time in classic 12-hour format with AM/PM on same line
                time_str = now.strftime("%I:%M")
                if time_str.startswith("0"):
                    time_str = time_str[1:]  # Remove leading zero (e.g., "01:30" -> "1:30")
                ampm_str = now.strftime("%p")
                full_time_str = f"{time_str} {ampm_str}"
                
                print(f"Date: {date_str}")
                print(f"Time: {full_time_str}")
                
                # Calculate text positions for perfect centering
                # Use tighter character spacing - actual font widths are smaller
                date_width = len(date_str) * 4  # 5x7 font is about 4px wide per char (tighter)
                date_x = (64 - date_width) // 2
                date_y = 8  # Top area for date
                
                # Time font spacing - much tighter
                full_time_width = len(full_time_str) * 6  # 9x18B font is about 6px wide per char (much tighter)
                time_x = (64 - full_time_width) // 2
                time_y = 26  # Below the date
                
                print(f"Positions - Date: ({date_x},{date_y}), Time: ({time_x},{time_y})")
                
                # Draw the date and time
                graphics.DrawText(self.canvas, self.date_font, date_x, date_y, self.date_color, date_str)
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