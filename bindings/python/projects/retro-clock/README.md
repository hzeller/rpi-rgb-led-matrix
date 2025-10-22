# �️ Retro Flip Clock

A classic 1970s-style flip clock display for RGB LED matrices, inspired by iconic Twemco and similar vintage timepieces.

## ✨ Features

### Authentic Flip Clock Design
- **Minimalist Aesthetic**: Clean white digits on black background
- **Classic Typography**: Large, blocky digits mimicking flip cards
- **Proper Proportions**: Authentic spacing and layout
- **Simple Elegance**: No distracting animations, just pure timekeeping

### Display Elements
- **Time**: Large, centered 12-hour format (10:15 style)
- **AM/PM**: Small indicator in bottom-right corner
- **Colon**: Two-dot separator between hours and minutes
- **Leading Space**: Hour display uses space instead of leading zero (like real flip clocks)

### Technical Features
- 1-second update interval (like real flip clocks)
- Shared component architecture (MatrixBase, FontManager, ColorPalette)
- Clean, readable font selection
- Minimal resource usage

## 🎮 Usage

### Basic Usage
```bash
cd /path/to/rpi-rgb-led-matrix/bindings/python/projects/retro-clock
sudo python retro-clock.py
```

### From Projects Directory
```bash
sudo python retro-clock/retro-clock.py
```

## 🎨 Color Scheme

Classic flip clock colors:
- **White Digits**: Clean, high-contrast main time display
- **Gray AM/PM**: Subtle indicator that doesn't compete with time
- **Black Background**: Pure black for maximum digit clarity
- **Dark Gray Frame**: Optional subtle corner accents

## 🔧 Customization

### Enable/Disable AM/PM
```python
# In the constructor
self.show_ampm = False  # Hide AM/PM for 24-hour style
self.show_ampm = True   # Show AM/PM (default)
```

### Add Subtle Frame
Uncomment in the run() method:
```python
# Optional: Draw subtle frame (uncomment if desired)
self.draw_subtle_frame()
```

### Change Color Theme
```python
# For a green monochrome look
self.color_palette = ColorPalette('matrix')
self.digit_color = self.color_palette.get_color('primary')
```

## 📋 Requirements

- Raspberry Pi with RGB LED matrix
- Python 3.7+
- rpi-rgb-led-matrix library
- Shared components (MatrixBase, FontManager, ColorPalette)

## 🎯 Perfect For

- Vintage electronics enthusiasts
- Minimalist desk displays
- Classic interior design
- Authentic retro aesthetics
- Bedside or office timekeeping

## 🚀 Example Output

```
�️  Retro Flip Clock initialized - Classic 1970s style
Matrix size: 64x32
🕰️  Starting Classic Flip Clock - Press CTRL-C to stop
📟 Authentic 1970s styling - simple and elegant
```

## 📐 Display Layout

```
┌─────────────────────────┐
│                         │
│                         │
│        10:15            │
│                      AM │
│                         │
└─────────────────────────┘
```

Press `Ctrl+C` to stop the display gracefully.