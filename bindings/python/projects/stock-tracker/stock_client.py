#!/usr/bin/env python
"""
Stock API client for retrieving current and historical stock data.

This module provides classes for:
- StockClient: Handles multiple stock API sources (Yahoo Finance, Alpha Vantage)
- StockUpdateThread: Manages background updates in a separate thread
- TrendingStockFetcher: Fetches trending stocks (gainers/losers)
"""

import time
import requests
import json
import os
import queue
import random
import threading
from datetime import datetime, timedelta


class StockClient:
    """Handles stock data retrieval from multiple API sources."""
    
    def __init__(self):
        self.alpha_vantage_key = os.getenv("ALPHA_VANTAGE_API_KEY")
        self.last_error_time = None
        self.headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
        }
    
    def fetch_yahoo_finance_data(self, symbol):
        """Fetch stock data from Yahoo Finance (free, no API key required)."""
        try:
            url = f"https://query1.finance.yahoo.com/v8/finance/chart/{symbol}"
            
            response = requests.get(url, headers=self.headers, timeout=10)
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
    
    def fetch_alpha_vantage_data(self, symbol):
        """Fetch stock data from Alpha Vantage API."""
        if not self.alpha_vantage_key:
            return None
            
        try:
            url = f"https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol={symbol}&apikey={self.alpha_vantage_key}"
            response = requests.get(url, timeout=15)
            response.raise_for_status()
            json_data = response.json()
            
            if "Note" in json_data:
                print(f"Alpha Vantage rate limit reached")
                return None
            elif "Global Quote" in json_data:
                quote = json_data["Global Quote"]
                if quote and quote.get("05. price") != "0.0000":
                    price = float(quote["05. price"])
                    change = float(quote["09. change"])
                    change_percent = float(quote["10. change percent"].replace('%', ''))
                    
                    return {
                        'price': round(price, 2),
                        'change': round(change, 2),
                        'change_percent': round(change_percent, 2),
                        'timestamp': datetime.now().strftime('%H:%M')
                    }
            
        except Exception as e:
            print(f"Alpha Vantage error for {symbol}: {e}")
            return None
    
    def fetch_historical_data(self, symbol, days=64):
        """Fetch historical stock data for charts."""
        if not self.alpha_vantage_key:
            return self._generate_demo_history(symbol, days)
            
        try:
            url = f"https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol={symbol}&apikey={self.alpha_vantage_key}"
            response = requests.get(url, timeout=15)
            response.raise_for_status()
            data = response.json()
            
            if "Time Series (Daily)" in data:
                time_series = data["Time Series (Daily)"]
                prices = []
                
                sorted_dates = sorted(time_series.keys(), reverse=True)
                for date in sorted_dates[:days]:
                    close_price = float(time_series[date]["4. close"])
                    prices.append(close_price)
                
                return prices[::-1] if prices else self._generate_demo_history(symbol, days)
            else:
                return self._generate_demo_history(symbol, days)
                
        except Exception as e:
            print(f"Error fetching historical data for {symbol}: {e}")
            return self._generate_demo_history(symbol, days)
    
    def _generate_demo_history(self, symbol, days, current_price=None):
        """Generate realistic demo historical data."""
        if current_price is None:
            current_price = random.uniform(50, 500)
        
        history = []
        price = current_price * 0.9  # Start 10% lower
        
        for i in range(days):
            # Random walk with slight upward trend
            change_percent = random.uniform(-3, 3.5) / 100
            price *= (1 + change_percent)
            price = max(price, current_price * 0.3)  # Don't go too low
            history.append(round(price, 2))
        
        return history
    
    def get_current_data(self, symbols):
        """Get current stock data for multiple symbols."""
        current_data = {}
        
        for symbol in symbols:
            # Try Yahoo Finance first (free)
            data = self.fetch_yahoo_finance_data(symbol)
            
            # Fallback to Alpha Vantage if available
            if not data and self.alpha_vantage_key:
                data = self.fetch_alpha_vantage_data(symbol)
            
            if data:
                current_data[symbol] = data
                print(f"✓ Got {symbol}: ${data['price']:.2f}")
            else:
                print(f"✗ Failed to get {symbol}")
            
            time.sleep(0.2)  # Rate limiting
        
        return current_data
    
    def get_demo_data(self, symbols):
        """Generate demo stock data."""
        current_data = {}
        
        for symbol in symbols:
            base_price = random.uniform(50, 500)
            change_percent = random.uniform(-5, 5)
            change_amount = base_price * (change_percent / 100)
            
            current_data[symbol] = {
                'price': round(base_price, 2),
                'change': round(change_amount, 2),
                'change_percent': round(change_percent, 2),
                'timestamp': datetime.now().strftime('%H:%M')
            }
        
        return current_data


