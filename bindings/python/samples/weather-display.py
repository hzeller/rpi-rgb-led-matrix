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
        """Draw a large, detailed sun icon with pulsing effect"""
        # Sun center with brightness pulse
        base_brightness = 255
        pulse_brightness = 200
        if current_time:
            # Gentle pulse every 4 seconds
            pulse = (math.sin(current_time * 0.5) + 1) / 2  # 0 to 1
            brightness = int(pulse_brightness + (base_brightness - pulse_brightness) * pulse)
        else:
            brightness = base_brightness
            
        sun_core = graphics.Color(brightness, brightness, 0)  # Bright yellow core
        sun_mid = graphics.Color(brightness - 30, brightness - 30, 0)  # Slightly dimmer
        sun_outer = graphics.Color(brightness - 60, brightness - 60, 0)  # Outer glow
        ray_color = graphics.Color(brightness - 80, brightness - 50, 0)  # Orange rays
        
        # Draw large sun center (7x7 core with gradual falloff)
        for dx in range(-3, 4):
            for dy in range(-3, 4):
                distance = math.sqrt(dx*dx + dy*dy)
                if distance <= 1.5:  # Inner core
                    self.canvas.SetPixel(x + dx, y + dy, sun_core.red, sun_core.green, sun_core.blue)
                elif distance <= 2.5:  # Middle ring
                    self.canvas.SetPixel(x + dx, y + dy, sun_mid.red, sun_mid.green, sun_mid.blue)
                elif distance <= 3.2:  # Outer glow
                    self.canvas.SetPixel(x + dx, y + dy, sun_outer.red, sun_outer.green, sun_outer.blue)
        
        # Draw detailed sun rays (12 directions - long and short alternating)
        long_rays = [(-6, 0), (6, 0), (0, -6), (0, 6), (-4, -4), (4, -4), (-4, 4), (4, 4)]
        short_rays = [(-5, -2), (5, -2), (-5, 2), (5, 2), (-2, -5), (2, -5), (-2, 5), (2, 5)]
        
        # Long rays (3 pixels each)
        for rx, ry in long_rays:
            # Draw ray with tapering brightness
            for i in range(3):
                ray_x, ray_y = x + rx + (1 if rx > 0 else -1) * i, y + ry + (1 if ry > 0 else -1) * i
                if 0 <= ray_x < 64 and 0 <= ray_y < 32:
                    brightness_factor = 1.0 - (i * 0.3)
                    r = int(ray_color.red * brightness_factor)
                    g = int(ray_color.green * brightness_factor)
                    self.canvas.SetPixel(ray_x, ray_y, r, g, 0)
        
        # Short rays (2 pixels each)
        for rx, ry in short_rays:
            for i in range(2):
                ray_x, ray_y = x + rx + (1 if rx > 0 else -1) * i, y + ry + (1 if ry > 0 else -1) * i
                if 0 <= ray_x < 64 and 0 <= ray_y < 32:
                    brightness_factor = 1.0 - (i * 0.4)
                    r = int(ray_color.red * brightness_factor)
                    g = int(ray_color.green * brightness_factor)
                    self.canvas.SetPixel(ray_x, ray_y, r, g, 0)
    
    def draw_clear_night(self, x, y, current_time=None):
        """Draw a large, detailed crescent moon with stars"""
        # Moon with subtle glow variation
        base_brightness = 220
        if current_time:
            # Very gentle glow every 6 seconds
            glow = (math.sin(current_time * 0.33) + 1) / 2  # 0 to 1
            brightness = int(base_brightness - 30 + 30 * glow)
        else:
            brightness = base_brightness
            
        moon_bright = graphics.Color(brightness, brightness, 255)  # Bright moon
        moon_mid = graphics.Color(brightness - 40, brightness - 40, 255)  # Medium moon
        moon_dim = graphics.Color(brightness - 80, brightness - 80, 255)  # Dim edge
        dark_color = graphics.Color(30, 30, 60)  # Dark blue sky
        star_color = graphics.Color(200, 200, 255)  # Stars
        
        # Draw large moon circle (radius 4)
        for dx in range(-5, 6):
            for dy in range(-5, 6):
                distance = math.sqrt(dx*dx + dy*dy)
                if distance <= 2.0:  # Bright center
                    self.canvas.SetPixel(x + dx, y + dy, moon_bright.red, moon_bright.green, moon_bright.blue)
                elif distance <= 3.5:  # Medium ring
                    self.canvas.SetPixel(x + dx, y + dy, moon_mid.red, moon_mid.green, moon_mid.blue)
                elif distance <= 4.5:  # Dim outer edge
                    self.canvas.SetPixel(x + dx, y + dy, moon_dim.red, moon_dim.green, moon_dim.blue)
        
        # Draw dark overlay for crescent (offset circle)
        for dx in range(-4, 6):
            for dy in range(-5, 6):
                distance = math.sqrt((dx-2)*(dx-2) + dy*dy)
                if distance <= 4.0:  # Dark overlay
                    self.canvas.SetPixel(x + dx, y + dy, dark_color.red, dark_color.green, dark_color.blue)
        
        # Add some small stars around the moon
        stars = [(-7, -3), (-6, 2), (7, -4), (6, 3), (-8, 0), (8, -1)]
        for sx, sy in stars:
            if 0 <= x + sx < 64 and 0 <= y + sy < 32:
                self.canvas.SetPixel(x + sx, y + sy, star_color.red, star_color.green, star_color.blue)
    
    def draw_partly_cloudy_day(self, x, y):
        """Draw large sun partially covered by detailed cloud"""
        # Sun (upper left, smaller but still detailed)
        sun_color = graphics.Color(255, 255, 0)
        sun_rays = graphics.Color(255, 200, 0)
        
        # Sun center
        for dx in range(-1, 2):
            for dy in range(-1, 2):
                self.canvas.SetPixel(x + dx - 4, y + dy - 3, sun_color.red, sun_color.green, sun_color.blue)
        
        # Sun rays (shorter)
        sun_ray_positions = [(-6, -3), (-2, -3), (-4, -5), (-4, -1), (-6, -5), (-2, -5), (-6, -1), (-2, -1)]
        for rx, ry in sun_ray_positions:
            if 0 <= x + rx < 64 and 0 <= y + ry < 32:
                self.canvas.SetPixel(x + rx, y + ry, sun_rays.red, sun_rays.green, sun_rays.blue)
        
        # Large detailed cloud (covering lower right)
        cloud_bright = graphics.Color(240, 240, 255)
        cloud_mid = graphics.Color(200, 200, 240)
        cloud_shadow = graphics.Color(160, 160, 200)
        
        # Cloud main body - much larger and more detailed
        cloud_pixels = [
            # Top row of cloud
            (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2),
            # Second row - wider
            (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1),
            # Third row - widest
            (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0), (4, 0), (5, 0),
            # Fourth row - wide
            (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1), (3, 1), (4, 1),
            # Bottom row - smaller
            (-1, 2), (0, 2), (1, 2), (2, 2), (3, 2),
            # Extra puffs for realistic cloud shape
            (-1, 3), (0, 3), (1, 3), (2, 3)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                # Add depth with different brightness levels
                if dy >= 2:  # Bottom shadow
                    color = cloud_shadow
                elif dx >= 3:  # Right side highlight
                    color = cloud_bright
                else:  # Main body
                    color = cloud_mid
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_partly_cloudy_night(self, x, y):
        """Draw large moon partially covered by detailed cloud"""
        # Moon (upper left, smaller crescent)
        moon_color = graphics.Color(200, 200, 255)
        dark_moon = graphics.Color(40, 40, 80)
        
        # Moon shape
        moon_pixels = [(-5, -3), (-4, -3), (-5, -2), (-4, -2), (-3, -2), (-5, -1), (-4, -1)]
        for mx, my in moon_pixels:
            if 0 <= x + mx < 64 and 0 <= y + my < 32:
                self.canvas.SetPixel(x + mx, y + my, moon_color.red, moon_color.green, moon_color.blue)
        
        # Dark overlay for crescent
        dark_pixels = [(-4, -3), (-3, -2), (-4, -2), (-3, -1), (-4, -1)]
        for mx, my in dark_pixels:
            if 0 <= x + mx < 64 and 0 <= y + my < 32:
                self.canvas.SetPixel(x + mx, y + my, dark_moon.red, dark_moon.green, dark_moon.blue)
        
        # Large detailed cloud (darker for night)
        cloud_bright = graphics.Color(140, 140, 180)
        cloud_mid = graphics.Color(120, 120, 160)
        cloud_shadow = graphics.Color(80, 80, 120)
        
        # Same cloud pattern as day version but darker
        cloud_pixels = [
            (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2),
            (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1),
            (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0), (4, 0), (5, 0),
            (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1), (3, 1), (4, 1),
            (-1, 2), (0, 2), (1, 2), (2, 2), (3, 2),
            (-1, 3), (0, 3), (1, 3), (2, 3)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                if dy >= 2:
                    color = cloud_shadow
                elif dx >= 3:
                    color = cloud_bright
                else:
                    color = cloud_mid
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_scattered_clouds(self, x, y):
        """Draw multiple detailed clouds scattered across the icon"""
        cloud_bright = graphics.Color(220, 220, 255)
        cloud_mid = graphics.Color(180, 180, 220)
        cloud_shadow = graphics.Color(140, 140, 180)
        
        # Cloud 1 (upper left) - medium size
        cloud1_pixels = [
            (-6, -4), (-5, -4), (-4, -4), (-3, -4),
            (-6, -3), (-5, -3), (-4, -3), (-3, -3), (-2, -3),
            (-5, -2), (-4, -2), (-3, -2), (-2, -2),
            (-4, -1), (-3, -1), (-2, -1)
        ]
        for dx, dy in cloud1_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = cloud_shadow if dy >= -2 else cloud_bright
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Cloud 2 (center right) - large
        cloud2_pixels = [
            (2, -2), (3, -2), (4, -2), (5, -2), (6, -2),
            (1, -1), (2, -1), (3, -1), (4, -1), (5, -1), (6, -1), (7, -1),
            (2, 0), (3, 0), (4, 0), (5, 0), (6, 0), (7, 0),
            (3, 1), (4, 1), (5, 1), (6, 1)
        ]
        for dx, dy in cloud2_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = cloud_shadow if dy >= 0 else cloud_mid
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Cloud 3 (lower center) - medium
        cloud3_pixels = [
            (-2, 2), (-1, 2), (0, 2), (1, 2), (2, 2),
            (-3, 3), (-2, 3), (-1, 3), (0, 3), (1, 3), (2, 3), (3, 3),
            (-2, 4), (-1, 4), (0, 4), (1, 4), (2, 4)
        ]
        for dx, dy in cloud3_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = cloud_shadow if dy >= 4 else cloud_bright
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_broken_clouds(self, x, y):
        """Draw heavy, detailed cloud cover"""
        cloud_bright = graphics.Color(180, 180, 220)
        cloud_mid = graphics.Color(140, 140, 180)
        cloud_dark = graphics.Color(100, 100, 140)
        cloud_shadow = graphics.Color(80, 80, 120)
        
        # Large overlapping cloud formation covering most of the icon
        cloud_pixels = [
            # Top layer
            (-5, -5), (-4, -5), (-3, -5), (-2, -5), (-1, -5), (0, -5), (1, -5), (2, -5), (3, -5),
            (-6, -4), (-5, -4), (-4, -4), (-3, -4), (-2, -4), (-1, -4), (0, -4), (1, -4), (2, -4), (3, -4), (4, -4),
            # Main body
            (-7, -3), (-6, -3), (-5, -3), (-4, -3), (-3, -3), (-2, -3), (-1, -3), (0, -3), (1, -3), (2, -3), (3, -3), (4, -3), (5, -3),
            (-7, -2), (-6, -2), (-5, -2), (-4, -2), (-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2), (4, -2), (5, -2), (6, -2),
            (-6, -1), (-5, -1), (-4, -1), (-3, -1), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1), (5, -1), (6, -1),
            (-6, 0), (-5, 0), (-4, 0), (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0), (4, 0), (5, 0), (6, 0),
            # Lower section
            (-5, 1), (-4, 1), (-3, 1), (-2, 1), (-1, 1), (0, 1), (1, 1), (2, 1), (3, 1), (4, 1), (5, 1),
            (-4, 2), (-3, 2), (-2, 2), (-1, 2), (0, 2), (1, 2), (2, 2), (3, 2), (4, 2),
            (-3, 3), (-2, 3), (-1, 3), (0, 3), (1, 3), (2, 3), (3, 3),
            (-2, 4), (-1, 4), (0, 4), (1, 4), (2, 4)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                # Create depth and variation
                if dy >= 3:  # Bottom shadow
                    color = cloud_shadow
                elif dy >= 1:  # Lower mid section
                    color = cloud_dark
                elif abs(dx) >= 5:  # Side edges
                    color = cloud_mid
                else:  # Main body
                    color = cloud_bright
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_shower_rain(self, x, y, current_time=None):
        """Draw large cloud with detailed scattered rain drops that animate"""
        # Large detailed cloud
        cloud_color = graphics.Color(120, 120, 160)
        cloud_shadow = graphics.Color(90, 90, 130)
        
        # Big cloud shape
        cloud_pixels = [
            (-4, -4), (-3, -4), (-2, -4), (-1, -4), (0, -4), (1, -4), (2, -4),
            (-5, -3), (-4, -3), (-3, -3), (-2, -3), (-1, -3), (0, -3), (1, -3), (2, -3), (3, -3),
            (-5, -2), (-4, -2), (-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2), (4, -2),
            (-4, -1), (-3, -1), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1),
            (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = cloud_shadow if dy >= -1 else cloud_color
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Animated rain drops (blue) - different drops appear at different times
        rain_bright = graphics.Color(0, 180, 255)
        rain_medium = graphics.Color(0, 150, 220)
        rain_dim = graphics.Color(0, 120, 180)
        
        if current_time:
            # Rain drops cycle every 3 seconds with 3 different patterns
            rain_cycle = (current_time * 1.0) % 3.0
            if rain_cycle < 1.0:  # First second - left side heavy
                rain_drops = [(-4, 2), (-4, 3), (-3, 4), (-2, 1), (-2, 3), (-1, 5), (0, 2), (1, 4), (2, 3)]
            elif rain_cycle < 2.0:  # Second second - center heavy
                rain_drops = [(-3, 1), (-2, 4), (-1, 2), (-1, 6), (0, 3), (0, 5), (1, 1), (1, 6), (2, 4)]
            else:  # Third second - right side heavy
                rain_drops = [(-2, 2), (-1, 4), (0, 1), (0, 6), (1, 3), (2, 1), (2, 5), (3, 2), (3, 4)]
        else:
            rain_drops = [(-4, 2), (-3, 4), (-2, 1), (-1, 5), (0, 3), (1, 1), (1, 6), (2, 4), (3, 2)]
            
        for dx, dy in rain_drops:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                # Vary rain drop brightness for depth
                if dy <= 2:
                    color = rain_bright
                elif dy <= 4:
                    color = rain_medium
                else:
                    color = rain_dim
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_rain_day(self, x, y, current_time=None):
        """Draw large cloud with heavy steady rain that shifts"""
        # Large dark storm cloud
        cloud_dark = graphics.Color(100, 100, 140)
        cloud_shadow = graphics.Color(70, 70, 110)
        
        # Large cloud formation
        cloud_pixels = [
            (-5, -4), (-4, -4), (-3, -4), (-2, -4), (-1, -4), (0, -4), (1, -4), (2, -4), (3, -4),
            (-6, -3), (-5, -3), (-4, -3), (-3, -3), (-2, -3), (-1, -3), (0, -3), (1, -3), (2, -3), (3, -3), (4, -3),
            (-6, -2), (-5, -2), (-4, -2), (-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2), (4, -2), (5, -2),
            (-5, -1), (-4, -1), (-3, -1), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1), (5, -1),
            (-4, 0), (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0), (4, 0)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = cloud_shadow if dy >= -1 else cloud_dark
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Heavy animated steady rain lines that shift position
        rain_color = graphics.Color(0, 120, 255)
        rain_light = graphics.Color(0, 100, 220)
        
        if current_time:
            # Rain shifts every 2 seconds
            shift = int(current_time * 0.5) % 2
            if shift == 0:
                rain_lines = [
                    (-5, 1), (-5, 2), (-5, 3), (-4, 2), (-4, 3), (-4, 4),
                    (-3, 1), (-3, 3), (-3, 5), (-2, 2), (-2, 4), (-2, 6),
                    (-1, 1), (-1, 3), (-1, 5), (0, 2), (0, 4), (0, 6),
                    (1, 1), (1, 3), (1, 5), (2, 2), (2, 4), (2, 6),
                    (3, 1), (3, 3), (3, 5), (4, 2), (4, 4), (4, 6)
                ]
            else:
                rain_lines = [
                    (-4, 1), (-4, 3), (-4, 5), (-3, 2), (-3, 4), (-3, 6),
                    (-2, 1), (-2, 3), (-2, 5), (-1, 2), (-1, 4), (-1, 6),
                    (0, 1), (0, 3), (0, 5), (1, 2), (1, 4), (1, 6),
                    (2, 1), (2, 3), (2, 5), (3, 2), (3, 4), (3, 6),
                    (4, 1), (4, 3), (4, 5), (5, 2), (5, 4)
                ]
        else:
            rain_lines = [
                (-5, 1), (-5, 3), (-4, 2), (-4, 4), (-3, 1), (-3, 3), (-3, 5),
                (-2, 2), (-2, 4), (-1, 1), (-1, 3), (-1, 5), (0, 2), (0, 4),
                (1, 1), (1, 3), (1, 5), (2, 2), (2, 4), (3, 1), (3, 3), (3, 5)
            ]
            
        for dx, dy in rain_lines:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = rain_light if dy >= 4 else rain_color
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_rain_night(self, x, y, current_time=None):
        """Draw large cloud with heavy steady rain (darker for night) that shifts"""
        # Very large dark cloud for night
        cloud_dark = graphics.Color(60, 60, 100)
        cloud_shadow = graphics.Color(40, 40, 80)
        
        # Same large cloud pattern as day rain but darker
        cloud_pixels = [
            (-5, -4), (-4, -4), (-3, -4), (-2, -4), (-1, -4), (0, -4), (1, -4), (2, -4), (3, -4),
            (-6, -3), (-5, -3), (-4, -3), (-3, -3), (-2, -3), (-1, -3), (0, -3), (1, -3), (2, -3), (3, -3), (4, -3),
            (-6, -2), (-5, -2), (-4, -2), (-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2), (4, -2), (5, -2),
            (-5, -1), (-4, -1), (-3, -1), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1), (5, -1),
            (-4, 0), (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0), (4, 0)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = cloud_shadow if dy >= -1 else cloud_dark
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Rain (darker blue for night) with shifting animation
        rain_color = graphics.Color(0, 80, 180)
        rain_light = graphics.Color(0, 60, 150)
        
        if current_time:
            # Same rain pattern as day but darker colors
            shift = int(current_time * 0.5) % 2
            if shift == 0:
                rain_lines = [
                    (-5, 1), (-5, 2), (-5, 3), (-4, 2), (-4, 3), (-4, 4),
                    (-3, 1), (-3, 3), (-3, 5), (-2, 2), (-2, 4), (-2, 6),
                    (-1, 1), (-1, 3), (-1, 5), (0, 2), (0, 4), (0, 6),
                    (1, 1), (1, 3), (1, 5), (2, 2), (2, 4), (2, 6),
                    (3, 1), (3, 3), (3, 5), (4, 2), (4, 4), (4, 6)
                ]
            else:
                rain_lines = [
                    (-4, 1), (-4, 3), (-4, 5), (-3, 2), (-3, 4), (-3, 6),
                    (-2, 1), (-2, 3), (-2, 5), (-1, 2), (-1, 4), (-1, 6),
                    (0, 1), (0, 3), (0, 5), (1, 2), (1, 4), (1, 6),
                    (2, 1), (2, 3), (2, 5), (3, 2), (3, 4), (3, 6),
                    (4, 1), (4, 3), (4, 5), (5, 2), (5, 4)
                ]
        else:
            rain_lines = [
                (-5, 1), (-5, 3), (-4, 2), (-4, 4), (-3, 1), (-3, 3), (-3, 5),
                (-2, 2), (-2, 4), (-1, 1), (-1, 3), (-1, 5), (0, 2), (0, 4),
                (1, 1), (1, 3), (1, 5), (2, 2), (2, 4), (3, 1), (3, 3), (3, 5)
            ]
            
        for dx, dy in rain_lines:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = rain_light if dy >= 4 else rain_color
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
    
    def draw_thunderstorm(self, x, y, current_time=None):
        """Draw large storm cloud with detailed flashing lightning bolt"""
        # Large, dark, menacing storm cloud
        cloud_dark = graphics.Color(60, 60, 80)
        cloud_black = graphics.Color(40, 40, 60)
        
        # Massive storm cloud
        cloud_pixels = [
            (-6, -5), (-5, -5), (-4, -5), (-3, -5), (-2, -5), (-1, -5), (0, -5), (1, -5), (2, -5), (3, -5), (4, -5),
            (-7, -4), (-6, -4), (-5, -4), (-4, -4), (-3, -4), (-2, -4), (-1, -4), (0, -4), (1, -4), (2, -4), (3, -4), (4, -4), (5, -4),
            (-7, -3), (-6, -3), (-5, -3), (-4, -3), (-3, -3), (-2, -3), (-1, -3), (0, -3), (1, -3), (2, -3), (3, -3), (4, -3), (5, -3), (6, -3),
            (-6, -2), (-5, -2), (-4, -2), (-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2), (4, -2), (5, -2), (6, -2),
            (-5, -1), (-4, -1), (-3, -1), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1), (5, -1),
            (-4, 0), (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0), (4, 0)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = cloud_black if dy >= -2 else cloud_dark
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Large, detailed flashing lightning bolt
        if current_time:
            # Lightning flashes every 4 seconds for 0.5 seconds
            flash_cycle = current_time % 4.0
            if flash_cycle < 0.3:  # Bright flash
                lightning_bright = graphics.Color(255, 255, 220)
                lightning_core = graphics.Color(255, 255, 180)
            elif flash_cycle < 0.6:  # Medium flash
                lightning_bright = graphics.Color(220, 220, 160)
                lightning_core = graphics.Color(200, 200, 120)
            else:  # Dim between flashes
                lightning_bright = graphics.Color(180, 180, 100)
                lightning_core = graphics.Color(160, 160, 80)
        else:
            lightning_bright = graphics.Color(255, 255, 180)
            lightning_core = graphics.Color(255, 255, 150)
        
        # Detailed zigzag lightning bolt
        lightning_pixels = [
            # Main bolt going down and zigzagging
            (0, 1), (0, 2),  # Straight down from cloud
            (-1, 3), (-1, 4),  # Zig left
            (0, 5), (1, 6),  # Zag right
            (0, 7), (-1, 8),  # Zig left again
            (0, 9), (0, 10)  # Straight down
        ]
        
        # Lightning core (brighter)
        for dx, dy in lightning_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                self.canvas.SetPixel(x + dx, y + dy, lightning_core.red, lightning_core.green, lightning_core.blue)
        
        # Lightning glow (around the core)
        lightning_glow = [
            # Glow around main bolt
            (-1, 1), (1, 1), (-1, 2), (1, 2),  # Around top
            (-2, 3), (0, 3), (-2, 4), (0, 4),  # Around left zig
            (-1, 5), (1, 5), (0, 6), (2, 6),   # Around right zag
            (-1, 7), (1, 7), (-2, 8), (0, 8),  # Around left zig
            (-1, 9), (1, 9), (-1, 10), (1, 10) # Around bottom
        ]
        
        for dx, dy in lightning_glow:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                self.canvas.SetPixel(x + dx, y + dy, lightning_bright.red, lightning_bright.green, lightning_bright.blue)
    
    def draw_snow(self, x, y, current_time=None):
        """Draw large cloud with detailed animated snowflakes"""
        # Large gray/white cloud
        cloud_color = graphics.Color(160, 160, 180)
        cloud_bright = graphics.Color(200, 200, 220)
        cloud_shadow = graphics.Color(120, 120, 140)
        
        # Large cloud formation
        cloud_pixels = [
            (-4, -4), (-3, -4), (-2, -4), (-1, -4), (0, -4), (1, -4), (2, -4), (3, -4),
            (-5, -3), (-4, -3), (-3, -3), (-2, -3), (-1, -3), (0, -3), (1, -3), (2, -3), (3, -3), (4, -3),
            (-5, -2), (-4, -2), (-3, -2), (-2, -2), (-1, -2), (0, -2), (1, -2), (2, -2), (3, -2), (4, -2), (5, -2),
            (-4, -1), (-3, -1), (-2, -1), (-1, -1), (0, -1), (1, -1), (2, -1), (3, -1), (4, -1),
            (-3, 0), (-2, 0), (-1, 0), (0, 0), (1, 0), (2, 0), (3, 0)
        ]
        
        for dx, dy in cloud_pixels:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                if dy >= -1:
                    color = cloud_shadow
                elif abs(dx) >= 4:
                    color = cloud_bright
                else:
                    color = cloud_color
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        # Detailed animated snowflakes with cross patterns that gently fall
        snow_bright = graphics.Color(255, 255, 255)
        snow_medium = graphics.Color(220, 220, 255)
        snow_dim = graphics.Color(180, 180, 220)
        
        if current_time:
            # Snowflakes animate every 4 seconds, falling down in 4 phases
            snow_phase = (current_time * 0.6) % 4.0
            if snow_phase < 1.0:  # Top positions
                flake_positions = [(-4, 1), (-1, 2), (2, 1), (4, 2)]
            elif snow_phase < 2.0:  # Upper-middle positions  
                flake_positions = [(-4, 3), (-1, 4), (2, 3), (4, 4)]
            elif snow_phase < 3.0:  # Lower-middle positions
                flake_positions = [(-4, 5), (-1, 6), (2, 5), (4, 6)]
            else:  # Bottom positions
                flake_positions = [(-4, 7), (-1, 8), (2, 7), (4, 8)]
        else:
            flake_positions = [(-4, 3), (-1, 4), (2, 3), (4, 6)]
        
        # Draw detailed snowflakes at calculated positions
        for sx, sy in flake_positions:
            if 0 <= x + sx < 64 and 0 <= y + sy < 32:
                # Draw large cross pattern for each snowflake
                # Center pixel (brightest)
                self.canvas.SetPixel(x + sx, y + sy, snow_bright.red, snow_bright.green, snow_bright.blue)
                
                # Main cross arms (medium brightness)
                cross_arms = [(sx, sy-1), (sx, sy+1), (sx-1, sy), (sx+1, sy)]
                for cx, cy in cross_arms:
                    if 0 <= x + cx < 64 and 0 <= y + cy < 32:
                        self.canvas.SetPixel(x + cx, y + cy, snow_medium.red, snow_medium.green, snow_medium.blue)
                
                # Diagonal arms (dimmer)
                diag_arms = [(sx-1, sy-1), (sx+1, sy-1), (sx-1, sy+1), (sx+1, sy+1)]
                for dx, dy in diag_arms:
                    if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                        self.canvas.SetPixel(x + dx, y + dy, snow_dim.red, snow_dim.green, snow_dim.blue)
        
        # Add some smaller scattered snow pixels for atmosphere
        small_snow = [(-6, 4), (-2, 7), (1, 9), (5, 5), (-3, 9), (3, 8)]
        for sx, sy in small_snow:
            if 0 <= x + sx < 64 and 0 <= y + sy < 32:
                self.canvas.SetPixel(x + sx, y + sy, snow_dim.red, snow_dim.green, snow_dim.blue)
    
    def draw_mist(self, x, y):
        """Draw detailed misty/foggy conditions with layered effect"""
        mist_bright = graphics.Color(180, 180, 180)
        mist_medium = graphics.Color(150, 150, 150)
        mist_light = graphics.Color(120, 120, 120)
        mist_dim = graphics.Color(100, 100, 100)
        
        # Multiple horizontal misty layers at different heights and densities
        # Top layer (lightest)
        top_layer = [
            (-6, -4), (-5, -4), (-4, -4), (-2, -4), (-1, -4), (1, -4), (2, -4), (4, -4), (5, -4),
            (-5, -3), (-3, -3), (-2, -3), (0, -3), (1, -3), (3, -3), (4, -3), (6, -3)
        ]
        
        # Middle layer (medium density)
        middle_layer = [
            (-7, -1), (-6, -1), (-4, -1), (-3, -1), (-1, -1), (0, -1), (2, -1), (3, -1), (5, -1), (6, -1),
            (-6, 0), (-5, 0), (-3, 0), (-2, 0), (-1, 0), (1, 0), (2, 0), (4, 0), (5, 0), (7, 0)
        ]
        
        # Lower layer (denser)
        lower_layer = [
            (-7, 2), (-6, 2), (-5, 2), (-3, 2), (-2, 2), (-1, 2), (0, 2), (1, 2), (3, 2), (4, 2), (5, 2), (6, 2),
            (-6, 3), (-5, 3), (-4, 3), (-2, 3), (-1, 3), (0, 3), (2, 3), (3, 3), (4, 3), (6, 3), (7, 3)
        ]
        
        # Bottom layer (densest)
        bottom_layer = [
            (-7, 5), (-6, 5), (-5, 5), (-4, 5), (-3, 5), (-1, 5), (0, 5), (1, 5), (2, 5), (4, 5), (5, 5), (6, 5), (7, 5),
            (-6, 6), (-5, 6), (-4, 6), (-3, 6), (-2, 6), (-1, 6), (0, 6), (1, 6), (2, 6), (3, 6), (4, 6), (5, 6), (6, 6)
        ]
        
        # Draw each layer with appropriate brightness
        for dx, dy in top_layer:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = mist_light if (dx + dy) % 2 == 0 else mist_medium
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        for dx, dy in middle_layer:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = mist_medium if (dx + dy) % 2 == 0 else mist_bright
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        for dx, dy in lower_layer:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = mist_bright if (dx + dy) % 2 == 0 else mist_medium
                self.canvas.SetPixel(x + dx, y + dy, color.red, color.green, color.blue)
        
        for dx, dy in bottom_layer:
            if 0 <= x + dx < 64 and 0 <= y + dy < 32:
                color = mist_bright if (dx + dy) % 3 == 0 else mist_dim
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
        
        # Calculate total group width: icon(22) + spacing(2) + max_temp_width  
        # Increased icon width to 22 to accommodate larger detailed icons
        max_temp_width = max(temp_width, low_width)
        total_group_width = 22 + 2 + max_temp_width  # Larger icon area with better spacing
        
        # Center the entire group horizontally
        group_start_x = (64 - total_group_width) // 2
        
        # Position icon on the left of the group, centered vertically in bottom area
        # Adjusted to account for larger icon size (-7 to +7 horizontally, -5 to +10 vertically)
        icon_x = group_start_x + 11  # Center of the 22px icon area
        icon_y = 20  # Moved up slightly to center the larger vertical span better
        
        print(f"Weather icon code: {icon_code}")
        print(f"Group layout: total_width={total_group_width}, start_x={group_start_x}")
        self.draw_weather_icon(icon_code, icon_x, icon_y)
        
        # Position temperatures to the right of icon with proper spacing for larger icons
        temp_start_x = group_start_x + 22 + 2  # After larger icon area + spacing
        
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