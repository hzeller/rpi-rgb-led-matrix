#!/usr/bin/env python3
# Simple digital clock for RGB LED matrix
# Displays current time in a clean, centered format

import time
import sys
import os
from datetime import datetime, timezone, timedelta

# Add shared components to path
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
# Import shared components
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette
from config_manager import ConfigManager

class SimpleClock(MatrixBase):
    def __init__(self):
        # Initialize matrix base with default configuration
        super().__init__(hardware_mapping='adafruit-hat-pwm')
        
        # Initialize managers
        self.font_manager = FontManager()
        self.colors = ColorPalette('default')
        self.config = ConfigManager()
        
        # Set up Mountain Time timezone (UTC-6 for MDT, UTC-7 for MST)
        # Currently in Daylight Saving Time (March-November)
        self.mountain_tz = timezone(timedelta(hours=-6))  # Mountain Daylight Time
        
        # Load fonts using font manager
        self.time_font = self.font_manager.get_font('xxlarge')  # 9x18B.bdf
        self.date_font = self.font_manager.get_font('medium_bold')  # 6x13B.bdf
        
        # Colors using color palette
        self.time_color = self.colors.get_color('WHITE')
        self.date_color = self.colors.get_color('GRAY_LIGHT')
        
        # Auto-dimming based on time
        self.auto_dim_enabled = self.config.get('AUTO_DIM_ENABLED', False, bool)
        self.dim_start_time = self.config.get('AUTO_DIM_TIME', '22:00')
        self.dim_brightness = self.config.get('AUTO_DIM_LEVEL', 20, int)
        self.normal_brightness = self.config.get('BRIGHTNESS_DEFAULT', 60, int)
        
    def get_ordinal_suffix(self, day):
        """Get the ordinal suffix for a day (1st, 2nd, 3rd, 4th, etc.)"""
        if 10 <= day % 100 <= 20:
            suffix = 'th'
        else:
            suffix = {1: 'st', 2: 'nd', 3: 'rd'}.get(day % 10, 'th')
        return suffix
        
    def check_auto_dim(self, current_time):
        """
        Check if auto-dimming should be applied based on current time.
        
        Args:
            current_time: Current datetime object
        """
        if not self.auto_dim_enabled:
            return
            
        current_hour_min = current_time.strftime("%H:%M")
        
        # Parse dim start time
        try:
            dim_hour, dim_min = map(int, self.dim_start_time.split(':'))
            dim_time_minutes = dim_hour * 60 + dim_min
            current_time_minutes = current_time.hour * 60 + current_time.minute
            
            # Check if it's dimming time (after dim start until 6 AM)
            should_be_dimmed = (current_time_minutes >= dim_time_minutes or 
                              current_time_minutes < 6 * 60)  # Before 6 AM
                              
            current_brightness = self.get_brightness()
            
            if should_be_dimmed and current_brightness > self.dim_brightness:
                self.set_brightness(self.dim_brightness)
                print(f"🌙 Auto-dimmed to {self.dim_brightness}% at {current_hour_min}")
            elif not should_be_dimmed and current_brightness < self.normal_brightness:
                self.set_brightness(self.normal_brightness) 
                print(f"☀️ Auto-brightened to {self.normal_brightness}% at {current_hour_min}")
                
        except ValueError:
            print(f"⚠️ Invalid dim start time format: {self.dim_start_time}")
            self.auto_dim_enabled = False

    def run(self):
        print("Starting classic alarm clock. Press CTRL-C to stop.")
        print(f"Matrix size: {self.width}x{self.height}")
        print(f"Font height: {self.time_font.height}")
        print(f"Date font height: {self.date_font.height}")
        
        try:
            while True:
                self.clear()
                # Get current time in Mountain Time
                now = datetime.now(self.mountain_tz)
                
                # Format date in a shorter format that will fit: "Sat Oct 18"
                day_name = now.strftime("%a")  # Short day name (Sat)
                month_name = now.strftime("%b")  # Short month name (Oct)
                day_num = now.day
                # Use smaller spaces between words for tighter packing
                date_str = f"{day_name} {month_name} {day_num}"
                
                # Format time in classic 12-hour format with AM/PM on same line
                time_str = now.strftime("%I:%M")
                if time_str.startswith("0"):
                    time_str = time_str[1:]  # Remove leading zero (e.g., "01:30" -> "1:30")
                ampm_str = now.strftime("%p")
                full_time_str = f"{time_str}{ampm_str}"
                
                print(f"Date: {date_str}")
                print(f"Time: {full_time_str} (Mountain Time)")
                
                # Calculate text positions with proper dynamic centering
                # Use actual text width measurements for perfect centering
                
                # Measure actual date text width
                date_text_width = 0
                for char in date_str:
                    date_text_width += self.date_font.CharacterWidth(ord(char))
                date_x = (64 - date_text_width) // 2 + 7  # Perfect horizontal center + 7 pixels right (5 + 2)
                
                # Vertical centering: keep them close together
                date_y = 13  # Close to original position + 2 pixels down
                
                # Measure actual time text width
                time_text_width = 0
                for char in full_time_str:
                    time_text_width += self.time_font.CharacterWidth(ord(char))
                time_x = (64 - time_text_width) // 2 + 3  # Perfect horizontal center + 3 pixels right
                time_y = 27  # Keep close to date + 2 pixels down
                
                print(f"Positions - Date: ({date_x},{date_y}), Time: ({time_x},{time_y})")
                
                # Draw the date and time with custom tight spacing
                # Draw date with tighter character spacing
                current_x = date_x
                for char in date_str:
                    if char == ' ':
                        current_x += 2  # Reduced space width
                    else:
                        char_width = self.draw_text(self.date_font, current_x, date_y, self.date_color, char)
                        current_x += char_width - 1  # Reduce spacing between characters
                
                # Draw time with tighter character spacing
                current_x = time_x
                for char in full_time_str:
                    if char == ' ':
                        current_x += 2  # Reduced space width
                    else:
                        char_width = self.draw_text(self.time_font, current_x, time_y, self.time_color, char)
                        current_x += char_width - 1  # Reduce spacing between characters
                
                # Swap buffers
                self.swap()
                
                # Check for auto-dimming
                self.check_auto_dim(now)
                
                # Wait until the next second boundary for accurate timing
                current_time = time.time()
                next_second = int(current_time) + 1
                sleep_time = next_second - current_time
                time.sleep(sleep_time)
                
        except KeyboardInterrupt:
            print("\nClock stopped.")
        finally:
            self.clear()

if __name__ == "__main__":
    clock = SimpleClock()
    clock.run()