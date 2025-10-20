#!/usr/bin/env python3
# Pomodoro Timer Test Version (Console Only)
# For testing the timer logic without LED matrix hardware

import time
import sys
import threading
import select
import argparse
from datetime import datetime, timedelta

class PomodoroTimerTest:
    def __init__(self, work_seconds=10, break_seconds=5):
        # Timer state
        self.work_duration = work_seconds
        self.break_duration = break_seconds
        self.remaining_time = self.work_duration
        self.is_running = False
        self.is_break = False
        self.is_paused = False
        
        # Session counter
        self.completed_sessions = 0
        
    def format_time(self, seconds):
        """Format seconds as MM:SS"""
        minutes = int(seconds // 60)
        secs = int(seconds % 60)
        return f"{minutes:02d}:{secs:02d}"
    
    def get_status_display(self):
        """Get current status for display"""
        if self.is_paused:
            status = "PAUSED"
        elif self.is_break:
            status = "BREAK"
        else:
            status = "FOCUS"
        return status
    
    def display_timer(self):
        """Display the timer in console"""
        # Clear screen (ANSI escape code)
        print("\033[2J\033[H", end="")
        
        time_str = self.format_time(self.remaining_time)
        status = self.get_status_display()
        
        # Calculate progress
        total_duration = self.break_duration if self.is_break else self.work_duration
        progress = 1 - (self.remaining_time / total_duration)
        progress_bar = "█" * int(20 * progress) + "░" * int(20 * (1 - progress))
        
        print("🍅 POMODORO TIMER")
        print("=" * 30)
        print(f"Time:   {time_str}")
        print(f"Progress: [{progress_bar}]")
        print()
    
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
        
        # Notification effect
        for i in range(3):
            print("🔔 DING! " * 5)
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
        return True
    
    def run(self):
        """Main timer loop"""
        print("🍅 Welcome to Pomodoro Timer Test!")
        print(f"Work sessions: {self.work_duration}s, Breaks: {self.break_duration}s")
        print("(Shortened durations for testing)")
        print()
        
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
                self.display_timer()
                
                # Small delay to prevent excessive CPU usage
                time.sleep(0.1)
        
        except KeyboardInterrupt:
            print("\n👋 Timer interrupted. Goodbye!")
        finally:
            # Restore terminal settings
            termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Pomodoro Timer Test - Console Version')
    parser.add_argument('-w', '--work', type=int, default=10, 
                       help='Work session duration in seconds (default: 10 for testing)')
    parser.add_argument('-b', '--break', type=int, default=5,
                       help='Break duration in seconds (default: 5 for testing)')
    parser.add_argument('--pomodoro', action='store_true',
                       help='Use standard pomodoro times: 25min work, 5min break')
    
    args = parser.parse_args()
    
    # Use standard pomodoro times if requested
    if args.pomodoro:
        work_time = 25 * 60
        break_time = 5 * 60
    else:
        work_time = args.work
        break_time = getattr(args, 'break')
    
    print(f"🍅 Starting Pomodoro Timer: {work_time//60 if work_time >= 60 else work_time}{'min' if work_time >= 60 else 's'} work, {break_time//60 if break_time >= 60 else break_time}{'min' if break_time >= 60 else 's'} break")
    
    timer = PomodoroTimerTest(work_seconds=work_time, break_seconds=break_time)
    timer.run()