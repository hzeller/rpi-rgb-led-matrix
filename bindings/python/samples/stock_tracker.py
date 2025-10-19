#!/usr/bin/env python
"""
Advanced Stock Tracker for RGB LED Matrix

A modular stock tracking application that displays stock prices with historical 
charts and color-coded performance indicators. Supports multiple data sources
including Yahoo Finance and Alpha Vantage APIs.

Usage:
    # Live mode with Yahoo Finance (free)
    python stock_tracker.py
    
    # Demo mode
    python stock_tracker.py --demo-mode
    
    # With specific stocks
    python stock_tracker.py --stocks "AAPL,GOOGL,TSLA"
    
    # Include trending stocks
    python stock_tracker.py --include-trending
"""

import sys
import time
import queue
import threading
import argparse
from datetime import datetime
from dotenv import load_dotenv

from stock_client import StockClient, StockUpdateThread, TrendingStockFetcher
from display import StockMatrixDisplay
from chart_renderer import StockChartRenderer
from stock_config import StockConfig


class StockTracker:
    """Main application class that orchestrates the stock tracking functionality."""
    
    def __init__(self, stocks=None, demo_mode=False, include_trending=False, 
                 trending_count=3, refresh_rate=5, display_time=10, 
                 alpha_vantage_key=None):
        
        # Configuration
        self.stocks = stocks or StockConfig.DEFAULT_STOCKS.split(',')
        self.stocks = [s.strip().upper() for s in self.stocks]
        self.demo_mode = demo_mode
        self.include_trending = include_trending
        self.trending_count = trending_count
        self.refresh_rate = refresh_rate  # minutes
        self.display_time = display_time  # seconds
        
        # Initialize components
        self.stock_client = StockClient()
        if alpha_vantage_key:
            self.stock_client.alpha_vantage_key = alpha_vantage_key
            
        self.display = StockMatrixDisplay()
        self.chart_renderer = StockChartRenderer(self.display.canvas)
        self.chart_renderer.display_instance = self.display
        
        # Data management
        self.update_queue = queue.Queue(maxsize=1)
        self.stock_update_thread = StockUpdateThread(
            self.stock_client, self.update_queue, self.stocks,
            self.refresh_rate, self.include_trending, self.trending_count, 
            self.demo_mode
        )
        
        # Display state
        self.current_data = {}
        self.historical_data = {}
        self.current_symbols = self.stocks.copy()
        self.current_stock_index = 0
        self.last_switch_time = time.time()
        self.last_data_update = None
        self.chart_needs_redraw = True
        self.running = True
    
    def start(self):
        """Start the stock tracker application."""
        try:
            print("Starting Advanced Stock Tracker...")
            print(f"Demo mode: {self.demo_mode}")
            print(f"Initial stocks: {self.stocks}")
            print(f"Include trending: {self.include_trending}")
            
            # Start background data update thread
            print("Starting background data update thread...")
            thread = threading.Thread(target=self.stock_update_thread.run, daemon=True)
            thread.start()
            
            # Wait for initial data
            print("Waiting for stock data to load...")
            self._wait_for_initial_data()
            
            print("Press CTRL-C to stop.")
            self._main_loop()
            
        except KeyboardInterrupt:
            print("\nShutting down...")
            self.stop()
    
    def stop(self):
        """Stop the application and cleanup resources."""
        self.running = False
        self.stock_update_thread.stop()
        print("Stopping background thread...")
        time.sleep(0.1)
    
    def _wait_for_initial_data(self):
        """Wait for initial stock data to be loaded."""
        max_wait_time = StockConfig.MAX_WAIT_TIME_SECONDS
        wait_start = time.time()
        
        while len(self.current_data) == 0 and (time.time() - wait_start) < max_wait_time:
            print(".", end="", flush=True)
            self._check_for_updates()
            time.sleep(0.5)
        
        print()  # New line after dots
        
        if len(self.current_data) > 0:
            print(f"✓ Stock data ready! Loaded {len(self.current_data)} stocks")
            print(f"✓ Display will cycle through: {self.current_symbols}")
        else:
            print("⚠ No stock data loaded, will retry in background")
    
    def _check_for_updates(self):
        """Check for stock data updates from the background thread."""
        try:
            update_data = self.update_queue.get_nowait()
            self.current_data = update_data['current']
            self.historical_data = update_data['history']
            self.current_symbols = update_data['symbols']
            self.last_data_update = update_data['timestamp']
            self.chart_needs_redraw = True  # New data means chart needs redraw
            self.update_queue.task_done()
            
            # Reset stock index if symbol list changed
            if self.current_stock_index >= len(self.current_symbols):
                self.current_stock_index = 0
                
        except queue.Empty:
            pass  # No update available
    
    def _should_switch_stock(self):
        """Check if it's time to switch to the next stock."""
        current_time = time.time()
        return current_time - self.last_switch_time >= self.display_time
    
    def _switch_to_next_stock(self):
        """Switch to the next stock in the list."""
        if self.current_symbols:
            old_index = self.current_stock_index
            self.current_stock_index = (self.current_stock_index + 1) % len(self.current_symbols)
            self.last_switch_time = time.time()
            self.chart_needs_redraw = True  # New stock means chart needs redraw
            print(f"Switched from stock {old_index} to {self.current_stock_index} "
                  f"(cycling through {len(self.current_symbols)} stocks)")
    
    def _draw_current_stock_minimal(self):
        """Draw only the chart area for data updates (no text redraw)."""
        if not self.current_symbols or len(self.current_data) == 0:
            return
            
        current_symbol = self.current_symbols[self.current_stock_index]
        
        # Only draw chart if historical data is available
        if current_symbol in self.historical_data:
            chart_area = self.display.get_chart_area()
            prices = self.historical_data[current_symbol]
            
            self.chart_renderer.draw_stock_chart(
                current_symbol, prices,
                chart_area['x'], chart_area['y'],
                chart_area['width'], chart_area['height'],
                chart_type='filled',
                is_demo=self.demo_mode
            )
    
    def _draw_current_stock(self):
        """Draw the current stock information and chart."""
        if not self.current_symbols or not self.current_data:
            self.display.draw_loading_message()
            return
        
        # Get current stock
        current_symbol = self.current_symbols[self.current_stock_index]
        
        if current_symbol not in self.current_data:
            self.display.draw_loading_message(current_symbol)
            return
        
        # Get stock info
        stock_info = self.current_data[current_symbol]
        
        # Draw stock information (symbol, price, change)
        self.display.draw_stock_info(
            current_symbol,
            stock_info['price'],
            stock_info['change'],
            stock_info['change_percent']
        )
        
        # Draw chart if historical data is available
        if current_symbol in self.historical_data:
            chart_area = self.display.get_chart_area()
            prices = self.historical_data[current_symbol]
            
            # Always clear chart area before drawing to ensure visibility
            self.display.clear_chart_area()
            
            self.chart_renderer.draw_stock_chart(
                current_symbol, prices,
                chart_area['x'], chart_area['y'],
                chart_area['width'], chart_area['height'],
                chart_type='filled',
                is_demo=self.demo_mode
            )
    
    def _main_loop(self):
        """Main display loop."""
        last_symbol = None
        last_redraw_time = time.time()
        last_data_redraw_time = time.time()  # Separate timer for data redraws
        
        while self.running:
            # Check for data updates
            data_updated = False
            old_chart_redraw = self.chart_needs_redraw
            self._check_for_updates()
            if self.chart_needs_redraw and not old_chart_redraw:
                data_updated = True
            
            # Check if we need to switch stocks
            stock_switched = False
            if self._should_switch_stock():
                self._switch_to_next_stock()
                stock_switched = True
            
            # Get current symbol
            current_symbol = None
            if self.current_symbols and len(self.current_data) > 0:
                current_symbol = self.current_symbols[self.current_stock_index]
            
            # Check if symbol changed
            symbol_changed = current_symbol != last_symbol
            
            # Only redraw if something actually changed or it's been more than 30 seconds
            current_time = time.time()
            force_redraw = current_time - last_redraw_time > 30  # Force redraw every 30 seconds
            
            # Throttle data updates to prevent excessive blinking, but allow stock switches
            data_redraw_allowed = current_time - last_data_redraw_time > 5.0  # Only allow data redraws every 5 seconds
            
            should_redraw = (stock_switched or symbol_changed or  # Always allow stock/symbol changes
                           ((data_updated or self.chart_needs_redraw) and data_redraw_allowed) or 
                           force_redraw)
            
            if should_redraw:
                # Try to minimize visual disruption
                if stock_switched or symbol_changed:
                    # Full redraw for stock switches - ensure chart appears
                    self.display.clear()
                    self._draw_current_stock()  # This includes both text and chart
                    self.display.swap_canvas()
                    last_data_redraw_time = current_time  # Reset data timer on stock switch
                elif (data_updated or self.chart_needs_redraw) and data_redraw_allowed:
                    # For data updates only, minimal redraw
                    self._draw_current_stock_minimal()
                    self.display.swap_canvas()
                    last_data_redraw_time = current_time  # Update data redraw timer
                else:
                    # Force redraw case - full redraw but no clear
                    self._draw_current_stock()
                    self.display.swap_canvas()
                
                # Mark chart as drawn and update tracking
                if self.chart_needs_redraw:
                    self.chart_needs_redraw = False
                last_symbol = current_symbol
                last_redraw_time = current_time
                print(f"Display updated: data={data_updated}, switched={stock_switched}, "
                      f"symbol_changed={symbol_changed}, force={force_redraw}")
            
            # Wait for next frame - even longer delay since we're not redrawing often
            time.sleep(2.0)  # 0.5fps check rate - much slower to reduce blinking


