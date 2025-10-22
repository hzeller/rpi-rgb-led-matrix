#!/usr/bin/env python
"""
Stock Tracker with Charts for RGB LED Matrix
Displays stock prices with historical charts and color-coded performance
Uses Alpha Vantage API for stock data and historical prices
"""
import sys
import os
import time
import json
import requests
import threading
from datetime import datetime, timedelta
import math
import random
import argparse

# Add shared components to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette
from config_manager import ConfigManager


class StockTracker(MatrixBase):
    def __init__(self, api_key=None, stocks=None, refresh_rate=5, display_time=10, 
                 demo_mode=False, chart_days=30, include_trending=False, trending_count=3):
        super().__init__(hardware_mapping='adafruit-hat-pwm')
        
        # Initialize managers
        self.font_manager = FontManager()
        self.colors = ColorPalette('default')
        self.config = ConfigManager()
        
        # Configuration
        self.api_key = api_key or self.config.get('ALPHA_VANTAGE_API_KEY')
        self.stocks = stocks or self.config.get('DEFAULT_STOCKS', 'AAPL,GOOGL,MSFT,TSLA')
        self.refresh_rate = refresh_rate or self.config.get('REFRESH_RATE', 5, int)
        self.display_time = display_time or self.config.get('DISPLAY_TIME', 10, int)
        self.demo_mode = demo_mode
        self.chart_days = chart_days or self.config.get('CHART_DAYS', 30, int)
        self.include_trending = include_trending
        self.trending_count = trending_count or self.config.get('TRENDING_COUNT', 3, int)
        
        # Stock data storage
        self.stock_data = {}
        self.stock_history = {}  # Historical data for charts
        self.last_update = None
        self.data_lock = threading.Lock()
        
        # Display-safe copies (no locking required)
        self.display_stock_data = {}
        self.display_stock_history = {}
        
        # Display settings
        self.font_large = self.font_manager.get_font('medium')  # 7x13.bdf equivalent
        self.font_small = self.font_manager.get_font('small')   # 5x7.bdf equivalent
        self.current_stock_index = 0
        self.stock_symbols = []
        
        # Colors for different states using color palette
        self.display_colors = {
            'gain_bright': self.colors.get_color('GAIN_BRIGHT'),
            'gain_dim': self.colors.get_color('GAIN_DIM'),
            'loss_bright': self.colors.get_color('LOSS_BRIGHT'),
            'loss_dim': self.colors.get_color('LOSS_DIM'),
            'neutral': self.colors.get_color('WHITE'),
            'chart_line': self.colors.get_color('GAIN_DIM'),
            'text_secondary': self.colors.get_color('GRAY_LIGHT')
        }

    def get_demo_data(self):
        """Generate demo stock data with historical prices for charts"""
        import random
        demo_stocks = self.stocks.split(',')
        current_data = {}
        history_data = {}
        
        for symbol in demo_stocks:
            symbol = symbol.strip().upper()
            
            # Generate current data
            base_price = random.uniform(50, 500)
            change_percent = random.uniform(-5, 5)
            change_amount = base_price * (change_percent / 100)
            
            current_data[symbol] = {
                'price': round(base_price, 2),
                'change': round(change_amount, 2),
                'change_percent': round(change_percent, 2),
                'timestamp': datetime.now().strftime('%H:%M')
            }
            
            # Generate historical data for chart
            history_data[symbol] = []
            for i in range(64, 0, -1):  # Last 64 data points
                # Create realistic price movement
                if i == 64:
                    price = base_price - (change_amount * 2)  # Starting price
                else:
                    # Random walk with slight upward trend if current change is positive
                    trend = 0.1 if change_percent > 0 else -0.1
                    price_change = random.uniform(-2, 2) + trend
                    price = max(history_data[symbol][-1] + price_change, 10)  # Minimum $10
                
                history_data[symbol].append(round(price, 2))
        
        return current_data, history_data

    def fetch_historical_data(self, symbol):
        """Fetch historical stock data for chart"""
        if self.demo_mode or not self.api_key:
            return None
            
        try:
            url = f"https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol={symbol}&apikey={self.api_key}"
            response = requests.get(url, timeout=15)
            response.raise_for_status()
            data = response.json()
            
            if "Time Series (Daily)" in data:
                time_series = data["Time Series (Daily)"]
                prices = []
                
                # Get last 64 closing prices (or fewer if not available)
                sorted_dates = sorted(time_series.keys(), reverse=True)
                for date in sorted_dates[:64]:
                    close_price = float(time_series[date]["4. close"])
                    prices.append(close_price)
                
                # Reverse to get chronological order (oldest first)
                return prices[::-1] if prices else None
            else:
                print(f"No historical data for {symbol}: {data}")
                return None
                
        except Exception as e:
            print(f"Error fetching historical data for {symbol}: {e}")
            return None

    def fetch_trending_stocks(self):
        """Fetch trending stocks (biggest gainers and losers) from Yahoo Finance"""
        trending_symbols = []
        
        try:
            # Try Yahoo Finance screener for gainers/losers
            # This endpoint gives us the biggest movers
            screener_urls = [
                "https://query1.finance.yahoo.com/v1/finance/screener/predefined/saved?formatted=true&lang=en-US&region=US&scrIds=day_gainers&count=10",
                "https://query1.finance.yahoo.com/v1/finance/screener/predefined/saved?formatted=true&lang=en-US&region=US&scrIds=day_losers&count=10"
            ]
            
            headers = {
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
            }
            
            for url in screener_urls:
                try:
                    response = requests.get(url, headers=headers, timeout=10)
                    response.raise_for_status()
                    data = response.json()
                    
                    if 'finance' in data and 'result' in data['finance'] and data['finance']['result']:
                        quotes = data['finance']['result'][0].get('quotes', [])
                        
                        # Get top movers
                        for quote in quotes[:self.trending_count]:
                            symbol = quote.get('symbol', '').upper()
                            change_percent = quote.get('regularMarketChangePercent', {}).get('raw', 0)
                            
                            if symbol and symbol not in trending_symbols:
                                trending_symbols.append(symbol)
                                print(f"✓ Found trending stock: {symbol} ({change_percent:+.1f}%)")
                                
                                if len(trending_symbols) >= self.trending_count:
                                    break
                    
                    if len(trending_symbols) >= self.trending_count:
                        break
                        
                except Exception as inner_e:
                    print(f"Screener API failed: {inner_e}")
                    continue
                    
        except Exception as e:
            print(f"Could not fetch trending stocks: {e}")
        
        # Fallback: use some popular volatile stocks if API fails
        if not trending_symbols:
            fallback_trending = ['GME', 'AMC', 'PLTR', 'RIVN', 'LCID', 'SOFI']
            trending_symbols = fallback_trending[:self.trending_count]
            print(f"Using fallback trending stocks: {trending_symbols}")
            
        return trending_symbols[:self.trending_count]

    def fetch_yahoo_finance_data(self, symbol):
        """Fetch stock data from Yahoo Finance (free, no API key required)"""
        try:
            # Yahoo Finance API endpoint
            url = f"https://query1.finance.yahoo.com/v8/finance/chart/{symbol}"
            
            headers = {
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
            }
            
            response = requests.get(url, headers=headers, timeout=10)
            response.raise_for_status()
            data = response.json()
            
            if 'chart' in data and 'result' in data['chart'] and data['chart']['result']:
                result = data['chart']['result'][0]
                meta = result['meta']
                
                current_price = meta.get('regularMarketPrice', 0)
                prev_close = meta.get('previousClose', current_price)
                change = current_price - prev_close
                change_percent = (change / prev_close * 100) if prev_close != 0 else 0
                
                return {
                    'price': round(current_price, 2),
                    'change': round(change, 2),
                    'change_percent': round(change_percent, 2),
                    'timestamp': datetime.now().strftime('%H:%M')
                }
            
        except Exception as e:
            print(f"Yahoo Finance error for {symbol}: {e}")
            return None
    
    def fetch_stock_data(self):
        """Fetch real stock data from multiple free APIs"""
        if self.demo_mode:
            return self.get_demo_data()
        
        # Start with hardcoded stocks
        stocks = [s.strip().upper() for s in self.stocks.split(',')]
        
        # Add trending stocks if requested
        if self.include_trending:
            print("Fetching trending stocks...")
            trending_stocks = self.fetch_trending_stocks()
            if trending_stocks:
                # Add trending stocks, avoid duplicates
                for symbol in trending_stocks:
                    if symbol not in stocks:
                        stocks.append(symbol)
                        print(f"+ Added trending stock: {symbol}")
            else:
                print("No trending stocks found, using hardcoded list only")
        
        print(f"Final stock list: {stocks}")
        current_data = {}
        history_data = {}
        successful_fetches = 0
        
        # Try Yahoo Finance first (free, no API key needed)
        print("Fetching data from Yahoo Finance (free API)...")
        for symbol in stocks:
            yahoo_data = self.fetch_yahoo_finance_data(symbol)
            if yahoo_data:
                current_data[symbol] = yahoo_data
                successful_fetches += 1
                print(f"✓ Got {symbol}: ${yahoo_data['price']:.2f}")
            else:
                print(f"✗ Failed to get {symbol}")
            
            time.sleep(0.2)  # Small delay to be respectful
        
        # If Yahoo Finance worked, generate demo historical data
        if successful_fetches > 0:
            print("Generating demo historical data for charts...")
            for symbol in current_data:
                # Generate realistic historical data based on current price
                base_price = current_data[symbol]['price']
                change_percent = current_data[symbol]['change_percent']
                print(f"DEBUG: Generating history for {symbol}: base_price={base_price:.2f}, change%={change_percent:.2f}")
                
                history_data[symbol] = []
                
                # Generate 64 historical data points (one for each pixel width)
                for i in range(64):
                    if i == 0:
                        # Starting price (oldest data point)
                        start_price = base_price - (current_data[symbol]['change'] * 2)  # Simpler calculation
                        start_price = max(start_price, base_price * 0.3)  # Don't go too low
                    else:
                        # Random walk towards current price
                        prev_price = history_data[symbol][-1]
                        trend = 0.01 if change_percent > 0 else -0.01
                        price_change = (random.uniform(-0.5, 0.5) + trend) * prev_price * 0.02
                        start_price = max(prev_price + price_change, base_price * 0.3)
                    
                    history_data[symbol].append(round(start_price, 2))
                
                print(f"DEBUG: Generated {len(history_data[symbol])} historical prices for {symbol}: {history_data[symbol][:5]}...{history_data[symbol][-5:]}")
        
        # Fallback to Alpha Vantage if available
        elif self.api_key:
            print("Yahoo Finance failed, trying Alpha Vantage...")
            return self.fetch_alpha_vantage_data()
        
        # Final fallback to demo data
        if not current_data:
            print("All APIs failed, using demo data")
            return self.get_demo_data()
        
        print(f"Successfully fetched data for {successful_fetches}/{len(stocks)} stocks")
        return current_data, history_data

    def fetch_alpha_vantage_data(self):
        """Fallback to Alpha Vantage API if available"""
        stocks = [s.strip().upper() for s in self.stocks.split(',')]
        current_data = {}
        history_data = {}
        successful_fetches = 0
        
        for symbol in stocks:
            try:
                url = f"https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol={symbol}&apikey={self.api_key}"
                response = requests.get(url, timeout=15)
                response.raise_for_status()
                json_data = response.json()
                
                if "Note" in json_data:
                    print(f"Alpha Vantage rate limit reached")
                    break
                elif "Global Quote" in json_data:
                    quote = json_data["Global Quote"]
                    if quote and quote.get("05. price") != "0.0000":
                        price = float(quote["05. price"])
                        change = float(quote["09. change"])
                        change_percent = float(quote["10. change percent"].replace('%', ''))
                        
                        current_data[symbol] = {
                            'price': round(price, 2),
                            'change': round(change, 2),
                            'change_percent': round(change_percent, 2),
                            'timestamp': datetime.now().strftime('%H:%M')
                        }
                        successful_fetches += 1
                
                time.sleep(0.5)
                
            except Exception as e:
                print(f"Alpha Vantage error for {symbol}: {e}")
                continue
        
        return current_data, history_data

    def update_stock_data(self):
        """Background thread to update stock data periodically"""
        print("Data update thread started")
        while True:
            try:
                print("Fetching stock data...")
                current_data, history_data = self.fetch_stock_data()
                print(f"Fetched data for symbols: {list(current_data.keys())}")
                
                # Update both locked and display-safe data
                with self.data_lock:
                    self.stock_data = current_data
                    self.stock_history.update(history_data)  
                    self.last_update = datetime.now()
                
                # Update display-safe copies (no lock needed for reading)
                self.display_stock_data = dict(current_data)  # Make a copy
                self.display_stock_history = {}
                for symbol, prices in history_data.items():
                    self.display_stock_history[symbol] = list(prices)  # Make a copy
                    print(f"DEBUG: Stored {len(prices)} historical prices for {symbol}")
                
                print(f"DEBUG: Historical data now available for: {list(self.display_stock_history.keys())}")
                
                print(f"✓ Updated at {self.last_update.strftime('%H:%M:%S')} - {len(self.stock_data)} stocks")
                
                # Wait for next update
                time.sleep(self.refresh_rate * 60)
                
            except Exception as e:
                print(f"Error in update thread: {e}")
                import traceback
                traceback.print_exc()
                time.sleep(30)

    def draw_stock_chart(self, symbol, x_start, y_start, width, height):
        """Draw a simple, safe time series chart for the given symbol"""
        try:
            # Safety bounds check
            if x_start < 0 or y_start < 0 or width <= 0 or height <= 0:
                return
            if x_start + width > 64 or y_start + height > 32:
                return
                
            # Get historical data from display-safe copy (no locking needed!)
            prices = None
            
            # Use display-safe data that doesn't require locking
            if symbol in self.display_stock_history and self.display_stock_history[symbol]:
                prices = self.display_stock_history[symbol][-width:]  # Last 'width' data points (should be 64)
            else:
                prices = None
            
            if not prices or len(prices) < 2:
                self.draw_demo_chart(x_start, y_start, width, height)
                return
                
            # Simple safety checks
            min_price = min(prices)
            max_price = max(prices)
            price_range = max_price - min_price
            
            if price_range <= 0:
                return
                
            # Draw filled area chart with lighter top line
            pixels_drawn = 0
            for i in range(min(len(prices), width)):
                # Calculate position
                x = x_start + i
                price_ratio = (prices[i] - min_price) / price_range
                chart_height = max(1, int(price_ratio * (height - 2)))
                top_y = y_start + height - 1 - chart_height
                
                # Fill all pixels from bottom up to the chart line
                for fill_y in range(y_start + height - 1, top_y - 1, -1):  # From bottom to top_y
                    if 0 <= x < 64 and 0 <= fill_y < 32:
                        if fill_y <= top_y + 1:  # Top 2 pixels get lighter green
                            color = self.display_colors['gain_bright']
                        else:
                            color = self.display_colors['gain_dim']
                        self.set_pixel(x, fill_y, color)
                        pixels_drawn += 1
            
            # Chart drawn successfully
                
        except Exception as e:
            print(f"DEBUG: Chart drawing error: {e}")
            import traceback
            traceback.print_exc()
    
    def draw_demo_chart(self, x_start, y_start, width, height):
        """Draw a simple, safe demo chart pattern when no real data is available"""
        try:
            # Safety bounds check
            if x_start < 0 or y_start < 0 or width <= 0 or height <= 0:
                return
            if x_start + width > 64 or y_start + height > 32:
                return
                
            import math
            pixels_drawn = 0
            # Draw filled wave pattern
            for x in range(min(width, 64)):
                if x_start + x >= 64:
                    break
                    
                # Create a gentle sine wave
                wave = math.sin(x * 0.15) * 0.3 + 0.5  # Normalize to 0.2-0.8
                chart_height = max(1, min(height - 2, int(wave * (height - 2))))
                top_y = y_start + height - 1 - chart_height
                
                # Fill from bottom up to the wave line
                for fill_y in range(y_start + height - 1, top_y - 1, -1):
                    if 0 <= x_start + x < 64 and 0 <= fill_y < 32:
                        if fill_y <= top_y + 1:  # Top 2 pixels get lighter green
                            color = self.display_colors['chart_line']
                        else:
                            color = self.display_colors['gain_dim']
                        self.set_pixel(x_start + x, fill_y, color)
                        pixels_drawn += 1
            
            # Demo chart drawn
                    
        except Exception as e:
            print(f"DEBUG: Demo chart error: {e}")
            import traceback
            traceback.print_exc()

    def draw_line(self, canvas, x0, y0, x1, y1, color):
        """Draw a line between two points using Bresenham's algorithm"""
        dx = abs(x1 - x0)
        dy = abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx - dy
        
        x, y = x0, y0
        
        while True:
            canvas.SetPixel(x, y, color.red, color.green, color.blue)
            
            if x == x1 and y == y1:
                break
                
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x += sx
            if e2 < dx:
                err += dx
                y += sy

    def run(self):
        print("🚀 Starting stock tracker...")
        
        # Fonts are already loaded in __init__
        print("✓ Fonts loaded")
        
        print(f"API key configured: {bool(self.api_key)}")
        print(f"Demo mode: {self.demo_mode}")
        
        # Initialize with stock symbols
        self.stock_symbols = [s.strip().upper() for s in self.stocks.split(',')]
        print(f"Initial stock symbols: {self.stock_symbols}")
        
        # Start background data update thread
        print("Starting background data update thread...")
        update_thread = threading.Thread(target=self.update_stock_data, daemon=True)
        update_thread.start()
        
        # Wait for initial data to be loaded
        print("Waiting for stock data to load...")
        max_wait_time = 15  # Wait up to 15 seconds
        wait_start = time.time()
        
        while len(self.stock_data) == 0 and (time.time() - wait_start) < max_wait_time:
            print(".", end="", flush=True)
            time.sleep(0.5)
        
        print()  # New line after dots
        
        if len(self.stock_data) > 0:
            print(f"✓ Stock data ready! Loaded {len(self.stock_data)} stocks")
            # Update stock symbols to include all fetched stocks (hardcoded + trending)
            self.stock_symbols = list(self.stock_data.keys())
            print(f"✓ Display will cycle through: {self.stock_symbols}")
        else:
            print("⚠ No stock data loaded, using demo mode")
            # Force demo data if nothing loaded
            demo_current, demo_history = self.get_demo_data()
            with self.data_lock:
                self.stock_data = demo_current
                self.stock_history = demo_history
                # Update symbols for demo data too
                self.stock_symbols = list(demo_current.keys())
                print(f"✓ Demo data loaded for {len(self.stock_data)} stocks")
                print(f"✓ Demo display will cycle through: {self.stock_symbols}")
        
        print("✓ Matrix canvas ready")
        
        last_switch_time = time.time()
        print("Starting main display loop...")
        
        loop_count = 0
        while True:
            loop_count += 1
            # Reduce debug spam - only print important events
            
            self.clear()
            
            # Check if we need to switch to the next stock
            current_time = time.time()
            if current_time - last_switch_time >= self.display_time:
                old_index = self.current_stock_index
                self.current_stock_index = (self.current_stock_index + 1) % len(self.stock_symbols)
                last_switch_time = current_time
                print(f"DEBUG: Switched from stock index {old_index} to {self.current_stock_index} (cycling through {len(self.stock_symbols)} stocks)")
            
            # Get current stock to display  
            if self.stock_symbols and len(self.display_stock_data) > 0:
                print(f"DEBUG: Current stock index {self.current_stock_index} of {len(self.stock_symbols)} stocks")
                current_symbol = self.stock_symbols[self.current_stock_index]
                
                # Use display-safe data (no locking needed)
                if current_symbol in self.display_stock_data:
                    stock_info = self.display_stock_data[current_symbol]
                    
                    # Left side uses white color for symbol and price
                    left_color = self.display_colors['neutral']  # White color
                    
                    # Right side uses green/red based on performance
                    is_positive = stock_info['change'] >= 0
                    right_color = self.display_colors['gain_bright'] if is_positive else self.display_colors['loss_bright']
                    
                    # Left side - Stock symbol and price (white) with 2px padding
                    self.draw_text(self.font_large, 2, 2, left_color, current_symbol)  # 2px left, 2px top padding
                    price_text = f"{stock_info['price']:.2f}"
                    self.draw_text(self.font_large, 2, 9, left_color, price_text)  # 1px gap from symbol (2+6+1=9)
                    
                    # Right side - Change amount and percentage (green/red based on value)
                    # Use fixed positioning to prevent text from going off screen
                    
                    # Top right: Change amount (colored) - positioned at middle of display
                    change_text = f"{stock_info['change']:+.2f}"  # Include +/- sign
                    self.draw_text(self.font_large, 32, 2, right_color, change_text)  # Start at middle of 64px width
                    
                    # Bottom right: Percentage (colored)  
                    pct_text = f"{stock_info['change_percent']:+.1f}%"  # Include +/- sign
                    self.draw_text(self.font_large, 32, 9, right_color, pct_text)  # 1px gap from change text
                    
                    # Draw time series chart in remaining space (after text area)
                    chart_x = 0
                    chart_y = 16  # Start after text area (2px top + 6px font + 1px gap + 6px font + 1px = 16px)
                    chart_width = 64
                    chart_height = 16  # Remaining pixels to bottom of 32px display
                    
                    self.draw_stock_chart(current_symbol, chart_x, chart_y, chart_width, chart_height)
                    
                else:
                    # Loading state - match the same positioning as normal display
                    self.draw_text(self.font_large, 2, 2, self.display_colors['neutral'], current_symbol)
                    self.draw_text(self.font_large, 2, 9, self.display_colors['neutral'], "Loading...")
            else:
                # No data yet
                self.draw_text(self.font_large, 2, 9, self.display_colors['neutral'], "Loading...")
            
            self.swap()
            time.sleep(0.5)  # Slower refresh rate


