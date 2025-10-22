#!/usr/bin/env python3
"""
Map descriptive weather icon names to OpenWeatherMap codes and resize them appropriately.
This script processes the beautiful weather icons in assets/weather_icons/ and creates
properly named and sized versions for the weather display.
"""

import os
from PIL import Image
import shutil

def create_icon_mapping():
    """Create mapping from OpenWeatherMap codes to descriptive icon filenames"""
    
    # Mapping OpenWeatherMap codes to the best matching descriptive icon names
    icon_mapping = {
        # Clear sky
        "01d": "Sun 1.png",                           # Clear day - bright sun
        "01n": "Moon + Star.png",                     # Clear night - moon and stars
        
        # Few clouds  
        "02d": "Sun 2.png",                           # Few clouds day - sun with some clouds
        "02n": "Dark Cloudy + Moon + Star.png",      # Few clouds night - moon with clouds
        
        # Scattered clouds
        "03d": "Cloud 2.png",                         # Scattered clouds day
        "03n": "Dark Cloudy + Moon.png",              # Scattered clouds night
        
        # Broken clouds / Overcast
        "04d": "Cloud 4.png",                         # Broken clouds day - more cloudy
        "04n": "Dark Cloud.png",                      # Broken clouds night - dark clouds
        
        # Shower rain
        "09d": "Cloudy + Heavy Rain.png",             # Shower rain day
        "09n": "Dark Cloudy + Heavy Rain.png",        # Shower rain night
        
        # Rain
        "10d": "Sunny + Rainy.png",                   # Rain day - rain with some sun
        "10n": "Dark Cloudy + Heavy Rain + Moon.png", # Rain night - rain with moon
        
        # Thunderstorm
        "11d": "Cloudy + Rainy + Lightning.png",      # Thunderstorm day
        "11n": "Dark Cloudy + Lightning + Moon.png",  # Thunderstorm night
        
        # Snow
        "13d": "Cloudy + Snowy.png",                  # Snow day
        "13n": "Dark Cloudy + Snowy + Moon.png",      # Snow night
        
        # Mist / Fog
        "50d": "Light Fog.png",                       # Mist day
        "50n": "Dark Fog.png",                        # Mist night
    }
    
    # Alternative mappings for variety (you can choose which ones to use)
    alternative_mappings = {
        # More sun options
        "01d_alt1": "Sun 1.png",
        "01d_alt2": "Sun 3.png",
        
        # More cloud options
        "03d_alt1": "Cloud 1.png",
        "03d_alt2": "Cloud 3.png",
        "04d_alt1": "Cloud 5.png",
        "04d_alt2": "Cloud 6.png",
        
        # More rain options
        "09d_alt": "Cloudy + Rainy.png",
        "10d_alt": "Umbrealla + Rainy.png",
        
        # Heavy weather alternatives
        "11d_heavy": "Sunny + Rainy + Lightning.png",
        "11n_heavy": "Dark Cloudy + Rainy +Lightning + Moon.png",
        
        # Snow alternatives
        "13d_alt": "Sunny + Snowy.png",
        "13d_light": "Snowflake.png",
        
        # Special weather
        "tornado": "Tornado.png",
        "rainbow": "Rainbow.png",
        "windy_day": "Sunny + Windy.png",
        "windy_night": "Dark Cloudy + Windy + Moon.png",
    }
    
    return icon_mapping, alternative_mappings

