#!/usr/bin/env python3
"""
Script to extract individual weather icons from the provided weather icons image.
The image contains a 4x3 grid of weather icons that need to be separated into individual files.
"""

from PIL import Image, ImageDraw
import os
import base64
import io

def create_sample_icon(name, color, size=(64, 64)):
    """Create a simple sample icon as fallback"""
    img = Image.new('RGBA', size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Draw a simple shape based on the icon type
    if 'sun' in name.lower() or 'clear_day' in name.lower():
        # Draw sun
        center = (size[0]//2, size[1]//2)
        radius = min(size) // 4
        draw.ellipse([center[0]-radius, center[1]-radius, 
                     center[0]+radius, center[1]+radius], 
                    fill=(255, 255, 0, 255))
        # Rays
        for i in range(0, 360, 45):
            import math
            x = center[0] + int(radius * 1.5 * math.cos(math.radians(i)))
            y = center[1] + int(radius * 1.5 * math.sin(math.radians(i)))
            draw.line([center[0], center[1], x, y], fill=(255, 200, 0, 255), width=3)
    
    elif 'moon' in name.lower() or 'clear_night' in name.lower():
        # Draw crescent moon
        center = (size[0]//2, size[1]//2)
        radius = min(size) // 4
        draw.ellipse([center[0]-radius, center[1]-radius, 
                     center[0]+radius, center[1]+radius], 
                    fill=(220, 220, 255, 255))
        # Dark overlay for crescent
        draw.ellipse([center[0]-radius//2, center[1]-radius, 
                     center[0]+radius, center[1]+radius], 
                    fill=(0, 0, 0, 0))
    
    elif 'cloud' in name.lower():
        # Draw cloud
        center = (size[0]//2, size[1]//2)
        radius = min(size) // 6
        # Multiple circles to form cloud
        positions = [(-radius, 0), (radius, 0), (0, -radius//2), (-radius//2, radius//2), (radius//2, radius//2)]
        for dx, dy in positions:
            draw.ellipse([center[0]+dx-radius//2, center[1]+dy-radius//2,
                         center[0]+dx+radius//2, center[1]+dy+radius//2],
                        fill=(200, 200, 255, 255))
    
    elif 'rain' in name.lower():
        # Draw cloud with rain
        center = (size[0]//2, size[1]//4)
        radius = min(size) // 6
        # Cloud
        positions = [(-radius, 0), (radius, 0), (0, -radius//2)]
        for dx, dy in positions:
            draw.ellipse([center[0]+dx-radius//2, center[1]+dy-radius//2,
                         center[0]+dx+radius//2, center[1]+dy+radius//2],
                        fill=(150, 150, 200, 255))
        # Rain drops
        for i in range(5):
            x = center[0] - radius + (i * radius // 2)
            y1 = center[1] + radius
            y2 = y1 + radius
            draw.line([x, y1, x, y2], fill=(0, 150, 255, 255), width=2)
    
    elif 'thunder' in name.lower():
        # Draw cloud with lightning
        center = (size[0]//2, size[1]//4)
        radius = min(size) // 6
        # Dark cloud
        positions = [(-radius, 0), (radius, 0), (0, -radius//2)]
        for dx, dy in positions:
            draw.ellipse([center[0]+dx-radius//2, center[1]+dy-radius//2,
                         center[0]+dx+radius//2, center[1]+dy+radius//2],
                        fill=(100, 100, 150, 255))
        # Lightning bolt
        bolt_points = [(center[0], center[1]+radius), 
                      (center[0]-radius//4, center[1]+radius*2),
                      (center[0]+radius//4, center[1]+radius*2),
                      (center[0], center[1]+radius*3)]
        draw.polygon(bolt_points, fill=(255, 255, 0, 255))
    
    elif 'snow' in name.lower():
        # Draw cloud with snowflakes
        center = (size[0]//2, size[1]//4)
        radius = min(size) // 6
        # Cloud
        positions = [(-radius, 0), (radius, 0), (0, -radius//2)]
        for dx, dy in positions:
            draw.ellipse([center[0]+dx-radius//2, center[1]+dy-radius//2,
                         center[0]+dx+radius//2, center[1]+dy+radius//2],
                        fill=(180, 180, 220, 255))
        # Snowflakes
        for i in range(4):
            x = center[0] - radius + (i * radius // 2)
            y = center[1] + radius + (i % 2) * radius // 2
            # Draw snowflake cross
            draw.line([x-4, y, x+4, y], fill=(255, 255, 255, 255), width=2)
            draw.line([x, y-4, x, y+4], fill=(255, 255, 255, 255), width=2)
    
    else:
        # Default icon - simple circle
        center = (size[0]//2, size[1]//2)
        radius = min(size) // 4
        draw.ellipse([center[0]-radius, center[1]-radius, 
                     center[0]+radius, center[1]+radius], 
                    fill=color)
    
    return img

def extract_weather_icons_from_grid():
    """Extract icons and create the weather icon files"""
    
    output_dir = "weather_icons"
    os.makedirs(output_dir, exist_ok=True)
    
    # Define the weather icon mapping for OpenWeatherMap codes
    icon_definitions = [
        # Standard OpenWeatherMap icon codes with descriptive names
        ("01d", "clear_day", (255, 255, 0, 255)),        # Clear sky day
        ("01n", "clear_night", (220, 220, 255, 255)),    # Clear sky night
        ("02d", "partly_cloudy_day", (255, 200, 100, 255)), # Few clouds day
        ("02n", "partly_cloudy_night", (150, 150, 200, 255)), # Few clouds night
        ("03d", "scattered_clouds", (200, 200, 255, 255)), # Scattered clouds
        ("03n", "scattered_clouds_night", (150, 150, 200, 255)), # Scattered clouds night
        ("04d", "broken_clouds", (150, 150, 200, 255)),   # Broken clouds
        ("04n", "broken_clouds_night", (120, 120, 160, 255)), # Broken clouds night
        ("09d", "shower_rain", (100, 150, 200, 255)),     # Shower rain
        ("09n", "shower_rain_night", (80, 120, 160, 255)), # Shower rain night
        ("10d", "rain_day", (0, 150, 255, 255)),          # Rain day
        ("10n", "rain_night", (0, 120, 200, 255)),        # Rain night
        ("11d", "thunderstorm", (100, 100, 150, 255)),    # Thunderstorm
        ("11n", "thunderstorm_night", (80, 80, 120, 255)), # Thunderstorm night
        ("13d", "snow", (255, 255, 255, 255)),            # Snow
        ("13n", "snow_night", (200, 200, 255, 255)),      # Snow night
        ("50d", "mist", (180, 180, 180, 255)),            # Mist
        ("50n", "mist_night", (140, 140, 140, 255)),      # Mist night
    ]
    
    print("Creating weather icons...")
    print("=" * 40)
    
    for code, name, color in icon_definitions:
        # Create the icon
        icon = create_sample_icon(name, color, (32, 32))
        
        # Save the icon
        output_path = os.path.join(output_dir, f"{code}.png")
        icon.save(output_path)
        print(f"Created {name} -> {output_path}")
    
    print("\nWeather icons created successfully!")
    print(f"Icons saved to: {output_dir}/")
    print("\nThese are placeholder icons. To use the beautiful icons from your image:")
    print("1. Save your weather icons image as 'weather_icons_source.png'")
    print("2. Update this script to extract from the actual image")
    print("3. The weather display will automatically use these icons!")

def extract_from_source_image(image_path="weather_icons_source.png"):
    """Extract icons from the actual source image if available"""
    if not os.path.exists(image_path):
        print(f"Source image not found: {image_path}")
        print("Creating placeholder icons instead...")
        extract_weather_icons_from_grid()
        return
    
    try:
        img = Image.open(image_path)
        print(f"Found source image: {img.size}, mode: {img.mode}")
        
        # Assuming 4x3 grid as visible in the provided image
        grid_cols = 4
        grid_rows = 3
        
        icon_width = img.width // grid_cols
        icon_height = img.height // grid_rows
        
        print(f"Extracting {grid_cols}x{grid_rows} grid, icon size: {icon_width}x{icon_height}")
        
        # Icon mapping based on the image layout
        icon_mapping = [
            # Row 1: Sun, Moon, Clouds, Partly Cloudy Sun
            ("01d", "clear_day"),
            ("01n", "clear_night"), 
            ("03d", "scattered_clouds"),
            ("02d", "partly_cloudy_day"),
            
            # Row 2: More clouds, Light rain, Heavy rain, Thunderstorm
            ("04d", "broken_clouds"),
            ("09d", "shower_rain"),
            ("10d", "rain_day"),
            ("11d", "thunderstorm"),
            
            # Row 3: Storm with rain, Light snow, Heavy snow, Mixed
            ("11d", "thunderstorm_heavy"),
            ("13d", "snow_light"),
            ("13d", "snow_heavy"),  
            ("50d", "mist"),
        ]
        
        output_dir = "weather_icons"
        os.makedirs(output_dir, exist_ok=True)
        
        for row in range(grid_rows):
            for col in range(grid_cols):
                left = col * icon_width
                top = row * icon_height
                right = left + icon_width
                bottom = top + icon_height
                
                icon = img.crop((left, top, right, bottom))
                
                index = row * grid_cols + col
                if index < len(icon_mapping):
                    code, name = icon_mapping[index]
                    output_path = os.path.join(output_dir, f"{code}.png")
                    icon.save(output_path)
                    print(f"Extracted {name} -> {output_path}")
        
        # Create night versions by copying day versions for weather that looks similar
        night_copies = [
            ("02d", "02n"), ("03d", "03n"), ("04d", "04n"),
            ("09d", "09n"), ("10d", "10n"), ("11d", "11n"),
            ("13d", "13n"), ("50d", "50n")
        ]
        
        for day_code, night_code in night_copies:
            day_path = os.path.join(output_dir, f"{day_code}.png")
            night_path = os.path.join(output_dir, f"{night_code}.png")
            if os.path.exists(day_path):
                day_icon = Image.open(day_path)
                # Slightly darken for night version
                pixels = day_icon.load()
                for y in range(day_icon.height):
                    for x in range(day_icon.width):
                        if day_icon.mode == 'RGBA':
                            r, g, b, a = pixels[x, y]
                            # Darken slightly for night
                            r = int(r * 0.8)
                            g = int(g * 0.8) 
                            b = min(255, int(b * 1.1))  # Slight blue tint
                            pixels[x, y] = (r, g, b, a)
                        else:
                            r, g, b = pixels[x, y]
                            r = int(r * 0.8)
                            g = int(g * 0.8)
                            b = min(255, int(b * 1.1))
                            pixels[x, y] = (r, g, b)
                
                day_icon.save(night_path)
                print(f"Created night version: {night_code}.png")
        
        print(f"\nSuccessfully extracted icons from {image_path}!")
        
    except Exception as e:
        print(f"Error processing source image: {e}")
        print("Creating placeholder icons instead...")
        extract_weather_icons_from_grid()

if __name__ == "__main__":
    print("Weather Icon Extractor")
    print("=" * 40)
    
    # Try to extract from source image, fall back to placeholders
    extract_from_source_image("weather_icons_source.png")