#!/usr/bin/env python3
# Weather display for RGB LED matrix - Denver, CO
# Shows current weather conditions and temperature

import time
import sys
import os
import requests
import json
import argparse
import math
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
        
        # Custom weather icon mapping - maps OpenWeatherMap icon codes to our custom drawing functions
        self.icon_mapping = {
            # Clear sky
            '01d': 'clear_day',        # clear sky day
            '01n': 'clear_night',      # clear sky night
            
            # Few clouds
            '02d': 'partly_cloudy_day',   # few clouds day
            '02n': 'partly_cloudy_night', # few clouds night
            
            # Scattered clouds
            '03d': 'scattered_clouds',    # scattered clouds day
            '03n': 'scattered_clouds',    # scattered clouds night
            
            # Broken clouds
            '04d': 'broken_clouds',       # broken clouds day
            '04n': 'broken_clouds',       # broken clouds night
            
            # Shower rain
            '09d': 'shower_rain',         # shower rain day
            '09n': 'shower_rain',         # shower rain night
            
            # Rain
            '10d': 'rain_day',            # rain day
            '10n': 'rain_night',          # rain night
            
            # Thunderstorm
            '11d': 'thunderstorm',        # thunderstorm day
            '11n': 'thunderstorm',        # thunderstorm night
            
            # Snow
            '13d': 'snow',                # snow day
            '13n': 'snow',                # snow night
            
            # Mist/fog
            '50d': 'mist',                # mist day
            '50n': 'mist',                # mist night
        }
        
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
    
    def draw_clear_day(self, x, y, current_time=None):
        """Draw a bright sun icon with subtle pulsing effect"""
        # Sun center (yellow/orange) with brightness pulse
        base_brightness = 255
        pulse_brightness = 200
        if current_time:
            # Gentle pulse every 4 seconds
            pulse = (math.sin(current_time * 0.5) + 1) / 2  # 0 to 1
            brightness = int(pulse_brightness + (base_brightness - pulse_brightness) * pulse)
        else:
            brightness = base_brightness
            
        sun_color = graphics.Color(brightness, brightness, 0)  # Pulsing yellow
        ray_color = graphics.Color(brightness - 55, brightness - 55, 0)  # Dimmer rays
        
        # Draw sun center (3x3 square)
        for dx in range(-1, 2):
            for dy in range(-1, 2):
                self.canvas.SetPixel(x + dx, y + dy, sun_color.red, sun_color.green, sun_color.blue)
        
        # Draw sun rays (8 directions)
        rays = [(-3, 0), (3, 0), (0, -3), (0, 3), (-2, -2), (2, -2), (-2, 2), (2, 2)]
        for rx, ry in rays:
            self.canvas.SetPixel(x + rx, y + ry, ray_color.red, ray_color.green, ray_color.blue)
    
    def draw_clear_night(self, x, y, current_time=None):
        """Draw a crescent moon with gentle glow effect"""
        # Moon with subtle glow variation
        base_brightness = 220
        if current_time:
            # Very gentle glow every 6 seconds
            glow = (math.sin(current_time * 0.33) + 1) / 2  # 0 to 1
            brightness = int(base_brightness - 30 + 30 * glow)
        else:
            brightness = base_brightness
            
        moon_color = graphics.Color(brightness, brightness, 255)  # Pale blue-white with glow
        dark_color = graphics.Color(30, 30, 50)     # Dark blue
        
        # Draw full moon shape
        for dx in range(-2, 3):
            for dy in range(-2, 3):
                if dx*dx + dy*dy <= 4:  # Circle shape
                    self.canvas.SetPixel(x + dx, y + dy, moon_color.red, moon_color.green, moon_color.blue)
        
        # Draw dark overlay for crescent
        for dx in range(-1, 3):
            for dy in range(-2, 3):
                if (dx-1)*(dx-1) + dy*dy <= 3:  # Offset circle
                    self.canvas.SetPixel(x + dx, y + dy, dark_color.red, dark_color.green, dark_color.blue)
    
    def draw_partly_cloudy_day(self, x, y):
        """Draw sun partially covered by cloud"""
        # Sun (smaller, upper left)
        sun_color = graphics.Color(255, 255, 0)
        self.canvas.SetPixel(x - 2, y - 2, sun_color.red, sun_color.green, sun_color.blue)
        self.canvas.SetPixel(x - 1, y - 2, sun_color.red, sun_color.green, sun_color.blue)
        self.canvas.SetPixel(x - 2, y - 1, sun_color.red, sun_color.green, sun_color.blue)
        
        # Cloud (white/gray, lower right)
        cloud_color = graphics.Color(200, 200, 255)
        cloud_shadow = graphics.Color(150, 150, 200)
        
        # Cloud shape
        cloud_pixels = [(0, 0), (1, 0), (2, 0), (-1, 1), (0, 1), (1, 1), (2, 1), (0, 2), (1, 2)]
        for dx, dy in cloud_pixels:
            color = cloud_shadow if dy == 2 else cloud_color
            self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_partly_cloudy_night(self, x, y):
        """Draw moon partially covered by cloud"""
        # Moon (smaller, upper left)
        moon_color = graphics.Color(200, 200, 255)
        self.canvas.SetPixel(x - 2, y - 2, moon_color.red, moon_color.green, moon_color.blue)
        self.canvas.SetPixel(x - 1, y - 2, moon_color.red, moon_color.green, moon_color.blue)
        self.canvas.SetPixel(x - 2, y - 1, moon_color.red, moon_color.green, moon_color.blue)
        
        # Cloud (darker for night)
        cloud_color = graphics.Color(120, 120, 180)
        cloud_shadow = graphics.Color(80, 80, 140)
        
        cloud_pixels = [(0, 0), (1, 0), (2, 0), (-1, 1), (0, 1), (1, 1), (2, 1), (0, 2), (1, 2)]
        for dx, dy in cloud_pixels:
            color = cloud_shadow if dy == 2 else cloud_color
            self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_scattered_clouds(self, x, y):
        """Draw scattered cloud formation"""
        cloud_color = graphics.Color(220, 220, 255)
        cloud_shadow = graphics.Color(180, 180, 220)
        
        # Multiple small clouds
        # Cloud 1 (upper left)
        cloud1_pixels = [(-2, -2), (-1, -2), (-2, -1), (-1, -1)]
        for dx, dy in cloud1_pixels:
            self.canvas.SetPixel(x + dx, y + dy, cloud_color.red, cloud_color.green, cloud_color.blue)
        
        # Cloud 2 (center right)
        cloud2_pixels = [(1, -1), (2, -1), (1, 0), (2, 0), (3, 0)]
        for dx, dy in cloud2_pixels:
            color = cloud_shadow if dx == 3 else cloud_color
            self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Cloud 3 (lower center)
        cloud3_pixels = [(-1, 1), (0, 1), (1, 1), (0, 2)]
        for dx, dy in cloud3_pixels:
            color = cloud_shadow if dy == 2 else cloud_color
            self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_broken_clouds(self, x, y):
        """Draw heavy cloud cover"""
        cloud_color = graphics.Color(180, 180, 220)
        cloud_dark = graphics.Color(140, 140, 180)
        
        # Large cloud formation
        cloud_pixels = [
            (-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2),
            (-3, -1), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1),
            (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0),
            (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1),
            (-1, 2), (0, 2), (1, 2)
        ]
        
        for dx, dy in cloud_pixels:
            # Add some variation in color for depth
            color = cloud_dark if (dx + dy) % 3 == 0 else cloud_color
            self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_shower_rain(self, x, y, current_time=None):
        """Draw cloud with scattered rain drops that animate"""
        # Cloud
        cloud_color = graphics.Color(120, 120, 160)
        cloud_pixels = [(-2, -2), (-1, -2), (0, -2), (1, -2), (-1, -1), (0, -1), (1, -1), (2, -1)]
        for dx, dy in cloud_pixels:
            self.canvas.SetPixel(x + dx, y + dy, cloud_color.red, cloud_color.green, cloud_color.blue)
        
        # Animated rain drops (blue) - different drops appear at different times
        rain_color = graphics.Color(0, 150, 255)
        if current_time:
            # Rain drops cycle every 3 seconds
            rain_cycle = (current_time * 1.0) % 3.0
            if rain_cycle < 1.0:  # First second - left drops
                rain_drops = [(-2, 1), (-1, 3)]
            elif rain_cycle < 2.0:  # Second second - center drops
                rain_drops = [(0, 0), (1, 1)]
            else:  # Third second - right drops
                rain_drops = [(2, 2), (1, 3)]
        else:
            rain_drops = [(-2, 1), (0, 0), (2, 2), (-1, 3), (1, 1)]
            
        for dx, dy in rain_drops:
            if 0 <= y + dy < 32:  # Make sure we don't go off screen
                self.canvas.SetPixel(x + dx, y + dy, rain_color.red, rain_color.green, rain_color.blue)
    
    def draw_rain_day(self, x, y, current_time=None):
        """Draw cloud with steady rain that shifts"""
        # Dark cloud
        cloud_color = graphics.Color(100, 100, 140)
        cloud_pixels = [(-2, -2), (-1, -2), (0, -2), (1, -2), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1)]
        for dx, dy in cloud_pixels:
            self.canvas.SetPixel(x + dx, y + dy, cloud_color.red, cloud_color.green, cloud_color.blue)
        
        # Animated steady rain lines that shift position
        rain_color = graphics.Color(0, 120, 255)
        if current_time:
            # Rain shifts every 2 seconds
            shift = int(current_time * 0.5) % 2
            if shift == 0:
                rain_lines = [(-2, 0), (-2, 1), (-1, 1), (-1, 2), (0, 0), (0, 1), (1, 1), (1, 2), (2, 0), (2, 1)]
            else:
                rain_lines = [(-1, 0), (-1, 1), (0, 1), (0, 2), (1, 0), (1, 1), (2, 1), (2, 2)]
        else:
            rain_lines = [(-2, 0), (-2, 1), (-1, 1), (-1, 2), (0, 0), (0, 1), (1, 1), (1, 2), (2, 0), (2, 1)]
            
        for dx, dy in rain_lines:
            if 0 <= y + dy < 32:
                self.canvas.SetPixel(x + dx, y + dy, rain_color.red, rain_color.green, rain_color.blue)
    
    def draw_rain_night(self, x, y, current_time=None):
        """Draw cloud with steady rain (darker for night) that shifts"""
        # Very dark cloud
        cloud_color = graphics.Color(60, 60, 100)
        cloud_pixels = [(-2, -2), (-1, -2), (0, -2), (1, -2), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1)]
        for dx, dy in cloud_pixels:
            self.canvas.SetPixel(x + dx, y + dy, cloud_color.red, cloud_color.green, cloud_color.blue)
        
        # Rain (darker blue for night) with shifting animation
        rain_color = graphics.Color(0, 80, 180)
        if current_time:
            # Rain shifts every 2 seconds
            shift = int(current_time * 0.5) % 2
            if shift == 0:
                rain_lines = [(-2, 0), (-2, 1), (-1, 1), (-1, 2), (0, 0), (0, 1), (1, 1), (1, 2), (2, 0), (2, 1)]
            else:
                rain_lines = [(-1, 0), (-1, 1), (0, 1), (0, 2), (1, 0), (1, 1), (2, 1), (2, 2)]
        else:
            rain_lines = [(-2, 0), (-2, 1), (-1, 1), (-1, 2), (0, 0), (0, 1), (1, 1), (1, 2), (2, 0), (2, 1)]
            
        for dx, dy in rain_lines:
            if 0 <= y + dy < 32:
                self.canvas.SetPixel(x + dx, y + dy, rain_color.red, rain_color.green, rain_color.blue)
    
    def draw_thunderstorm(self, x, y, current_time=None):
        """Draw cloud with lightning bolt that flashes"""
        # Dark storm cloud
        cloud_color = graphics.Color(60, 60, 80)
        cloud_pixels = [(-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1)]
        for dx, dy in cloud_pixels:
            self.canvas.SetPixel(x + dx, y + dy, cloud_color.red, cloud_color.green, cloud_color.blue)
        
        # Flashing lightning bolt (bright yellow/white)
        if current_time:
            # Lightning flashes every 4 seconds for 0.5 seconds
            flash_cycle = current_time % 4.0
            if flash_cycle < 0.5:  # Flash for half a second
                lightning_color = graphics.Color(255, 255, 200)  # Bright flash
            else:
                lightning_color = graphics.Color(200, 200, 100)  # Dimmer between flashes
        else:
            lightning_color = graphics.Color(255, 255, 150)
            
        lightning_pixels = [(0, 0), (-1, 1), (0, 1), (1, 2), (0, 3)]
        for dx, dy in lightning_pixels:
            if 0 <= y + dy < 32:
                self.canvas.SetPixel(x + dx, y + dy, lightning_color.red, lightning_color.green, lightning_color.blue)
    
    def draw_snow(self, x, y, current_time=None):
        """Draw cloud with animated snowflakes"""
        # Gray cloud
        cloud_color = graphics.Color(160, 160, 180)
        cloud_pixels = [(-2, -2), (-1, -2), (0, -2), (1, -2), (-1, -1), (0, -1), (1, -1), (2, -1)]
        for dx, dy in cloud_pixels:
            self.canvas.SetPixel(x + dx, y + dy, cloud_color.red, cloud_color.green, cloud_color.blue)
        
        # Animated snowflakes (white with cross pattern) that gently fall
        snow_color = graphics.Color(255, 255, 255)
        dim_snow = graphics.Color(200, 200, 200)
        
        if current_time:
            # Snowflakes animate every 3 seconds, falling down
            snow_phase = (current_time * 0.7) % 3.0
            if snow_phase < 1.0:  # Top positions
                positions = [(-2, 0), (0, 1), (2, 0)]
            elif snow_phase < 2.0:  # Middle positions  
                positions = [(-2, 1), (0, 2), (2, 1)]
            else:  # Bottom positions
                positions = [(-2, 2), (0, 3), (2, 2)]
        else:
            positions = [(-2, 1), (0, 2), (2, 1)]
        
        # Draw snowflakes at calculated positions
        for sx, sy in positions:
            if 0 <= y + sy < 32:
                # Draw cross pattern for each snowflake
                self.canvas.SetPixel(x + sx, y + sy, snow_color.red, snow_color.green, snow_color.blue)
                # Add smaller cross arms
                if 0 <= y + sy - 1 < 32:
                    self.canvas.SetPixel(x + sx, y + sy - 1, dim_snow.red, dim_snow.green, dim_snow.blue)
                if 0 <= y + sy + 1 < 32:
                    self.canvas.SetPixel(x + sx, y + sy + 1, dim_snow.red, dim_snow.green, dim_snow.blue)
                if 0 <= x + sx - 1 < 64:
                    self.canvas.SetPixel(x + sx - 1, y + sy, dim_snow.red, dim_snow.green, dim_snow.blue)
                if 0 <= x + sx + 1 < 64:
                    self.canvas.SetPixel(x + sx + 1, y + sy, dim_snow.red, dim_snow.green, dim_snow.blue)
    
    def draw_mist(self, x, y):
        """Draw misty/foggy conditions"""
        mist_color = graphics.Color(150, 150, 150)
        mist_light = graphics.Color(200, 200, 200)
        
        # Horizontal misty lines
        mist_pixels = [
            (-3, -1), (-2, -1), (-1, -1), (1, -1), (2, -1),
            (-2, 0), (-1, 0), (0, 0), (2, 0), (3, 0),
            (-3, 1), (-1, 1), (0, 1), (1, 1), (3, 1),
            (-2, 2), (-1, 2), (1, 2), (2, 2)
        ]
        
        for dx, dy in mist_pixels:
            # Alternate between two mist colors for layered effect
            color = mist_light if (dx + dy) % 2 == 0 else mist_color
            self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_custom_weather_icon(self, icon_code, x, y):
        """Draw custom weather icon based on condition with time-based effects"""
        if icon_code not in self.icon_mapping:
            # Fallback to a question mark or generic icon
            self.draw_scattered_clouds(x, y)
            return
        
        icon_type = self.icon_mapping[icon_code]
        
        # Get current time for animation effects
        current_time = time.time()
        
        # Call the appropriate drawing function with time parameter
        if icon_type == 'clear_day':
            self.draw_clear_day(x, y, current_time)
        elif icon_type == 'clear_night':
            self.draw_clear_night(x, y, current_time)
        elif icon_type == 'partly_cloudy_day':
            self.draw_partly_cloudy_day(x, y)
        elif icon_type == 'partly_cloudy_night':
            self.draw_partly_cloudy_night(x, y)
        elif icon_type == 'scattered_clouds':
            self.draw_scattered_clouds(x, y)
        elif icon_type == 'broken_clouds':
            self.draw_broken_clouds(x, y)
        elif icon_type == 'shower_rain':
            self.draw_shower_rain(x, y, current_time)
        elif icon_type == 'rain_day':
            self.draw_rain_day(x, y, current_time)
        elif icon_type == 'rain_night':
            self.draw_rain_night(x, y, current_time)
        elif icon_type == 'thunderstorm':
            self.draw_thunderstorm(x, y, current_time)
        elif icon_type == 'snow':
            self.draw_snow(x, y, current_time)
        elif icon_type == 'mist':
            self.draw_mist(x, y)
        else:
            # Unknown icon type, draw default
            self.draw_scattered_clouds(x, y)
        
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
        """Download and cache weather icon from OpenWeatherMap - NO LONGER USED, replaced with custom drawing"""
        # This method is now unused since we draw custom icons directly
        # Keeping it for potential future use or fallback
        return None

    def draw_weather_icon(self, icon_code, x, y):
        """Draw custom weather icon based on OpenWeatherMap icon code"""
        print(f"Drawing custom weather icon: {icon_code} at position ({x}, {y})")
        
        # Use our custom drawing system
        self.draw_custom_weather_icon(icon_code, x, y)

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
        self.draw_weather_icon(icon_code, icon_x, icon_y)
        
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