#!/usr/bin/env python
"""
Configuration constants for the stock tracker application.

This module contains all configurable parameters for:
- Matrix hardware settings (reusing from DisplayConfig)
- Stock-specific display settings
- API configuration and timing
- Chart rendering options
"""

import os
from config import DisplayConfig


class StockConfig(DisplayConfig):
    """Stock tracker configuration extending base DisplayConfig."""
    
    # Stock API settings
    DEFAULT_STOCKS = os.environ.get('DEFAULT_STOCKS', "AAPL,GOOGL,MSFT,TSLA")
    ALPHA_VANTAGE_API_KEY = os.environ.get('ALPHA_VANTAGE_API_KEY')
    REFRESH_RATE_MINUTES = int(os.environ.get('REFRESH_RATE', '5'))
    
    # Display timing
    DISPLAY_TIME_PER_STOCK = int(os.environ.get('DISPLAY_TIME', '10'))  # seconds
    
    # Chart settings
    CHART_DAYS = int(os.environ.get('CHART_DAYS', '30'))
    CHART_WIDTH = 64
    CHART_HEIGHT = 16
    CHART_Y_POSITION = 16  # Bottom half of 32px display
    
    # Trending stocks
    TRENDING_COUNT = int(os.environ.get('TRENDING_COUNT', '3'))
    
    # Stock-specific colors (RGB tuples)
    STOCK_COLORS = {
        'gain_bright': (0, 255, 0),      # Bright green for positive
        'gain_dim': (0, 180, 0),         # Dim green for chart
        'loss_bright': (255, 0, 0),      # Bright red for negative  
        'loss_dim': (180, 0, 0),         # Dim red for chart
        'neutral': (255, 255, 255),      # White for neutral
        'chart_line': (0, 120, 0),       # Chart line color
        'text_secondary': (200, 200, 200), # Secondary text
        'chart_fill': (0, 100, 0),       # Chart fill color
        'demo_fill': (0, 60, 0),         # Demo chart fill
        'demo_line': (0, 120, 0)         # Demo chart line
    }
    
    # Font settings for stock display
    FONT_LARGE_PATH = "../../../fonts/5x7.bdf"
    FONT_SMALL_PATH = "../../../fonts/4x6.bdf"
    
    # Layout settings
    STOCK_SYMBOL_X = 2
    STOCK_SYMBOL_Y = 8
    STOCK_PRICE_X = 2
    STOCK_PRICE_Y = 15
    
    # Text alignment settings
    RIGHT_MARGIN = 2  # Pixels from right edge
    
    # Update intervals
    DISPLAY_REFRESH_RATE = 0.1  # seconds between display updates (10fps to reduce flicker)
    API_RATE_LIMIT_DELAY = 0.2  # seconds between API calls
    
    # Demo mode settings
    DEMO_PRICE_RANGE = (50, 500)  # Min/max for demo prices
    DEMO_CHANGE_RANGE = (-5, 5)   # Min/max change percentage
    
    # Error handling
    MAX_WAIT_TIME_SECONDS = 15
    ERROR_RETRY_DELAY = 30