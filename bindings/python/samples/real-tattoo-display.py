#!/usr/bin/env python3
# Real Traditional Tattoo Image Display for RGB LED matrix
# Downloads and displays actual traditional tattoo artwork

import time
import sys
import os
import requests
import argparse
from PIL import Image, ImageEnhance, ImageOps
from io import BytesIO
import threading

# Add the parent directory to Python path to import the rgbmatrix module
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from samplebase import SampleBase

class RealTattooDisplay(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RealTattooDisplay, self).__init__(*args, **kwargs)
        self.parser.add_argument("-t", "--tattoo-type", help="Specific tattoo to display: anchor, heart, rose, swallow, skull, dagger, all", default="all", type=str)
        self.parser.add_argument("-s", "--speed", help="Animation speed (seconds between images). Default: 5", default=5, type=float)
        self.parser.add_argument("--enhance", help="Enhance contrast for better LED display", action="store_true")
        self.parser.add_argument("--local-images", help="Use local images instead of downloading", action="store_true")
        
    def setup(self):
        """Initialize and download tattoo images"""
        # Create images directory
        self.images_dir = "tattoo_images"
        if not os.path.exists(self.images_dir):
            os.makedirs(self.images_dir)
        
        # Traditional tattoo image URLs mapped to designs
        self.tattoo_urls = {
            'anchor': 'https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQLPU-ENS6le3fi4YwZTh6FF9NzqEqVzAF7nQ&s',
            'heart': 'https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcT_uQgp78o1k8SDUa3S62b3hT8zDL4e9h45WA&s',
            'rose': 'https://i.pinimg.com/736x/a1/b5/82/a1b5829e0600f07ac21a56f92e97f980.jpg',
            'swallow': 'https://i.pinimg.com/736x/db/7f/c3/db7fc3bdb6b4b41c4752077eca5800c3.jpg',
            'skull': 'https://i.pinimg.com/474x/eb/2f/75/eb2f75108653a27e4da172862e900313.jpg',
            'dagger': 'https://www.shutterstock.com/image-vector/traditional-dagger-tattoo-vector-design-260nw-2436282185.jpg'
        }
        
        # Load fonts for titles
        try:
            self.title_font = graphics.Font()
            self.title_font.LoadFont("../../../fonts/7x13B.bdf")
            
            self.subtitle_font = graphics.Font()
            self.subtitle_font.LoadFont("../../../fonts/6x10.bdf")
        except:
            print("Warning: Could not load fonts. Titles may not display properly.")
            self.title_font = graphics.Font()
            self.subtitle_font = graphics.Font()
        
        # Colors for text overlays
        self.text_color = graphics.Color(255, 255, 255)  # White
        self.shadow_color = graphics.Color(0, 0, 0)      # Black shadow
        
        # Download or load images
        self.tattoo_images = {}
        if self.args.local_images:
            self.load_local_images()
        else:
            self.download_images()
            
    def download_image(self, name, url):
        """Download a single tattoo image"""
        try:
            print(f"Downloading {name} tattoo...")
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            
            # Save locally for future use
            filename = os.path.join(self.images_dir, f"{name}_tattoo.jpg")
            with open(filename, 'wb') as f:
                f.write(response.content)
            
            # Load and process the image
            image = Image.open(BytesIO(response.content))
            self.tattoo_images[name] = self.process_image(image, name)
            print(f"✓ {name.capitalize()} tattoo loaded successfully")
            
        except Exception as e:
            print(f"✗ Failed to download {name} tattoo: {e}")
            # Create a placeholder image
            self.tattoo_images[name] = self.create_placeholder(name)
            
    def download_images(self):
        """Download all tattoo images"""
        print("Downloading traditional tattoo images...")
        
        # Use threading to download multiple images simultaneously
        threads = []
        for name, url in self.tattoo_urls.items():
            thread = threading.Thread(target=self.download_image, args=(name, url))
            threads.append(thread)
            thread.start()
        
        # Wait for all downloads to complete
        for thread in threads:
            thread.join()
            
        print(f"Download complete! Loaded {len(self.tattoo_images)} tattoo images.")
        
    def load_local_images(self):
        """Load images from local directory"""
        print("Loading local tattoo images...")
        for name in self.tattoo_urls.keys():
            filename = os.path.join(self.images_dir, f"{name}_tattoo.jpg")
            if os.path.exists(filename):
                try:
                    image = Image.open(filename)
                    self.tattoo_images[name] = self.process_image(image, name)
                    print(f"✓ {name.capitalize()} tattoo loaded from local file")
                except Exception as e:
                    print(f"✗ Failed to load local {name} tattoo: {e}")
                    self.tattoo_images[name] = self.create_placeholder(name)
            else:
                print(f"✗ Local {name} tattoo not found, creating placeholder")
                self.tattoo_images[name] = self.create_placeholder(name)
                
    def process_image(self, image, name):
        """Process image for LED matrix display"""
        # Convert to RGB if needed
        if image.mode != 'RGB':
            image = image.convert('RGB')
        
        # Get matrix dimensions
        matrix_width = self.args.led_cols
        matrix_height = self.args.led_rows
        
        # Calculate aspect ratio preserving resize
        img_ratio = image.width / image.height
        matrix_ratio = matrix_width / matrix_height
        
        if img_ratio > matrix_ratio:
            # Image is wider - fit to width
            new_width = matrix_width
            new_height = int(matrix_width / img_ratio)
        else:
            # Image is taller - fit to height  
            new_height = matrix_height
            new_width = int(matrix_height * img_ratio)
        
        # Resize image
        image = image.resize((new_width, new_height), Image.Resampling.LANCZOS)
        
        # Create a black canvas of matrix size
        canvas_image = Image.new('RGB', (matrix_width, matrix_height), (0, 0, 0))
        
        # Center the resized image on the canvas
        x_offset = (matrix_width - new_width) // 2
        y_offset = (matrix_height - new_height) // 2
        canvas_image.paste(image, (x_offset, y_offset))
        
        # Enhance for LED display if requested
        if self.args.enhance:
            # Increase contrast
            enhancer = ImageEnhance.Contrast(canvas_image)
            canvas_image = enhancer.enhance(1.5)
            
            # Increase saturation
            enhancer = ImageEnhance.Color(canvas_image)
            canvas_image = enhancer.enhance(1.3)
            
            # Slightly increase brightness
            enhancer = ImageEnhance.Brightness(canvas_image)
            canvas_image = enhancer.enhance(1.1)
        
        return canvas_image
        
    def create_placeholder(self, name):
        """Create a placeholder image if download fails"""
        matrix_width = self.args.led_cols
        matrix_height = self.args.led_rows
        
        # Create simple colored placeholder
        colors = {
            'anchor': (0, 100, 150),    # Navy blue
            'heart': (150, 0, 50),      # Deep red
            'rose': (100, 50, 50),      # Rose color
            'swallow': (50, 50, 100),   # Blue
            'skull': (100, 100, 100),   # Gray
            'dagger': (150, 150, 150)   # Light gray
        }
        
        color = colors.get(name, (100, 100, 100))
        image = Image.new('RGB', (matrix_width, matrix_height), color)
        
        return image
        
    def draw_text_with_shadow(self, canvas, font, x, y, color, shadow_color, text):
        """Draw text with shadow for better visibility"""
        # Draw shadow (offset by 1 pixel)
        graphics.DrawText(canvas, font, x + 1, y + 1, shadow_color, text)
        # Draw main text
        graphics.DrawText(canvas, font, x, y, color, text)
        
    def display_image(self, canvas, image, title):
        """Display a tattoo image on the canvas"""
        # Convert PIL image to matrix format
        width, height = image.size
        
        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                canvas.SetPixel(x, y, r, g, b)
        
        # Add title at the top if there's space
        if height < self.args.led_rows - 8:
            title_text = title.upper()
            text_width = len(title_text) * 6  # Approximate width
            x_pos = max(1, (self.args.led_cols - text_width) // 2)
            
            self.draw_text_with_shadow(canvas, self.title_font, x_pos, 7, 
                                     self.text_color, self.shadow_color, title_text)
        
        # Add timestamp in corner
        current_time = time.strftime("%H:%M")
        self.draw_text_with_shadow(canvas, self.subtitle_font, 2, self.args.led_rows - 2,
                                 self.text_color, self.shadow_color, current_time)

    def run(self):
        """Main display loop"""
        print("Setting up Real Traditional Tattoo Display...")
        self.setup()
        
        if not self.tattoo_images:
            print("No tattoo images loaded! Exiting.")
            return
        
        canvas = self.matrix.CreateFrameCanvas()
        
        # Determine which tattoos to show
        if self.args.tattoo_type == "all":
            tattoo_list = list(self.tattoo_images.keys())
        else:
            if self.args.tattoo_type in self.tattoo_images:
                tattoo_list = [self.args.tattoo_type]
            else:
                print(f"Unknown tattoo type: {self.args.tattoo_type}")
                print(f"Available types: {', '.join(self.tattoo_images.keys())}, all")
                return
        
        current_tattoo = 0
        print(f"Starting display with {len(tattoo_list)} tattoo(s)")
        print("Press Ctrl+C to exit")
        
        while True:
            canvas.Clear()
            
            # Display current tattoo
            tattoo_name = tattoo_list[current_tattoo]
            tattoo_image = self.tattoo_images[tattoo_name]
            
            self.display_image(canvas, tattoo_image, tattoo_name)
            
            canvas = self.matrix.SwapOnVSync(canvas)
            
            print(f"Displaying: {tattoo_name.capitalize()} tattoo")
            time.sleep(self.args.speed)
            
            # Move to next tattoo
            current_tattoo = (current_tattoo + 1) % len(tattoo_list)

# Main execution
if __name__ == "__main__":
    tattoo_display = RealTattooDisplay()
    if not tattoo_display.process():
        tattoo_display.print_help()