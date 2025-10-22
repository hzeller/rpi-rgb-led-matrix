#!/usr/bin/env python
"""
Retro Flip Clock - Classic         # Store previous time for flip detection
        self.previous_time = ""
        self.previous_hour = ""
        self.previous_minute = ""
        self.show_ampm = True  # Option to show AM/PM
        
        # Flip animation state
        self.manual_flip_triggered = False
        self.flip_animation_frames = 8  # Number of frames in flip animation
        self.flip_duration = 0.4  # Total duration in seconds
        
        print("🕰️  Retro Flip Clock initialized - Classic 1970s style")
        print(f"Matrix size: {self.width}x{self.height}")
        print("💡 Press SPACE to trigger manual flip animation")Style LED Matrix Display

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
import threading
from datetime import datetime
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
    
    def animate_flip(self, window_rect, old_text, new_text, is_hour=True):
        """Animate a flip card transition between old and new text."""
        frame_duration = self.flip_duration / self.flip_animation_frames
        
        for frame in range(self.flip_animation_frames):
            # Clear the window area
            for x in range(window_rect['x'], window_rect['x'] + window_rect['width']):
                for y in range(window_rect['y'], window_rect['y'] + window_rect['height']):
                    if 0 <= x < 64 and 0 <= y < 32:
                        self.set_pixel(x, y, self.window_color)
            
            # Calculate flip progress (0.0 to 1.0)
            progress = frame / (self.flip_animation_frames - 1)
            
            # Create flipping effect by squashing the window vertically
            if progress < 0.5:
                # First half: show old text squashing down
                squash_factor = 1.0 - (progress * 2)  # 1.0 to 0.0
                text_to_show = old_text
            else:
                # Second half: show new text expanding up
                squash_factor = (progress - 0.5) * 2  # 0.0 to 1.0
                text_to_show = new_text
            
            # Calculate squashed height and position
            normal_height = window_rect['height']
            squashed_height = max(2, int(normal_height * squash_factor))
            y_offset = (normal_height - squashed_height) // 2
            
            # Draw squashed window
            squashed_rect = {
                'x': window_rect['x'],
                'y': window_rect['y'] + y_offset,
                'width': window_rect['width'],
                'height': squashed_height
            }
            
            # Fill squashed window with black
            for x in range(squashed_rect['x'], squashed_rect['x'] + squashed_rect['width']):
                for y in range(squashed_rect['y'], squashed_rect['y'] + squashed_rect['height']):
                    if 0 <= x < 64 and 0 <= y < 32:
                        self.set_pixel(x, y, self.window_color)
            
            # Draw text only if window is tall enough and squash factor is significant
            if squashed_height > 6 and squash_factor > 0.3:
                # Calculate text position
                text_width = 0
                for char in text_to_show:
                    if char != ' ':
                        text_width += self.digit_font.CharacterWidth(ord(char))
                
                text_x = squashed_rect['x'] + (squashed_rect['width'] - text_width) // 2
                text_y = squashed_rect['y'] + int(squashed_rect['height'] * 0.8)  # Near bottom of squashed window
                
                # Draw the text
                current_x = text_x
                for char in text_to_show:
                    if char != ' ':
                        char_width = self.draw_text(self.digit_font, current_x, text_y,
                                                  self.digit_color, char)
                        current_x += char_width
            
            # Update display
            self.swap()
            time.sleep(frame_duration)
    
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
            else:
                # Unix/Linux implementation
                if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
                    line = sys.stdin.readline().strip()
                    if line == ' ' or line == '':  # Space or Enter key
                        self.manual_flip_triggered = True
                        return True
        except:
            # Fallback - no input detection
            pass
        return False
    
    def run(self):
        """Main display loop with flip animations."""
        print("🕰️  Starting Authentic Twemco-Style Flip Clock - Press CTRL-C to stop")
        print("🧡 Orange background with white frame and black digit windows")
        print("⌨️  Press SPACE + ENTER to manually trigger flip animation")
        
        # Initialize previous time values
        now = datetime.now()
        self.previous_hour = now.strftime("%I").replace("0", " ", 1) if now.strftime("%I").startswith("0") else now.strftime("%I")
        self.previous_minute = now.strftime("%M")
        
        try:
            while True:
                # Get current time
                now = datetime.now()
                current_hour = now.strftime("%I").replace("0", " ", 1) if now.strftime("%I").startswith("0") else now.strftime("%I")
                current_minute = now.strftime("%M")
                
                # Check for keyboard input
                self.check_for_input()
                
                # Check if time changed or manual flip triggered
                hour_changed = current_hour != self.previous_hour
                minute_changed = current_minute != self.previous_minute
                
                if hour_changed or minute_changed or self.manual_flip_triggered:
                    # Define window rectangles
                    hour_window = {'x': 6, 'y': 8, 'width': 22, 'height': 16}
                    minute_window = {'x': 36, 'y': 8, 'width': 22, 'height': 16}
                    
                    # Draw static background and frame first
                    self.clear()
                    self.draw_background_and_frame()
                    
                    # Draw AM/PM (doesn't flip)
                    if self.show_ampm:
                        ampm = now.strftime("%p").lower()
                        ampm_x = 4
                        ampm_y = 7
                        current_x = ampm_x
                        for char in ampm:
                            char_width = self.draw_text(self.ampm_font, current_x, ampm_y,
                                                      self.ampm_color, char)
                            current_x += char_width
                    
                    # Animate flips for changed digits
                    if hour_changed or self.manual_flip_triggered:
                        print(f"🔄 Flipping hour: {self.previous_hour} → {current_hour}")
                        self.animate_flip(hour_window, self.previous_hour, current_hour, is_hour=True)
                        
                    if minute_changed or self.manual_flip_triggered:
                        print(f"🔄 Flipping minute: {self.previous_minute} → {current_minute}")
                        self.animate_flip(minute_window, self.previous_minute, current_minute, is_hour=False)
                    
                    # Update previous values
                    self.previous_hour = current_hour
                    self.previous_minute = current_minute
                    self.manual_flip_triggered = False
                
                # Draw normal time display
                self.clear()
                self.draw_background_and_frame()
                self.draw_flip_time()
                self.swap()
                
                # Update every 0.1 seconds for responsive input
                time.sleep(0.1)
                
        except KeyboardInterrupt:
            print("\n⏰️  Flip clock stopped - Time stands still!")
        finally:
            self.clear()


if __name__ == "__main__":
    flip_clock = RetroFlipClock()
    flip_clock.run()