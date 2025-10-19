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
    
    def draw_weather_icon(self, condition, x, y):
        """Draw a simple weather icon using ASCII-style graphics"""
        # Simple weather icons using basic shapes
        if "clear" in condition.lower() or "sun" in condition.lower():
            # Sun icon - circle with rays
            graphics.DrawText(self.canvas, self.condition_font, x, y, graphics.Color(255, 255, 0), "☀")
        elif "cloud" in condition.lower():
            # Cloud icon  
            graphics.DrawText(self.canvas, self.condition_font, x, y, graphics.Color(200, 200, 200), "☁")
        elif "rain" in condition.lower() or "drizzle" in condition.lower():
            # Rain icon
            graphics.DrawText(self.canvas, self.condition_font, x, y, graphics.Color(100, 150, 255), "🌧")
        elif "snow" in condition.lower():
            # Snow icon
            graphics.DrawText(self.canvas, self.condition_font, x, y, graphics.Color(255, 255, 255), "❄")
        elif "storm" in condition.lower() or "thunder" in condition.lower():
            # Storm icon
            graphics.DrawText(self.canvas, self.condition_font, x, y, graphics.Color(255, 255, 100), "⚡")
        else:
            # Default - simple dot
            graphics.DrawText(self.canvas, self.condition_font, x, y, self.condition_color, "●")

    def draw_weather(self):
        """Draw weather information in the layout style shown in image"""
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
        
        # Day abbreviation at top left
        day_str = now.strftime("%a").upper()  # "FRI", "SAT", etc.
        graphics.DrawText(self.canvas, self.condition_font, 2, 10, self.condition_color, day_str)
        
        # Time at top right (24-hour format like image)
        time_str = now.strftime("%H:%M")
        time_width = len(time_str) * 4
        time_x = 64 - time_width - 2  # Right aligned with padding
        graphics.DrawText(self.canvas, self.small_font, time_x, 10, self.detail_color, time_str)
        
        # Extract weather info
        temp = int(weather['main']['temp'])
        condition = weather['weather'][0]['main']
        
        # Temperature at bottom left (large)
        temp_str = f"{temp}°F"
        temp_width = 0
        for char in temp_str:
            temp_width += self.temp_font.CharacterWidth(ord(char))
        
        # Draw temperature with tight spacing at bottom left
        current_x = 2
        temp_y = 28
        for char in temp_str:
            char_width = graphics.DrawText(self.canvas, self.temp_font, current_x, temp_y, self.temp_color, char)
            current_x += char_width - 1
        
        # Weather icon in center
        icon_x = 32  # Center of 64-pixel display
        icon_y = 20
        self.draw_weather_icon(condition, icon_x, icon_y)

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