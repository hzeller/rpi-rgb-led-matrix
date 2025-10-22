#!/usr/bin/env python
"""
Retro Flip Clock - Classic 1970s Style LED Matrix Display

A minimalist flip clock display inspired by vintage Twemco and similar designs:
- Clean, blocky digit display mimicking flip cards
- Simple black background with white digits
- Hour:minute format in large, readable font
- Classic proportions and spacing
- Optional AM/PM indicator

Usage:
    sudo python retro-clock.py
"""

import time
import sys
import os
from datetime import datetime

# Add shared components to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette


class RetroFlipClock(MatrixBase):
    """Classic flip clock display with vintage 1970s aesthetic."""
    
    def __init__(self):
        # Initialize matrix with standard configuration
        super().__init__(
            rows=32,
            cols=64,
            hardware_mapping='adafruit-hat-pwm'
        )
        
        # Initialize shared components with clean default theme
        self.font_manager = FontManager()
        self.color_palette = ColorPalette('default')  # Clean white on black
        
        # Load fonts - use the largest available for authentic flip clock look
        self.digit_font = self.font_manager.get_font('xxlarge')  # 9x18B for main digits
        self.ampm_font = self.font_manager.get_font('tiny')      # 4x6 for AM/PM
        
        # Authentic Twemco flip clock colors
        self.background_color = self.color_palette.get_color((200, 80, 0))    # Darker orange background
        self.frame_color = self.color_palette.get_color('white')               # White trim/frame
        self.window_color = self.color_palette.get_color('black')              # Black digit windows
        self.digit_color = self.color_palette.get_color('white')               # White digits
        self.ampm_color = self.color_palette.get_color('white')                # White AM indicator
        
        # Store previous time for flip detection
        self.previous_time = ""
        self.show_ampm = True  # Option to show AM/PM
        
        print("�️  Retro Flip Clock initialized - Classic 1970s style")
        print(f"Matrix size: {self.width}x{self.height}")
        

    
    def draw_flip_time(self):
        """Draw time in authentic Twemco flip clock style with separate windows."""
        now = datetime.now()
        
        # Format time components separately
        hour = now.strftime("%I")
        if hour.startswith("0"):
            hour = " " + hour[1:]  # Replace leading zero with space for authenticity
        
        minute = now.strftime("%M")  # Always two digits for minutes
        
        # Draw the digit windows and digits
        self.draw_digit_windows(hour, minute)
        
        # Draw AM indicator (like the original - small text on orange background)
        if self.show_ampm:
            ampm = now.strftime("%p").lower()  # Use lowercase "am" like the original
            
            # Position AM/PM in upper left area on orange background
            ampm_x = 4
            ampm_y = 7
            
            current_x = ampm_x
            for char in ampm:
                char_width = self.draw_text(self.ampm_font, current_x, ampm_y,
                                          self.ampm_color, char)
                current_x += char_width
    
    def draw_background_and_frame(self):
        """Draw the orange background and dual frame like a real Twemco clock."""
        # Fill entire background with orange
        for x in range(64):
            for y in range(32):
                self.set_pixel(x, y, self.background_color)
        
        # Draw orange outer frame border (1-pixel thick)
        # Top and bottom borders
        for x in range(64):
            self.set_pixel(x, 0, self.background_color)
            self.set_pixel(x, 31, self.background_color)
        
        # Left and right borders  
        for y in range(32):
            self.set_pixel(0, y, self.background_color)
            self.set_pixel(63, y, self.background_color)
        
        # Draw white inner frame border (1-pixel thick)
        # Top and bottom borders
        for x in range(1, 63):
            self.set_pixel(x, 1, self.frame_color)
            self.set_pixel(x, 30, self.frame_color)
        
        # Left and right borders  
        for y in range(1, 31):
            self.set_pixel(1, y, self.frame_color)
            self.set_pixel(62, y, self.frame_color)
    
    def draw_digit_windows(self, hour_str, minute_str):
        """Draw black rectangular windows for the digits like flip cards."""
        # Define window positions (two separate rectangles for hour and minute)
        # Hour window (left side)
        hour_window = {
            'x': 6, 'y': 8, 'width': 22, 'height': 16
        }
        
        # Minute window (right side)  
        minute_window = {
            'x': 36, 'y': 8, 'width': 22, 'height': 16
        }
        
        # Draw hour window (black rectangle)
        for x in range(hour_window['x'], hour_window['x'] + hour_window['width']):
            for y in range(hour_window['y'], hour_window['y'] + hour_window['height']):
                if 0 <= x < 64 and 0 <= y < 32:
                    self.set_pixel(x, y, self.window_color)
        
        # Draw minute window (black rectangle)
        for x in range(minute_window['x'], minute_window['x'] + minute_window['width']):
            for y in range(minute_window['y'], minute_window['y'] + minute_window['height']):
                if 0 <= x < 64 and 0 <= y < 32:
                    self.set_pixel(x, y, self.window_color)
        
        # Draw white digits centered in their respective windows
        # Hour digits
        hour_width = 0
        for char in hour_str:
            if char != ' ':
                hour_width += self.digit_font.CharacterWidth(ord(char))
        hour_x = hour_window['x'] + (hour_window['width'] - hour_width) // 2
        hour_y = hour_window['y'] + 14  # Adjust for font baseline
        
        current_x = hour_x
        for char in hour_str:
            if char != ' ':
                char_width = self.draw_text(self.digit_font, current_x, hour_y, 
                                          self.digit_color, char)
                current_x += char_width
        
        # Minute digits
        minute_width = 0
        for char in minute_str:
            minute_width += self.digit_font.CharacterWidth(ord(char))
        minute_x = minute_window['x'] + (minute_window['width'] - minute_width) // 2  
        minute_y = minute_window['y'] + 14  # Adjust for font baseline
        
        current_x = minute_x
        for char in minute_str:
            char_width = self.draw_text(self.digit_font, current_x, minute_y,
                                      self.digit_color, char)
            current_x += char_width
    

    
    def run(self):
        """Main display loop - simple and clean like a real flip clock."""
        print("🕰️  Starting Authentic Twemco-Style Flip Clock - Press CTRL-C to stop")
        print("🧡 Orange background with white frame and black digit windows")
        
        try:
            while True:
                # Start with clean slate
                self.clear()
                
                # Draw background, frame, and time windows
                self.draw_background_and_frame()
                self.draw_flip_time()
                
                # Update display
                self.swap()
                
                # Update every second (like a real flip clock)
                time.sleep(1.0)
                
        except KeyboardInterrupt:
            print("\n�️  Flip clock stopped - Time stands still!")
        finally:
            self.clear()


if __name__ == "__main__":
    flip_clock = RetroFlipClock()
    flip_clock.run()