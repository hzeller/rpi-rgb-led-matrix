#!/usr/bin/env python
"""
Brightness Test - Demonstration of brightness control across all apps.

This script shows how to control brightness dynamically and save preferences.
Run this to test the brightness system before using it in your applications.
"""

import sys
import os
import time

# Add the shared directory to the path
shared_dir = os.path.join(os.path.dirname(__file__), 'shared')
sys.path.insert(0, shared_dir)

from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette

class BrightnessTest(MatrixBase):
    """
    Test class to demonstrate brightness control functionality.
    """
    
    def __init__(self):
        super().__init__()
        self.font_manager = FontManager()
        self.colors = ColorPalette()
        
    def run_brightness_demo(self):
        """
        Run a demonstration of different brightness levels.
        """
        print("🌟 Starting Brightness Control Demo")
        print("=" * 50)
        
        # Load font and color
        font = self.font_manager.load_font('small')
        color = self.colors.get_color('primary')
        
        # Test different brightness levels
        brightness_levels = [100, 80, 60, 40, 20, 10, 5, 1]
        
        for brightness in brightness_levels:
            print(f"Setting brightness to {brightness}%...")
            self.set_brightness(brightness)
            
            # Clear and draw test content
            self.clear()
            self.draw_text(font, 2, 10, color, f"Brightness")
            self.draw_text(font, 12, 20, color, f"{brightness}%")
            self.swap()
            
            time.sleep(1.5)
            
        print("\n📊 Testing brightness getter...")
        current = self.get_brightness()
        print(f"Current brightness: {current}%")
        
        print("\n🔅 Testing dim function...")
        self.set_brightness(80)
        print(f"Set to 80%, now dimming by 50%...")
        self.dim_display(0.5)
        
        time.sleep(2)
        
        print("\n💾 Testing brightness preference saving...")
        self.save_brightness_preference(60)
        
        # Final display
        self.clear()
        self.draw_text(font, 5, 10, color, "Brightness")
        self.draw_text(font, 8, 20, color, "Test Done!")
        self.swap()
        
        time.sleep(3)
        self.clear()
        self.swap()
        
        print("\n✅ Brightness demo completed!")
        print("💡 Tips:")
        print("  - Use set_brightness(1-100) for direct control")
        print("  - Use dim_display(0.5) for relative dimming")
        print("  - Use save_brightness_preference() to persist settings")
        print("  - Brightness settings are loaded from .env file automatically")

def main():
    """
    Main function to run the brightness test.
    """
    try:
        test = BrightnessTest()
        test.run_brightness_demo()
        
    except KeyboardInterrupt:
        print("\n🛑 Test interrupted by user")
    except Exception as e:
        print(f"❌ Error during brightness test: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()