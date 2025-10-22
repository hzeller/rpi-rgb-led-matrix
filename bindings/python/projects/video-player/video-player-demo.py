#!/usr/bin/env python
"""
Video Player Demo Script

This script demonstrates various ways to use the video-player.py with different settings.
Uncomment the sections you want to test.
"""

import os
import subprocess
import time

def run_command(cmd, description):
    """Run a command and handle interruption."""
    print(f"\n{'='*60}")
    print(f"DEMO: {description}")
    print(f"Command: {' '.join(cmd)}")
    print(f"{'='*60}")
    print("Press CTRL+C to stop current demo and go to next one...")
    
    try:
        # Run for a few seconds then ask user
        process = subprocess.Popen(cmd)
        time.sleep(5)  # Let it run for 5 seconds
        
        print(f"\nDemo running... Press Enter to stop and continue to next demo (or wait)")
        
        # Give user choice to continue or stop
        import select
        import sys
        
        if select.select([sys.stdin], [], [], 10.0)[0]:  # Wait 10 seconds for input
            input()  # User pressed enter
            
        process.terminate()
        process.wait()
        
    except KeyboardInterrupt:
        print("\nStopping demo...")
        try:
            process.terminate()
            process.wait()
        except:
            pass

def main():
    """Run video player demos."""
    
    print("RGB LED Matrix Video Player Demo")
    print("This will run several demonstration scenarios.")
    print("Each demo runs for a few seconds - press Enter to skip to next demo.")
    
    # Change to the samples directory
    samples_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(samples_dir)
    
    demos = [
        # Basic demos
        (
            ["python", "video-player.py"],
            "Basic video playback with hardcoded URL"
        ),
        
        # Different fit modes
        (
            ["python", "video-player.py", "--fit-mode", "stretch"],
            "Stretch mode - ignore aspect ratio"
        ),
        (
            ["python", "video-player.py", "--fit-mode", "fill"],
            "Fill mode - maintain aspect ratio, crop to fill"
        ),
        (
            ["python", "video-player.py", "--fit-mode", "center"],
            "Center mode - no scaling, just center"
        ),
        
        # Speed and timing demos
        (
            ["python", "video-player.py", "--speed", "2.0"],
            "Double speed playback"
        ),
        (
            ["python", "video-player.py", "--speed", "0.5"],
            "Half speed playback"
        ),
        (
            ["python", "video-player.py", "--start-time", "5", "--end-time", "15"],
            "Play only seconds 5-15"
        ),
        
        # Enhancement demos
        (
            ["python", "video-player.py", "--brightness-adjust", "1.5"],
            "Increased brightness"
        ),
        (
            ["python", "video-player.py", "--contrast-adjust", "1.3", "--saturation-adjust", "1.2"],
            "Enhanced contrast and saturation"
        ),
        
        # Performance demos
        (
            ["python", "video-player.py", "--fps-override", "15"],
            "Limited to 15 FPS for smooth playback"
        ),
        (
            ["python", "video-player.py", "--cache-frames", "50", "--verbose"],
            "Frame caching for smoother playback (verbose mode)"
        ),
        
        # Matrix configuration demos (adjust these for your setup)
        (
            ["python", "video-player.py", "--led-brightness", "50"],
            "Reduced LED brightness to 50%"
        ),
        
        # Loop demo
        (
            ["python", "video-player.py", "--loop-count", "2", "--speed", "3.0"],
            "Fast playback, loop 2 times"
        ),
    ]
    
    print(f"\nWill run {len(demos)} demos. Starting in 3 seconds...")
    time.sleep(3)
    
    for i, (cmd, description) in enumerate(demos, 1):
        print(f"\n\n--- DEMO {i}/{len(demos)} ---")
        run_command(cmd, description)
        time.sleep(1)  # Brief pause between demos
    
    print(f"\n{'='*60}")
    print("All demos completed!")
    print("You can now run individual commands or modify the video-player.py script.")
    print("See VIDEO-PLAYER-README.md for complete documentation.")
    print(f"{'='*60}")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nDemo interrupted by user. Goodbye!")