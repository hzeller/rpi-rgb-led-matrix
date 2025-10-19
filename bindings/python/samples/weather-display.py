#!/usr/bin/env python3
# Weather display for RGB LED matrix - Denver, CO
# Shows current weather conditions and temperature

import time
import sys
import os
import requests
import json
import argparse
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
    def __init__(self, city="Denver, CO"):
        # Configuration for the matrix - use same settings as other displays
        options = RGBMatrixOptions()
        options.rows = 32
        options.cols = 64
        options.chain_length = 1
        options.parallel = 1
        options.hardware_mapping = 'adafruit-hat-pwm'
        
        self.matrix = RGBMatrix(options=options)
        self.canvas = self.matrix.CreateFrameCanvas()
        
        # Set up location
        self.city = city
        self.coords = None  # Will be set by geocoding
        self.timezone = None  # Will be set by timezone lookup
        
        # Load fonts
        self.temp_font = graphics.Font()
        self.temp_font.LoadFont("../../../fonts/9x18B.bdf")  # Bold font for temperature
        
        self.condition_font = graphics.Font()
        self.condition_font.LoadFont("../../../fonts/6x13B.bdf")  # Bold font for conditions
        
        self.medium_font = graphics.Font()
        self.medium_font.LoadFont("../../../fonts/6x13.bdf")  # Regular (non-bold) medium font for time
        
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
        
        # Get coordinates for the city
        self.get_city_coordinates()
        
        # Get timezone for the coordinates
        self.get_timezone()
        
    def get_city_coordinates(self):
        """Get latitude and longitude for a city using OpenWeatherMap Geocoding API"""
        try:
            if not self.api_key:
                print("Warning: No API key found. Using default Denver coordinates.")
                self.coords = {"lat": 39.7392, "lon": -104.9903}
                return
                
            url = "http://api.openweathermap.org/geo/1.0/direct"
            params = {
                "q": self.city,
                "limit": 1,
                "appid": self.api_key
            }
            
            response = requests.get(url, params=params, timeout=10)
            if response.status_code == 200:
                data = response.json()
                if data:
                    self.coords = {"lat": data[0]["lat"], "lon": data[0]["lon"]}
                    country = data[0].get("country", "")
                    state = data[0].get("state", "")
                    location_info = f"{data[0]['name']}"
                    if state:
                        location_info += f", {state}"
                    if country:
                        location_info += f", {country}"
                    print(f"Found coordinates for {location_info}: {self.coords['lat']:.4f}, {self.coords['lon']:.4f}")
                else:
                    print(f"City '{self.city}' not found. Using default Denver coordinates.")
                    self.coords = {"lat": 39.7392, "lon": -104.9903}
            else:
                print(f"Geocoding API error: {response.status_code}. Using default Denver coordinates.")
                self.coords = {"lat": 39.7392, "lon": -104.9903}
                
        except Exception as e:
            print(f"Geocoding error: {e}. Using default Denver coordinates.")
            self.coords = {"lat": 39.7392, "lon": -104.9903}
    
    def get_timezone(self):
        """Get timezone for coordinates using TimeZoneDB API or fallback to simple estimation"""
        try:
            if not self.coords:
                print("No coordinates available. Using Mountain Time.")
                self.timezone = timezone(timedelta(hours=-6))
                return
            
            # Simple timezone estimation based on longitude
            # This is a basic approximation - for more accuracy, you'd use a timezone API
            lng = self.coords["lon"]
            
            # Very rough timezone estimation (UTC offset = longitude / 15)
            estimated_offset = round(lng / 15)
            
            # Apply some common timezone corrections for major regions
            if -125 <= lng <= -60:  # North America
                if lng >= -75:  # Eastern time zone area
                    estimated_offset = -5
                elif lng >= -90:  # Central time zone area
                    estimated_offset = -6
                elif lng >= -105:  # Mountain time zone area
                    estimated_offset = -7
                else:  # Pacific time zone area
                    estimated_offset = -8
            elif -10 <= lng <= 40:  # Europe/Africa
                if lng <= 15:
                    estimated_offset = 1  # Central European Time
                else:
                    estimated_offset = 2  # Eastern European Time
            elif lng >= 100 and lng <= 150:  # Asia-Pacific
                if lng <= 120:
                    estimated_offset = 8  # China Standard Time
                else:
                    estimated_offset = 9  # Japan Standard Time
            
            self.timezone = timezone(timedelta(hours=estimated_offset))
            print(f"Estimated timezone: UTC{estimated_offset:+d}")
            
        except Exception as e:
            print(f"Timezone estimation error: {e}. Using Mountain Time.")
            self.timezone = timezone(timedelta(hours=-6))
        
    def get_weather_data(self):
        """Fetch weather data from OpenWeatherMap API"""
        try:
            # Only update every 10 minutes to avoid API limits
            if time.time() - self.last_update < 600:
                return self.weather_data
                
            url = f"https://api.openweathermap.org/data/2.5/weather"
            params = {
                "lat": self.coords["lat"],
                "lon": self.coords["lon"],
                "appid": self.api_key,
                "units": "imperial"  # Fahrenheit
            }
            
            response = requests.get(url, params=params, timeout=10)
            if response.status_code == 200:
                self.weather_data = response.json()
                self.last_update = time.time()
                print(f"Weather updated for {self.city}: {self.weather_data['weather'][0]['description']}, {self.weather_data['main']['temp']}°F")
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
        
        # Get current time in specified timezone
        now = datetime.now(self.timezone)
        
        # Time at top center in 12-hour format like "5:27 PM" - using small font with proper spacing
        time_str = now.strftime("%I:%M %p")
        if time_str.startswith("0"):
            time_str = time_str[1:]  # Remove leading zero
        
        # Calculate width for perfect centering using small font with normal spacing
        time_width = 0
        for char in time_str:
            if char == ' ':
                time_width += 4  # Larger space between time and AM/PM
            else:
                time_width += self.small_font.CharacterWidth(ord(char))
                time_width += 1  # Add 1 pixel spacing between all characters
        time_width -= 1  # Remove trailing spacing
        
        time_x = (64 - time_width) // 2
        time_y = 8  # Back to small font position
        
        # Draw time with 1 pixel spacing between characters using small font
        current_x = time_x
        for char in time_str:
            if char == ' ':
                current_x += 4  # Larger space between time and AM/PM
            else:
                char_width = graphics.DrawText(self.canvas, self.small_font, current_x, time_y, self.temp_color, char)
                current_x += char_width + 1  # Add 1 pixel spacing after each character
        
        # Extract weather info
        temp = int(weather['main']['temp'])
        temp_low = int(weather['main']['temp_min'])
        condition = weather['weather'][0]['main']
        icon_code = weather['weather'][0]['icon']  # Back to real weather data
        
        # Calculate temperature text widths for layout with 1 pixel spacing
        temp_str = f"{temp}°"
        temp_width = 0
        for char in temp_str:
            temp_width += self.small_font.CharacterWidth(ord(char))
            temp_width += 1  # Add 1 pixel spacing between characters
        temp_width -= 1  # Remove trailing spacing
            
        low_str = f"{temp_low}°"
        low_width = 0
        for char in low_str:
            low_width += self.small_font.CharacterWidth(ord(char))
            low_width += 1  # Add 1 pixel spacing between characters
        low_width -= 1  # Remove trailing spacing
        
        # Calculate total group width: icon(20) + spacing(-1) + max_temp_width
        max_temp_width = max(temp_width, low_width)
        total_group_width = 20 + (-1) + max_temp_width  # Reduced spacing by 2 pixels (from +1 to -1)
        
        # Center the entire group horizontally
        group_start_x = (64 - total_group_width) // 2
        
        # Position icon on the left of the group, centered vertically in bottom area
        icon_x = group_start_x + 10  # Center of the 20px icon
        icon_y = 22  # Moved back up 2 pixels from 24 to 22
        
        print(f"Weather icon code: {icon_code}")
        print(f"Group layout: total_width={total_group_width}, start_x={group_start_x}")
        icon_image = self.get_weather_icon(icon_code)
        print(f"Got icon image: {icon_image is not None}")
        self.draw_weather_icon(icon_image, icon_x, icon_y)
        
        # Position temperatures to the right of icon with -1 pixel spacing (overlapping by 1 pixel)
        temp_start_x = group_start_x + 20 + (-1)  # After icon - 1 pixel spacing (2 pixels closer)
        
        # High temperature (white) - center it within the temp area
        temp_x = temp_start_x + (max_temp_width - temp_width) // 2
        temp_y = 21  # Moved back up 2 pixels from 23 to 21
        
        # Draw high temp with 1 pixel spacing (white)
        current_x = temp_x
        for char in temp_str:
            char_width = graphics.DrawText(self.canvas, self.small_font, current_x, temp_y, self.temp_color, char)
            current_x += char_width + 1  # Add 1 pixel spacing between characters
        
        # Low temperature (blue) - center it within the temp area
        low_x = temp_start_x + (max_temp_width - low_width) // 2
        low_y = 28  # Moved back up 2 pixels from 30 to 28
        
        # Draw low temp with 1 pixel spacing (blue)
        current_x = low_x
        for char in low_str:
            char_width = graphics.DrawText(self.canvas, self.small_font, current_x, low_y, self.detail_color, char)
            current_x += char_width + 1  # Add 1 pixel spacing between characters

    def run(self):
        print(f"Starting {self.city} weather display. Press CTRL-C to stop.")
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

def parse_arguments():
    """Parse command line arguments for city"""
    parser = argparse.ArgumentParser(description='Weather display for RGB LED matrix')
    parser.add_argument('--city', '-c', default='Denver, CO', 
                        help='City name (e.g., "New York", "London", "Tokyo") (default: Denver, CO)')
    
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_arguments()
    
    print(f"Weather Display Configuration:")
    print(f"  City: {args.city}")
    print()
    
    weather = WeatherDisplay(city=args.city)
    weather.run()