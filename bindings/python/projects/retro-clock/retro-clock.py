#!/usr/bin/env python
"""
Retro Clock - 80s/90s Style LED Matrix Display

A nostalgic clock display featuring:
- Retro color scheme (lime green, yellow, magenta)
- Digital-style time display with classic fonts
- Animated elements and effects
- Day/date display with retro styling

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


class RetroClock(MatrixBase):
    """Retro-styled clock display with 80s/90s aesthetic."""
    
    def __init__(self):
        # Initialize matrix with standard configuration
        super().__init__(
            rows=32,
            cols=64,
            hardware_mapping='adafruit-hat-pwm'
        )
        
        # Initialize shared components with retro theme
        self.font_manager = FontManager()
        self.color_palette = ColorPalette('retro')  # Use retro theme!
        
        # Load fonts for different elements
        self.time_font = self.font_manager.get_font('xxlarge')     # 9x18B for time
        self.date_font = self.font_manager.get_font('medium_bold') # 6x13B for date
        self.day_font = self.font_manager.get_font('small')        # 5x7 for day
        
        # Get retro theme colors
        self.primary_color = self.color_palette.get_color('primary')    # Lime green
        self.secondary_color = self.color_palette.get_color('secondary') # Yellow
        self.accent_color = self.color_palette.get_color('accent')       # Magenta
        
        # Animation state
        self.animation_frame = 0
        self.blink_state = True
        
        print("🕒 Retro Clock initialized with 80s theme!")
        print(f"Matrix size: {self.width}x{self.height}")
        
    def draw_retro_border(self):
        """Draw a retro-style animated border."""
        # Animate border color between accent and secondary
        if (self.animation_frame // 30) % 2 == 0:  # Switch every 30 frames (~1 second)
            border_color = self.accent_color      # Magenta
        else:
            border_color = self.secondary_color   # Yellow
            
        # Draw corner pixels for retro look
        corners = [(0, 0), (63, 0), (0, 31), (63, 31)]
        for x, y in corners:
            self.set_pixel(x, y, border_color)
            
        # Draw small side decorations
        mid_y = 15  # Middle of 32px display
        for offset in [-1, 0, 1]:
            if 0 <= mid_y + offset < 32:
                self.set_pixel(0, mid_y + offset, border_color)   # Left side
                self.set_pixel(63, mid_y + offset, border_color)  # Right side
    
    def draw_retro_time(self):
        """Draw time in retro digital style with animations."""
        now = datetime.now()
        
        # Format time in 12-hour format
        time_str = now.strftime("%I:%M")
        if time_str.startswith("0"):
            time_str = time_str[1:]  # Remove leading zero
        
        # Add blinking colon effect
        if self.blink_state:
            time_display = time_str
        else:
            time_display = time_str.replace(":", " ")  # Replace colon with space for blink
            
        # Calculate position for centered time
        time_width = 0
        for char in time_display:
            time_width += self.time_font.CharacterWidth(ord(char))
        time_x = (64 - time_width) // 2
        time_y = 20  # Positioned in lower half
        
        # Draw time with retro primary color (lime green)
        current_x = time_x
        for char in time_display:
            if char == ' ':
                current_x += 4
            else:
                char_width = self.draw_text(self.time_font, current_x, time_y, 
                                          self.primary_color, char)
                current_x += char_width
    
    def draw_retro_date(self):
        """Draw date in retro style."""
        now = datetime.now()
        
        # Day of week in small font (secondary color - yellow)
        day_str = now.strftime("%a").upper()  # "WED"
        day_width = 0
        for char in day_str:
            day_width += self.day_font.CharacterWidth(ord(char))
        day_x = (64 - day_width) // 2
        day_y = 8
        
        current_x = day_x
        for char in day_str:
            char_width = self.draw_text(self.day_font, current_x, day_y, 
                                      self.secondary_color, char)
            current_x += char_width
        
        # Date in medium font (accent color - magenta)
        date_str = now.strftime("%m/%d")  # "10/21"
        date_width = 0
        for char in date_str:
            date_width += self.date_font.CharacterWidth(ord(char))
        date_x = (64 - date_width) // 2
        date_y = 30
        
        current_x = date_x
        for char in date_str:
            if char == '/':
                char_width = self.draw_text(self.date_font, current_x, date_y, 
                                          self.secondary_color, char)  # Yellow slash
            else:
                char_width = self.draw_text(self.date_font, current_x, date_y, 
                                          self.accent_color, char)     # Magenta numbers
            current_x += char_width
    
    def draw_retro_effects(self):
        """Add retro visual effects."""
        # Animated dots on sides
        dot_positions = [
            (2, 5), (2, 10), (2, 15), (2, 20), (2, 25),    # Left side
            (61, 5), (61, 10), (61, 15), (61, 20), (61, 25)  # Right side
        ]
        
        # Animate dots - create a "scanning" effect
        active_dot = (self.animation_frame // 10) % len(dot_positions)
        
        for i, (x, y) in enumerate(dot_positions[:5]):  # Left side only
            if i == active_dot % 5:  # Active dot
                color = self.accent_color      # Bright magenta
            else:
                color = self.color_palette.dim_color(self.secondary_color, 0.3)  # Dim yellow
            self.set_pixel(x, y, color)
            self.set_pixel(63 - x + 2, y, color)  # Mirror on right side
    
    def run(self):
        """Main display loop with retro effects."""
        print("🎮 Starting Retro Clock - Press CTRL-C to stop")
        print("✨ Featuring 80s-style animations and colors!")
        
        try:
            while True:
                self.clear()
                
                # Draw all retro elements
                self.draw_retro_border()
                self.draw_retro_time()
                self.draw_retro_date()
                self.draw_retro_effects()
                
                # Update display
                self.swap()
                
                # Update animation states
                self.animation_frame += 1
                
                # Blink colon every 30 frames (~1 second at 30fps)
                if self.animation_frame % 30 == 0:
                    self.blink_state = not self.blink_state
                
                # Reset animation frame to prevent overflow
                if self.animation_frame >= 1800:  # Reset every minute
                    self.animation_frame = 0
                
                # 30fps refresh rate for smooth animations
                time.sleep(1.0 / 30.0)
                
        except KeyboardInterrupt:
            print("\n🛑 Retro Clock stopped - Back to the future!")
        finally:
            self.clear()


if __name__ == "__main__":
    retro_clock = RetroClock()
    retro_clock.run()