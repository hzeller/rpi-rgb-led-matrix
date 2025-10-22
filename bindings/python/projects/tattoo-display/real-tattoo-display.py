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

# Add shared components to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette
from config_manager import ConfigManager
from image_utils import ImageUtils

class RealTattooDisplay(MatrixBase):
    def __init__(self, tattoo_type="all", speed=5, enhance=False, remove_background=True, 
                 background_threshold=200, advanced_bg_removal=False, local_images=False, no_download=False):
        super().__init__(hardware_mapping='adafruit-hat-pwm')
        
        # Initialize managers
        self.font_manager = FontManager()
        self.colors = ColorPalette('default')
        self.config = ConfigManager()
        self.image_utils = ImageUtils()
        
        # Configuration
        self.tattoo_type = tattoo_type
        self.speed = speed
        self.enhance = enhance
        self.remove_background = remove_background
        self.background_threshold = background_threshold
        self.advanced_bg_removal = advanced_bg_removal
        self.local_images = local_images
        self.no_download = no_download
        
        # Initialize display components
        self.setup_fonts_and_colors()
        
    def setup_fonts_and_colors(self):
        """Setup fonts and colors for the display"""
        # Load fonts using font manager
        self.title_font = self.font_manager.get_font('medium')
        self.subtitle_font = self.font_manager.get_font('small')
        
        # Setup colors
        self.text_color = self.colors.get_color('WHITE')
        self.shadow_color = self.colors.get_color('BLACK')
        
    def get_writable_directory(self):
        """Find a writable directory for storing images"""
        # Try different locations in order of preference
        possible_dirs = [
            "tattoo_images",  # Current directory
            os.path.expanduser("~/tattoo_images"),  # Home directory
            "/tmp/tattoo_images",  # Temp directory (Linux/Mac)
            os.path.join(os.getenv('TEMP', '/tmp'), 'tattoo_images')  # Windows temp
        ]
        
        for directory in possible_dirs:
            try:
                # Test if we can create the directory
                test_dir = directory + "_test"
                os.makedirs(test_dir, exist_ok=True)
                os.rmdir(test_dir)
                return directory
            except (PermissionError, OSError):
                continue
        
        print("Warning: No writable directory found. Images will be temporary only.")
        return None
        
    def setup(self):
        """Initialize and download tattoo images"""
        # Create images directory with proper error handling
        self.images_dir = self.get_writable_directory()
        
        if self.images_dir and not os.path.exists(self.images_dir):
            try:
                os.makedirs(self.images_dir)
                print(f"Created images directory: {self.images_dir}")
            except PermissionError:
                print(f"Warning: Cannot create directory {self.images_dir}. Using temporary images only.")
                self.images_dir = None
        
        # Traditional tattoo image URLs mapped to designs
        self.tattoo_urls = {
            'anchor': 'https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQLPU-ENS6le3fi4YwZTh6FF9NzqEqVzAF7nQ&s',
            'heart': 'https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcT_uQgp78o1k8SDUa3S62b3hT8zDL4e9h45WA&s',
            'rose': 'https://i.pinimg.com/736x/a1/b5/82/a1b5829e0600f07ac21a56f92e97f980.jpg',
            'swallow': 'https://i.pinimg.com/736x/db/7f/c3/db7fc3bdb6b4b41c4752077eca5800c3.jpg',
            'skull': 'https://i.pinimg.com/474x/eb/2f/75/eb2f75108653a27e4da172862e900313.jpg',
            'dagger': 'https://www.shutterstock.com/image-vector/traditional-dagger-tattoo-vector-design-260nw-2436282185.jpg'
        }
        
        # Download or load images
        self.tattoo_images = {}
        if self.no_download:
            print("Using placeholder images only (no download)")
            self.create_all_placeholders()
        elif self.local_images:
            self.load_local_images()
        else:
            self.download_images()
            
    def download_image(self, name, url):
        """Download a single tattoo image"""
        try:
            print(f"Downloading {name} tattoo...")
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            
            # Try to save locally for future use (if we have a writable directory)
            if self.images_dir:
                try:
                    filename = os.path.join(self.images_dir, f"{name}_tattoo.jpg")
                    with open(filename, 'wb') as f:
                        f.write(response.content)
                    print(f"✓ {name.capitalize()} tattoo saved to {filename}")
                except (PermissionError, OSError) as e:
                    print(f"Warning: Could not save {name} tattoo locally: {e}")
            
            # Load and process the image (always from memory)
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
        
    def create_all_placeholders(self):
        """Create placeholder images for all tattoos"""
        for name in self.tattoo_urls.keys():
            self.tattoo_images[name] = self.create_placeholder(name)
            print(f"✓ Created placeholder for {name.capitalize()} tattoo")
        
    def load_local_images(self):
        """Load images from local directory"""
        if not self.images_dir:
            print("No local directory available, will create placeholders")
            for name in self.tattoo_urls.keys():
                self.tattoo_images[name] = self.create_placeholder(name)
            return
            
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
        
        # Remove background if requested
        if self.args.remove_background:
            try:
                if self.args.advanced_bg_removal:
                    image = self.remove_background_advanced(image)
                else:
                    image = self.remove_background(image)
                print(f"   🎭 Background removed from {name} tattoo")
            except Exception as e:
                print(f"   ⚠️  Background removal failed for {name}: {e}")
                print(f"   📦 Install numpy with: pip install numpy")
        
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
        
    def remove_background(self, image):
        """Remove white/light background from tattoo images"""
        try:
            import numpy as np
            return self.remove_background_numpy(image)
        except ImportError:
            print("   📦 numpy not available, using basic background removal")
            return self.remove_background_basic(image)
    
    def remove_background_basic(self, image):
        """Basic background removal without numpy"""
        # Create a new image with transparency
        width, height = image.size
        new_image = Image.new('RGB', (width, height), (0, 0, 0))
        
        threshold = self.args.background_threshold
        
        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                
                # Calculate brightness
                brightness = r * 0.299 + g * 0.587 + b * 0.114
                
                # If pixel is dark enough (not background), keep it
                if brightness < threshold and not (r > threshold and g > threshold and b > threshold):
                    new_image.putpixel((x, y), (r, g, b))
                # Otherwise leave it black (background removed)
        
        return new_image
        
    def remove_background_numpy(self, image):
        """Remove white/light background from tattoo images using numpy"""
        import numpy as np
        
        # Convert PIL image to numpy array
        img_array = np.array(image)
        
        # Create a mask for light pixels (likely background)
        # Check if pixels are close to white/light colors
        threshold = self.args.background_threshold
        
        # Calculate brightness for each pixel (weighted average of RGB)
        brightness = (img_array[:, :, 0] * 0.299 + 
                     img_array[:, :, 1] * 0.587 + 
                     img_array[:, :, 2] * 0.114)
        
        # Create mask where bright pixels (background) are True
        background_mask = brightness > threshold
        
        # Also check for near-white pixels
        white_mask = ((img_array[:, :, 0] > threshold) & 
                     (img_array[:, :, 1] > threshold) & 
                     (img_array[:, :, 2] > threshold))
        
        # Combine masks
        combined_mask = background_mask | white_mask
        
        # Set background pixels to black (transparent effect)
        img_array[combined_mask] = [0, 0, 0]
        
        # Convert back to PIL Image
        return Image.fromarray(img_array)
        
    def remove_background_advanced(self, image):
        """Advanced background removal using edge detection and flood fill"""
        try:
            import numpy as np
        except ImportError:
            print("   📦 numpy required for advanced background removal, falling back to basic")
            return self.remove_background_basic(image)
            
        from PIL import ImageFilter, ImageOps
        
        # Convert to grayscale for edge detection
        gray = image.convert('L')
        
        # Apply edge detection to find the tattoo outline
        edges = gray.filter(ImageFilter.FIND_EDGES)
        
        # Enhance edges
        edges = ImageOps.autocontrast(edges)
        
        # Convert back to numpy for processing
        img_array = np.array(image)
        edge_array = np.array(edges)
        
        # Create mask based on edges and brightness
        threshold = self.args.background_threshold
        brightness = (img_array[:, :, 0] * 0.299 + 
                     img_array[:, :, 1] * 0.587 + 
                     img_array[:, :, 2] * 0.114)
        
        # Background is bright AND has low edge intensity
        background_mask = (brightness > threshold) & (edge_array < 30)
        
        # Set background to black
        img_array[background_mask] = [0, 0, 0]
        
        return Image.fromarray(img_array)
        
    def create_placeholder(self, name):
        """Create a placeholder image if download fails"""
        matrix_width = self.args.led_cols
        matrix_height = self.args.led_rows
        
        # Create more interesting colored placeholders with patterns
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
        
        # Add a simple pattern to make placeholders more interesting
        from PIL import ImageDraw
        draw = ImageDraw.Draw(image)
        
        # Create a simple border
        border_color = tuple(min(255, c + 50) for c in color)
        draw.rectangle([0, 0, matrix_width-1, matrix_height-1], outline=border_color, width=2)
        
        # Add a center rectangle
        center_x, center_y = matrix_width // 2, matrix_height // 2
        rect_size = min(matrix_width, matrix_height) // 3
        draw.rectangle([center_x - rect_size//2, center_y - rect_size//2,
                       center_x + rect_size//2, center_y + rect_size//2],
                      fill=border_color)
        
        return image
        
    def draw_text_with_shadow(self, font, x, y, color, shadow_color, text):
        """Draw text with shadow for better visibility"""
        # Draw shadow (offset by 1 pixel)
        self.draw_text(font, x + 1, y + 1, shadow_color, text)
        # Draw main text
        self.draw_text(font, x, y, color, text)
        
    def display_image(self, image, title):
        """Display a tattoo image on the canvas"""
        # Convert PIL image to matrix format
        width, height = image.size
        
        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                color = self.colors.create_color(r, g, b)
                self.set_pixel(x, y, color)
        
        # Add title at the top if there's space
        if height < self.height - 8:
            title_text = title.upper()
            text_width = len(title_text) * 6  # Approximate width
            x_pos = max(1, (self.width - text_width) // 2)
            
            self.draw_text_with_shadow(self.title_font, x_pos, 7, 
                                     self.text_color, self.shadow_color, title_text)

    def run(self):
        """Main display loop"""
        print("Setting up Real Traditional Tattoo Display...")
        
        try:
            self.setup()
        except Exception as e:
            print(f"Setup failed: {e}")
            print("Try running with --no-download flag to skip image downloading")
            return
        
        if not self.tattoo_images:
            print("No tattoo images loaded! Try running with --no-download to use placeholders.")
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
def main():
    """Main function to run tattoo display with argument parsing"""
    parser = argparse.ArgumentParser(description='Traditional Tattoo Display')
    parser.add_argument("-t", "--tattoo-type", help="Specific tattoo to display", default="all", type=str)
    parser.add_argument("-s", "--speed", help="Animation speed (seconds)", default=5.0, type=float)
    parser.add_argument("--enhance", help="Enhance contrast", action="store_true")
    parser.add_argument("--remove-background", help="Remove backgrounds", action="store_true", default=True)
    parser.add_argument("--background-threshold", help="Background threshold", type=int, default=200)
    parser.add_argument("--advanced-bg-removal", help="Advanced background removal", action="store_true")
    parser.add_argument("--local-images", help="Use local images", action="store_true")
    parser.add_argument("--no-download", help="Skip downloading", action="store_true")
    
    args = parser.parse_args()
    
    try:
        tattoo_display = RealTattooDisplay(
            tattoo_type=args.tattoo_type,
            speed=args.speed,
            enhance=args.enhance,
            remove_background=args.remove_background,
            background_threshold=args.background_threshold,
            advanced_bg_removal=args.advanced_bg_removal,
            local_images=args.local_images,
            no_download=args.no_download
        )
        
        print("🎨 Starting tattoo display... Press CTRL-C to stop")
        tattoo_display.run()
        
    except KeyboardInterrupt:
        print("\n🛑 Tattoo display stopped.")
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()