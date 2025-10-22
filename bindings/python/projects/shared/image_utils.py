#!/usr/bin/env python
"""
ImageUtils - Image processing utilities for RGB LED Matrix applications.

Provides common image operations like scaling, cropping, and format conversion
optimized for small matrix displays.
"""

import os
from PIL import Image, ImageEnhance, ImageOps, ImageFilter, ImageDraw
import requests
from io import BytesIO


class ImageUtils:
    """
    Utility class for image processing operations commonly needed
    in RGB LED Matrix applications.
    """
    
    @staticmethod
    def scale_image_to_fit(image, target_width, target_height, fit_mode='fit'):
        """
        Scale an image to fit within target dimensions.
        
        Args:
            image: PIL Image object
            target_width: Target width in pixels
            target_height: Target height in pixels
            fit_mode: Scaling mode:
                'fit' - Scale to fit within bounds, maintaining aspect ratio
                'fill' - Scale to fill bounds, cropping if necessary
                'stretch' - Stretch to exact dimensions, ignoring aspect ratio
                'center' - Center without scaling (crop if too large)
                
        Returns:
            PIL Image object
        """
        if fit_mode == 'stretch':
            return image.resize((target_width, target_height), Image.Resampling.LANCZOS)
            
        elif fit_mode == 'center':
            # Center the image without scaling
            if image.size[0] <= target_width and image.size[1] <= target_height:
                # Image fits, create centered version
                result = Image.new('RGB', (target_width, target_height), (0, 0, 0))
                x = (target_width - image.size[0]) // 2
                y = (target_height - image.size[1]) // 2
                result.paste(image, (x, y))
                return result
            else:
                # Image too large, crop from center
                return ImageUtils.crop_center(image, target_width, target_height)
                
        else:  # 'fit' or 'fill'
            # Calculate scaling factors
            scale_x = target_width / image.size[0]
            scale_y = target_height / image.size[1]
            
            if fit_mode == 'fit':
                # Use smaller scale to fit within bounds
                scale = min(scale_x, scale_y)
            else:  # 'fill'
                # Use larger scale to fill bounds
                scale = max(scale_x, scale_y)
                
            # Scale the image
            new_width = int(image.size[0] * scale)
            new_height = int(image.size[1] * scale)
            scaled = image.resize((new_width, new_height), Image.Resampling.LANCZOS)
            
            # For 'fill' mode, crop to exact size if needed
            if fit_mode == 'fill' and (new_width > target_width or new_height > target_height):
                scaled = ImageUtils.crop_center(scaled, target_width, target_height)
            
            return scaled
            
    @staticmethod
    def crop_center(image, crop_width, crop_height):
        """
        Crop an image from the center.
        
        Args:
            image: PIL Image object
            crop_width: Desired width
            crop_height: Desired height
            
        Returns:
            PIL Image object
        """
        img_width, img_height = image.size
        
        # Calculate crop box
        left = (img_width - crop_width) // 2
        top = (img_height - crop_height) // 2
        right = left + crop_width
        bottom = top + crop_height
        
        # Ensure crop box is within image bounds
        left = max(0, left)
        top = max(0, top)
        right = min(img_width, right)
        bottom = min(img_height, bottom)
        
        return image.crop((left, top, right, bottom))
        
    @staticmethod
    def download_image(url, max_size=(512, 512), timeout=10):
        """
        Download an image from a URL.
        
        Args:
            url: Image URL
            max_size: Maximum image dimensions (width, height)
            timeout: Request timeout in seconds
            
        Returns:
            PIL Image object or None if download fails
        """
        try:
            headers = {
                'User-Agent': 'Mozilla/5.0 (Matrix Display Bot)'
            }
            response = requests.get(url, headers=headers, timeout=timeout, stream=True)
            response.raise_for_status()
            
            # Load image
            image = Image.open(BytesIO(response.content))
            
            # Convert to RGB if necessary
            if image.mode != 'RGB':
                image = image.convert('RGB')
                
            # Resize if too large
            if max_size and (image.size[0] > max_size[0] or image.size[1] > max_size[1]):
                image.thumbnail(max_size, Image.Resampling.LANCZOS)
                
            return image
            
        except Exception as e:
            print(f"Failed to download image from {url}: {e}")
            return None
            
    @staticmethod
    def enhance_image(image, brightness=1.0, contrast=1.0, saturation=1.0, sharpness=1.0):
        """
        Enhance image properties.
        
        Args:
            image: PIL Image object
            brightness: Brightness factor (1.0 = no change)
            contrast: Contrast factor (1.0 = no change)
            saturation: Saturation factor (1.0 = no change)
            sharpness: Sharpness factor (1.0 = no change)
            
        Returns:
            PIL Image object
        """
        result = image
        
        if brightness != 1.0:
            enhancer = ImageEnhance.Brightness(result)
            result = enhancer.enhance(brightness)
            
        if contrast != 1.0:
            enhancer = ImageEnhance.Contrast(result)
            result = enhancer.enhance(contrast)
            
        if saturation != 1.0:
            enhancer = ImageEnhance.Color(result)
            result = enhancer.enhance(saturation)
            
        if sharpness != 1.0:
            enhancer = ImageEnhance.Sharpness(result)
            result = enhancer.enhance(sharpness)
            
        return result
        
    @staticmethod
    def remove_background(image, background_color=(255, 255, 255), tolerance=50):
        """
        Remove background color from an image.
        
        Args:
            image: PIL Image object
            background_color: RGB tuple of background color to remove
            tolerance: Color matching tolerance (0-255)
            
        Returns:
            PIL Image object with transparent background
        """
        # Convert to RGBA for transparency
        if image.mode != 'RGBA':
            image = image.convert('RGBA')
            
        # Get image data
        data = image.getdata()
        new_data = []
        
        bg_r, bg_g, bg_b = background_color
        
        for pixel in data:
            r, g, b = pixel[:3]  # Get RGB values
            
            # Check if pixel is close to background color
            if (abs(r - bg_r) <= tolerance and 
                abs(g - bg_g) <= tolerance and 
                abs(b - bg_b) <= tolerance):
                # Make transparent
                new_data.append((r, g, b, 0))
            else:
                # Keep original
                if len(pixel) == 4:
                    new_data.append(pixel)
                else:
                    new_data.append((r, g, b, 255))
                    
        # Update image data
        image.putdata(new_data)
        return image
        
    @staticmethod
    def create_placeholder(width, height, color=(128, 128, 128), text=None, text_color=(255, 255, 255)):
        """
        Create a placeholder image with optional text.
        
        Args:
            width: Image width
            height: Image height
            color: Background color as RGB tuple
            text: Optional text to display
            text_color: Text color as RGB tuple
            
        Returns:
            PIL Image object
        """
        image = Image.new('RGB', (width, height), color)
        
        if text:
            draw = ImageDraw.Draw(image)
            
            # Try to use a reasonable font size
            try:
                from PIL import ImageFont
                # Try to find a reasonable font size
                font_size = min(width, height) // len(text) if text else 12
                font_size = max(8, min(font_size, 20))  # Clamp to reasonable range
                
                try:
                    # Try to load a system font
                    font = ImageFont.truetype("arial.ttf", font_size)
                except:
                    font = ImageFont.load_default()
            except ImportError:
                font = None
                
            # Get text size and position for centering
            if font:
                bbox = draw.textbbox((0, 0), text, font=font)
                text_width = bbox[2] - bbox[0]
                text_height = bbox[3] - bbox[1]
            else:
                # Estimate without font
                text_width = len(text) * 6
                text_height = 11
                
            x = (width - text_width) // 2
            y = (height - text_height) // 2
            
            draw.text((x, y), text, fill=text_color, font=font)
            
        return image
        
    @staticmethod
    def apply_matrix_effect(image, green_tint=True):
        """
        Apply a matrix-like effect to an image.
        
        Args:
            image: PIL Image object
            green_tint: Whether to apply green tinting
            
        Returns:
            PIL Image object
        """
        # Convert to grayscale first
        gray = ImageOps.grayscale(image)
        
        # Enhance contrast
        enhancer = ImageEnhance.Contrast(gray)
        gray = enhancer.enhance(1.5)
        
        if green_tint:
            # Convert back to RGB and apply green tint
            rgb_image = Image.new('RGB', gray.size)
            rgb_image.paste(gray)
            
            # Apply green tint
            pixels = list(rgb_image.getdata())
            tinted_pixels = []
            
            for r, g, b in pixels:
                # Reduce red and blue, enhance green
                new_r = int(r * 0.2)
                new_g = min(255, int(g * 1.2))
                new_b = int(b * 0.2)
                tinted_pixels.append((new_r, new_g, new_b))
                
            rgb_image.putdata(tinted_pixels)
            return rgb_image
        else:
            return gray.convert('RGB')
            
    @staticmethod
    def ensure_rgb(image):
        """
        Ensure image is in RGB mode.
        
        Args:
            image: PIL Image object
            
        Returns:
            PIL Image object in RGB mode
        """
        if image.mode != 'RGB':
            return image.convert('RGB')
        return image