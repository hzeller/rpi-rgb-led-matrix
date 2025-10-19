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
                return self.weather_icon
                
            # OpenWeatherMap icon URL
            icon_url = f"https://openweathermap.org/img/wn/{icon_code}@2x.png"
            
            response = requests.get(icon_url, timeout=10)
            if response.status_code == 200:
                # Load image and resize to fit display
                icon_image = Image.open(BytesIO(response.content))
                
                # Resize to about 16x16 pixels for the display
                resample_mode = getattr(Image, "Resampling", Image).LANCZOS
                icon_image = icon_image.resize((16, 16), resample=resample_mode)
                
                # Convert to RGB
                self.weather_icon = icon_image.convert('RGB')
                self.last_icon_update = time.time()
                
                print(f"Downloaded weather icon: {icon_code}")
                return self.weather_icon
            else:
                print(f"Icon download failed: {response.status_code}")
                return None
                
        except Exception as e:
            print(f"Icon download error: {e}")
            return None

    def draw_weather_icon(self, icon_image, x, y):
        """Draw the weather icon image on the display"""
        if icon_image is not None:
            # Calculate position to center the 16x16 icon
            icon_x = x - 8  # Center horizontally
            icon_y = y - 8  # Center vertically
            
            # Draw the icon
            self.canvas.SetImage(icon_image, icon_x, icon_y)
        else:
            # Fallback - simple colored square
            self.canvas.SetPixel(x, y, 200, 200, 255)

    def draw_weather(self):
        """Draw weather information exactly like the reference image"""
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
        
        # Get and draw weather icon in center
        icon_x = 32
        icon_y = 16
        icon_image = self.get_weather_icon(icon_code)
        self.draw_weather_icon(icon_image, icon_x, icon_y)
        
        # High temperature below icon (large number)
        temp_str = str(temp)
        temp_width = 0
        for char in temp_str:
            temp_width += self.temp_font.CharacterWidth(ord(char))
        
        temp_x = (64 - temp_width) // 2
        temp_y = 24
        
        # Draw high temp with tight spacing
        current_x = temp_x
        for char in temp_str:
            char_width = graphics.DrawText(self.canvas, self.temp_font, current_x, temp_y, self.temp_color, char)
            current_x += char_width - 1
        
        # Low temperature with degree symbol below (smaller)
        low_str = f"{temp_low}°"
        low_width = 0
        for char in low_str:
            low_width += self.small_font.CharacterWidth(ord(char))
        
        low_x = (64 - low_width) // 2  
        low_y = 32
        
        # Draw low temp with tight spacing
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