def create_argument_parser():
    """Create command line argument parser."""
    parser = argparse.ArgumentParser(description="Advanced Stock Tracker for RGB LED Matrix")
    
    parser.add_argument("--stocks", 
                       help="Comma-separated stock symbols (e.g., AAPL,GOOGL,MSFT)", 
                       default=StockConfig.DEFAULT_STOCKS, type=str)
    
    parser.add_argument("--demo-mode", 
                       help="Use demo data instead of API", 
                       action="store_true")
    
    parser.add_argument("--include-trending", 
                       help="Include trending stocks (gainers/losers)", 
                       action="store_true")
    
    parser.add_argument("--trending-count", 
                       help="Number of trending stocks to include", 
                       default=StockConfig.TRENDING_COUNT, type=int)
    
    parser.add_argument("--refresh-rate", 
                       help="Data refresh rate in minutes", 
                       default=StockConfig.REFRESH_RATE_MINUTES, type=int)
    
    parser.add_argument("--display-time", 
                       help="Time to show each stock in seconds", 
                       default=StockConfig.DISPLAY_TIME_PER_STOCK, type=int)
    
    parser.add_argument("--alpha-vantage-key", 
                       help="Alpha Vantage API key", 
                       default=StockConfig.ALPHA_VANTAGE_API_KEY, type=str)
    
    return parser


def main():
    """Main entry point for the application."""
    # Load environment variables
    load_dotenv('stock-tracker.env')
    
    # Parse command line arguments
    parser = create_argument_parser()
    args = parser.parse_args()
    
    # Create and start the stock tracker
    stock_tracker = StockTracker(
        stocks=args.stocks.split(',') if args.stocks else None,
        demo_mode=args.demo_mode,
        include_trending=args.include_trending,
        trending_count=args.trending_count,
        refresh_rate=args.refresh_rate,
        display_time=args.display_time,
        alpha_vantage_key=args.alpha_vantage_key
    )
    
    stock_tracker.start()


if __name__ == "__main__":
    main()