# Video Player for RGB LED Matrix

A Python-based video player that displays video content on RGB LED matrices using the rpi-rgb-led-matrix library.

## Features

- **Multiple Video Sources**: Play from local files, URLs, or hardcoded video
- **Platform Support**: YouTube, Vimeo, Reddit, and other video platforms via yt-dlp
- **Flexible Scaling**: Multiple fit modes (stretch, fit, fill, center)
- **Video Controls**: Speed adjustment, looping, start/end time selection
- **Image Enhancement**: Brightness, contrast, and saturation adjustments
- **Environment Configuration**: Settings via environment variables or command line
- **Format Support**: Supports most common video formats via OpenCV (MP4, AVI, MOV, etc.)

## Installation

1. Install required dependencies:
```bash
pip install -r video-requirements.txt
```

2. Ensure the rpi-rgb-led-matrix Python bindings are properly installed

## Usage

### Basic Usage

```bash
# Play hardcoded demo video
python video-player.py

# Play local video file
python video-player.py /path/to/video.mp4

# Play video from direct URL
python video-player.py https://example.com/video.mp4

# Play YouTube video
python video-player.py https://youtu.be/VIDEO_ID
# or
python video-player.py --url https://www.youtube.com/watch?v=VIDEO_ID

# Play from other platforms (Vimeo, Reddit, etc.)
python video-player.py --url https://vimeo.com/VIDEO_ID
```

### Advanced Options

```bash
# Custom matrix configuration
python video-player.py video.mp4 --led-rows 64 --led-cols 64 --led-chain 2

# Video scaling and enhancement
python video-player.py video.mp4 --fit-mode fill --brightness-adjust 1.2 --contrast-adjust 1.1

# Playback controls
python video-player.py video.mp4 --speed 0.5 --loop-count 3 --start-time 10 --end-time 60

# Custom FPS override
python video-player.py video.mp4 --fps-override 15
```

## Fit Modes

- **`fit`** (default): Maintains aspect ratio with letterboxing if needed
- **`fill`**: Maintains aspect ratio but crops to fill the matrix completely
- **`stretch`**: Ignores aspect ratio and stretches to fill exactly
- **`center`**: No scaling, centers the video (crops if larger than matrix)

## Environment Variables

You can set default values using environment variables:

```bash
# Matrix configuration
MATRIX_ROWS=32
MATRIX_COLS=64
MATRIX_CHAIN_LENGTH=1
MATRIX_PARALLEL=1
MATRIX_BRIGHTNESS=100
MATRIX_GPIO_MAPPING=adafruit-hat-pwm
MATRIX_PWM_BITS=11
MATRIX_SLOWDOWN_GPIO=1

# Video settings
VIDEO_FIT_MODE=fit
VIDEO_SPEED=1.0
VIDEO_BRIGHTNESS_ADJUST=1.0
VIDEO_CONTRAST_ADJUST=1.0
VIDEO_SATURATION_ADJUST=1.0
VIDEO_LOOP_COUNT=0
VIDEO_BACKGROUND_COLOR=black
VIDEO_FPS_OVERRIDE=0
VIDEO_START_TIME=0
VIDEO_END_TIME=0
VIDEO_CONFIG_FILE=.env
```

Create a `.env` file in the same directory as the script with these variables.

## Command Line Arguments

### Matrix Configuration
- `--led-rows`: Display rows (default: 32)
- `--led-cols`: Panel columns (default: 64)
- `--led-chain`: Daisy-chained boards (default: 1)
- `--led-parallel`: Parallel chains (default: 1)
- `--led-brightness`: Brightness level 1-100 (default: 100)
- `--led-gpio-mapping`: Hardware mapping (regular, adafruit-hat, adafruit-hat-pwm)

