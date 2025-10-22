#!/usr/bin/env python
"""
Stock Tracker for RGB LED Matrix
Displays real-time stock prices with color-coded changes
Uses Alpha Vantage API for stock data (free tier available)
"""
import sys
import os
import time
import json
import requests
import threading
from datetime import datetime, timedelta
import argparse

# Add shared components to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'shared'))
from matrix_base import MatrixBase
from font_manager import FontManager
from color_palette import ColorPalette
from config_manager import ConfigManager


class StockTracker(MatrixBase):
    def __init__(self, api_key=None, stocks=None, refresh_rate=5, scroll_speed=0.05, demo_mode=False):
        super().__init__(hardware_mapping='adafruit-hat-pwm')
        
        # Initialize managers
        self.font_manager = FontManager()
        self.colors = ColorPalette('default')
        self.config = ConfigManager()
        
        # Configuration
        self.api_key = api_key or self.config.get('ALPHA_VANTAGE_API_KEY')
        self.stocks = stocks or self.config.get('DEFAULT_STOCKS', 'AAPL,GOOGL,MSFT,TSLA,AMZN')
        self.refresh_rate = refresh_rate or self.config.get('REFRESH_RATE', 5, int)
        self.scroll_speed = scroll_speed or self.config.get('SCROLL_SPEED', 0.05, float)
        self.demo_mode = demo_mode
        
        # Stock data storage
        self.stock_data = {}
        self.last_update = None
        self.data_lock = threading.Lock()
        
        # Load fonts using font manager
        self.font = self.font_manager.get_font('small')  # 7x13.bdf equivalent
        
        # Color mapping using color palette
        self.display_colors = {
            'green': self.colors.get_color('GREEN'),      # Price up
            'red': self.colors.get_color('RED'),          # Price down  
            'white': self.colors.get_color('WHITE'),      # Neutral/symbol
            'blue': self.colors.get_color('BLUE'),        # Price value
            'yellow': self.colors.get_color('YELLOW')     # Percentage change
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
            return self.display_colors['green']
        elif '-' in text and '%' in text:
            return self.display_colors['red']
        elif '$' in text:
            return self.display_colors['blue']
        else:
            return self.display_colors['white']

    def draw_multi_color_text(self, font, y_pos, text_parts):
        """Draw text with multiple colors"""
        x_pos = 0
        
        for part, color in text_parts:
            width = self.draw_text(font, x_pos, y_pos, color, part)
            x_pos += width
        
        return x_pos

    def run(self):
        print("🚀 Starting stock tracker...")
        
        # Start background data update thread
        update_thread = threading.Thread(target=self.update_stock_data, daemon=True)
        update_thread.start()
        
        # Wait a moment for initial data
        time.sleep(2)
        
        pos = self.width  # Use MatrixBase width property
        
        while True:
            self.clear()  # Use MatrixBase clear method
            
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
                    color = self.display_colors['green'] if char == '+' else self.display_colors['red']
                    text_parts.append((change_text, color))
                    current_part = ""
                    i = change_end - 1
                else:
                    current_part += char
                
                i += 1
            
            # Add any remaining text
            if current_part:
                text_parts.append((current_part, self.display_colors['white']))
            
            # If parsing failed, use simple approach
            if not text_parts:
                color = self.get_text_color(display_text)
                text_len = self.draw_text(self.font, pos, 10, color, display_text)
            else:
                # Draw multicolored text
                temp_pos = pos
                for part, color in text_parts:
                    width = self.draw_text(self.font, temp_pos, 10, color, part)
                    temp_pos += width
                text_len = temp_pos - pos
            
            # Update position for scrolling
            pos -= 1
            if pos + text_len < 0:
                pos = self.width
            
            # Add status line if matrix is tall enough
            if self.height > 16:
                status_color = self.display_colors['yellow']
                if self.last_update:
                    time_diff = datetime.now() - self.last_update
                    if time_diff.total_seconds() > 300:  # 5 minutes
                        status_text = "Data may be stale"
                        status_color = self.display_colors['red']
                    else:
                        status_text = f"Live • {len(self.stock_data)} stocks"
                else:
                    status_text = "Connecting..."
                
                self.draw_text(self.font, 0, 25, status_color, status_text)
            
            time.sleep(self.scroll_speed)
            self.swap()


# Main function with argument parsing
def main():
    parser = argparse.ArgumentParser(description='Stock Tracker for RGB LED Matrix')
    parser.add_argument("--api-key", help="Alpha Vantage API key", type=str)
    parser.add_argument("--stocks", help="Comma-separated stock symbols", 
                       default="AAPL,GOOGL,MSFT,TSLA,AMZN", type=str)
    parser.add_argument("--refresh-rate", help="Data refresh rate in minutes", 
                       default=5, type=int)
    parser.add_argument("--scroll-speed", help="Text scroll speed", 
                       default=0.05, type=float)
    parser.add_argument("--demo-mode", help="Use demo data", action="store_true")
    
    args = parser.parse_args()
    
    try:
        stock_tracker = StockTracker(
            api_key=args.api_key,
            stocks=args.stocks,
            refresh_rate=args.refresh_rate,
            scroll_speed=args.scroll_speed,
            demo_mode=args.demo_mode
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