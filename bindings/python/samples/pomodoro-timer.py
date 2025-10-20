#!/usr/bin/env python3
# Pomodoro Timer for RGB LED Matrix
# 25-minute work timer with visual countdown and break reminders

import time
import sys
import os
import threading
import select
from datetime import datetime, timedelta

# Add the parent directory to Python path to import the rgbmatrix module
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics

class PomodoroTimer:
    def __init__(self):
        # Configuration for the matrix
        options = RGBMatrixOptions()
        options.rows = 32
        options.cols = 64
        options.chain_length = 1
        options.parallel = 1
        options.hardware_mapping = 'adafruit-hat-pwm'
        options.brightness = 80
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        
        # Timer state
        self.work_duration = 25 * 60  # 25 minutes in seconds
        self.break_duration = 5 * 60  # 5 minutes in seconds
        self.remaining_time = self.work_duration
        self.is_running = False
        self.is_break = False
        self.is_paused = False
        
        # Load fonts
        self.time_font = graphics.Font()
        self.time_font.LoadFont("../../../fonts/9x18B.bdf")  # Bold font for timer
        
        self.label_font = graphics.Font()
        self.label_font.LoadFont("../../../fonts/6x13B.bdf")  # Font for labels
        
        self.small_font = graphics.Font()
        self.small_font.LoadFont("../../../fonts/5x8.bdf")  # Small font for instructions
        
        # Colors
        self.work_color = graphics.Color(0, 255, 0)      # Green for work time
        self.warning_color = graphics.Color(255, 165, 0)  # Orange for last 5 minutes
        self.urgent_color = graphics.Color(255, 0, 0)     # Red for last minute
        self.break_color = graphics.Color(0, 150, 255)    # Blue for break time
        self.paused_color = graphics.Color(128, 128, 128) # Gray for paused
        self.text_color = graphics.Color(255, 255, 255)   # White for text
        
        # Session counter
        self.completed_sessions = 0
        
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
        bar_y = 30
        bar_height = 2
        bar_width = 60
        bar_x = 2
        
        # Calculate progress
        total_duration = self.break_duration if self.is_break else self.work_duration
        progress = 1 - (self.remaining_time / total_duration)
        filled_width = int(bar_width * progress)
        
        # Draw background (empty bar)
        for x in range(bar_width):
            for y in range(bar_height):
                self.canvas.SetPixel(bar_x + x, bar_y + y, 50, 50, 50)  # Dark gray
        
        # Draw progress (filled bar)
        color = self.get_timer_color()
        for x in range(filled_width):
            for y in range(bar_height):
                self.canvas.SetPixel(bar_x + x, bar_y + y, color.red, color.green, color.blue)
    
    def draw_display(self):
        """Draw the complete timer display"""
        self.canvas.Clear()
        
        # Draw main timer
        time_str = self.format_time(self.remaining_time)
        timer_color = self.get_timer_color()
        
        # Center the time display
        time_width = len(time_str) * 9  # Approximate width with 9x18 font
        time_x = (64 - time_width) // 2
        graphics.DrawText(self.canvas, self.time_font, time_x, 18, timer_color, time_str)
        
        # Draw session label
        if self.is_break:
            label = "BREAK"
            label_color = self.break_color
        elif self.is_paused:
            label = "PAUSED"
            label_color = self.paused_color
        else:
            label = "FOCUS"
            label_color = self.work_color
        
        # Center the label
        label_width = len(label) * 6  # Approximate width
        label_x = (64 - label_width) // 2
        graphics.DrawText(self.canvas, self.label_font, label_x, 8, label_color, label)
        
        # Draw session counter
        sessions_text = f"Sessions: {self.completed_sessions}"
        graphics.DrawText(self.canvas, self.small_font, 2, 28, self.text_color, sessions_text)
        
        # Draw progress bar
        self.draw_progress_bar()
        
        # Draw controls hint if paused or not running
        if not self.is_running or self.is_paused:
            controls = "SPACE=Start R=Reset Q=Quit"
            # Draw in small text at the top
            graphics.DrawText(self.canvas, self.small_font, 1, 5, self.text_color, controls[:21])
        
        self.canvas = self.matrix.SwapOnVSync(self.canvas)
    
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
        print("25-minute focus sessions with 5-minute breaks")
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
            self.canvas.Clear()
            self.canvas = self.matrix.SwapOnVSync(self.canvas)

if __name__ == "__main__":
    timer = PomodoroTimer()
    timer.run()