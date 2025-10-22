#!/usr/bin/env python
import time
import sys
import os
import argparse
import warnings

# Add path to access samplebase from samples directory
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/../../samples'))
from samplebase import SampleBase
from PIL import Image, ImageEnhance, ImageOps
import cv2
import threading
from dotenv import load_dotenv
import requests
from io import BytesIO
import numpy as np

# Suppress Pillow deprecation warnings for cleaner output
warnings.filterwarnings("ignore", category=DeprecationWarning, module="PIL")

try:
    import yt_dlp
    YT_DLP_AVAILABLE = True
except ImportError:
    YT_DLP_AVAILABLE = False
    print("Warning: yt-dlp not available. YouTube and other platform URLs may not work.")
    print("Install with: pip install yt-dlp")


class VideoPlayer(SampleBase):
    def __init__(self, *args, **kwargs):
        super(VideoPlayer, self).__init__(*args, **kwargs)
        
        # Hardcoded video URL for demonstration
        self.video_url = "https://preview.redd.it/xddbh2ffcxvf1.gif?width=821&format=mp4&s=46b2a913d08c3a4eed8bc7f19ad182306aca8edc"
        
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
        
        # Add video-specific arguments with environment variable defaults
        self.parser.add_argument("video_source", nargs='?', help="Path to video file or URL to display (optional - will use hardcoded URL if not provided)")
        self.parser.add_argument("--url", type=str, help="URL of video to download and display")
        self.parser.add_argument("--fit-mode", choices=['stretch', 'fit', 'fill', 'center'], 
                               default=os.environ.get('VIDEO_FIT_MODE', 'fit'),
                               help="How to fit the video to matrix: fit=maintain aspect ratio with letterbox (default), fill=maintain aspect ratio and crop, stretch=ignore aspect ratio, center=no scaling, just center")
        self.parser.add_argument("--speed", type=float, 
                               default=float(os.environ.get('VIDEO_SPEED', '1.0')),
                               help="Playback speed multiplier (1.0=normal, 2.0=double speed, 0.5=half speed)")
        self.parser.add_argument("--brightness-adjust", type=float, 
                               default=float(os.environ.get('VIDEO_BRIGHTNESS_ADJUST', '1.0')),
                               help="Brightness adjustment multiplier (1.0=normal, >1.0=brighter, <1.0=darker)")
        self.parser.add_argument("--contrast-adjust", type=float, 
                               default=float(os.environ.get('VIDEO_CONTRAST_ADJUST', '1.0')),
                               help="Contrast adjustment multiplier (1.0=normal, >1.0=higher contrast)")
        self.parser.add_argument("--saturation-adjust", type=float, 
                               default=float(os.environ.get('VIDEO_SATURATION_ADJUST', '1.0')),
                               help="Saturation adjustment multiplier (1.0=normal, >1.0=more saturated)")
        self.parser.add_argument("--loop-count", type=int, 
                               default=int(os.environ.get('VIDEO_LOOP_COUNT', '0')),
                               help="Number of times to loop the video (0=infinite)")
        self.parser.add_argument("--background-color", 
                               default=os.environ.get('VIDEO_BACKGROUND_COLOR', 'black'),
                               help="Background color for letterbox/centering (black, white, or hex like #FF0000)")
        self.parser.add_argument("--fps-override", type=float,
                               default=float(os.environ.get('VIDEO_FPS_OVERRIDE', '0')),
                               help="Override video FPS (0=use original fps)")
        self.parser.add_argument("--start-time", type=float,
                               default=float(os.environ.get('VIDEO_START_TIME', '0')),
                               help="Start playback at this time in seconds")
        self.parser.add_argument("--end-time", type=float,
                               default=float(os.environ.get('VIDEO_END_TIME', '0')),
                               help="End playback at this time in seconds (0=play to end)")
        self.parser.add_argument("--config-file", 
                               default=os.environ.get('VIDEO_CONFIG_FILE', '.env'),
                               help="Path to environment configuration file")
        self.parser.add_argument("--cache-frames", type=int,
                               default=int(os.environ.get('VIDEO_CACHE_FRAMES', '0')),
                               help="Number of frames to preload and cache (0=no caching, improves performance but uses more memory)")
        self.parser.add_argument("--verbose", action="store_true",
                               help="Enable verbose output for debugging")
        self.parser.add_argument("--quiet", action="store_true",
                               help="Suppress OpenCV and streaming messages for cleaner output")

    def validate_video_file(self, video_path):
        """Validate that the file exists and is a valid video."""
        if not os.path.isfile(video_path):
            print(f"Error: File '{video_path}' not found")
            return False
            
        try:
            cap = cv2.VideoCapture(video_path)
            if not cap.isOpened():
                print(f"Error: Cannot open '{video_path}' as a video file")
                return False
            
            # Check if video has frames
            ret, frame = cap.read()
            cap.release()
            
            if not ret:
                print(f"Error: '{video_path}' contains no readable frames")
                return False
                
            return True
        except Exception as e:
            print(f"Error: Cannot process '{video_path}' as a video: {e}")
            return False

    def is_url(self, source):
        """Check if the source is a URL."""
        return source and (source.startswith('http://') or source.startswith('https://'))

    def is_video_platform_url(self, url):
        """Check if URL is from a video platform like YouTube, Vimeo, etc."""
        if not url:
            return False
        
        video_platforms = [
            'youtube.com', 'youtu.be', 'vimeo.com', 'dailymotion.com',
            'twitch.tv', 'tiktok.com', 'instagram.com', 'twitter.com',
            'facebook.com', 'reddit.com'
        ]
        
        return any(platform in url.lower() for platform in video_platforms)

    def extract_video_url_with_ytdlp(self, url):
        """Extract direct video URL using yt-dlp for video platforms."""
        if not YT_DLP_AVAILABLE:
            print("Error: yt-dlp is required for video platform URLs")
            print("Install with: pip install yt-dlp")
            return None
        
        print(f"Extracting video stream from: {url}")
        
        try:
            # Configure yt-dlp options for best quality that works with OpenCV
            ydl_opts = {
                'format': 'best[height<=720]/best[height<=480]/best',  # Prefer lower resolution for better performance
                'quiet': True,
                'no_warnings': True,
                'extractaudio': False,
                'noplaylist': True,
                # Add headers to improve streaming compatibility
                'http_headers': {
                    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36'
                }
            }
            
            with yt_dlp.YoutubeDL(ydl_opts) as ydl:
                # Extract video info
                info = ydl.extract_info(url, download=False)
                
                if 'url' in info:
                    video_url = info['url']
                    title = info.get('title', 'Unknown')
                    duration = info.get('duration', 0)
                    
                    print(f"Found video: {title}")
                    if duration:
                        print(f"Duration: {duration//60}:{duration%60:02d}")
                    
                    return video_url
                else:
                    print("Error: Could not extract video URL")
                    return None
                    
        except Exception as e:
            print(f"Error extracting video from platform: {e}")
            print("This might be due to platform restrictions or unsupported URL format")
            return None

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

    def download_video_from_url(self, url):
        """Download video from URL and save to temporary file."""
        # Check if this is a video platform URL that needs special handling
        if self.is_video_platform_url(url):
            # Extract direct video URL using yt-dlp
            direct_url = self.extract_video_url_with_ytdlp(url)
            if not direct_url:
                return None
            
            # For some platforms, we can use the direct URL with OpenCV without downloading
            print(f"Using direct stream URL from platform")
            return direct_url
        
        # Regular direct video URL download
        print(f"Downloading video from: {url}")
        try:
            response = requests.get(url, timeout=30, stream=True)
            response.raise_for_status()
            
            # Create temporary file
            import tempfile
            temp_file = tempfile.NamedTemporaryFile(delete=False, suffix='.mp4')
            
            # Download with progress indication
            total_size = int(response.headers.get('content-length', 0))
            downloaded = 0
            
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    temp_file.write(chunk)
                    downloaded += len(chunk)
                    if total_size > 0:
                        progress = (downloaded / total_size) * 100
                        print(f"\rDownload progress: {progress:.1f}%", end="", flush=True)
            
            temp_file.close()
            print(f"\nSuccessfully downloaded video ({downloaded} bytes)")
            return temp_file.name
            
        except requests.exceptions.RequestException as e:
            print(f"Error downloading video: {e}")
            return None
        except Exception as e:
            print(f"Error saving downloaded video: {e}")
            return None

    def resize_frame(self, frame, target_width, target_height, fit_mode, background_color):
        """Resize frame according to the specified fit mode."""
        height, width = frame.shape[:2]
        
        # Ensure target dimensions are positive
        target_width = max(1, target_width)
        target_height = max(1, target_height)
        
        if fit_mode == 'stretch':
            # Ignore aspect ratio, stretch to fill exactly
            return cv2.resize(frame, (target_width, target_height))
        
        elif fit_mode == 'center':
            # No scaling, just center the image
            if width <= target_width and height <= target_height:
                # Image is smaller, center it with background
                new_frame = np.full((target_height, target_width, 3), background_color, dtype=np.uint8)
                y_offset = (target_height - height) // 2
                x_offset = (target_width - width) // 2
                new_frame[y_offset:y_offset+height, x_offset:x_offset+width] = frame
                return new_frame
            else:
                # Image is larger, crop from center
                y_start = max(0, (height - target_height) // 2)
                x_start = max(0, (width - target_width) // 2)
                y_end = min(height, y_start + target_height)
                x_end = min(width, x_start + target_width)
                cropped = frame[y_start:y_end, x_start:x_end]
                
                # Ensure exact dimensions
                if cropped.shape[:2] != (target_height, target_width):
                    cropped = cv2.resize(cropped, (target_width, target_height))
                return cropped
        
        elif fit_mode == 'fit':
            # Maintain aspect ratio, letterbox if needed
            original_ratio = width / height
            target_ratio = target_width / target_height
            
            if original_ratio > target_ratio:
                # Image is wider, scale by width
                new_width = target_width
                new_height = int(target_width / original_ratio)
            else:
                # Image is taller, scale by height
                new_height = target_height
                new_width = int(target_height * original_ratio)
            
            # Resize frame
            resized_frame = cv2.resize(frame, (new_width, new_height))
            
            # Create background and center the resized image
            new_frame = np.full((target_height, target_width, 3), background_color, dtype=np.uint8)
            y_offset = (target_height - new_height) // 2
            x_offset = (target_width - new_width) // 2
            new_frame[y_offset:y_offset+new_height, x_offset:x_offset+new_width] = resized_frame
            return new_frame
        
        elif fit_mode == 'fill':
            # Maintain aspect ratio, crop if needed to fill completely
            original_ratio = width / height
            target_ratio = target_width / target_height
            
            if original_ratio > target_ratio:
                # Image is wider, scale by height and crop width
                new_height = target_height
                new_width = int(target_height * original_ratio)
            else:
                # Image is taller, scale by width and crop height
                new_width = target_width
                new_height = int(target_width / original_ratio)
            
            # Resize frame
            resized_frame = cv2.resize(frame, (new_width, new_height))
            
            # Crop from center to exact target dimensions
            y_start = max(0, (new_height - target_height) // 2)
            x_start = max(0, (new_width - target_width) // 2)
            y_end = min(new_height, y_start + target_height)
            x_end = min(new_width, x_start + target_width)
            
            final_frame = resized_frame[y_start:y_end, x_start:x_end]
            
            # Ensure exact dimensions
            if final_frame.shape[:2] != (target_height, target_width):
                final_frame = cv2.resize(final_frame, (target_width, target_height))
            
            return final_frame

    def enhance_frame(self, frame):
        """Apply brightness, contrast, and saturation adjustments using OpenCV."""
        if self.args.brightness_adjust != 1.0:
            # Adjust brightness by adding/subtracting value
            brightness_delta = int((self.args.brightness_adjust - 1.0) * 50)
            frame = cv2.convertScaleAbs(frame, alpha=1.0, beta=brightness_delta)
        
        if self.args.contrast_adjust != 1.0:
            # Adjust contrast by scaling values
            frame = cv2.convertScaleAbs(frame, alpha=self.args.contrast_adjust, beta=0)
        
        if self.args.saturation_adjust != 1.0:
            # Convert to HSV for saturation adjustment
            hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV).astype(np.float32)
            hsv[:, :, 1] = hsv[:, :, 1] * self.args.saturation_adjust
            hsv[:, :, 1] = np.clip(hsv[:, :, 1], 0, 255)
            frame = cv2.cvtColor(hsv.astype(np.uint8), cv2.COLOR_HSV2BGR)
        
        return frame

    def get_video_info(self, video_path):
        """Get video information including duration, fps, and frame count."""
        cap = cv2.VideoCapture(video_path)
        
        if not cap.isOpened():
            return None, None, None, None
            
        fps = cap.get(cv2.CAP_PROP_FPS)
        frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        duration = frame_count / fps if fps > 0 else 0
        
        cap.release()
        return fps, frame_count, duration, (width, height)

    def preload_frames(self, video_path, start_frame, end_frame, cache_size):
        """Preload and cache video frames for smoother playback."""
        if cache_size <= 0:
            return None
        
        print(f"Preloading {cache_size} frames starting from frame {start_frame}...")
        
        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            return None
        
        cap.set(cv2.CAP_PROP_POS_FRAMES, start_frame)
        
        cached_images = []  # Store PIL images instead of canvas objects
        background_color = self.parse_background_color(self.args.background_color)
        
        frames_to_cache = min(cache_size, end_frame - start_frame)
        
        for i in range(frames_to_cache):
            ret, frame = cap.read()
            if not ret:
                break
            
            # Convert BGR to RGB
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            
            # Resize and enhance frame
            frame = self.resize_frame(frame, self.matrix.width, self.matrix.height, 
                                    self.args.fit_mode, background_color)
            frame = self.enhance_frame(frame)
            
            # Convert to PIL Image and store (don't create canvas yet)
            pil_image = Image.fromarray(frame)
            cached_images.append(pil_image)
            
            if self.args.verbose and (i + 1) % 10 == 0:
                print(f"  Cached {i + 1}/{frames_to_cache} frames")
        
        cap.release()
        print(f"Successfully cached {len(cached_images)} frames")
        return cached_images

    def run(self):
        """Main video playback loop."""
        # Set OpenCV logging level for quieter output if requested
        if self.args.quiet:
            cv2.setLogLevel(0)  # Suppress OpenCV messages
        
        # Determine video source priority: --url flag > video_source parameter > hardcoded URL
        video_path = None
        temp_file = None
        is_stream_url = False  # Track if we're using a stream URL instead of a file
        
        try:
            if self.args.url:
                print(f"Using URL parameter: {self.args.url}")
                result = self.download_video_from_url(self.args.url)
                if result and not os.path.exists(str(result)):
                    # This is a stream URL, not a downloaded file
                    video_path = result
                    is_stream_url = True
                else:
                    temp_file = result
                    video_path = temp_file
            elif self.args.video_source:
                if self.is_url(self.args.video_source):
                    print(f"Using URL from parameter: {self.args.video_source}")
                    result = self.download_video_from_url(self.args.video_source)
                    if result and not os.path.exists(str(result)):
                        # This is a stream URL, not a downloaded file
                        video_path = result
                        is_stream_url = True
                    else:
                        temp_file = result
                        video_path = temp_file
                else:
                    # Validate local video file
                    if not self.validate_video_file(self.args.video_source):
                        return False
                    print(f"Using local file: {self.args.video_source}")
                    video_path = self.args.video_source
            else:
                print(f"Using hardcoded URL: {self.video_url}")
                result = self.download_video_from_url(self.video_url)
                if result and not os.path.exists(str(result)):
                    # This is a stream URL, not a downloaded file
                    video_path = result
                    is_stream_url = True
                else:
                    temp_file = result
                    video_path = temp_file
            
            if not video_path:
                print("Error: No valid video source available")
                return False
            
            # Get video information
            fps, frame_count, duration, original_size = self.get_video_info(video_path)
            if fps is None:
                print("Error: Cannot read video information")
                return False
            
            # Use override FPS if specified
            if self.args.fps_override > 0:
                fps = self.args.fps_override
            
            # Calculate frame timing
            frame_delay = 1.0 / (fps * self.args.speed) if fps > 0 else 1.0/30.0
            
            print(f"Video Info:")
            print(f"  Original size: {original_size[0]}x{original_size[1]}")
            print(f"  Matrix size: {self.matrix.width}x{self.matrix.height}")
            print(f"  Duration: {duration:.1f} seconds")
            print(f"  Frame rate: {fps:.1f} fps")
            print(f"  Total frames: {frame_count}")
            print(f"  Fit mode: {self.args.fit_mode}")
            print(f"  Playback speed: {self.args.speed}x")
            if self.args.start_time > 0 or self.args.end_time > 0:
                print(f"  Time range: {self.args.start_time:.1f}s - {self.args.end_time if self.args.end_time > 0 else duration:.1f}s")
            if self.args.loop_count > 0:
                print(f"  Will loop {self.args.loop_count} times")
            else:
                print("  Looping infinitely")
            print("Press CTRL-C to stop.")
            
            # Open video capture
            cap = cv2.VideoCapture(video_path)
            if not cap.isOpened():
                print("Error: Cannot open video for playback")
                return False
            
            # Set start position if specified
            if self.args.start_time > 0:
                start_frame = int(self.args.start_time * fps)
                cap.set(cv2.CAP_PROP_POS_FRAMES, start_frame)
            
            # Calculate end frame
            end_frame = int(self.args.end_time * fps) if self.args.end_time > 0 else frame_count
            start_frame = int(self.args.start_time * fps) if self.args.start_time > 0 else 0
            
            background_color = self.parse_background_color(self.args.background_color)
            loop_counter = 0
            
            # Preload frames if caching is enabled
            cached_images = None
            if self.args.cache_frames > 0:
                cached_images = self.preload_frames(video_path, start_frame, end_frame, self.args.cache_frames)
                if cached_images:
                    print(f"Using cached frames for smoother playback")
            
            use_caching = cached_images is not None and len(cached_images) > 0
            
            # Create reusable canvas objects for double-buffering (fix memory leak)
            # Only create canvas objects once, then reuse them throughout playback
            offscreen_canvas = self.matrix.CreateFrameCanvas()  # For cached frames
            current_canvas = self.matrix.CreateFrameCanvas()    # For real-time frames
            
            try:
                frame_index = 0  # For cached frames
                
                while True:
                    if use_caching:
                        # Use cached frames for smoother playback
                        if frame_index >= len(cached_images):
                            # End of cached frames, loop if needed
                            loop_counter += 1
                            if self.args.loop_count > 0 and loop_counter >= self.args.loop_count:
                                print("Completed all loops. Exiting.")
                                break
                            frame_index = 0  # Reset to beginning
                            continue
                        
                        # Display cached frame using reusable canvas
                        pil_image = cached_images[frame_index]
                        offscreen_canvas.SetImage(pil_image)
                        self.matrix.SwapOnVSync(offscreen_canvas)
                        frame_index += 1
                        
                    else:
                        # Real-time frame processing
                        current_frame = int(cap.get(cv2.CAP_PROP_POS_FRAMES))
                        
                        # Check if we've reached the end time or end of video
                        if current_frame >= end_frame:
                            loop_counter += 1
                            
                            # Check loop count
                            if self.args.loop_count > 0 and loop_counter >= self.args.loop_count:
                                print("Completed all loops. Exiting.")
                                break
                            
                            # Reset to beginning (or start time)
                            cap.set(cv2.CAP_PROP_POS_FRAMES, start_frame)
                            continue
                        
                        ret, frame = cap.read()
                        if not ret:
                            # End of video, loop if needed
                            loop_counter += 1
                            if self.args.loop_count > 0 and loop_counter >= self.args.loop_count:
                                print("Completed all loops. Exiting.")
                                break
                            
                            # Reset to beginning
                            cap.set(cv2.CAP_PROP_POS_FRAMES, start_frame)
                            continue
                        
                        # Convert BGR to RGB (OpenCV uses BGR by default)
                        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                        
                        # Resize frame according to fit mode
                        frame = self.resize_frame(frame, self.matrix.width, self.matrix.height, 
                                                self.args.fit_mode, background_color)
                        
                        # Apply enhancements
                        frame = self.enhance_frame(frame)
                        
                        # Convert numpy array to PIL Image for the matrix
                        pil_image = Image.fromarray(frame)
                        
                        # Use reusable canvas for display (fix memory leak)
                        current_canvas.SetImage(pil_image)
                        self.matrix.SwapOnVSync(current_canvas)
                    
                    # Wait for next frame
                    time.sleep(frame_delay)
                    
                    # Debug output for verbose mode
                    if self.args.verbose and (frame_index if use_caching else current_frame) % 100 == 0:
                        current_pos = frame_index if use_caching else current_frame
                        print(f"  Frame {current_pos}, Loop {loop_counter}")
                    
            except KeyboardInterrupt:
                print("\nStopping video playback...")
            
            finally:
                cap.release()
                
        finally:
            # Clean up temporary file if it was downloaded (not for stream URLs)
            if temp_file and not is_stream_url and os.path.exists(temp_file):
                try:
                    os.unlink(temp_file)
                    print(f"Cleaned up temporary file: {temp_file}")
                except:
                    pass
        
        return True


# Main function
if __name__ == "__main__":
    # Load environment variables
    load_dotenv()
    
    video_player = VideoPlayer()
    if not video_player.process():
        video_player.print_help()