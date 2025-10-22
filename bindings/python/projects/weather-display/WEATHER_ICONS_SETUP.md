# Weather Icons Setup Instructions

## Step 1: Save the Weather Icons Image
1. Save the weather icons image you provided as `weather_icons_source.png` in the `samples` directory
2. The image should contain a 4x3 grid of beautiful weather icons

## Step 2: Extract Individual Icons
Run the extraction script:
```bash
cd "C:\for me\rpi-rgb-led-matrix\bindings\python\samples"
python extract_icons.py
```

This will create a `weather_icons` directory with individual icon files.

## Step 3: Icon Mapping
The extracted icons will be mapped to OpenWeatherMap codes as follows:

### Row 1 (Top):
- **Column 1**: Sun → `01d.png` (Clear Day)
- **Column 2**: Moon → `01n.png` (Clear Night)  
- **Column 3**: Clouds → `03d.png` (Scattered Clouds)
- **Column 4**: Partly Cloudy → `02d.png` (Partly Cloudy Day)

### Row 2 (Middle):
- **Column 1**: Heavy Clouds → `04d.png` (Broken Clouds)
- **Column 2**: Light Rain → `09d.png` (Shower Rain)
- **Column 3**: Heavy Rain → `10d.png` (Rain)
- **Column 4**: Thunderstorm → `11d.png` (Thunderstorm)

### Row 3 (Bottom):
- **Column 1**: Thunderstorm w/ Rain → `11d.png` (Alt Thunderstorm)
- **Column 2**: Snowflake → `13d.png` (Snow)
- **Column 3**: Heavy Snow → `13d.png` (Alt Snow)
- **Column 4**: Mixed Conditions → `50d.png` (Mist/Fog)

## Step 4: Additional Icon Mappings
You may need to copy some icons for night versions:
```bash
# Copy day icons to night versions
cp weather_icons/02d.png weather_icons/02n.png  # Partly cloudy night
cp weather_icons/03d.png weather_icons/03n.png  # Scattered clouds night
cp weather_icons/04d.png weather_icons/04n.png  # Broken clouds night
cp weather_icons/09d.png weather_icons/09n.png  # Shower rain night
cp weather_icons/10d.png weather_icons/10n.png  # Rain night
cp weather_icons/11d.png weather_icons/11n.png  # Thunderstorm night
cp weather_icons/13d.png weather_icons/13n.png  # Snow night
cp weather_icons/50d.png weather_icons/50n.png  # Mist night
```

The weather display will then automatically load these beautiful icons instead of using the simple pixel art!