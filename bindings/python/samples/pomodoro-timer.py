#!/usr/bin/env python3
# Pomodoro Timer for RGB LED Matrix
# 25-minute work timer with visual countdown and break reminders

import time
import sys
import os
import threading
import select
import argparse
from datetime import datetime, timedelta

# Add the parent directory to Python path to import the rgbmatrix module
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics

class PomodoroTimer:
    """Manages the LED matrix pomodoro timer display and logic."""
    
    def __init__(self, work_minutes=25, break_minutes=5, options=None):
        # Set up matrix options
        if options is None:
            options = RGBMatrixOptions()
            options.rows = 32
            options.cols = 64
            options.chain_length = 1
            options.parallel = 1
            options.hardware_mapping = 'adafruit-hat-pwm'
            options.brightness = 80
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        self.width = self.matrix.width
        self.height = self.matrix.height
        
        # Timer state
        self.work_duration = work_minutes * 60  # Convert to seconds
        self.break_duration = break_minutes * 60  # Convert to seconds
        self.remaining_time = self.work_duration
        self.is_running = False
        self.is_break = False
        self.is_paused = False
        
        # Load fonts
        self.time_font = graphics.Font()
        self.time_font.LoadFont("../../../fonts/8x13B.bdf")  # Slightly smaller bold font for timer
        
        self.label_font = graphics.Font()
        self.label_font.LoadFont("../../../fonts/6x13B.bdf")  # Font for labels
        
        self.small_font = graphics.Font()
        self.small_font.LoadFont("../../../fonts/5x8.bdf")  # Small font for instructions
        
        # Colors
        self.work_color = graphics.Color(0, 255, 0)      # Green for work time
        self.warning_color = graphics.Color(255, 165, 0)  # Orange for last 5 minutes
        self.urgent_color = graphics.Color(255, 120, 0)   # Softer orange-red for last minute
        self.break_color = graphics.Color(0, 150, 255)    # Blue for break time
        self.paused_color = graphics.Color(128, 128, 128) # Gray for paused
        self.text_color = graphics.Color(255, 255, 255)   # White for text
        
        # Session counter
        self.completed_sessions = 0
        
        # Layout constants
        self.padding = 2
        self.progress_bar_y = 30
        self.progress_bar_height = 2
        self.progress_bar_width = 60
        self.progress_bar_x = 2
    
    def clear(self):
        """Clear the display canvas."""
        self.canvas.Clear()
    
    def swap_canvas(self):
        """Swap the canvas to display the updated content."""
        self.canvas = self.matrix.SwapOnVSync(self.canvas)
        
    def format_time(self, seconds):
        """Format seconds as MM:SS"""
        minutes = int(seconds // 60)
        secs = int(seconds % 60)
        return f"{minutes:02d}:{secs:02d}"
    
    def get_timer_color(self):
        """Get the appropriate color based on remaining time and state"""
        if self.is_paused:
            return self.paused_color
        elif self.is_break:
            return self.break_color
        elif self.remaining_time <= 60:  # Last minute
            return self.urgent_color
        elif self.remaining_time <= 300:  # Last 5 minutes
            return self.warning_color
        else:
            return self.work_color
    
    def draw_progress_bar(self):
        """Draw a progress bar at the bottom of the screen"""
        bar_y = self.progress_bar_y
        bar_height = self.progress_bar_height
        bar_width = self.progress_bar_width
        bar_x = self.progress_bar_x
        
        # Calculate progress
        total_duration = self.break_duration if self.is_break else self.work_duration
        progress = 1 - (self.remaining_time / total_duration)
        filled_width = int(bar_width * progress)
        
        # Draw the entire progress bar starting with very light grey
        base_brightness = 40  # Much lighter grey base
        max_brightness = 255  # Bright white at full progress
        
        for x in range(bar_width):
            for y in range(bar_height):
                if x < filled_width:
                    # Progress area: start brighter and gradually brighten to full white
                    progress_ratio = (x + 1) / bar_width if bar_width > 0 else 0
                    min_progress_brightness = 180  # Start progress at brighter level
                    brightness = int(min_progress_brightness + (max_brightness - min_progress_brightness) * progress_ratio)
                    self.canvas.SetPixel(bar_x + x, bar_y + y, brightness, brightness, brightness)
                else:
                    # Empty area: very light grey
                    self.canvas.SetPixel(bar_x + x, bar_y + y, base_brightness, base_brightness, base_brightness)
    
    def draw_display(self):
        """Draw the complete timer display"""
        self.clear()
        
        # Draw main timer
        time_str = self.format_time(self.remaining_time)
        timer_color = self.text_color  # White font
        
        # Center the time display horizontally and vertically
        time_width = len(time_str) * 8  # Approximate width with 8x13 font
        time_x = (self.width - time_width) // 2
        
        # Calculate vertical center excluding progress bar area
        available_height = self.progress_bar_y - self.padding  # Height minus progress bar
        font_height = 13  # 8x13 font height
        time_y = (available_height + font_height) // 2
        
        graphics.DrawText(self.canvas, self.time_font, time_x, time_y, timer_color, time_str)
        
        # Draw progress bar
        self.draw_progress_bar()
        
        # Draw pause/play icon
        self.draw_status_icon()
        
        self.swap_canvas()
    
    def draw_status_icon(self):
        """Draw pause (red) or play (green) icon in top left corner."""
        icon_x = self.padding
        icon_y = self.padding
        
        if not self.is_running or self.is_paused:
            # Draw green play icon (filled triangle pointing right) - 6px tall
            green = graphics.Color(0, 255, 0)
            
            # Filled triangle shape (6 pixels tall)
            # Row 0
            self.canvas.SetPixel(icon_x, icon_y, green.red, green.green, green.blue)
            
            # Row 1
            self.canvas.SetPixel(icon_x, icon_y + 1, green.red, green.green, green.blue)
            self.canvas.SetPixel(icon_x + 1, icon_y + 1, green.red, green.green, green.blue)
            
            # Row 2
            self.canvas.SetPixel(icon_x, icon_y + 2, green.red, green.green, green.blue)
            self.canvas.SetPixel(icon_x + 1, icon_y + 2, green.red, green.green, green.blue)
            self.canvas.SetPixel(icon_x + 2, icon_y + 2, green.red, green.green, green.blue)
            
            # Row 3
            self.canvas.SetPixel(icon_x, icon_y + 3, green.red, green.green, green.blue)
            self.canvas.SetPixel(icon_x + 1, icon_y + 3, green.red, green.green, green.blue)
            self.canvas.SetPixel(icon_x + 2, icon_y + 3, green.red, green.green, green.blue)
            
            # Row 4
            self.canvas.SetPixel(icon_x, icon_y + 4, green.red, green.green, green.blue)
            self.canvas.SetPixel(icon_x + 1, icon_y + 4, green.red, green.green, green.blue)
            
            # Row 5
            self.canvas.SetPixel(icon_x, icon_y + 5, green.red, green.green, green.blue)
        else:
            # Draw red pause icon (two vertical bars) - 6px tall
            red = graphics.Color(255, 0, 0)
            
            # Left bar (1 pixel wide, 6 pixels tall)
            for y in range(6):
                self.canvas.SetPixel(icon_x, icon_y + y, red.red, red.green, red.blue)
            
            # Right bar (1 pixel wide, 6 pixels tall)
            for y in range(6):
                self.canvas.SetPixel(icon_x + 2, icon_y + y, red.red, red.green, red.blue)
    
    def start_timer(self):
        """Start or resume the timer"""
        self.is_running = True
        self.is_paused = False
    
    def pause_timer(self):
        """Pause the timer"""
        self.is_paused = True
    
    def reset_timer(self):
        """Reset the timer to work duration"""
        self.is_running = False
        self.is_paused = False
        self.is_break = False
        self.remaining_time = self.work_duration
    
    def start_break(self):
        """Start a break timer"""
        self.is_break = True
        self.remaining_time = self.break_duration
        self.is_running = True
        self.is_paused = False
    
    def timer_finished(self):
        """Handle timer completion"""
        if self.is_break:
            # Break finished, back to work
            self.is_break = False
            self.remaining_time = self.work_duration
            self.is_running = False
            print("\n🎯 Break finished! Ready for another focus session?")
        else:
            # Work session finished
            self.completed_sessions += 1
            print(f"\n🎉 Pomodoro #{self.completed_sessions} completed!")
            print("Time for a break! Press 'b' to start break timer, or 'r' to reset.")
            self.is_running = False
        
        # Flash the display for notification
        for i in range(6):
            self.canvas.Clear()
            if i % 2 == 0:
                # Draw "DONE!" message
                graphics.DrawText(self.canvas, self.time_font, 10, 18, self.urgent_color, "DONE!")
            self.canvas = self.matrix.SwapOnVSync(self.canvas)
            time.sleep(0.5)
    
    def get_input(self):
        """Get keyboard input in a non-blocking way"""
        if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
            return sys.stdin.read(1).lower()
        return None
    
    def handle_input(self, key):
        """Handle keyboard input"""
        if key == ' ':  # Space bar
            if self.is_running and not self.is_paused:
                self.pause_timer()
                print("⏸️  Timer paused")
            else:
                self.start_timer()
                print("▶️  Timer started/resumed")
        elif key == 'r':
            self.reset_timer()
            print("🔄 Timer reset")
        elif key == 'b' and not self.is_running and not self.is_break:
            self.start_break()
            print("☕ Break timer started")
        elif key == 'q':
            print("👋 Goodbye! Great job on your focus sessions!")
            return False
        elif key == 'h':
            self.print_help()
        return True
    
    def print_help(self):
        """Print help information"""
        print("\n📋 Pomodoro Timer Controls:")
        print("  SPACE - Start/Pause timer")
        print("  R     - Reset timer")
        print("  B     - Start break timer (when work session is complete)")
        print("  H     - Show this help")
        print("  Q     - Quit")
        print(f"\n📊 Current session: {self.completed_sessions} completed")
    
    def run(self):
        """Main timer loop"""
        print("🍅 Welcome to Pomodoro Timer!")
        print(f"{self.work_duration//60}-minute focus sessions with {self.break_duration//60}-minute breaks")
        self.print_help()
        
        # Set terminal to non-blocking mode for input
        import termios, tty
        old_settings = termios.tcgetattr(sys.stdin)
        try:
            tty.setraw(sys.stdin.fileno())
            
            last_update = time.time()
            
            while True:
                current_time = time.time()
                
                # Handle input
                key = self.get_input()
                if key and not self.handle_input(key):
                    break
                
                # Update timer
                if self.is_running and not self.is_paused:
                    elapsed = current_time - last_update
                    self.remaining_time -= elapsed
                    
                    if self.remaining_time <= 0:
                        self.remaining_time = 0
                        self.timer_finished()
                
                last_update = current_time
                
                # Update display
                self.draw_display()
                
                # Small delay to prevent excessive CPU usage
                time.sleep(0.1)
        
        except KeyboardInterrupt:
            print("\n👋 Timer interrupted. Goodbye!")
        finally:
            # Restore terminal settings
            termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
            self.clear()
            self.swap_canvas()

def main():
    """Main entry point for the pomodoro timer."""
    parser = argparse.ArgumentParser(description='Pomodoro Timer for RGB LED Matrix')
    parser.add_argument('-w', '--work', type=int, default=25,
                       help='Work session duration in minutes (default: 25)')
    parser.add_argument('--break-time', type=int, default=5,
                       help='Break duration in minutes (default: 5)')
    
    args = parser.parse_args()
    
    print(f"🍅 Starting Pomodoro Timer: {args.work}min work, {args.break_time}min break")
    
    timer = PomodoroTimer(work_minutes=args.work, break_minutes=args.break_time)
    timer.run()

if __name__ == "__main__":
    main()