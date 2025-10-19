# Advanced Stock Tracker for RGB LED Matrix

A modular, refactored stock tracking application for Raspberry Pi RGB LED matrices that displays real-time stock prices with historical charts and color-coded performance indicators.

## File Structure

### Core Modules

- **`stock_tracker.py`** - Main application entry point and orchestration
- **`stock_client.py`** - Stock API integration (Yahoo Finance, Alpha Vantage) and background updates
- **`chart_renderer.py`** - Chart rendering utilities for time series data
- **`stock_config.py`** - Stock-specific configuration extending base config
- **`display.py`** - Extended with `StockMatrixDisplay` for stock-specific display operations

### Shared Modules (Reused from Music Display)

- **`config.py`** - Base configuration class (reused and extended)
- **`display.py`** - Base `MatrixDisplay` class (extended for stocks)

### Legacy Files

- **`advanced-stock-tracker.py`** - Original monolithic implementation (deprecated)

## Features

- 📈 **Multiple Data Sources**: Yahoo Finance (free) and Alpha Vantage APIs
- 📊 **Live Charts**: Real-time filled area charts showing price history
- 🔥 **Trending Stocks**: Automatic inclusion of biggest gainers/losers
- 🎨 **Color Coding**: Green/red indicators for gains/losses
- ⚙️ **Configurable**: Easy customization via config files
- 🎯 **Demo Mode**: Realistic demo data for testing without API keys

## Usage

### Basic Mode (Yahoo Finance - Free)
```bash
python stock_tracker.py
```

### Demo Mode
```bash
python stock_tracker.py --demo-mode
```

### Custom Stocks
```bash
python stock_tracker.py --stocks "AAPL,GOOGL,TSLA,NVDA"
```

### Include Trending Stocks
```bash
python stock_tracker.py --include-trending --trending-count 3
```

### All Options
```bash
python stock_tracker.py \
    --stocks "AAPL,MSFT" \
    --include-trending \
    --trending-count 2 \
    --refresh-rate 5 \
    --display-time 8 \
    --alpha-vantage-key YOUR_KEY
```

## Configuration

### Environment Variables (.env or stock-tracker.env)
```bash
DEFAULT_STOCKS=AAPL,GOOGL,MSFT,TSLA
ALPHA_VANTAGE_API_KEY=your_key_here
REFRESH_RATE=5
DISPLAY_TIME=10
TRENDING_COUNT=3
CHART_DAYS=30
```

### Customizable Settings in `stock_config.py`
- Matrix hardware configuration
- Display colors and layout
- Chart appearance and dimensions
- API timing and rate limits
- Font paths and sizes

## Architecture

### Modular Design Benefits

**Code Reuse from Music Display:**
- Base `MatrixDisplay` class extended for stocks
- Configuration pattern reused and extended
- Application orchestration pattern shared
- Background threading approach consistent

**New Stock-Specific Components:**
- `StockClient`: Multi-source API handling (Yahoo + Alpha Vantage)
- `StockUpdateThread`: Non-blocking background updates
- `TrendingStockFetcher`: Automatic trending stock discovery
- `StockChartRenderer`: Reusable chart rendering engine
- `StockMatrixDisplay`: Stock-specific display logic

### Class Hierarchy

```
DisplayConfig (base)
└── StockConfig (extends with stock settings)

MatrixDisplay (base, from music display)
└── StockMatrixDisplay (extends with stock display methods)

ChartRenderer (base)
├── TimeSeriesChart (filled area charts)
├── LineChart (simple line charts)
└── StockChartRenderer (high-level stock charts)

StockClient (API handling)
├── Yahoo Finance integration
├── Alpha Vantage integration
└── Demo data generation

StockUpdateThread (background updates)
TrendingStockFetcher (trending stocks)
StockTracker (main orchestrator)
```

## API Sources

### 1. Yahoo Finance (Default - Free)
- No API key required
- Real-time stock prices
- Rate limited but generous
- Primary data source

### 2. Alpha Vantage (Fallback)
- Requires free API key from alphavantage.co
- Historical data support
- 500 calls/day free tier
- Backup data source

### 3. Demo Mode
- Realistic simulated data
- Perfect for development/testing
- No internet required

## Improvements Over Original

### 🚀 Code Quality
- **From**: 1 monolithic 800+ line file
- **To**: 6 focused, single-purpose modules
- **Reduced Duplication**: Shared components with music display
- **Better Error Handling**: Robust API failure management
- **Cleaner Architecture**: Clear separation of concerns

### 🔧 Maintainability
- **Modular Components**: Easy to modify individual features
- **Shared Infrastructure**: Reuses proven patterns from music display
- **Configuration-Driven**: Easy customization without code changes
- **Extensible**: Simple to add new data sources or chart types

### 📊 Enhanced Features
- **Multiple APIs**: Automatic fallback between data sources
- **Trending Stocks**: Dynamic inclusion of market movers
- **Better Charts**: Dedicated chart rendering engine
- **Improved Display**: Cleaner text layout and alignment

## Requirements

- Raspberry Pi with RGB LED matrix
- Python 3.7+
- Required packages: `rgbmatrix`, `requests`, `python-dotenv`
- Optional: Alpha Vantage API key for backup data source

## Migration from advanced-stock-tracker.py

The original `advanced-stock-tracker.py` has been completely refactored into this modular structure. The new implementation:

- ✅ Eliminates code duplication through shared components
- ✅ Improves error handling and API reliability
- ✅ Provides better configuration management
- ✅ Enables easier feature additions and modifications
- ✅ Maintains all original functionality while adding new features
- ✅ Follows the same proven patterns as the music display application

## Development

### Adding New Data Sources
1. Extend `StockClient` with new fetch method
2. Update `get_current_data()` to include new source
3. Add configuration options to `StockConfig`

### Adding New Chart Types
1. Create new chart class in `chart_renderer.py`
2. Add chart type option to `StockChartRenderer`
3. Update configuration as needed

### Customizing Display
1. Modify `StockMatrixDisplay` methods in `display.py`
2. Update colors and layout in `stock_config.py`
3. Adjust timing and positioning constants