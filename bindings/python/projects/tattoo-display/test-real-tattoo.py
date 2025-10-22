#!/usr/bin/env python3
# Test script for Real Tattoo Display - Image Download Test
# Verifies that tattoo images can be downloaded and processed

import os
import requests
from PIL import Image
from io import BytesIO

def test_image_download():
    """Test downloading and processing tattoo images"""
    print("🎨 Testing Real Tattoo Image Download")
    print("=" * 50)
    
    # Test URLs
    test_urls = {
        'anchor': 'https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQLPU-ENS6le3fi4YwZTh6FF9NzqEqVzAF7nQ&s',
        'heart': 'https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcT_uQgp78o1k8SDUa3S62b3hT8zDL4e9h45WA&s',
        'rose': 'https://i.pinimg.com/736x/a1/b5/82/a1b5829e0600f07ac21a56f92e97f980.jpg',
        'swallow': 'https://i.pinimg.com/736x/db/7f/c3/db7fc3bdb6b4b41c4752077eca5800c3.jpg',
        'skull': 'https://i.pinimg.com/474x/eb/2f/75/eb2f75108653a27e4da172862e900313.jpg',
        'dagger': 'https://www.shutterstock.com/image-vector/traditional-dagger-tattoo-vector-design-260nw-2436282185.jpg'
    }
    
    # Create test directory
    test_dir = "test_tattoo_images"
    if not os.path.exists(test_dir):
        os.makedirs(test_dir)
        print(f"✓ Created test directory: {test_dir}")
    
    successful_downloads = 0
    total_images = len(test_urls)
    
    for name, url in test_urls.items():
        try:
            print(f"\n🔄 Testing {name.capitalize()} tattoo...")
            print(f"   URL: {url}")
            
            # Download image
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            
            # Check if it's a valid image
            image = Image.open(BytesIO(response.content))
            print(f"   ✓ Downloaded successfully")
            print(f"   📏 Original size: {image.width}x{image.height}")
            print(f"   🎨 Mode: {image.mode}")
            
            # Convert to RGB if needed
            if image.mode != 'RGB':
                image = image.convert('RGB')
                print(f"   🔄 Converted to RGB mode")
            
            # Test resizing for LED matrix (64x32)
            matrix_width, matrix_height = 64, 32
            img_ratio = image.width / image.height
            matrix_ratio = matrix_width / matrix_height
            
            if img_ratio > matrix_ratio:
                new_width = matrix_width
                new_height = int(matrix_width / img_ratio)
            else:
                new_height = matrix_height
                new_width = int(matrix_height * img_ratio)
            
            resized = image.resize((new_width, new_height), Image.Resampling.LANCZOS)
            print(f"   📐 Resized to: {resized.width}x{resized.height}")
            
            # Save test image
            test_filename = os.path.join(test_dir, f"{name}_test.jpg")
            resized.save(test_filename, "JPEG", quality=85)
            print(f"   💾 Saved test image: {test_filename}")
            
            successful_downloads += 1
            
        except requests.RequestException as e:
            print(f"   ✗ Network error downloading {name}: {e}")
        except Exception as e:
            print(f"   ✗ Error processing {name}: {e}")
    
    print(f"\n📊 Test Results:")
    print(f"   Successfully processed: {successful_downloads}/{total_images} images")
    print(f"   Success rate: {(successful_downloads/total_images)*100:.1f}%")
    
    if successful_downloads > 0:
        print(f"\n✅ Image download test PASSED")
        print(f"   Test images saved in: {test_dir}/")
        print(f"   You can view these images to see how they'll look on the LED matrix")
    else:
        print(f"\n❌ Image download test FAILED")
        print(f"   Check your internet connection and URL accessibility")
    
    print(f"\n🚀 Ready to run the real tattoo display:")
    print(f"   python real-tattoo-display.py --led-cols=64 --led-rows=32")
    print(f"   python real-tattoo-display.py -t anchor --enhance --speed 3")

if __name__ == "__main__":
    test_image_download()