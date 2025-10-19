#!/usr/bin/env python3
# Weather display for RGB LED matrix - Denver, CO
# Shows current weather conditions and temperature

import time
import sys
import os
import requests
import json
from datetime import datetime, timezone, timedelta

# Load environment variables
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    print("python-dotenv not installed. Using system environment variables only.")

# Add the parent directory to Python path to import the rgbmatrix module
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from PIL import Image
from io import BytesIO

class WeatherDisplay:
    def __init__(self):
        # Configuration for the matrix - use same settings as other displays
        options = RGBMatrixOptions()
        options.rows = 32
        options.cols = 64
        options.chain_length = 1
        options.parallel = 1
        options.hardware_mapping = 'adafruit-hat-pwm'
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        
        # Set up Mountain Time timezone
        self.mountain_tz = timezone(timedelta(hours=-6))  # Mountain Daylight Time
        
        # Load fonts
        self.temp_font = graphics.Font()
        self.temp_font.LoadFont("../../../fonts/9x18B.bdf")  # Bold font for temperature
        
        self.condition_font = graphics.Font()
        self.condition_font.LoadFont("../../../fonts/6x13B.bdf")  # Bold font for conditions
        
        self.small_font = graphics.Font()
        self.small_font.LoadFont("../../../fonts/5x7.bdf")  # Small font for details
        
        # Colors
        self.temp_color = graphics.Color(255, 255, 255)     # White for temperature
        self.condition_color = graphics.Color(100, 200, 255) # Light blue for conditions
        self.detail_color = graphics.Color(200, 200, 200)   # Gray for details
        
        # Weather data
        self.weather_data = None
        self.last_update = 0
        self.weather_icon = None
        self.last_icon_update = 0
        
        # OpenWeatherMap API setup - get API key from environment variable
        self.api_key = os.getenv('OPENWEATHER_API_KEY')
        self.denver_coords = {"lat": 39.7392, "lon": -104.9903}
        
    def get_weather_data(self):
        """Fetch weather data from OpenWeatherMap API"""
        try:
            # Only update every 10 minutes to avoid API limits
            if time.time() - self.last_update < 600:
                return self.weather_data
                
            url = f"https://api.openweathermap.org/data/2.5/weather"
            params = {
                "lat": self.denver_coords["lat"],
                "lon": self.denver_coords["lon"],
                "appid": self.api_key,
                "units": "imperial"  # Fahrenheit
            }
            
            response = requests.get(url, params=params, timeout=10)
            if response.status_code == 200:
                self.weather_data = response.json()
                self.last_update = time.time()
                print(f"Weather updated: {self.weather_data['weather'][0]['description']}, {self.weather_data['main']['temp']}°F")
                return self.weather_data
            else:
                print(f"Weather API error: {response.status_code}")
                return None
                
        except Exception as e:
            print(f"Weather fetch error: {e}")
            return None
    
    def get_weather_icon(self, icon_code):
        """Download and cache weather icon from OpenWeatherMap"""
        try:
            # Only update icon every 10 minutes to avoid excessive downloads
            if time.time() - self.last_icon_update < 600 and self.weather_icon is not None:
                print(f"Using cached icon: {icon_code}, size: {self.weather_icon.size}, mode: {self.weather_icon.mode}")
                return self.weather_icon
                
            print(f"Downloading new weather icon: {icon_code}")
            
            # OpenWeatherMap icon URL
            icon_url = f"https://openweathermap.org/img/wn/{icon_code}@2x.png"
            print(f"Icon URL: {icon_url}")
            
            response = requests.get(icon_url, timeout=10)
            if response.status_code == 200:
                print(f"Download successful, content length: {len(response.content)} bytes")
                
                # Load image and resize to fit display - make it taller to span both temp lines
                icon_image = Image.open(BytesIO(response.content))
                print(f"Original icon size: {icon_image.size}, mode: {icon_image.mode}")
                
                # Handle transparency properly - create white background first
                if icon_image.mode in ('RGBA', 'LA') or 'transparency' in icon_image.info:
                    # Create a black background for better visibility on LED matrix
                    background = Image.new('RGB', icon_image.size, (0, 0, 0))
                    if icon_image.mode == 'RGBA':
                        background.paste(icon_image, mask=icon_image.split()[3])  # Use alpha channel as mask
                    else:
                        background.paste(icon_image, mask=icon_image.convert('RGBA').split()[3])
                    icon_image = background
                    print("Applied transparency with black background")
                
                # Resize to about 20x20 pixels to span both temperature lines
                resample_mode = getattr(Image, "Resampling", Image).LANCZOS
                icon_image = icon_image.resize((20, 20), resample=resample_mode)
                print(f"Resized icon to: {icon_image.size}")
                
                # Convert to RGB and enhance brightness for LED matrix
                self.weather_icon = icon_image.convert('RGB')
                
                # Enhance brightness since LED matrices can be dim
                pixels = self.weather_icon.load()
                for y in range(self.weather_icon.height):
                    for x in range(self.weather_icon.width):
                        r, g, b = pixels[x, y]
                        # Boost brightness but cap at 255
                        r = min(255, int(r * 1.5))
                        g = min(255, int(g * 1.5))
                        b = min(255, int(b * 1.5))
                        pixels[x, y] = (r, g, b)
                
                print(f"Enhanced brightness and converted to RGB: {self.weather_icon.size}, mode: {self.weather_icon.mode}")
                
                # Debug: Print some pixel values to see if image has content
                pixel_samples = []
                for y in range(0, 20, 5):
                    for x in range(0, 20, 5):
                        pixel = self.weather_icon.getpixel((x, y))
                        pixel_samples.append(f"({x},{y}):{pixel}")
                print(f"Sample pixels: {', '.join(pixel_samples[:6])}")
                
                self.last_icon_update = time.time()
                
                print(f"Successfully processed weather icon: {icon_code}")
                return self.weather_icon
            else:
                print(f"Icon download failed: {response.status_code}, response: {response.text[:100]}")
                return None
                
        except Exception as e:
            print(f"Icon download error: {e}")
            import traceback
            traceback.print_exc()
            return None

    def draw_weather_icon(self, icon_image, x, y):
        """Draw the weather icon image on the display"""
        if icon_image is not None:
            # Calculate position to center the 20x20 icon
            icon_x = max(0, x - 10)  # Center horizontally but don't go negative
            icon_y = max(0, y - 10)  # Center vertically but don't go negative
            
            # Make sure icon doesn't go off the right/bottom edge
            if icon_x + icon_image.width > 64:
                icon_x = 64 - icon_image.width
            if icon_y + icon_image.height > 32:
                icon_y = 32 - icon_image.height
            
            print(f"Drawing icon at position: icon_x={icon_x}, icon_y={icon_y}")
            print(f"Icon size: {icon_image.size}, mode: {icon_image.mode}")
            
            # Check if icon has any non-black pixels
            pixel_count = 0
            non_black_count = 0
            brightness_sum = 0
            
            for py in range(icon_image.height):
                for px in range(icon_image.width):
                    pixel = icon_image.getpixel((px, py))
                    pixel_count += 1
                    brightness = sum(pixel) / 3  # Average RGB
                    brightness_sum += brightness
                    if brightness > 10:  # Not nearly black
                        non_black_count += 1
            
            avg_brightness = brightness_sum / pixel_count if pixel_count > 0 else 0
            print(f"Icon analysis: {non_black_count}/{pixel_count} non-black pixels, avg brightness: {avg_brightness:.1f}")
            
            # Draw the icon
            self.canvas.SetImage(icon_image, icon_x, icon_y)
            print(f"Icon drawn successfully at ({icon_x},{icon_y})")
        else:
            print(f"No icon image provided, drawing fallback at x={x}, y={y}")
            # Fallback - simple colored square
            for dx in range(-8, 8):
                for dy in range(-8, 8):
                    if 0 <= x+dx < 64 and 0 <= y+dy < 32:
                        self.canvas.SetPixel(x+dx, y+dy, 200, 200, 255)

    def draw_weather(self):
        """Draw weather information like the reference image: time top, icon left, temps right"""
        weather = self.get_weather_data()
        
        if weather is None:
            # Show error message
            error_text = "No Weather Data"
            text_width = len(error_text) * 3
            x = (64 - text_width) // 2
            graphics.DrawText(self.canvas, self.small_font, x, 16, self.detail_color, error_text)
            return
        
        # Get current time in Mountain Time
        now = datetime.now(self.mountain_tz)
        
        # Time at top center in 12-hour format like "5:27 PM"
        time_str = now.strftime("%I:%M %p")
        if time_str.startswith("0"):
            time_str = time_str[1:]  # Remove leading zero
        
        # Calculate width for centering
        time_width = 0
        for char in time_str:
            if char == ' ':
                time_width += 2
            else:
                time_width += self.small_font.CharacterWidth(ord(char))
        
        time_x = (64 - time_width) // 2
        time_y = 8
        
        # Draw time with tight spacing
        current_x = time_x
        for char in time_str:
            if char == ' ':
                current_x += 2
            else:
                char_width = graphics.DrawText(self.canvas, self.small_font, current_x, time_y, self.temp_color, char)
                current_x += char_width - 1
        
        # Extract weather info
        temp = int(weather['main']['temp'])
        temp_low = int(weather['main']['temp_min'])
        condition = weather['weather'][0]['main']
        icon_code = weather['weather'][0]['icon']
        
        # Calculate temperature text widths for layout
        temp_str = f"{temp}°"
        temp_width = 0
        for char in temp_str:
            temp_width += self.small_font.CharacterWidth(ord(char))
            
        low_str = f"{temp_low}°"
        low_width = 0
        for char in low_str:
            low_width += self.small_font.CharacterWidth(ord(char))
        
        # Calculate total group width: icon(20) + spacing(1) + max_temp_width
        max_temp_width = max(temp_width, low_width)
        total_group_width = 20 + 1 + max_temp_width
        
        # Center the entire group horizontally
        group_start_x = (64 - total_group_width) // 2
        
        # Position icon on the left of the group, centered vertically in bottom area
        icon_x = group_start_x + 10  # Center of the 20px icon
        icon_y = 22  # Center vertically in lower area
        
        print(f"Weather icon code: {icon_code}")
        print(f"Group layout: total_width={total_group_width}, start_x={group_start_x}")
        icon_image = self.get_weather_icon(icon_code)
        print(f"Got icon image: {icon_image is not None}")
        self.draw_weather_icon(icon_image, icon_x, icon_y)
        
        # Position temperatures to the right of icon with 1 pixel spacing
        temp_start_x = group_start_x + 20 + 1  # After icon + 1 pixel spacing
        
        # High temperature (white) - center it within the temp area
        temp_x = temp_start_x + (max_temp_width - temp_width) // 2
        temp_y = 18
        
        # Draw high temp with tight spacing (white)
        current_x = temp_x
        for char in temp_str:
            char_width = graphics.DrawText(self.canvas, self.small_font, current_x, temp_y, self.temp_color, char)
            current_x += char_width - 1
        
        # Low temperature (blue) - center it within the temp area
        low_x = temp_start_x + (max_temp_width - low_width) // 2
        low_y = 28
        
        # Draw low temp with tight spacing (blue)
        current_x = low_x
        for char in low_str:
            char_width = graphics.DrawText(self.canvas, self.small_font, current_x, low_y, self.detail_color, char)
            current_x += char_width - 1

    def run(self):
        print("Starting Denver weather display. Press CTRL-C to stop.")
        print("Note: Set OPENWEATHER_API_KEY environment variable!")
        
        try:
            while True:
                self.canvas.Clear()
                
                if not self.api_key:
                    # Show instructions to set environment variable
                    line1 = "Set API Key"
                    line2 = "OPENWEATHER_API_KEY"
                    line3 = "in .env file"
                    
                    graphics.DrawText(self.canvas, self.small_font, 2, 8, self.condition_color, line1)
                    graphics.DrawText(self.canvas, self.small_font, 2, 18, self.detail_color, line2)
                    graphics.DrawText(self.canvas, self.small_font, 2, 28, self.detail_color, line3)
                else:
                    self.draw_weather()
                
                # Swap buffers
                self.canvas = self.matrix.SwapOnVSync(self.canvas)
                
                # Update every 30 seconds
                time.sleep(30)
                
        except KeyboardInterrupt:
            print("\nWeather display stopped.")
        finally:
            self.matrix.Clear()

if __name__ == "__main__":
    weather = WeatherDisplay()
    weather.run()