class TrendingStockFetcher:
    """Fetches trending stocks (biggest gainers and losers)."""
    
    def __init__(self):
        self.headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
        }
    
    def fetch_trending_stocks(self, count=3):
        """Fetch trending stocks from Yahoo Finance screener."""
        trending_symbols = []
        
        try:
            screener_urls = [
                "https://query1.finance.yahoo.com/v1/finance/screener/predefined/saved?formatted=true&lang=en-US&region=US&scrIds=day_gainers&count=10",
                "https://query1.finance.yahoo.com/v1/finance/screener/predefined/saved?formatted=true&lang=en-US&region=US&scrIds=day_losers&count=10"
            ]
            
            for url in screener_urls:
                try:
                    response = requests.get(url, headers=self.headers, timeout=10)
                    response.raise_for_status()
                    data = response.json()
                    
                    if 'finance' in data and 'result' in data['finance'] and data['finance']['result']:
                        quotes = data['finance']['result'][0].get('quotes', [])
                        
                        for quote in quotes[:count]:
                            symbol = quote.get('symbol', '').upper()
                            change_percent = quote.get('regularMarketChangePercent', {}).get('raw', 0)
                            
                            if symbol and symbol not in trending_symbols:
                                trending_symbols.append(symbol)
                                print(f"✓ Found trending stock: {symbol} ({change_percent:+.1f}%)")
                                
                                if len(trending_symbols) >= count:
                                    break
                    
                    if len(trending_symbols) >= count:
                        break
                        
                except Exception as inner_e:
                    print(f"Screener API failed: {inner_e}")
                    continue
                    
        except Exception as e:
            print(f"Could not fetch trending stocks: {e}")
        
        # Fallback trending stocks
        if not trending_symbols:
            fallback_trending = ['GME', 'AMC', 'PLTR', 'RIVN', 'LCID', 'SOFI']
            trending_symbols = fallback_trending[:count]
            print(f"Using fallback trending stocks: {trending_symbols}")
            
        return trending_symbols[:count]


class StockUpdateThread:
    """Handles background stock data updates using a separate thread."""
    
    def __init__(self, stock_client, update_queue, symbols, refresh_rate_minutes=5, 
                 include_trending=False, trending_count=3, demo_mode=False):
        self.stock_client = stock_client
        self.update_queue = update_queue
        self.base_symbols = symbols
        self.refresh_rate_minutes = refresh_rate_minutes
        self.include_trending = include_trending
        self.trending_count = trending_count
        self.demo_mode = demo_mode
        self.running = True
        self.trending_fetcher = TrendingStockFetcher() if include_trending else None
    
    def stop(self):
        """Stop the background thread."""
        self.running = False
    
    def run(self):
        """Background thread that continuously updates stock data."""
        while self.running:
            try:
                # Build symbol list
                symbols = list(self.base_symbols)
                
                # Add trending stocks if requested
                if self.include_trending and self.trending_fetcher:
                    trending_stocks = self.trending_fetcher.fetch_trending_stocks(self.trending_count)
                    for symbol in trending_stocks:
                        if symbol not in symbols:
                            symbols.append(symbol)
                
                print(f"Updating data for symbols: {symbols}")
                
                # Get current data
                if self.demo_mode:
                    current_data = self.stock_client.get_demo_data(symbols)
                else:
                    current_data = self.stock_client.get_current_data(symbols)
                
                # Get historical data for charts
                history_data = {}
                for symbol in current_data.keys():
                    if self.demo_mode:
                        history_data[symbol] = self.stock_client._generate_demo_history(
                            symbol, 64, current_data[symbol]['price']
                        )
                    else:
                        history_data[symbol] = self.stock_client.fetch_historical_data(symbol, 64)
                
                # Prepare update data
                update_data = {
                    'current': current_data,
                    'history': history_data,
                    'symbols': symbols,
                    'timestamp': datetime.now()
                }
                
                # Put update in queue (non-blocking)
                try:
                    self.update_queue.put_nowait(update_data)
                    print(f"Queued update for {len(current_data)} stocks")
                except queue.Full:
                    pass  # Skip if queue is full
                
                # Wait for next update
                time.sleep(self.refresh_rate_minutes * 60)
                
            except Exception as e:
                print(f"Background thread error: {e}")
                time.sleep(30)  # Wait on error