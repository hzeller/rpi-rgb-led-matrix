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
        
        # Classic flip clock colors - simple and clean
        self.digit_color = self.color_palette.get_color('white')    # White digits
        self.background_color = self.color_palette.get_color('black') # Black background
        self.ampm_color = self.color_palette.get_color('gray_light')  # Gray AM/PM
        
        # Store previous time for flip detection
        self.previous_time = ""
        self.show_ampm = True  # Option to show AM/PM
        
        print("�️  Retro Flip Clock initialized - Classic 1970s style")
        print(f"Matrix size: {self.width}x{self.height}")
        
    def draw_digit_blocks(self, digit_str, start_x, y):
        """Draw digits with flip-card style blocks."""
        current_x = start_x
        
        for char in digit_str:
            if char == ':':
                # Draw colon as two dots (flip clock style)
                dot_color = self.digit_color
                self.set_pixel(current_x + 1, y - 6, dot_color)   # Upper dot
                self.set_pixel(current_x + 1, y - 2, dot_color)   # Lower dot
                current_x += 4  # Space for colon
            elif char == ' ':
                current_x += 6  # Space between digits
            else:
                # Draw the actual digit
                char_width = self.draw_text(self.digit_font, current_x, y, 
                                          self.digit_color, char)
                current_x += char_width + 2  # Add slight spacing between digits
        
        return current_x
    
    def draw_flip_time(self):
        """Draw time in classic flip clock style."""
        now = datetime.now()
        
        # Format time in 12-hour format with leading space instead of zero
        hour = now.strftime("%I")
        if hour.startswith("0"):
            hour = " " + hour[1:]  # Replace leading zero with space
        
        minute = now.strftime("%M")
        time_str = f"{hour}:{minute}"
        
        # Calculate total width to center the time
        total_width = 0
        for char in time_str:
            if char == ':':
                total_width += 4  # Colon width
            elif char == ' ':
                total_width += 6  # Space width
            else:
                total_width += self.digit_font.CharacterWidth(ord(char)) + 2
        
        # Center horizontally and vertically
        start_x = (64 - total_width) // 2
        time_y = 20  # Vertically centered for 32px display
        
        # Draw the time digits
        self.draw_digit_blocks(time_str, start_x, time_y)
        
        # Draw AM/PM if enabled
        if self.show_ampm:
            ampm = now.strftime("%p")
            ampm_width = 0
            for char in ampm:
                ampm_width += self.ampm_font.CharacterWidth(ord(char))
            
            # Position AM/PM in bottom right corner
            ampm_x = 64 - ampm_width - 2
            ampm_y = 30
            
            current_x = ampm_x
            for char in ampm:
                char_width = self.draw_text(self.ampm_font, current_x, ampm_y,
                                          self.ampm_color, char)
                current_x += char_width
    
    def draw_subtle_frame(self):
        """Draw a subtle frame around the display area (optional)."""
        # Very minimal frame - just corner pixels for classic look
        frame_color = self.color_palette.get_color('gray_dark')
        
        # Just small corner indicators
        corners = [(1, 1), (62, 1), (1, 30), (62, 30)]
        for x, y in corners:
            self.set_pixel(x, y, frame_color)
    

    
    def run(self):
        """Main display loop - simple and clean like a real flip clock."""
        print("🕰️  Starting Classic Flip Clock - Press CTRL-C to stop")
        print("📟 Authentic 1970s styling - simple and elegant")
        
        try:
            while True:
                # Clear the display
                self.clear()
                
                # Draw the time in flip clock style
                self.draw_flip_time()
                
                # Optional: Draw subtle frame (uncomment if desired)
                # self.draw_subtle_frame()
                
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