#!/usr/bin/env python
"""
Advanced Stock Tracker with Charts for RGB LED Matrix
Displays stock prices with historical charts and color-coded performance
Uses Alpha Vantage API for stock data and historical prices
"""
from samplebase import SampleBase
from rgbmatrix import graphics
import time
import json
import requests
import threading
from datetime import datetime, timedelta
import os
import math
try:
    from dotenv import load_dotenv
    load_dotenv('stock-tracker.env')
except ImportError:
    print("python-dotenv not installed. Install with: pip install python-dotenv")
    pass


class AdvancedStockTracker(SampleBase):
    def __init__(self, *args, **kwargs):
        super(AdvancedStockTracker, self).__init__(*args, **kwargs)
        
        # Override default matrix settings to match your hardware configuration
        self.parser.set_defaults(
            led_rows=32,
            led_cols=64,
            led_chain=1,
            led_parallel=1,
            led_gpio_mapping='adafruit-hat-pwm'
        )
        
        self.parser.add_argument("--api-key", help="Alpha Vantage API key (get free at alphavantage.co)", 
                               default=os.environ.get('ALPHA_VANTAGE_API_KEY'), type=str)
        self.parser.add_argument("--stocks", help="Comma-separated stock symbols (e.g., AAPL,GOOGL,MSFT)", 
                               default=os.environ.get('DEFAULT_STOCKS', "AAPL,GOOGL,MSFT,TSLA"), type=str)
        self.parser.add_argument("--refresh-rate", help="Data refresh rate in minutes", 
                               default=int(os.environ.get('REFRESH_RATE', '5')), type=int)
        self.parser.add_argument("--display-time", help="Time to show each stock in seconds", 
                               default=int(os.environ.get('DISPLAY_TIME', '10')), type=int)
        self.parser.add_argument("--demo-mode", help="Use demo data instead of API", action="store_true")
        self.parser.add_argument("--chart-days", help="Number of days of historical data for chart", 
                               default=int(os.environ.get('CHART_DAYS', '30')), type=int)
        
        # Stock data storage
        self.stock_data = {}
        self.stock_history = {}  # Historical data for charts
        self.last_update = None
        self.data_lock = threading.Lock()
        self.api_key = None
        
        # Display settings
        self.font_large = None
        self.font_small = None
        self.current_stock_index = 0
        self.stock_symbols = []
        
        # Colors for different states
        self.colors = {
            'gain_bright': graphics.Color(0, 255, 0),     # Bright green for positive
            'gain_dim': graphics.Color(0, 180, 0),        # Dim green for chart
            'loss_bright': graphics.Color(255, 0, 0),     # Bright red for negative  
            'loss_dim': graphics.Color(180, 0, 0),        # Dim red for chart
            'neutral': graphics.Color(255, 255, 255),     # White for neutral
            'chart_line': graphics.Color(0, 120, 0),      # Chart line color
            'text_secondary': graphics.Color(200, 200, 200)  # Secondary text
        }

    def get_demo_data(self):
        """Generate demo stock data with historical prices for charts"""
        import random
        demo_stocks = self.args.stocks.split(',')
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
            for i in range(50, 0, -1):  # Last 50 data points
                # Create realistic price movement
                if i == 50:
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
        if self.args.demo_mode or not self.api_key:
            return None
            
        try:
            url = f"https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol={symbol}&apikey={self.api_key}"
            response = requests.get(url, timeout=15)
            response.raise_for_status()
            data = response.json()
            
            if "Time Series (Daily)" in data:
                time_series = data["Time Series (Daily)"]
                prices = []
                
                # Get last 50 closing prices (or fewer if not available)
                sorted_dates = sorted(time_series.keys(), reverse=True)
                for date in sorted_dates[:50]:
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

    def fetch_stock_data(self):
        """Fetch real stock data from Alpha Vantage API"""
        if self.args.demo_mode:
            return self.get_demo_data()
        
        if not self.api_key:
            print("No API key provided. Use --demo-mode or get a free key at alphavantage.co")
            return self.get_demo_data()
        
        stocks = [s.strip().upper() for s in self.args.stocks.split(',')]
        current_data = {}
        history_data = {}
        successful_fetches = 0
        
        for symbol in stocks:
            try:
                # Fetch current quote
                url = f"https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol={symbol}&apikey={self.api_key}"
                response = requests.get(url, timeout=15)
                response.raise_for_status()
                json_data = response.json()
                
                if "Error Message" in json_data:
                    print(f"API Error for {symbol}: {json_data['Error Message']}")
                    continue
                elif "Note" in json_data:
                    print(f"API Rate limit reached: {json_data['Note']}")
                    break
                elif "Global Quote" in json_data:
                    quote = json_data["Global Quote"]
                    
                    if not quote or quote.get("05. price") == "0.0000":
                        print(f"No data available for {symbol}")
                        continue
                    
                    try:
                        price = float(quote["05. price"])
                        change = float(quote["09. change"])
                        change_percent = float(quote["10. change percent"].replace('%', ''))
                        
                        current_data[symbol] = {
                            'price': round(price, 2),
                            'change': round(change, 2),
                            'change_percent': round(change_percent, 2),
                            'timestamp': datetime.now().strftime('%H:%M')
                        }
                        
                        # Fetch historical data if not already cached
                        if symbol not in self.stock_history:
                            historical_prices = self.fetch_historical_data(symbol)
                            if historical_prices:
                                history_data[symbol] = historical_prices
                        
                        successful_fetches += 1
                        
                    except (KeyError, ValueError) as e:
                        print(f"Error parsing data for {symbol}: {e}")
                        continue
                        
                # Add delay to respect API rate limits
                time.sleep(1)  # Longer delay for historical data calls
                
            except requests.exceptions.RequestException as e:
                print(f"Network error fetching {symbol}: {e}")
                continue
            except Exception as e:
                print(f"Unexpected error fetching {symbol}: {e}")
                continue
        
        print(f"Successfully fetched data for {successful_fetches}/{len(stocks)} stocks")
        
        if not current_data and not self.args.demo_mode:
            print("No stock data available, using demo data")
            return self.get_demo_data()
        
        return current_data, history_data

    def update_stock_data(self):
        """Background thread to update stock data periodically"""
        print("Data update thread started")
        while True:
            try:
                print("Fetching stock data...")
                current_data, history_data = self.fetch_stock_data()
                print(f"Fetched data for symbols: {list(current_data.keys())}")
                
                with self.data_lock:
                    self.stock_data = current_data
                    # Update historical data
                    for symbol, prices in history_data.items():
                        self.stock_history[symbol] = prices
                    self.last_update = datetime.now()
                
                print(f"Updated stock data at {self.last_update.strftime('%H:%M:%S')}")
                print(f"Stock data now contains: {list(self.stock_data.keys())}")
                
                # Wait for next update
                print(f"Waiting {self.args.refresh_rate} minutes for next update...")
                time.sleep(self.args.refresh_rate * 60)
                
            except Exception as e:
                print(f"Error in update thread: {e}")
                import traceback
                traceback.print_exc()
                time.sleep(30)

    def draw_stock_chart(self, canvas, symbol, x_start, y_start, width, height):
        """Draw a mini stock chart for the given symbol"""
        with self.data_lock:
            if symbol not in self.stock_history or not self.stock_history[symbol]:
                return
            
            prices = self.stock_history[symbol]
            if len(prices) < 2:
                return
            
            # Normalize prices to fit in the chart area
            min_price = min(prices)
            max_price = max(prices)
            price_range = max_price - min_price
            
            if price_range == 0:  # All prices the same
                y = y_start + height // 2
                for x in range(width):
                    canvas.SetPixel(x_start + x, y, *self.colors['chart_line'])
                return
            
            # Determine chart color based on overall trend
            trend_color = self.colors['gain_dim'] if prices[-1] > prices[0] else self.colors['loss_dim']
            
            # Draw the price line
            prev_x = None
            prev_y = None
            
            for i, price in enumerate(prices[-width:]):  # Only use last 'width' data points
                x = x_start + (i * width) // len(prices[-width:])
                # Invert Y because screen coordinates are flipped
                y = y_start + height - int(((price - min_price) / price_range) * height)
                y = max(y_start, min(y_start + height - 1, y))  # Clamp to bounds
                
                if prev_x is not None:
                    # Draw line between points
                    self.draw_line(canvas, prev_x, prev_y, x, y, trend_color)
                
                prev_x = x
                prev_y = y

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
        print("Starting advanced stock tracker...")
        
        # Load fonts - exact same method as image-viewer.py (no error checking)
        self.font_large = graphics.Font()
        self.font_large.LoadFont("../../../fonts/7x13.bdf")
        self.font_small = graphics.Font()
        self.font_small.LoadFont("../../../fonts/5x7.bdf")
        
        # Get API key (already loaded from environment in argument defaults)
        self.api_key = self.args.api_key
        print(f"API key configured: {bool(self.api_key)}")
        print(f"Demo mode: {self.args.demo_mode}")
        
        # Initialize stock symbols list
        self.stock_symbols = [s.strip().upper() for s in self.args.stocks.split(',')]
        print(f"Stock symbols: {self.stock_symbols}")
        
        # Start background data update thread
        print("Starting background data update thread...")
        update_thread = threading.Thread(target=self.update_stock_data, daemon=True)
        update_thread.start()
        
        # Wait for initial data
        print("Waiting for initial data...")
        time.sleep(3)
        
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        
        # Simple test first - just like runtext.py
        textColor = graphics.Color(255, 255, 0)
        pos = 0
        
        while True:
            offscreen_canvas.Clear()
            
            # Simple scrolling text test first
            text = "AAPL $150.25 +2.15 (+1.4%)"
            len_text = graphics.DrawText(offscreen_canvas, self.font_large, pos, 15, textColor, text)
            pos -= 1
            if (pos + len_text < 0):
                pos = offscreen_canvas.width
            
            time.sleep(0.05)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)


# Main function
if __name__ == "__main__":
    stock_tracker = AdvancedStockTracker()
    if not stock_tracker.process():
        stock_tracker.print_help()
    else:
        try:
            stock_tracker.run()
        except KeyboardInterrupt:
            print("\nAdvanced stock tracker stopped.")