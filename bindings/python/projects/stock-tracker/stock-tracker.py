#!/usr/bin/env python
"""
Stock Tracker for RGB LED Matrix
Displays real-time stock prices with color-coded changes
Uses Alpha Vantage API for stock data (free tier available)
"""
import sys
import os
sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/../../samples'))
from samplebase import SampleBase
from rgbmatrix import graphics
import time
import json
import requests
import threading
from datetime import datetime, timedelta
import os
try:
    from dotenv import load_dotenv
    load_dotenv('stock-tracker.env')
except ImportError:
    print("python-dotenv not installed. Install with: pip install python-dotenv")
    pass


class StockTracker(SampleBase):
    def __init__(self, *args, **kwargs):
        super(StockTracker, self).__init__(*args, **kwargs)
        
        # Override default matrix settings to match your hardware configuration
        self.parser.set_defaults(
            led_rows=32,
            led_cols=64,
            led_chain=1,
            led_parallel=1,
            led_gpio_mapping='adafruit-hat-pwm'
        )
        
        self.parser.add_argument("--api-key", help="Alpha Vantage API key (get free at alphavantage.co)", type=str)
        self.parser.add_argument("--stocks", help="Comma-separated stock symbols (e.g., AAPL,GOOGL,MSFT)", 
                               default=os.environ.get('DEFAULT_STOCKS', "AAPL,GOOGL,MSFT,TSLA,AMZN"), type=str)
        self.parser.add_argument("--refresh-rate", help="Data refresh rate in minutes", 
                               default=int(os.environ.get('REFRESH_RATE', '5')), type=int)
        self.parser.add_argument("--scroll-speed", help="Text scroll speed (lower = faster)", 
                               default=float(os.environ.get('SCROLL_SPEED', '0.05')), type=float)
        self.parser.add_argument("--demo-mode", help="Use demo data instead of API", action="store_true")
        self.parser.add_argument("--config-file", help="Path to configuration file", default="stock-tracker.env", type=str)
        
        # Stock data storage
        self.stock_data = {}
        self.last_update = None
        self.data_lock = threading.Lock()
        self.api_key = None
        
        # Display settings
        self.font = None
        self.colors = {
            'green': graphics.Color(0, 255, 0),    # Price up
            'red': graphics.Color(255, 0, 0),      # Price down
            'white': graphics.Color(255, 255, 255), # Neutral/symbol
            'blue': graphics.Color(0, 150, 255),   # Price value
            'yellow': graphics.Color(255, 255, 0)   # Percentage change
        }

    def get_demo_data(self):
        """Generate demo stock data for testing"""
        import random
        demo_stocks = self.args.stocks.split(',')
        data = {}
        
        for symbol in demo_stocks:
            # Generate realistic-looking stock data
            base_price = random.uniform(50, 500)
            change_percent = random.uniform(-5, 5)
            change_amount = base_price * (change_percent / 100)
            
            data[symbol.strip().upper()] = {
                'price': round(base_price, 2),
                'change': round(change_amount, 2),
                'change_percent': round(change_percent, 2),
                'timestamp': datetime.now().strftime('%H:%M')
            }
        
        return data

    def fetch_stock_data(self):
        """Fetch real stock data from Alpha Vantage API"""
        if self.args.demo_mode:
            return self.get_demo_data()
        
        if not self.api_key:
            print("No API key provided. Use --demo-mode or get a free key at alphavantage.co")
            return self.get_demo_data()
        
        stocks = [s.strip().upper() for s in self.args.stocks.split(',')]
        data = {}
        successful_fetches = 0
        
        for symbol in stocks:
            try:
                url = f"https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol={symbol}&apikey={self.api_key}"
                response = requests.get(url, timeout=15)
                response.raise_for_status()  # Raise exception for HTTP errors
                json_data = response.json()
                
                # Check for API error responses
                if "Error Message" in json_data:
                    print(f"API Error for {symbol}: {json_data['Error Message']}")
                    continue
                elif "Note" in json_data:
                    print(f"API Rate limit reached: {json_data['Note']}")
                    break  # Stop trying more symbols
                elif "Global Quote" in json_data:
                    quote = json_data["Global Quote"]
                    
                    # Validate that we got actual data
                    if not quote or quote.get("05. price") == "0.0000":
                        print(f"No data available for {symbol}")
                        continue
                    
                    try:
                        price = float(quote["05. price"])
                        change = float(quote["09. change"])
                        change_percent = float(quote["10. change percent"].replace('%', ''))
                        
                        data[symbol] = {
                            'price': round(price, 2),
                            'change': round(change, 2),
                            'change_percent': round(change_percent, 2),
                            'timestamp': datetime.now().strftime('%H:%M')
                        }
                        successful_fetches += 1
                        
                    except (KeyError, ValueError) as e:
                        print(f"Error parsing data for {symbol}: {e}")
                        continue
                else:
                    print(f"Unexpected response for {symbol}: {json_data}")
                    continue
                    
                # Add delay to respect API rate limits (5 calls per minute for free tier)
                time.sleep(0.5)
                
            except requests.exceptions.RequestException as e:
                print(f"Network error fetching {symbol}: {e}")
                continue
            except json.JSONDecodeError as e:
                print(f"JSON decode error for {symbol}: {e}")
                continue
            except Exception as e:
                print(f"Unexpected error fetching {symbol}: {e}")
                continue
        
        print(f"Successfully fetched data for {successful_fetches}/{len(stocks)} stocks")
        
        # If no data was fetched and we're not in demo mode, fall back to demo data
        if not data and not self.args.demo_mode:
            print("No stock data available, using demo data")
            return self.get_demo_data()
        
        return data

    def update_stock_data(self):
        """Background thread to update stock data periodically"""
        while True:
            try:
                new_data = self.fetch_stock_data()
                
                with self.data_lock:
                    self.stock_data = new_data
                    self.last_update = datetime.now()
                
                print(f"Updated stock data at {self.last_update.strftime('%H:%M:%S')}")
                
                # Wait for next update
                time.sleep(self.args.refresh_rate * 60)
                
            except Exception as e:
                print(f"Error in update thread: {e}")
                time.sleep(30)  # Retry after 30 seconds on error

    def format_stock_display(self):
        """Format stock data for scrolling display"""
        with self.data_lock:
            if not self.stock_data:
                return "Loading stock data...", self.colors['white']
            
            display_parts = []
            
            for symbol, data in self.stock_data.items():
                price = data['price']
                change = data['change']
                change_percent = data['change_percent']
                
                # Format the display string
                if change >= 0:
                    change_str = f"+${change:.2f} (+{change_percent:.1f}%)"
                else:
                    change_str = f"-${abs(change):.2f} ({change_percent:.1f}%)"
                
                stock_str = f"{symbol}: ${price:.2f} {change_str}"
                display_parts.append(stock_str)
            
            # Add timestamp
            if self.last_update:
                time_str = f"Updated: {self.last_update.strftime('%H:%M')}"
                display_parts.append(time_str)
            
            return "  |  ".join(display_parts), None

    def get_text_color(self, text):
        """Determine color based on stock performance"""
        if '+' in text and '%' in text:
            return self.colors['green']
        elif '-' in text and '%' in text:
            return self.colors['red']
        elif '$' in text:
            return self.colors['blue']
        else:
            return self.colors['white']

    def draw_multi_color_text(self, canvas, font, y_pos, text_parts):
        """Draw text with multiple colors"""
        x_pos = 0
        
        for part, color in text_parts:
            width = graphics.DrawText(canvas, font, x_pos, y_pos, color, part)
            x_pos += width
        
        return x_pos

    def run(self):
        # Load font
        self.font = graphics.Font()
        self.font.LoadFont("../../../../fonts/7x13.bdf")
        
        # Get API key
        self.api_key = self.args.api_key or os.environ.get('ALPHA_VANTAGE_API_KEY')
        
        # Start background data update thread
        update_thread = threading.Thread(target=self.update_stock_data, daemon=True)
        update_thread.start()
        
        # Wait a moment for initial data
        time.sleep(2)
        
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        pos = offscreen_canvas.width
        
        while True:
            offscreen_canvas.Clear()
            
            # Get formatted stock text
            display_text, _ = self.format_stock_display()
            
            # Parse text for color coding
            text_parts = []
            current_part = ""
            
            i = 0
            while i < len(display_text):
                char = display_text[i]
                
                # Look for stock symbols (before colon)
                if char == ':' and current_part:
                    text_parts.append((current_part + ':', self.colors['white']))
                    current_part = ""
                # Look for prices (after $ before space)
                elif char == '$':
                    if current_part:
                        text_parts.append((current_part, self.colors['white']))
                    # Find the end of the price
                    price_end = i + 1
                    while price_end < len(display_text) and display_text[price_end] not in [' ', '+', '-']:
                        price_end += 1
                    price_text = display_text[i:price_end]
                    text_parts.append((price_text, self.colors['blue']))
                    current_part = ""
                    i = price_end - 1
                # Look for changes (+ or -)
                elif char in ['+', '-'] and i > 0 and display_text[i-1] == ' ':
                    if current_part:
                        text_parts.append((current_part, self.colors['white']))
                    # Find the end of the change
                    change_end = i
                    while change_end < len(display_text) and display_text[change_end] not in ['|', ' Updated']:
                        change_end += 1
                    change_text = display_text[i:change_end].strip()
                    color = self.colors['green'] if char == '+' else self.colors['red']
                    text_parts.append((change_text, color))
                    current_part = ""
                    i = change_end - 1
                else:
                    current_part += char
                
                i += 1
            
            # Add any remaining text
            if current_part:
                text_parts.append((current_part, self.colors['white']))
            
            # If parsing failed, use simple approach
            if not text_parts:
                color = self.get_text_color(display_text)
                text_len = graphics.DrawText(offscreen_canvas, self.font, pos, 10, color, display_text)
            else:
                # Draw multicolored text
                temp_pos = pos
                for part, color in text_parts:
                    width = graphics.DrawText(offscreen_canvas, self.font, temp_pos, 10, color, part)
                    temp_pos += width
                text_len = temp_pos - pos
            
            # Update position for scrolling
            pos -= 1
            if pos + text_len < 0:
                pos = offscreen_canvas.width
            
            # Add status line if matrix is tall enough
            if offscreen_canvas.height > 16:
                status_color = self.colors['yellow']
                if self.last_update:
                    time_diff = datetime.now() - self.last_update
                    if time_diff.total_seconds() > 300:  # 5 minutes
                        status_text = "Data may be stale"
                        status_color = self.colors['red']
                    else:
                        status_text = f"Live • {len(self.stock_data)} stocks"
                else:
                    status_text = "Connecting..."
                
                graphics.DrawText(offscreen_canvas, self.font, 0, 25, status_color, status_text)
            
            time.sleep(self.args.scroll_speed)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)


# Main function
if __name__ == "__main__":
    stock_tracker = StockTracker()
    if not stock_tracker.process():
        stock_tracker.print_help()
    else:
        try:
            stock_tracker.run()
        except KeyboardInterrupt:
            print("\nStock tracker stopped.")