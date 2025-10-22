# 🔆 Brightness Control System

This enhancement adds comprehensive brightness control capabilities to all LED matrix applications. You can now adjust brightness dynamically, save preferences, and even set up automatic dimming schedules.

## 🌟 Features

### ✨ Dynamic Brightness Control
- **Real-time adjustment**: Change brightness from 1-100% while your app is running
- **Smooth transitions**: Brightness changes are applied immediately without flickering
- **Validation**: Automatic bounds checking ensures safe brightness levels

### 💾 Persistent Preferences
- **Save settings**: Store your preferred brightness level for future sessions
- **Environment variables**: Configure defaults through `.env` file
- **Per-project settings**: Different apps can have different brightness preferences

### 🌙 Auto-Dimming
- **Time-based dimming**: Automatically lower brightness at night
- **Customizable schedule**: Set your own dimming times and levels
- **Smart transitions**: Gradual brightness changes based on time of day

### 🎛️ Flexible Configuration
- **Multiple methods**: Set brightness through code, environment variables, or config files
- **Range validation**: Configurable minimum and maximum brightness limits
- **Easy integration**: Works seamlessly with all existing projects

## 🚀 Quick Start

### Basic Brightness Control
```python
from matrix_base import MatrixBase

class MyApp(MatrixBase):
    def __init__(self):
        super().__init__()
        
    def adjust_brightness(self):
        # Set specific brightness
        self.set_brightness(80)  # 80%
        
        # Get current brightness
        current = self.get_brightness()
        print(f"Current: {current}%")
        
        # Dim by 50% of current
        self.dim_display(0.5)
        
        # Save preference for next time
        self.save_brightness_preference(60)
```

### Environment Configuration
Copy `.env.example` to `.env` and customize:

```bash
# Default brightness (1-100)
BRIGHTNESS_DEFAULT=60

# Auto-dimming settings
BRIGHTNESS_AUTO_DIM=true
BRIGHTNESS_AUTO_DIM_TIME=22:00
BRIGHTNESS_AUTO_DIM_LEVEL=20
```

## 📚 API Reference

### MatrixBase Methods

#### `set_brightness(brightness)`
Set display brightness immediately.
- **Args**: `brightness` (int) - Brightness level 1-100
- **Example**: `self.set_brightness(75)`

#### `get_brightness()`
Get current brightness level.
- **Returns**: Current brightness (int) 1-100
- **Example**: `level = self.get_brightness()`

#### `dim_display(factor=0.5)`
Dim display by percentage of current brightness.
- **Args**: `factor` (float) - Dimming factor (0.1 = 10% of current)
- **Example**: `self.dim_display(0.3)  # Dim to 30% of current`

#### `save_brightness_preference(brightness)`
Save brightness as default for future sessions.
- **Args**: `brightness` (int) - Brightness level to save
- **Example**: `self.save_brightness_preference(60)`

### Configuration Manager

#### `get_brightness_config()`
Get all brightness-related settings.
- **Returns**: Dictionary with brightness configuration
- **Keys**: `default_brightness`, `min_brightness`, `max_brightness`, etc.

## ⚙️ Configuration Options

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `BRIGHTNESS_DEFAULT` | 60 | Default brightness level (1-100) |
| `BRIGHTNESS_MIN` | 1 | Minimum allowed brightness |
| `BRIGHTNESS_MAX` | 100 | Maximum allowed brightness |
| `BRIGHTNESS_AUTO_DIM` | false | Enable automatic dimming |
| `BRIGHTNESS_AUTO_DIM_TIME` | 22:00 | Time to start auto-dimming (24hr format) |
| `BRIGHTNESS_AUTO_DIM_LEVEL` | 20 | Brightness level when auto-dimmed |

### Matrix Configuration
The brightness system integrates with existing matrix configuration:

```python
# Override brightness at initialization
matrix = MatrixBase(brightness=50)

# Or use environment variable MATRIX_BRIGHTNESS
```

## 🛠️ Integration Examples

### Simple Clock with Auto-Dimming
```python
class SimpleClock(MatrixBase):
    def __init__(self):
        super().__init__()
        # Auto-dimming is configured via environment variables
        # and handled automatically in the clock loop
        
    def run(self):
        while True:
            # ... clock logic ...
            self.check_auto_dim(current_time)  # Built-in method
```

### Interactive Brightness Control
```python
class InteractiveApp(MatrixBase):
    def handle_input(self, key):
        if key == '+':
            current = self.get_brightness()
            self.set_brightness(min(100, current + 10))
        elif key == '-':
            current = self.get_brightness()
            self.set_brightness(max(1, current - 10))
        elif key == 's':
            self.save_brightness_preference(self.get_brightness())
```