# Main function with argument parsing
def main():
    parser = argparse.ArgumentParser(description='Stock Tracker with Charts')
    parser.add_argument("--api-key", help="Alpha Vantage API key", type=str)
    parser.add_argument("--stocks", help="Comma-separated stock symbols", 
                       default="AAPL,GOOGL,MSFT,TSLA", type=str)
    parser.add_argument("--refresh-rate", help="Data refresh rate in minutes", 
                       default=5, type=int)
    parser.add_argument("--display-time", help="Time to show each stock in seconds",
                       default=10, type=int)
    parser.add_argument("--demo-mode", help="Use demo data", action="store_true")
    parser.add_argument("--chart-days", help="Days of historical data", 
                       default=30, type=int)
    parser.add_argument("--include-trending", help="Include trending stocks", 
                       action="store_true")
    parser.add_argument("--trending-count", help="Number of trending stocks",
                       default=3, type=int)
    
    args = parser.parse_args()
    
    try:
        stock_tracker = StockTracker(
            api_key=args.api_key,
            stocks=args.stocks,
            refresh_rate=args.refresh_rate,
            display_time=args.display_time,
            demo_mode=args.demo_mode,
            chart_days=args.chart_days,
            include_trending=args.include_trending,
            trending_count=args.trending_count
        )
        stock_tracker.run()
    except KeyboardInterrupt:
        print("\n🛑 Stock tracker stopped.")
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()