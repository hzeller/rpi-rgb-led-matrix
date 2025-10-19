#!/usr/bin/env python
import time
import sys
import os
import argparse
from samplebase import SampleBase
from PIL import Image, ImageEnhance, ImageOps
import threading
from dotenv import load_dotenv
import requests
from io import BytesIO


class GifViewer(SampleBase):
    def __init__(self, *args, **kwargs):
        super(GifViewer, self).__init__(*args, **kwargs)
        
        # Hardcoded GIF URL
        self.gif_url = "https://www.thisiscolossal.com/wp-content/uploads/2014/03/120430.gif"
        
        # Override default matrix settings from environment variables
        self.parser.set_defaults(
            led_rows=int(os.environ.get('MATRIX_ROWS', '32')),
            led_cols=int(os.environ.get('MATRIX_COLS', '64')),
            led_chain=int(os.environ.get('MATRIX_CHAIN_LENGTH', '1')),
            led_parallel=int(os.environ.get('MATRIX_PARALLEL', '1')),
            led_brightness=int(os.environ.get('MATRIX_BRIGHTNESS', '100')),
            led_gpio_mapping=os.environ.get('MATRIX_GPIO_MAPPING', 'adafruit-hat-pwm'),
            led_pwm_bits=int(os.environ.get('MATRIX_PWM_BITS', '11')),
            led_slowdown_gpio=int(os.environ.get('MATRIX_SLOWDOWN_GPIO', '1'))
        )
        
        # Add gif-specific arguments with environment variable defaults (gif_file is now optional)
        self.parser.add_argument("gif_source", nargs='?', help="Path to GIF file or URL to display (optional - will use hardcoded URL if not provided)")
        self.parser.add_argument("--url", type=str, help="URL of GIF to download and display")
        self.parser.add_argument("--fit-mode", choices=['stretch', 'fit', 'fill', 'center'], 
                               default=os.environ.get('GIF_FIT_MODE', 'fit'),
                               help="How to fit the gif to matrix: fit=maintain aspect ratio with letterbox (default), fill=maintain aspect ratio and crop, stretch=ignore aspect ratio, center=no scaling, just center")
        self.parser.add_argument("--speed", type=float, 
                               default=float(os.environ.get('GIF_SPEED', '1.0')),
                               help="Playback speed multiplier (1.0=normal, 2.0=double speed, 0.5=half speed)")
        self.parser.add_argument("--brightness-adjust", type=float, 
                               default=float(os.environ.get('GIF_BRIGHTNESS_ADJUST', '1.0')),
                               help="Brightness adjustment multiplier (1.0=normal, >1.0=brighter, <1.0=darker)")
        self.parser.add_argument("--contrast-adjust", type=float, 
                               default=float(os.environ.get('GIF_CONTRAST_ADJUST', '1.0')),
                               help="Contrast adjustment multiplier (1.0=normal, >1.0=higher contrast)")
        self.parser.add_argument("--saturation-adjust", type=float, 
                               default=float(os.environ.get('GIF_SATURATION_ADJUST', '1.0')),
                               help="Saturation adjustment multiplier (1.0=normal, >1.0=more saturated)")
        self.parser.add_argument("--loop-count", type=int, 
                               default=int(os.environ.get('GIF_LOOP_COUNT', '0')),
                               help="Number of times to loop the gif (0=infinite)")
        self.parser.add_argument("--background-color", 
                               default=os.environ.get('GIF_BACKGROUND_COLOR', 'black'),
                               help="Background color for letterbox/centering (black, white, or hex like #FF0000)")
        self.parser.add_argument("--config-file", 
                               default=os.environ.get('GIF_CONFIG_FILE', '.env'),
                               help="Path to environment configuration file")

    def validate_gif_file(self, gif_path):
        """Validate that the file exists and is a valid gif."""
        if not os.path.isfile(gif_path):
            print(f"Error: File '{gif_path}' not found")
            return False
            
        try:
            with Image.open(gif_path) as test_gif:
                if not getattr(test_gif, "is_animated", False):
                    # Check if it has multiple frames
                    try:
                        test_gif.seek(1)
                        test_gif.seek(0)  # Reset to first frame
                    except EOFError:
                        print(f"Error: '{gif_path}' is not an animated image")
                        return False
                return True
        except Exception as e:
            print(f"Error: Cannot open '{gif_path}' as an image: {e}")
            return False

    def is_url(self, source):
        """Check if the source is a URL."""
        return source and (source.startswith('http://') or source.startswith('https://'))

    def parse_background_color(self, color_str):
        """Parse background color string to RGB tuple."""
        color_str = color_str.lower()
        if color_str == "black":
            return (0, 0, 0)
        elif color_str == "white":
            return (255, 255, 255)
        elif color_str.startswith("#") and len(color_str) == 7:
            try:
                return tuple(int(color_str[i:i+2], 16) for i in (1, 3, 5))
            except ValueError:
                print(f"Warning: Invalid hex color '{color_str}', using black")
                return (0, 0, 0)
        else:
            print(f"Warning: Unknown color '{color_str}', using black")
            return (0, 0, 0)

    def download_gif_from_url(self, url):
        """Download GIF from URL and return as PIL Image object."""
        print(f"Downloading GIF from: {url}")
        try:
            response = requests.get(url, timeout=30)
            response.raise_for_status()
            
            # Load image from downloaded content
            gif_data = BytesIO(response.content)
            gif = Image.open(gif_data)
            
            print(f"Successfully downloaded GIF ({len(response.content)} bytes)")
            return gif, gif_data
            
        except requests.exceptions.RequestException as e:
            print(f"Error downloading GIF: {e}")
            return None, None
        except Exception as e:
            print(f"Error loading downloaded GIF: {e}")
            return None, None

    def resize_frame(self, frame, target_width, target_height, fit_mode, background_color):
        """Resize frame according to the specified fit mode, ensuring it fits within display bounds."""
        original_width, original_height = frame.size
        
        # Ensure target dimensions are positive and reasonable
        target_width = max(1, target_width)
        target_height = max(1, target_height)
        
        if fit_mode == 'stretch':
            # Ignore aspect ratio, stretch to fill exactly
            return frame.resize((target_width, target_height), Image.LANCZOS)
        
        elif fit_mode == 'center':
            # No scaling, just center the image
            if original_width <= target_width and original_height <= target_height:
                # Image is smaller, center it with background
                new_image = Image.new('RGB', (target_width, target_height), background_color)
                x_offset = (target_width - original_width) // 2
                y_offset = (target_height - original_height) // 2
                new_image.paste(frame, (x_offset, y_offset))
                return new_image
            else:
                # Image is larger, crop from center to fit exactly
                left = max(0, (original_width - target_width) // 2)
                top = max(0, (original_height - target_height) // 2)
                right = min(original_width, left + target_width)
                bottom = min(original_height, top + target_height)
                cropped = frame.crop((left, top, right, bottom))
                
                # If cropped size doesn't match target (edge case), resize to exact dimensions
                if cropped.size != (target_width, target_height):
                    cropped = cropped.resize((target_width, target_height), Image.LANCZOS)
                return cropped
        
        elif fit_mode == 'fit':
            # Maintain aspect ratio, letterbox if needed - ensures nothing extends beyond bounds
            original_ratio = original_width / original_height
            target_ratio = target_width / target_height
            
            if original_ratio > target_ratio:
                # Image is wider, scale by width to fit
                new_width = target_width
                new_height = int(target_width / original_ratio)
            else:
                # Image is taller, scale by height to fit
                new_height = target_height
                new_width = int(target_height * original_ratio)
            
            # Resize frame to calculated dimensions (guaranteed to fit within target)
            resized_frame = frame.resize((new_width, new_height), Image.LANCZOS)
            
            # Create background and center the resized image
            new_image = Image.new('RGB', (target_width, target_height), background_color)
            x_offset = (target_width - new_width) // 2
            y_offset = (target_height - new_height) // 2
            new_image.paste(resized_frame, (x_offset, y_offset))
            return new_image
        
        elif fit_mode == 'fill':
            # Maintain aspect ratio, crop if needed to fill completely - ensures exact dimensions
            original_ratio = original_width / original_height
            target_ratio = target_width / target_height
            
            if original_ratio > target_ratio:
                # Image is wider, scale by height and crop width
                new_height = target_height
                new_width = int(target_height * original_ratio)
            else:
                # Image is taller, scale by width and crop height  
                new_width = target_width
                new_height = int(target_width / original_ratio)
            
            # Resize to calculated dimensions
            resized_frame = frame.resize((new_width, new_height), Image.LANCZOS)
            
            # Crop from center to exact target dimensions
            left = max(0, (new_width - target_width) // 2)
            top = max(0, (new_height - target_height) // 2)
            right = min(new_width, left + target_width)
            bottom = min(new_height, top + target_height)
            
            final_frame = resized_frame.crop((left, top, right, bottom))
            
            # Ensure exact dimensions (safety check)
            if final_frame.size != (target_width, target_height):
                final_frame = final_frame.resize((target_width, target_height), Image.LANCZOS)
            
            return final_frame
    
    def enhance_frame(self, frame):
        """Apply brightness, contrast, and saturation adjustments."""
        if self.args.brightness_adjust != 1.0:
            enhancer = ImageEnhance.Brightness(frame)
            frame = enhancer.enhance(self.args.brightness_adjust)
        
        if self.args.contrast_adjust != 1.0:
            enhancer = ImageEnhance.Contrast(frame)
            frame = enhancer.enhance(self.args.contrast_adjust)
        
        if self.args.saturation_adjust != 1.0:
            enhancer = ImageEnhance.Color(frame)
            frame = enhancer.enhance(self.args.saturation_adjust)
        
        return frame

    def preprocess_gif(self, gif_source, url=None):
        """Preprocess all gif frames for optimized playback."""
        print("Loading and preprocessing GIF frames...")
        
        # Determine if we're using URL or local file
        if url or self.is_url(gif_source):
            download_url = url or gif_source
            gif, gif_data = self.download_gif_from_url(download_url)
            if gif is None:
                return None, None, None, None
            source_name = f"Downloaded GIF ({download_url})"
        elif gif_source == "HARDCODED":
            gif, gif_data = self.download_gif_from_url(self.gif_url)
            if gif is None:
                return None, None, None, None
            source_name = "Hardcoded GIF"
        else:
            try:
                gif = Image.open(gif_source)
                gif_data = None
                source_name = os.path.basename(gif_source)
            except Exception as e:
                print(f"Error opening GIF file: {e}")
                return None, None, None, None
        
        try:
            num_frames = gif.n_frames
        except Exception:
            print("Error: Provided image is not an animated GIF")
            return None, None, None, None
        
        # Get original GIF dimensions for info
        gif.seek(0)
        original_size = gif.size
        print(f"Original GIF size: {original_size[0]}x{original_size[1]}")
        print(f"Target matrix size: {self.matrix.width}x{self.matrix.height}")
        print(f"Fit mode: {self.args.fit_mode}")
        
        # Get frame durations for proper timing
        frame_durations = []
        canvases = []
        background_color = self.parse_background_color(self.args.background_color)
        
        print(f"Processing {num_frames} frames...")
        
        for frame_index in range(num_frames):
            if frame_index % 10 == 0 or frame_index == num_frames - 1:
                print(f"  Processing frame {frame_index + 1}/{num_frames}")
            
            gif.seek(frame_index)
            
            # Get frame duration (in milliseconds)
            duration = gif.info.get('duration', 100)  # Default to 100ms if not specified
            frame_durations.append(duration / 1000.0 / self.args.speed)  # Convert to seconds and apply speed
            
            # Copy frame to avoid modifying original
            frame = gif.copy()
            
            # Convert to RGB if needed
            if frame.mode != 'RGB':
                frame = frame.convert('RGB')
            
            # Resize according to fit mode - this ensures exact matrix dimensions
            frame = self.resize_frame(frame, self.matrix.width, self.matrix.height, 
                                    self.args.fit_mode, background_color)
            
            # Verify frame size matches matrix (safety check)
            if frame.size != (self.matrix.width, self.matrix.height):
                print(f"Warning: Frame size {frame.size} doesn't match matrix size {self.matrix.width}x{self.matrix.height}, forcing resize")
                frame = frame.resize((self.matrix.width, self.matrix.height), Image.LANCZOS)
            
            # Apply enhancements
            frame = self.enhance_frame(frame)
            
            # Create canvas and set image
            canvas = self.matrix.CreateFrameCanvas()
            canvas.SetImage(frame)
            canvases.append(canvas)
        
        gif.close()
        if gif_data:
            gif_data.close()
        print("Preprocessing complete!")
        return canvases, frame_durations, num_frames, source_name

    def run(self):
        """Main display loop."""
        # Determine gif source priority: --url flag > gif_source parameter > hardcoded URL
        if self.args.url:
            print(f"Using URL parameter: {self.args.url}")
            result = self.preprocess_gif(None, self.args.url)
        elif self.args.gif_source:
            if self.is_url(self.args.gif_source):
                print(f"Using URL from parameter: {self.args.gif_source}")
                result = self.preprocess_gif(self.args.gif_source)
            else:
                # Validate local gif file
                if not self.validate_gif_file(self.args.gif_source):
                    return False
                print(f"Using local file: {self.args.gif_source}")
                result = self.preprocess_gif(self.args.gif_source)
        else:
            print(f"Using hardcoded URL: {self.gif_url}")
            result = self.preprocess_gif("HARDCODED")
        
        # Check preprocessing result
        if result[0] is None:
            return False
        
        canvases, frame_durations, num_frames, source_name = result
        
        print(f"Displaying GIF: {source_name}")
        print(f"Matrix size: {self.matrix.width}x{self.matrix.height}")
        print(f"GPIO mapping: {getattr(self.args, 'led_gpio_mapping', 'default')}")
        print(f"Brightness: {getattr(self.args, 'led_brightness', 100)}%")
        print(f"Fit mode: {self.args.fit_mode}")
        print(f"Playback speed: {self.args.speed}x")
        if self.args.brightness_adjust != 1.0 or self.args.contrast_adjust != 1.0 or self.args.saturation_adjust != 1.0:
            print(f"Image adjustments - Brightness: {self.args.brightness_adjust}x, Contrast: {self.args.contrast_adjust}x, Saturation: {self.args.saturation_adjust}x")
        if self.args.loop_count > 0:
            print(f"Will loop {self.args.loop_count} times")
        else:
            print("Looping infinitely")
        print("Press CTRL-C to stop.")
        
        try:
            cur_frame = 0
            loop_counter = 0
            
            while True:
                # Display current frame
                self.matrix.SwapOnVSync(canvases[cur_frame])
                
                # Wait for frame duration
                time.sleep(frame_durations[cur_frame])
                
                # Advance to next frame
                cur_frame += 1
                if cur_frame >= num_frames:
                    cur_frame = 0
                    loop_counter += 1
                    
                    # Check loop count
                    if self.args.loop_count > 0 and loop_counter >= self.args.loop_count:
                        print("Completed all loops. Exiting.")
                        break
                        
        except KeyboardInterrupt:
            print("\nStopping GIF display...")
            return True


# Main function
if __name__ == "__main__":
    # Load environment variables
    load_dotenv()
    
    gif_viewer = GifViewer()
    if not gif_viewer.process():
        gif_viewer.print_help()