### Video Settings
- `--fit-mode`: How to fit video (stretch, fit, fill, center)
- `--speed`: Playback speed multiplier (default: 1.0)
- `--brightness-adjust`: Brightness multiplier (default: 1.0)
- `--contrast-adjust`: Contrast multiplier (default: 1.0)
- `--saturation-adjust`: Saturation multiplier (default: 1.0)
- `--loop-count`: Number of loops (0=infinite, default: 0)
- `--background-color`: Background for letterboxing (black, white, #RRGGBB)
- `--fps-override`: Override video FPS (default: 0, use original)
- `--start-time`: Start playback time in seconds (default: 0)
- `--end-time`: End playback time in seconds (default: 0, play to end)

## Examples

### Display a video with custom scaling and timing
```bash
python video-player.py demo.mp4 \
    --fit-mode fill \
    --start-time 30 \
    --end-time 90 \
    --speed 1.5 \
    --loop-count 2
```

### Enhanced video with custom matrix setup
```bash
python video-player.py https://example.com/video.mp4 \
    --led-rows 64 \
    --led-cols 64 \
    --led-brightness 80 \
    --brightness-adjust 1.3 \
    --contrast-adjust 1.2 \
    --saturation-adjust 0.9
```

### Slow motion with custom frame rate
```bash
python video-player.py action.mp4 \
    --fps-override 10 \
    --speed 0.3 \
    --fit-mode center
```

## Platform Support

The video player supports multiple video platforms through yt-dlp:

### Supported Platforms
- **YouTube** (youtube.com, youtu.be)
- **Vimeo** (vimeo.com)
- **Reddit** (reddit.com) - MP4 format posts
- **Twitter/X** (twitter.com)
- **Instagram** (instagram.com)
- **TikTok** (tiktok.com)
- **Facebook** (facebook.com)
- **Dailymotion** (dailymotion.com)
- **Twitch** (twitch.tv)

### Platform Usage Examples
```bash
# YouTube videos
python video-player.py "https://youtu.be/dQw4w9WgXcQ"
python video-player.py "https://www.youtube.com/watch?v=dQw4w9WgXcQ"

# Vimeo videos
python video-player.py "https://vimeo.com/123456789"

# Reddit MP4 posts
python video-player.py "https://www.reddit.com/r/videos/comments/abc123/title/"
```

**Note**: Platform support requires yt-dlp. Install with `pip install yt-dlp`

## Performance Tips

1. **Video Resolution**: Lower resolution videos will process faster
2. **Frame Rate**: Use `--fps-override` to limit frame rate for smoother playback
3. **Fit Mode**: `stretch` mode is fastest as it doesn't maintain aspect ratio
4. **Matrix Size**: Smaller matrices will process frames faster
5. **Enhancement**: Disable brightness/contrast/saturation adjustments for best performance
6. **Platform Videos**: YouTube and other platforms automatically select appropriate quality
7. **Memory Efficiency**: The player uses canvas double-buffering to prevent memory leaks during long playbacks

## Troubleshooting

### Common Issues

**Video won't play:**
- Check that OpenCV is properly installed
- Verify the video file is not corrupted
- Ensure the video format is supported by OpenCV

**YouTube/Platform videos fail:**
- Install yt-dlp: `pip install yt-dlp`
- Check if the video is available in your region
- Some platforms may have age restrictions or require login
- Try a different video URL format (youtu.be vs youtube.com)

**Slow/choppy playback:**
- Try reducing the video resolution
- Use `--fps-override` to limit frame rate
- Disable image enhancements
- Use `stretch` fit mode

**Memory issues with large videos:**
- The player loads frames one at a time to minimize memory usage
- For very long videos, consider splitting them into smaller segments

**Download failures:**
- Check internet connection
- Verify the URL is correct and accessible
- Some servers may block automated downloads

### Debug Output

The player provides detailed information about:
- Video dimensions and properties
- Matrix configuration
- Processing settings
- Download progress (for URLs)

## Similar Tools

- `gif-viewer.py`: For animated GIF display
- C++ `video-viewer` (in utils/): FFmpeg-based video player
- `image-viewer.py`: For static image display