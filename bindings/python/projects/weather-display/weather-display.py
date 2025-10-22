#!/usr/bin/env python3
# Weather display for RGB LED matrix - Denver, CO
# Shows current weather conditions and temperature

import time
import sys
import os
import requests
import argparse
from datetime import datetime, timezone, timedelta

# Try to import pytz for proper timezone handling
try:
    import pytz
    HAS_PYTZ = True
except ImportError:
    HAS_PYTZ = False
    print("pytz not available. Install with: pip install pytz")

# Load environment variables
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    print("python-dotenv not installed. Using system environment variables only.")

# Add the parent directory to Python path to import the rgbmatrix module
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/../..'))
from rgbmatrix import graphics
from PIL import Image

# Import shared components
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette
class WeatherDisplay(MatrixBase):
    def __init__(self, city="Denver, CO"):
        # Initialize base matrix with standard configuration
        super().__init__(
            rows=32,
            cols=64,
            hardware_mapping='adafruit-hat-pwm'
        )
        
        # Initialize shared components
        self.font_manager = FontManager()
        self.color_palette = ColorPalette()
        
        # Set up location
        self.city = city
        self.city_display_name = city  # Will be updated with proper formatting after geocoding
        self.coords = None  # Will be set by geocoding
        self.timezone = None  # Will be set by timezone lookup
        
        # Load fonts
        self.temp_font = self.font_manager.get_font('9x18B')  # Bold font for temperature
        self.condition_font = self.font_manager.get_font('6x13B')  # Bold font for conditions
        self.medium_font = self.font_manager.get_font('6x13')  # Regular medium font for time
        self.small_font = self.font_manager.get_font('5x7')   # Small font for details
        self.tiny_font = self.font_manager.get_font('4x6')    # Tiny font for city name
        
        # Colors
        self.temp_color = self.color_palette.get_color('white')          # White for temperature
        self.condition_color = self.color_palette.get_color((100, 200, 255))  # Light blue for conditions
        self.detail_color = self.color_palette.get_color('gray_light')   # Gray for details
        
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
                    
                    # Create display name (City, State/Country)
                    self.city_display_name = data[0]['name']
                    if state and country in ('US', 'USA'):  # For US cities, show state
                        self.city_display_name += f", {state}"
                    elif country:  # For other countries, show country
                        self.city_display_name += f", {country}"
                    
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
        """Get accurate timezone for coordinates using API or pytz library"""
        try:
            if not self.coords:
                print("No coordinates available. Using Mountain Time.")
                self.timezone = timezone(timedelta(hours=-7))
                return
            
            lat = self.coords["lat"]
            lng = self.coords["lon"]
            
            # Method 1: Try timezone API (free tier available)
            try:
                tz_url = f"http://worldtimeapi.org/api/timezone"
                # First get all timezones and find the closest one
                response = requests.get(tz_url, timeout=5)
                if response.status_code == 200:
                    # Use coordinate-based lookup
                    coord_url = f"http://worldtimeapi.org/api/ip"  # This gets current timezone
                    # Better: use a proper coordinate-based service
                    
                    # Try TimeZoneDB API as backup (requires free API key)
                    timezonedb_key = os.getenv('TIMEZONEDB_API_KEY')
                    if timezonedb_key:
                        tz_api_url = f"http://api.timezonedb.com/v2.1/get-time-zone"
                        params = {
                            'key': timezonedb_key,
                            'format': 'json',
                            'by': 'position',
                            'lat': lat,
                            'lng': lng
                        }
                        tz_response = requests.get(tz_api_url, params=params, timeout=5)
                        if tz_response.status_code == 200:
                            tz_data = tz_response.json()
                            if tz_data.get('status') == 'OK':
                                offset_seconds = tz_data['gmtOffset']
                                offset_hours = offset_seconds / 3600
                                self.timezone = timezone(timedelta(seconds=offset_seconds))
                                print(f"TimeZoneDB timezone: UTC{offset_hours:+.1f} ({tz_data.get('zoneName', 'Unknown')})")
                                return
            except Exception as e:
                print(f"Timezone API error: {e}")
            
            # Method 2: Use pytz with coordinate-based estimation
            if HAS_PYTZ:
                try:
                    # Use a simple coordinate-to-timezone mapping for major regions
                    timezone_name = None
                    
                    # North America timezone mapping
                    if -125 <= lng <= -60 and 25 <= lat <= 70:
                        if lng >= -75:  # Eastern
                            timezone_name = 'US/Eastern'
                        elif lng >= -90:  # Central
                            timezone_name = 'US/Central'
                        elif lng >= -105:  # Mountain
                            timezone_name = 'US/Mountain'
                        else:  # Pacific
                            timezone_name = 'US/Pacific'
                    
                    # Europe timezone mapping
                    elif -10 <= lng <= 40 and 35 <= lat <= 70:
                        if lng <= 15:
                            timezone_name = 'Europe/Berlin'  # CET
                        else:
                            timezone_name = 'Europe/Athens'  # EET
                    
                    # Asia timezone mapping
                    elif 100 <= lng <= 150 and 20 <= lat <= 50:
                        if lng <= 120:
                            timezone_name = 'Asia/Shanghai'  # China
                        else:
                            timezone_name = 'Asia/Tokyo'    # Japan
                    
                    if timezone_name:
                        tz = pytz.timezone(timezone_name)
                        # Get current time in that timezone
                        local_time = datetime.now(tz)
                        self.timezone = local_time.tzinfo
                        print(f"pytz timezone: {timezone_name} (UTC{local_time.strftime('%z')})")
                        return
                        
                except Exception as e:
                    print(f"pytz error: {e}")
            
            # Method 3: Fallback to improved manual calculation
            print("Using fallback timezone calculation")
            if -125 <= lng <= -60:  # North America
                if lng >= -75:
                    offset = -5  # Eastern
                elif lng >= -90:
                    offset = -6  # Central  
                elif lng >= -105:
                    offset = -7  # Mountain
                else:
                    offset = -8  # Pacific
            elif -10 <= lng <= 40:  # Europe
                offset = 1 if lng <= 15 else 2
            elif 100 <= lng <= 150:  # Asia
                offset = 8 if lng <= 120 else 9
            else:
                offset = round(lng / 15)
            
            self.timezone = timezone(timedelta(hours=offset))
            print(f"Fallback timezone: UTC{offset:+d}")
            
        except Exception as e:
            print(f"Timezone error: {e}. Using Mountain Time.")
            self.timezone = timezone(timedelta(hours=-7))
    
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
        """Load weather icon from local files based on icon code"""
        try:
            # Try to load local weather icon file
            icon_path = f"weather_icons/{icon_code}.png"
            
            # Check if local icon file exists
            if os.path.exists(icon_path):
                print(f"Loading local weather icon: {icon_path}")
                icon_image = Image.open(icon_path)
                
                # Resize to appropriate size for LED matrix (24x24 - optimized size)
                icon_image = icon_image.resize((24, 24), Image.Resampling.LANCZOS)
                
                # Handle transparency - create black background for LED matrix
                if icon_image.mode in ('RGBA', 'LA') or 'transparency' in icon_image.info:
                    background = Image.new('RGB', icon_image.size, (0, 0, 0))
                    if icon_image.mode == 'RGBA':
                        background.paste(icon_image, mask=icon_image.split()[3])
                    else:
                        background.paste(icon_image, mask=icon_image.convert('RGBA').split()[3])
                    icon_image = background
                
                # Convert to RGB and enhance brightness for LED matrix
                icon_image = icon_image.convert('RGB')
                
                # Enhanced brightness processing with dim pixel cleanup for LED matrix
                pixels = icon_image.load()
                for y in range(icon_image.height):
                    for x in range(icon_image.width):
                        r, g, b = pixels[x, y]
                        
                        # Calculate brightness to identify dim pixels
                        brightness = (0.299 * r + 0.587 * g + 0.114 * b)
                        
                        # Remove only very dim pixels that create artifacts around icons
                        if brightness < 10:
                            pixels[x, y] = (0, 0, 0)
                        else:
                            # Apply light enhancement to visible pixels
                            r = min(255, int(r * 1.2))
                            g = min(255, int(g * 1.2))
                            b = min(255, int(b * 1.2))
                            pixels[x, y] = (r, g, b)
                
                print(f"Successfully loaded weather icon: {icon_code}")
                return icon_image
            else:
                print(f"Local icon file not found: {icon_path}")
                return None
                
        except Exception as e:
            print(f"Error loading weather icon: {e}")
            return None

    def draw_weather_icon(self, icon_image, x, y):
        """Draw weather icon image on the display"""
        if icon_image is not None:
            # Calculate position to center the icon
            icon_x = max(0, x - icon_image.width // 2)
            icon_y = max(0, y - icon_image.height // 2)
            
            # Make sure icon doesn't go off the edges
            if icon_x + icon_image.width > 64:
                icon_x = 64 - icon_image.width
            if icon_y + icon_image.height > 32:
                icon_y = 32 - icon_image.height
            
            # Draw the icon
            self.canvas.SetImage(icon_image, icon_x, icon_y)
            print(f"Weather icon drawn at ({icon_x},{icon_y}), size: {icon_image.size}")
        else:
            print(f"No icon image provided, skipping icon display")

    def draw_weather(self):
        """Draw weather information like the reference image: time top, icon left, temps right"""
        weather = self.get_weather_data()
        
        if weather is None:
            # Show error message
            error_text = "No Weather Data"
            text_width = len(error_text) * 3
            x = (64 - text_width) // 2
            self.draw_text(error_text, x, 16, self.detail_color, font=self.small_font)
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
        time_y = 7  # Positioned to slightly overlay the top of the icon
        
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
        
        # Calculate total group width: icon(26) + spacing(2) + max_temp_width (updated for 24x24 icons)
        max_temp_width = max(temp_width, low_width)
        total_group_width = 26 + 2 + max_temp_width
        
        # Center the entire group horizontally
        group_start_x = (64 - total_group_width) // 2
        
        # Position icon on the left of the group, centered vertically in middle area
        icon_x = group_start_x + 13  # Center of the 26px icon area (for 24x24 icons)
        icon_y = 16  # Positioned so time can overlay the top edge
        
        print(f"Weather icon code: {icon_code}")
        print(f"Group layout: total_width={total_group_width}, start_x={group_start_x}")
        icon_image = self.get_weather_icon(icon_code)
        self.draw_weather_icon(icon_image, icon_x, icon_y)
        
        # Position temperatures to the right of icon
        temp_start_x = group_start_x + 26 + 2  # After icon area + spacing (updated for 26px icon area)
        
        # High temperature (white) - center it within the temp area
        temp_x = temp_start_x + (max_temp_width - temp_width) // 2
        temp_y = 16  # Moved up 2 pixels
        
        # Draw high temp with 1 pixel spacing (white)
        current_x = temp_x
        for char in temp_str:
            char_width = self.draw_text(self.small_font, current_x, temp_y, self.temp_color, char)
            current_x += char_width + 1  # Add 1 pixel spacing between characters
        
        # Low temperature (blue) - center it within the temp area
        low_x = temp_start_x + (max_temp_width - low_width) // 2
        low_y = 23  # Moved up 2 pixels
        
        # Draw low temp with 1 pixel spacing (blue)
        current_x = low_x
        for char in low_str:
            char_width = self.draw_text(self.small_font, current_x, low_y, self.detail_color, char)
            current_x += char_width + 1  # Add 1 pixel spacing between characters
        
        # City name at bottom center in tiny font
        city_width = 0
        for char in self.city_display_name:
            city_width += self.tiny_font.CharacterWidth(ord(char))
            # No extra spacing between characters for compact display
        
        city_x = (64 - city_width) // 2
        city_y = 31  # At bottom of 32-pixel display (font baseline)
        
        # Draw city name with no extra spacing (gray)
        current_x = city_x
        for char in self.city_display_name:
            char_width = self.draw_text(self.tiny_font, current_x, city_y, self.detail_color, char)
            current_x += char_width  # No extra spacing between characters
        
        # Draw time LAST so it appears on top of the icon
        current_x = time_x
        for char in time_str:
            if char == ' ':
                current_x += 4  # Larger space between time and AM/PM
            else:
                char_width = self.draw_text(self.small_font, current_x, time_y, self.temp_color, char)
                current_x += char_width + 1  # Add 1 pixel spacing after each character

    def run(self):
        print(f"Starting {self.city} weather display. Press CTRL-C to stop.")
        print("Note: Set OPENWEATHER_API_KEY environment variable!")
        
        try:
            while True:
                self.clear()
                
                if not self.api_key:
                    # Show instructions to set environment variable
                    line1 = "Set API Key"
                    line2 = "OPENWEATHER_API_KEY"
                    line3 = "in .env file"
                    
                    self.draw_text(self.small_font, 2, 8, self.condition_color, line1)
                    self.draw_text(self.small_font, 2, 18, self.detail_color, line2)
                    self.draw_text(self.small_font, 2, 28, self.detail_color, line3)
                else:
                    self.draw_weather()
                
                # Swap buffers
                self.swap_canvas()
                
                # Update every 30 seconds
                time.sleep(30)
                
        except KeyboardInterrupt:
            print("\nWeather display stopped.")
        finally:
            self.clear()

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