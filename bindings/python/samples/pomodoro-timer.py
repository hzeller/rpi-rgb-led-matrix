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
from samplebase import SampleBase

class PomodoroTimer(SampleBase):
    def __init__(self, *args, **kwargs):
        super(PomodoroTimer, self).__init__(*args, **kwargs)
        
        # Add custom arguments for timer durations
        self.parser.add_argument('-w', '--work', type=int, default=25,
                               help='Work session duration in minutes (default: 25)')
        self.parser.add_argument('-b', '--break-time', type=int, default=5,
                               help='Break duration in minutes (default: 5)')
    
    def get_matrix_options(self):
        """Get matrix options with defaults"""
        options = RGBMatrixOptions()
        
        # Set defaults for LED matrix
        if self.args.led_gpio_mapping != None:
            options.hardware_mapping = self.args.led_gpio_mapping
        else:
            options.hardware_mapping = 'adafruit-hat-pwm'  # Default for this project
            
        options.rows = self.args.led_rows
        options.cols = self.args.led_cols
        options.chain_length = self.args.led_chain
        options.parallel = self.args.led_parallel
        options.row_address_type = self.args.led_row_addr_type
        options.multiplexing = self.args.led_multiplexing
        options.pwm_bits = self.args.led_pwm_bits
        options.brightness = self.args.led_brightness
        options.pwm_lsb_nanoseconds = self.args.led_pwm_lsb_nanoseconds
        options.led_rgb_sequence = self.args.led_rgb_sequence
        options.pixel_mapper_config = self.args.led_pixel_mapper
        options.panel_type = self.args.led_panel_type

        if self.args.led_show_refresh:
            options.show_refresh_rate = 1
        if self.args.led_slowdown_gpio != None:
            options.gpio_slowdown = self.args.led_slowdown_gpio
        if self.args.led_no_hardware_pulse:
            options.disable_hardware_pulsing = True
        if not self.args.drop_privileges:
            options.drop_privileges = False
            
        return options
    
    def process(self):
        """Override SampleBase process method"""
        self.args = self.parser.parse_args()

        options = self.get_matrix_options()
        self.matrix = RGBMatrix(options = options)

        try:
            # Start the timer
            print("Press CTRL-C to stop")
            self.run()
        except KeyboardInterrupt:
            print("Exiting\n")
            return True
        return True
    
    def setup_timer(self, work_minutes, break_minutes):
        """Set up the timer with specified durations"""
        self.canvas = self.matrix.CreateFrameCanvas()
        
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
        timer_color = graphics.Color(255, 255, 255)  # White font
        
        # Center the time display
        time_width = len(time_str) * 8  # Approximate width with 8x13 font
        time_x = (64 - time_width) // 2
        graphics.DrawText(self.canvas, self.time_font, time_x, 18, timer_color, time_str)
        
        # Draw progress bar
        self.draw_progress_bar()
        
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
        # Set up timer with custom durations
        work_minutes = self.args.work
        break_minutes = getattr(self.args, 'break_time')
        self.setup_timer(work_minutes, break_minutes)
        
        print("🍅 Welcome to Pomodoro Timer!")
        print(f"{work_minutes}-minute focus sessions with {break_minutes}-minute breaks")
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
    if not timer.process():
        timer.print_help()