### Ambient Light Adjustment
```python
class SmartBrightness(MatrixBase):
    def adjust_for_ambient_light(self, light_level):
        # Map light sensor reading (0-1) to brightness (20-100)
        brightness = int(20 + (light_level * 80))
        self.set_brightness(brightness)
        print(f"🔆 Adjusted brightness to {brightness}% for ambient light")
```

## 🧪 Testing

### Run the Brightness Test
```bash
cd projects
python brightness-test.py
```

This will demonstrate:
- Dynamic brightness changes from 100% to 1%
- Brightness getter functionality
- Relative dimming with `dim_display()`
- Preference saving to `.env` file

### Manual Testing
```bash
# Test different brightness levels
cd simple-clock
python simple-clock.py

# In another terminal, modify brightness via .env file
echo "BRIGHTNESS_DEFAULT=30" >> ../.env
```

## 🔧 Troubleshooting

### Common Issues

**Brightness not changing**
- Check that your matrix hardware supports brightness control
- Verify environment variables are properly loaded
- Ensure brightness values are within valid range (1-100)

**Auto-dimming not working**
- Check `BRIGHTNESS_AUTO_DIM=true` in your `.env` file
- Verify time format is correct (24-hour: "22:00")
- Make sure your system time is accurate

**Preferences not persisting**
- Check write permissions in the projects directory
- Verify `.env` file exists and is writable
- Look for error messages in console output

### Debug Tips
```python
# Check current configuration
config_manager = ConfigManager()
brightness_config = config_manager.get_brightness_config()
print(f"Brightness config: {brightness_config}")

# Monitor brightness changes
def debug_brightness_change(matrix):
    old_brightness = matrix.get_brightness()
    matrix.set_brightness(50)
    new_brightness = matrix.get_brightness()
    print(f"Changed: {old_brightness}% → {new_brightness}%")
```

## 🎯 Best Practices

### Performance
- **Avoid rapid changes**: Don't change brightness more than once per second
- **Batch updates**: If making multiple changes, do them together
- **Cache values**: Store brightness level if checking frequently

### User Experience
- **Gradual transitions**: Use `dim_display()` for smooth brightness changes
- **Save preferences**: Always call `save_brightness_preference()` after user adjustment
- **Provide feedback**: Use console output to confirm brightness changes

### Power Management
- **Lower for battery**: Use 20-40% brightness for battery-powered setups
- **Auto-dim at night**: Implement time-based dimming for 24/7 displays
- **Ambient sensing**: Consider light sensors for automatic adjustment

## 🔄 Migration Guide

### Updating Existing Projects
1. **No code changes needed**: Existing projects automatically get brightness support
2. **Add configuration**: Copy `.env.example` to `.env` for custom settings
3. **Enable auto-dimming**: Set environment variables for automatic brightness control
4. **Test functionality**: Run `brightness-test.py` to verify everything works

### New Project Template
```python
#!/usr/bin/env python3
import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from config_manager import ConfigManager

class MyNewProject(MatrixBase):
    def __init__(self):
        super().__init__()  # Brightness support is automatic!
        
    def run(self):
        # Your app logic here
        # Brightness control is available via self.set_brightness(), etc.
        pass

if __name__ == "__main__":
    app = MyNewProject()
    app.run()
```

## 📈 Future Enhancements

Planned features for future versions:
- **Smooth transitions**: Gradual brightness fading between levels
- **Multiple schedules**: Different dimming rules for weekdays/weekends
- **Ambient light sensor**: Automatic brightness based on room lighting
- **Remote control**: Web interface for brightness adjustment
- **Energy monitoring**: Track power usage at different brightness levels

---

## 💡 Tips & Tricks

### Quick Brightness Shortcuts
```bash
# Quickly set brightness via environment
export BRIGHTNESS_DEFAULT=30 && python your-app.py

# Test different levels rapidly
for level in 20 40 60 80 100; do
    echo "BRIGHTNESS_DEFAULT=$level" > .env
    timeout 3 python simple-clock/simple-clock.py
done
```

### Integration with System Events
```python
# Dim on system suspend/resume
import signal

class PowerAwareApp(MatrixBase):
    def __init__(self):
        super().__init__()
        signal.signal(signal.SIGUSR1, self.on_suspend)
        signal.signal(signal.SIGUSR2, self.on_resume)
    
    def on_suspend(self, signum, frame):
        self.set_brightness(1)  # Very dim for suspend
        
    def on_resume(self, signum, frame):
        self.set_brightness(self.normal_brightness)  # Restore
```

### Debugging Brightness Issues
```python
# Add to any project for brightness debugging
def debug_brightness_system(self):
    print("🔍 Brightness Debug Info:")
    print(f"  Current: {self.get_brightness()}%")
    print(f"  Config: {self.brightness_config}")
    print(f"  Matrix brightness: {self.matrix.brightness}")
    print(f"  Options brightness: {self.options.brightness}")
```

The brightness control system is now fully integrated into all your LED matrix projects! 🎉