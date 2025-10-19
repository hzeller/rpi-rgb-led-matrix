# Stock Tracker for RGB LED Matrix

A real-time stock price display for your RGB LED matrix that shows scrolling stock prices with color-coded gains/losses.

## Features

- **Real-time Stock Data**: Uses Alpha Vantage API for current stock prices
- **Color-coded Display**: Green for gains, red for losses, blue for prices
- **Scrolling Text**: Smooth horizontal scrolling of multiple stock symbols
- **Configurable**: Support for multiple stocks, refresh rates, and display options
- **Demo Mode**: Built-in demo data for testing without API key
- **Error Handling**: Robust error handling for network issues and API failures
- **Multi-threaded**: Background data updates while maintaining smooth display

## Setup

### 1. Install Dependencies

```bash
cd /path/to/rpi-rgb-led-matrix/bindings/python/samples
pip install -r stock-requirements.txt
```

### 2. Get API Key (Optional)

1. Visit [Alpha Vantage](https://www.alphavantage.co/support/#api-key)
2. Sign up for a free account
3. Get your API key (free tier: 5 calls/minute, 500 calls/day)

### 3. Configure Settings

Copy the example configuration:
```bash
cp stock-tracker.env .env
```

Edit the `.env` file with your settings:
```bash
# Your Alpha Vantage API key
ALPHA_VANTAGE_API_KEY=your_actual_key_here

# Stock symbols to track (comma-separated)
DEFAULT_STOCKS=AAPL,GOOGL,MSFT,TSLA,AMZN,NVDA

# Update frequency (minutes) - respect API limits
REFRESH_RATE=5

# Display settings
SCROLL_SPEED=0.05
BRIGHTNESS=75
```

## Usage

### Basic Usage (Demo Mode)
```bash
sudo python stock-tracker.py --demo-mode
```

### With API Key
```bash
sudo python stock-tracker.py --api-key YOUR_API_KEY
```

### Custom Stock List
```bash
sudo python stock-tracker.py --stocks "AAPL,GOOGL,MSFT" --demo-mode
```

### Full Configuration
```bash
sudo python stock-tracker.py \
  --api-key YOUR_API_KEY \
  --stocks "AAPL,GOOGL,MSFT,TSLA,AMZN" \
  --refresh-rate 10 \
  --scroll-speed 0.03 \
  --led-brightness 50
```

## Command Line Options

### Stock Tracker Specific
- `--api-key`: Alpha Vantage API key
- `--stocks`: Comma-separated stock symbols (default: AAPL,GOOGL,MSFT,TSLA,AMZN)
- `--refresh-rate`: Data refresh rate in minutes (default: 5)
- `--scroll-speed`: Text scroll speed, lower = faster (default: 0.05)
- `--demo-mode`: Use demo data instead of real API calls
- `--config-file`: Path to configuration file (default: stock-tracker.env)

### LED Matrix Options
- `--led-rows`: Display rows (16 or 32, default: 32)
- `--led-cols`: Panel columns (32 or 64, default: 32)
- `--led-chain`: Number of daisy-chained panels (default: 1)
- `--led-brightness`: Brightness level 1-100 (default: 100)
- `--led-gpio-mapping`: Hardware mapping (regular, adafruit-hat, etc.)

## Display Format

The display shows information in this format:
```
SYMBOL: $PRICE +/-$CHANGE (+/-PERCENT%)  |  SYMBOL: $PRICE...
```

### Color Coding
- **White**: Stock symbols and separators
- **Blue**: Current stock prices
- **Green**: Positive changes (gains)
- **Red**: Negative changes (losses)
- **Yellow**: Status information

### Example Display
```
AAPL: $150.25 +$2.15 (+1.4%)  |  GOOGL: $2,800.50 -$15.30 (-0.5%)
```

## API Rate Limits

**Alpha Vantage Free Tier:**
- 5 API calls per minute
- 500 API calls per day

The stock tracker automatically:
- Adds delays between API calls
- Handles rate limit responses
- Falls back to demo data on errors

For best results:
- Use `--refresh-rate 5` or higher (minutes)
- Limit stock symbols to 5-10 for free tier
- Monitor your daily usage

## Troubleshooting

### Common Issues

**"No API key provided"**
- Use `--demo-mode` for testing
- Get free API key from alphavantage.co
- Set API key in .env file or use `--api-key`

**"API Rate limit reached"**
- Increase `--refresh-rate` to 10+ minutes
- Reduce number of stock symbols
- Wait for rate limit to reset

**"Network error" or timeouts**
- Check internet connection
- Verify API key is correct
- Try `--demo-mode` to test display

**No display or garbled text**
- Check LED matrix wiring and power
- Verify `--led-rows` and `--led-cols` settings
- Try different `--led-gpio-mapping` options

### Debug Mode

Add verbose output by modifying the script or using demo mode:
```bash
sudo python stock-tracker.py --demo-mode --scroll-speed 0.1
```

## Customization

### Adding New Stock Symbols

1. Find the stock symbol (ticker) from Yahoo Finance or similar
2. Add to your stock list: `--stocks "AAPL,GOOGL,YOUR_SYMBOL"`
3. Test with demo mode first: `--demo-mode`

### Changing Colors

Edit the `colors` dictionary in `stock-tracker.py`:
```python
self.colors = {
    'green': graphics.Color(0, 255, 0),    # Gains - change RGB values
    'red': graphics.Color(255, 0, 0),      # Losses
    'white': graphics.Color(255, 255, 255), # Neutral
    'blue': graphics.Color(0, 150, 255),   # Prices
    'yellow': graphics.Color(255, 255, 0)   # Status
}
```

### Changing Display Format

Modify the `format_stock_display()` method to change:
- Text layout and separators
- Number formatting
- Additional information display

## Performance Tips

1. **Optimize Refresh Rate**: Don't update too frequently
2. **Limit Symbols**: 5-10 stocks work well for free API
3. **Use Appropriate Hardware**: Ensure adequate power supply
4. **Monitor Resource Usage**: Check CPU and memory on Pi

## License

This stock tracker follows the same license as the rpi-rgb-led-matrix library.

## Disclaimer

This software is for educational and personal use only. Stock prices are provided by third-party APIs and may be delayed. Do not use for financial trading decisions. Always verify information from official sources.