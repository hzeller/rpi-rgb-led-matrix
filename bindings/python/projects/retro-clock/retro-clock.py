#!/usr/bin/env python
"""
Retro Clock - Classic Style LED Matrix Display

A minimalist clock display inspired by vintage Twemco and similar designs:
- Clean, blocky digit display
- Orange background with white frame and black digit windows
- Hour:minute format in large, readable font
- Classic proportions and spacing
- Optional AM/PM indicator
- Simple number changes with no complex animations

Usage:
    sudo python retro-clock.py
"""

import time
import sys
import os
import threading
from datetime import datetime
try:
    from zoneinfo import ZoneInfo  # Python 3.9+
    TIMEZONE_MODULE = 'zoneinfo'
except ImportError:
    try:
        import pytz  # Fallback for older Python versions
        TIMEZONE_MODULE = 'pytz'
    except ImportError:
        TIMEZONE_MODULE = None
        print("Warning: Neither zoneinfo nor pytz available. Timezone functionality disabled.")

try:
    import msvcrt  # Windows
    WINDOWS = True
except ImportError:
    import select  # Unix/Linux
    WINDOWS = False

# Add shared components to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette
from config_manager import ConfigManager


class RetroClock(MatrixBase):
    """Classic clock display with vintage 1970s aesthetic."""
    
    def __init__(self):
        # Initialize configuration manager
        self.config = ConfigManager()
        
        # Initialize matrix with configuration from ConfigManager (handled by MatrixBase)
        super().__init__()
        
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
        self.previous_hour = ""
        self.previous_minute = ""
        self.show_ampm = True  # Option to show AM/PM
        
        # Flip animation state
        self.manual_flip_triggered = False
        self.flip_animation_frames = 8  # Number of frames in flip animation
        self.flip_duration = 0.4  # Total duration in seconds
        
        # Timezone configuration
        self.setup_timezones()
        
        print("🕰️  Retro Flip Clock initialized - Classic 1970s style")
        print(f"Matrix size: {self.width}x{self.height}")
        print(f"🌍 Current timezone: {self.get_current_timezone_name()}")
    
    def setup_timezones(self):
        """Initialize timezone configuration with common timezones."""
        self.timezone_list = [
            'UTC',
            'US/Eastern',
            'US/Central', 
            'US/Mountain',
            'US/Pacific',
            'Europe/London',
            'Europe/Paris',
            'Europe/Berlin',
            'Asia/Tokyo',
            'Asia/Shanghai',
            'Asia/Kolkata',
            'Australia/Sydney',
            'America/New_York',
            'America/Chicago',
            'America/Denver',
            'America/Los_Angeles'
        ]
        
        self.current_timezone_index = 0  # Default to UTC
        self.current_timezone = None
        self.show_timezone = True  # Show timezone abbreviation
        
        if TIMEZONE_MODULE:
            self.set_timezone_by_index(0)  # Set to UTC
        else:
            print("⚠️  Timezone functionality not available - using system local time")
    
    def set_timezone_by_index(self, index):
        """Set timezone by index in the timezone list."""
        if not TIMEZONE_MODULE or index >= len(self.timezone_list):
            return False
            
        timezone_name = self.timezone_list[index]
        return self.set_timezone(timezone_name)
    
    def set_timezone(self, timezone_name):
        """Set the current timezone."""
        if not TIMEZONE_MODULE:
            return False
            
        try:
            if TIMEZONE_MODULE == 'zoneinfo':
                self.current_timezone = ZoneInfo(timezone_name)
            else:  # pytz
                self.current_timezone = pytz.timezone(timezone_name)
            
            # Update index if this timezone is in our list
            if timezone_name in self.timezone_list:
                self.current_timezone_index = self.timezone_list.index(timezone_name)
            
            print(f"🌍 Timezone changed to: {timezone_name}")
            return True
        except Exception as e:
            print(f"❌ Error setting timezone '{timezone_name}': {e}")
            return False
    
    def get_current_timezone_name(self):
        """Get the current timezone name."""
        if not self.current_timezone:
            return "Local System Time"
        
        if TIMEZONE_MODULE == 'zoneinfo':
            return str(self.current_timezone)
        else:  # pytz
            return self.current_timezone.zone
    
    def get_current_time(self):
        """Get current time in the selected timezone."""
        if not TIMEZONE_MODULE or not self.current_timezone:
            return datetime.now()
        
        if TIMEZONE_MODULE == 'zoneinfo':
            return datetime.now(self.current_timezone)
        else:  # pytz
            utc_now = datetime.utcnow()
            return self.current_timezone.localize(utc_now) if utc_now.tzinfo is None else utc_now.astimezone(self.current_timezone)
    
    def cycle_timezone_forward(self):
        """Cycle to the next timezone in the list."""
        if not TIMEZONE_MODULE:
            return
            
        self.current_timezone_index = (self.current_timezone_index + 1) % len(self.timezone_list)
        self.set_timezone_by_index(self.current_timezone_index)
    
    def cycle_timezone_backward(self):
        """Cycle to the previous timezone in the list."""
        if not TIMEZONE_MODULE:
            return
            
        self.current_timezone_index = (self.current_timezone_index - 1) % len(self.timezone_list)
        self.set_timezone_by_index(self.current_timezone_index)
    
    def get_timezone_abbreviation(self):
        """Get a short abbreviation for the current timezone."""
        if not TIMEZONE_MODULE or not self.current_timezone:
            return "LOC"  # Local time
        
        try:
            # Get current time to determine if we're in DST
            current_time = self.get_current_time()
            
            if TIMEZONE_MODULE == 'zoneinfo':
                # Use the timezone's tzname method if available
                abbrev = current_time.strftime('%Z')
                if abbrev:
                    return abbrev[:3]  # Limit to 3 characters
            else:  # pytz
                abbrev = current_time.strftime('%Z')
                if abbrev:
                    return abbrev[:3]  # Limit to 3 characters
            
            # Fallback to creating abbreviation from timezone name
            tz_name = self.get_current_timezone_name()
            if '/' in tz_name:
                return tz_name.split('/')[-1][:3].upper()
            else:
                return tz_name[:3].upper()
                
        except Exception:
            return "UTC"

    def draw_flip_time(self):
        """Draw time in authentic Twemco flip clock style with separate windows."""
        now = self.get_current_time()
        
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
        
        # Draw timezone abbreviation in lower right corner if enabled
        if self.show_timezone and TIMEZONE_MODULE:
            tz_abbrev = self.get_timezone_abbreviation()
            
            # Position timezone in lower right corner
            tz_x = 64 - len(tz_abbrev) * 4 - 2  # Approximate positioning
            tz_y = 28
            
            current_x = tz_x
            for char in tz_abbrev:
                char_width = self.draw_text(self.ampm_font, current_x, tz_y,
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
    
    def simple_change(self, old_text, new_text, is_hour=True):
        """Simply change from old number to new number - no animation."""
        print(f"🔄 Changing {'hour' if is_hour else 'minute'}: {old_text} → {new_text}")
        # The display will be updated in the main loop, so we don't need to do anything here
        pass
    
    def check_for_input(self):
        """Check for keyboard input in a non-blocking way (cross-platform)."""
        try:
            if WINDOWS:
                # Windows implementation
                if msvcrt.kbhit():
                    key = msvcrt.getch()
                    if key == b' ' or key == b'\r':  # Space or Enter key
                        self.manual_flip_triggered = True
                        return True
                    elif key == b'+' or key == b'=':  # Plus key to increase brightness
                        self.increase_brightness()
                        return True
                    elif key == b'-' or key == b'_':  # Minus key to decrease brightness
                        self.decrease_brightness()
                        return True
                    elif key == b'z' or key == b'Z':  # Z key to cycle timezone forward
                        self.cycle_timezone_forward()
                        return True
                    elif key == b'x' or key == b'X':  # X key to cycle timezone backward
                        self.cycle_timezone_backward()
                        return True
                    elif key == b't' or key == b'T':  # T key to toggle timezone display
                        self.show_timezone = not self.show_timezone
                        print(f"🌍 Timezone display: {'ON' if self.show_timezone else 'OFF'}")
                        return True
            else:
                # Unix/Linux implementation
                if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
                    line = sys.stdin.readline().strip()
                    if line == ' ' or line == '':  # Space or Enter key
                        self.manual_flip_triggered = True
                        return True
                    elif line == '+' or line == '=':  # Plus key to increase brightness
                        self.increase_brightness()
                        return True
                    elif line == '-' or line == '_':  # Minus key to decrease brightness
                        self.decrease_brightness()
                        return True
                    elif line.lower() == 'z':  # Z key to cycle timezone forward
                        self.cycle_timezone_forward()
                        return True
                    elif line.lower() == 'x':  # X key to cycle timezone backward
                        self.cycle_timezone_backward()
                        return True
                    elif line.lower() == 't':  # T key to toggle timezone display
                        self.show_timezone = not self.show_timezone
                        print(f"🌍 Timezone display: {'ON' if self.show_timezone else 'OFF'}")
                        return True
        except:
            # Fallback - no input detection
            pass
        return False
    
    def increase_brightness(self):
        """Increase brightness by 10%."""
        current = self.matrix.brightness
        new_brightness = min(100, current + 10)
        self.set_brightness(new_brightness)
        print(f"🔆 Brightness: {new_brightness}%")
    
    def decrease_brightness(self):
        """Decrease brightness by 10%."""
        current = self.matrix.brightness
        new_brightness = max(1, current - 10)
        self.set_brightness(new_brightness)
        print(f"🔅 Brightness: {new_brightness}%")
    
    def run(self):
        """Main display loop with flip animations."""
        print("🕰️  Starting Authentic Twemco-Style Clock - Press CTRL-C to stop")
        print("🧡 Orange background with white frame and black digit windows")
        print("⌨️  Controls:")
        print("   SPACE = Manual refresh")
        print("   + = Increase brightness")
        print("   - = Decrease brightness")
        print("   Z = Next timezone")
        print("   X = Previous timezone") 
        print("   T = Toggle timezone display")
        
        # Initialize previous time values
        now = self.get_current_time()
        self.previous_hour = now.strftime("%I").replace("0", " ", 1) if now.strftime("%I").startswith("0") else now.strftime("%I")
        self.previous_minute = now.strftime("%M")
        
        try:
            while True:
                # Get current time
                now = self.get_current_time()
                current_hour = now.strftime("%I").replace("0", " ", 1) if now.strftime("%I").startswith("0") else now.strftime("%I")
                current_minute = now.strftime("%M")
                
                # Check for keyboard input
                self.check_for_input()
                
                # Check if time changed or manual flip triggered
                hour_changed = current_hour != self.previous_hour
                minute_changed = current_minute != self.previous_minute
                
                # Simple notifications when time changes
                if hour_changed:
                    self.simple_change(self.previous_hour, current_hour, is_hour=True)
                    
                if minute_changed:
                    self.simple_change(self.previous_minute, current_minute, is_hour=False)
                    
                # Handle manual flip trigger
                if self.manual_flip_triggered:
                    print("🔄 Manual change triggered - updating display")
                    self.manual_flip_triggered = False
                
                # Update previous values
                self.previous_hour = current_hour
                self.previous_minute = current_minute
                
                # Draw normal time display
                self.clear()
                self.draw_background_and_frame()
                self.draw_flip_time()
                self.swap()
                
                # Update every 0.1 seconds for responsive input
                time.sleep(0.1)
                
        except KeyboardInterrupt:
            print("\n⏰️  Clock stopped - Time stands still!")
        finally:
            self.clear()


if __name__ == "__main__":
    clock = RetroClock()
    clock.run()