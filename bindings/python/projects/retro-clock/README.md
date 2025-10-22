# 🕒 Retro Clock

An 80s/90s-inspired digital clock display for RGB LED matrices, featuring classic retro aesthetics and smooth animations.

## ✨ Features

### Retro Visual Design
- **Color Theme**: Lime green, yellow, and magenta color scheme
- **Typography**: Classic digital-style fonts
- **Animations**: Blinking colon, scanning side effects, animated borders

### Display Elements
- **Time**: Large, centered 12-hour format with blinking colon
- **Day**: Day of the week in bright yellow
- **Date**: Month/day in retro magenta with yellow separator
- **Effects**: Animated border corners and scanning side dots

### Technical Features
- 30fps smooth animations
- Shared component architecture (MatrixBase, FontManager, ColorPalette)
- Automatic animation cycling to prevent overflow
- Clean startup and shutdown

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

The retro theme uses:
- **Primary (Lime Green)**: Main time display
- **Secondary (Yellow)**: Day of week, date separator, dim effects
- **Accent (Magenta)**: Date numbers, animated highlights

## 🔧 Customization

### Change Colors
Modify the theme in the constructor:
```python
# Use different theme
self.color_palette = ColorPalette('matrix')  # Matrix green theme
self.color_palette = ColorPalette('dark')    # Dark theme
```

### Adjust Animation Speed
Change the refresh rate:
```python
# Slower animations (20fps)
time.sleep(1.0 / 20.0)

# Faster animations (60fps)
time.sleep(1.0 / 60.0)
```

### Modify Effects
- **Border animation**: Change `animation_frame // 30` divisor
- **Colon blink**: Adjust `animation_frame % 30` modulus
- **Dot scanning**: Modify `animation_frame // 10` divisor

## 📋 Requirements

- Raspberry Pi with RGB LED matrix
- Python 3.7+
- rpi-rgb-led-matrix library
- Shared components (MatrixBase, FontManager, ColorPalette)

## 🎯 Perfect For

- Retro gaming setups
- 80s/90s themed displays  
- Nostalgic room decor
- Demonstration of theme system

## 🚀 Example Output

```
🕒 Retro Clock initialized with 80s theme!
Matrix size: 64x32
🎮 Starting Retro Clock - Press CTRL-C to stop
✨ Featuring 80s-style animations and colors!
```

Press `Ctrl+C` to stop the display gracefully.