def resize_and_copy_icons(source_dir, target_dir, target_size=(32, 32)):
    """Resize and copy icons from source to target directory with OpenWeatherMap naming"""
    
    # Create target directory
    os.makedirs(target_dir, exist_ok=True)
    
    # Get the mapping
    icon_mapping, alternatives = create_icon_mapping()
    
    print(f"Processing weather icons from {source_dir}")
    print(f"Target size: {target_size}")
    print("=" * 60)
    
    processed_count = 0
    
    # Process main mappings
    for weather_code, source_filename in icon_mapping.items():
        source_path = os.path.join(source_dir, source_filename)
        target_filename = f"{weather_code}.png"
        target_path = os.path.join(target_dir, target_filename)
        
        if os.path.exists(source_path):
            try:
                # Open and process the image for LED matrix optimization
                with Image.open(source_path) as img:
                    # Convert to RGBA for proper transparency handling
                    if img.mode != 'RGBA':
                        img = img.convert('RGBA')
                    
                    # Resize with high quality resampling
                    resized_img = img.resize(target_size, Image.Resampling.LANCZOS)
                    
                    # Create RGB image with black background for LED matrix
                    rgb_img = Image.new('RGB', target_size, (0, 0, 0))
                    
                    # Paste the resized image using alpha channel as mask
                    if resized_img.mode == 'RGBA':
                        rgb_img.paste(resized_img, mask=resized_img.split()[3])
                    else:
                        rgb_img.paste(resized_img)
                    
                    # Enhance contrast and clean up artifacts for LED display
                    pixels = rgb_img.load()
                    for y in range(rgb_img.height):
                        for x in range(rgb_img.width):
                            r, g, b = pixels[x, y]
                            
                            # Calculate brightness
                            brightness = (0.299 * r + 0.587 * g + 0.114 * b)
                            
                            # Clean up dim pixels that create artifacts around icons
                            if brightness < 35:
                                pixels[x, y] = (0, 0, 0)
                            else:
                                # Enhance colors for better LED visibility
                                # Use gamma correction for better color reproduction
                                r = min(255, int(255 * ((r / 255) ** 0.8)))
                                g = min(255, int(255 * ((g / 255) ** 0.8)))
                                b = min(255, int(255 * ((b / 255) ** 0.8)))
                                pixels[x, y] = (r, g, b)
                    
                    # Save the optimized image
                    rgb_img.save(target_path, 'PNG')
                    
                    print(f"✓ {weather_code}: {source_filename} -> {target_filename}")
                    processed_count += 1
                    
            except Exception as e:
                print(f"✗ Error processing {source_filename}: {e}")
        else:
            print(f"✗ Source file not found: {source_filename}")
    
    print("=" * 60)
    print(f"Successfully processed {processed_count} weather icons")
    
    # List any unused source icons (for potential future mapping)
    print("\nUnused source icons (available for alternative mappings):")
    used_files = set(icon_mapping.values())
    all_files = [f for f in os.listdir(source_dir) if f.endswith('.png')]
    unused_files = [f for f in all_files if f not in used_files]
    
    for unused_file in sorted(unused_files):
        print(f"  - {unused_file}")
    
    return processed_count

def validate_weather_icons(icon_dir):
    """Validate that all required OpenWeatherMap icons exist"""
    
    required_codes = [
        "01d", "01n",  # Clear sky
        "02d", "02n",  # Few clouds
        "03d", "03n",  # Scattered clouds
        "04d", "04n",  # Broken clouds
        "09d", "09n",  # Shower rain
        "10d", "10n",  # Rain
        "11d", "11n",  # Thunderstorm
        "13d", "13n",  # Snow
        "50d", "50n",  # Mist
    ]
    
    print(f"\nValidating weather icons in {icon_dir}:")
    print("=" * 50)
    
    missing_icons = []
    existing_icons = []
    
    for code in required_codes:
        icon_path = os.path.join(icon_dir, f"{code}.png")
        if os.path.exists(icon_path):
            # Check file size and basic properties
            try:
                with Image.open(icon_path) as img:
                    print(f"✓ {code}.png - Size: {img.size}, Mode: {img.mode}")
                    existing_icons.append(code)
            except Exception as e:
                print(f"✗ {code}.png - Error: {e}")
                missing_icons.append(code)
        else:
            print(f"✗ {code}.png - Missing")
            missing_icons.append(code)
    
    print("=" * 50)
    print(f"Found: {len(existing_icons)} icons")
    print(f"Missing: {len(missing_icons)} icons")
    
    if missing_icons:
        print(f"Missing icons: {', '.join(missing_icons)}")
        return False
    else:
        print("All required weather icons are present! ✓")
        return True

def main():
    """Main function to process weather icons"""
    
    # Define paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    source_dir = os.path.join(script_dir, "assets", "weather_icons")
    target_dir = os.path.join(script_dir, "weather_icons")
    
    print("Weather Icon Mapping and Resizing Tool")
    print("=" * 60)
    print(f"Source directory: {source_dir}")
    print(f"Target directory: {target_dir}")
    
    # Check if source directory exists
    if not os.path.exists(source_dir):
        print(f"Error: Source directory not found: {source_dir}")
        return False
    
    # Process the icons
    processed_count = resize_and_copy_icons(source_dir, target_dir, target_size=(32, 32))
    
    if processed_count > 0:
        # Validate the results
        validation_success = validate_weather_icons(target_dir)
        
        if validation_success:
            print(f"\n🎉 Weather icons successfully processed!")
            print(f"Your weather display is ready to use these beautiful icons!")
        else:
            print(f"\n⚠️  Some icons may be missing. Check the validation results above.")
        
        return validation_success
    else:
        print("No icons were processed. Please check your source directory and file names.")
        return False

if __name__ == "__main__":
    success = main()
    if success:
        print("\nNext steps:")
        print("1. The weather icons are now ready in the weather_icons/ directory")
        print("2. Your weather-display.py will automatically use these icons")
        print("3. Run your weather display to see the beautiful new icons!")
    else:
        print("\nPlease check the error messages above and try again.")