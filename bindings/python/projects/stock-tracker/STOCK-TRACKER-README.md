# Stock Tracker with Charts

A sophisticated stock tracker that displays individual stocks with historical price charts, matching the professional layout shown in your reference image.

## Features

✅ **Individual Stock Display**: Shows one stock at a time with full details  
✅ **Professional Layout**: Symbol, price, change, percentage in organized quadrants  
✅ **Historical Charts**: Mini price graphs showing 30+ days of movement  
✅ **Dynamic Colors**: Green for gains, red for losses  
✅ **Auto-cycling**: Rotates through multiple stocks automatically  
✅ **Real-time Data**: Alpha Vantage API integration with fallback demo mode  

## Display Layout

Based on your reference image, the display shows:

```
AAPL        0.87
174.30     0.50%
________________
[Chart Graph Here]
```

### Layout Details:
- **Top Left**: Stock symbol (AAPL)
- **Bottom Left**: Current price (174.30)
- **Top Right**: Price change amount (0.87)
- **Middle Right**: Change percentage (0.50%)
- **Bottom**: Historical price chart (mini graph)

## Quick Start

### Demo Mode (No API Key Needed)
```bash
sudo python stock-tracker.py --demo-mode
```

### With Real Data
```bash
sudo python stock-tracker.py --api-key YOUR_KEY --stocks "AAPL,GOOGL,MSFT,TSLA"
```

## Command Line Options

### Stock Tracker Options
- `--display-time`: Seconds to show each stock (default: 10)
- `--chart-days`: Days of historical data for charts (default: 30)
- `--stocks`: Stock symbols to cycle through
- `--demo-mode`: Use realistic demo data
- `--api-key`: Alpha Vantage API key

### Inherited Matrix Options
- Matrix configuration automatically uses your 32x64 Adafruit HAT PWM setup
- All standard LED matrix options available (`--led-brightness`, etc.)

## Color Coding

The display uses intelligent color coding:
- **Bright Green**: Positive gains (symbol, price, change, chart)
- **Bright Red**: Losses (symbol, price, change, chart)
- **Dim Green/Red**: Chart lines and secondary elements
- **White**: Loading states and neutral info

## Chart Features

### Historical Price Charts
- Shows last 30-50 data points
- Automatically scales to fit display area
- Line color matches stock performance
- Smooth line interpolation between points

### Chart Data Sources
- **Real Mode**: Daily closing prices from Alpha Vantage
- **Demo Mode**: Realistic simulated price movements
- **Fallback**: Uses demo data if API fails

## Usage Examples

### Basic Usage
```bash
# Show AAPL, GOOGL, MSFT with 10 second intervals
sudo python stock-tracker.py --demo-mode --stocks "AAPL,GOOGL,MSFT"
```

### Custom Configuration  
```bash
# 5-second intervals, 60 days of chart data
sudo python stock-tracker.py \
  --api-key YOUR_KEY \
  --stocks "AAPL,TSLA,NVDA" \
  --display-time 5 \
  --chart-days 60
```

### Quick Test
```bash
# Demo with single stock for testing
sudo python stock-tracker.py --demo-mode --stocks "AAPL" --display-time 30
```

## API Requirements

### For Real Data (Optional)
1. Get free API key from [Alpha Vantage](https://www.alphavantage.co/support/#api-key)
2. Free tier includes:
   - 5 calls per minute
   - 500 calls per day
   - Historical daily data

### Rate Limit Optimization
- Fetches historical data once per symbol
- Caches chart data to minimize API calls  
- Respects API limits with delays
- Falls back to demo data on errors

## Display Cycle

The tracker automatically cycles through your configured stocks:

1. **Stock 1** → Display for X seconds
2. **Stock 2** → Display for X seconds  
3. **Stock 3** → Display for X seconds
4. **Return to Stock 1** → Repeat cycle

### Timing Control
- Use `--display-time 15` for 15 seconds per stock
- Shorter times good for many stocks
- Longer times better for detailed viewing

## Troubleshooting

### Common Issues

**"No chart data"**
- Historical data requires API key
- Demo mode generates realistic chart data
- Check API rate limits

**"Loading..." stuck**
- Verify internet connection
- Check API key validity
- Try `--demo-mode` first

**Chart not visible**
- Charts appear in bottom 6 pixels
- May be subtle with dim colors
- Increase `--led-brightness`

### Performance Tips

1. **Limit Stock Count**: 4-6 stocks work well
2. **Reasonable Display Time**: 8-15 seconds per stock
3. **Monitor API Usage**: Free tier has daily limits
4. **Use Demo Mode**: For development and testing

## Customization

### Adding More Stocks
```bash
--stocks "AAPL,GOOGL,MSFT,TSLA,AMZN,NVDA,META,NFLX"
```

### Changing Colors
Edit the `colors` dictionary in the code:
```python
self.colors = {
    'gain_bright': graphics.Color(0, 255, 0),     # Bright green
    'loss_bright': graphics.Color(255, 0, 0),     # Bright red
    # ... customize as needed
}
```

### Layout Modifications
- Font sizes: Change `7x13.bdf` and `5x7.bdf` font files
- Positioning: Modify x,y coordinates in the display code
- Chart size: Adjust `chart_height` and `chart_width`

## Comparison with Basic Tracker

| Feature | Old Stock Tracker | Current Stock Tracker |
|---------|-------------------|----------------------|
| Layout | Scrolling text | Professional quadrant |
| Stocks | All at once | One at a time |  
| Charts | None | Historical graphs |
| Colors | Simple coding | Dynamic performance |
| API Calls | Current quotes only | Quotes + historical |

## Example Output

The display will show something like:
```
AAPL        +2.15
174.30     +1.24%
▄▅▆▅▄▅▆▇▆▅▄▅▆▇█▇▆
```

Where the bottom line represents the price chart over time.

This stock tracker provides a much more professional and informative stock display that matches modern trading interfaces!