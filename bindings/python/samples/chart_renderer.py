#!/usr/bin/env python
"""
Chart rendering utilities for RGB LED matrix displays.

This module provides reusable chart rendering components:
- TimeSeriesChart: Renders filled area charts for time series data
- LineChart: Renders simple line charts
- ChartRenderer: Base class for all chart types
"""

import math
from rgbmatrix import graphics


class ChartRenderer:
    """Base class for chart rendering with common utilities."""
    
    def __init__(self, canvas):
        self.canvas = canvas
    
    def is_valid_position(self, x, y, max_width=64, max_height=32):
        """Check if position is within valid canvas bounds."""
        return 0 <= x < max_width and 0 <= y < max_height
    
    def draw_pixel_safe(self, x, y, color, max_width=64, max_height=32):
        """Draw a pixel with bounds checking."""
        if self.is_valid_position(x, y, max_width, max_height):
            if isinstance(color, tuple):
                self.canvas.SetPixel(x, y, *color)
            else:
                self.canvas.SetPixel(x, y, color.red, color.green, color.blue)
    
    def draw_line(self, x0, y0, x1, y1, color):
        """Draw a line between two points using Bresenham's algorithm."""
        dx = abs(x1 - x0)
        dy = abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx - dy
        
        x, y = x0, y0
        
        while True:
            self.draw_pixel_safe(x, y, color)
            
            if x == x1 and y == y1:
                break
                
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x += sx
            if e2 < dx:
                err += dx
                y += sy


class TimeSeriesChart(ChartRenderer):
    """Renders filled area charts for time series data."""
    
    def __init__(self, canvas):
        super().__init__(canvas)
        self.default_colors = {
            'fill': (0, 100, 0),        # Dark green fill
            'line': (0, 180, 0),        # Bright green line
            'demo_fill': (0, 60, 0),    # Demo dark green
            'demo_line': (0, 120, 0)    # Demo bright green
        }
    
    def draw_filled_chart(self, prices, x_start, y_start, width, height, 
                         colors=None, is_demo=False):
        """Draw a filled area chart from price data."""
        if colors is None:
            colors = self.default_colors
        
        # Safety bounds check
        if (x_start < 0 or y_start < 0 or width <= 0 or height <= 0 or
            x_start + width > 64 or y_start + height > 32):
            return False
        
        # Chart area clearing is now handled by the caller (stock_tracker)
        # This prevents conflicts and ensures proper clearing timing
        print(f"DEBUG: TimeSeriesChart.draw_filled_chart called for {len(prices)} prices")
        
        if not prices or len(prices) < 2:
            return self.draw_demo_chart(x_start, y_start, width, height, colors)
        
        # Calculate price range
        min_price = min(prices)
        max_price = max(prices)
        price_range = max_price - min_price
        
        if price_range <= 0:
            return False
        
        # Use appropriate data (limit to width)
        chart_prices = prices[-width:] if len(prices) >= width else prices
        
        # Choose colors based on demo mode
        fill_color = colors['demo_fill'] if is_demo else colors['fill']
        line_color = colors['demo_line'] if is_demo else colors['line']
        
        # Draw filled area chart
        pixels_drawn = 0
        for i in range(min(len(chart_prices), width)):
            x = x_start + i
            price_ratio = (chart_prices[i] - min_price) / price_range
            chart_height = max(1, int(price_ratio * (height - 2)))
            top_y = y_start + height - 1 - chart_height
            
            # Fill from bottom up to the chart line
            for fill_y in range(y_start + height - 1, top_y - 1, -1):
                if fill_y <= top_y + 1:  # Top 2 pixels get line color
                    self.draw_pixel_safe(x, fill_y, line_color)
                else:
                    self.draw_pixel_safe(x, fill_y, fill_color)
                pixels_drawn += 1
        
        # Store the last symbol for efficient clearing next time
        self.last_symbol = getattr(self, 'current_symbol', None)
        
        print(f"DEBUG: TimeSeriesChart drew {pixels_drawn} pixels")
        return pixels_drawn > 0
    
    def draw_demo_chart(self, x_start, y_start, width, height, colors=None):
        """Draw a demo sine wave chart when no data is available."""
        if colors is None:
            colors = self.default_colors
        
        # Safety bounds check
        if (x_start < 0 or y_start < 0 or width <= 0 or height <= 0 or
            x_start + width > 64 or y_start + height > 32):
            return False
        
        # Clear the chart area first to prevent flickering
        for clear_x in range(x_start, min(x_start + width, 64)):
            for clear_y in range(y_start, min(y_start + height, 32)):
                self.draw_pixel_safe(clear_x, clear_y, (0, 0, 0))
        
        pixels_drawn = 0
        
        # Draw filled sine wave pattern
        for x in range(min(width, 64)):
            if x_start + x >= 64:
                break
                
            # Create a gentle sine wave
            wave = math.sin(x * 0.15) * 0.3 + 0.5  # Normalize to 0.2-0.8
            chart_height = max(1, min(height - 2, int(wave * (height - 2))))
            top_y = y_start + height - 1 - chart_height
            
            # Fill from bottom up to the wave line
            for fill_y in range(y_start + height - 1, top_y - 1, -1):
                if fill_y <= top_y + 1:  # Top 2 pixels get lighter color
                    self.draw_pixel_safe(x_start + x, fill_y, colors['demo_line'])
                else:
                    self.draw_pixel_safe(x_start + x, fill_y, colors['demo_fill'])
                pixels_drawn += 1
        
        return pixels_drawn > 0


class LineChart(ChartRenderer):
    """Renders simple line charts."""
    
    def __init__(self, canvas):
        super().__init__(canvas)
    
    def draw_line_chart(self, prices, x_start, y_start, width, height, 
                       line_color=(0, 255, 0)):
        """Draw a simple line chart from price data."""
        # Safety bounds check
        if (x_start < 0 or y_start < 0 or width <= 0 or height <= 0 or
            x_start + width > 64 or y_start + height > 32):
            return False
        
        if not prices or len(prices) < 2:
            return False
        
        # Calculate price range
        min_price = min(prices)
        max_price = max(prices)
        price_range = max_price - min_price
        
        if price_range <= 0:
            return False
        
        # Use appropriate data (limit to width)
        chart_prices = prices[-width:] if len(prices) >= width else prices
        
        # Draw line chart
        prev_x, prev_y = None, None
        
        for i in range(min(len(chart_prices), width)):
            x = x_start + i
            price_ratio = (chart_prices[i] - min_price) / price_range
            y = y_start + height - 1 - int(price_ratio * (height - 1))
            
            if prev_x is not None and prev_y is not None:
                self.draw_line(prev_x, prev_y, x, y, line_color)
            
            prev_x, prev_y = x, y
        
        return True


class StockChartRenderer:
    """High-level stock chart renderer with multiple chart types."""
    
    def __init__(self, canvas):
        self.canvas = canvas
        self.time_series = TimeSeriesChart(canvas)
        self.line_chart = LineChart(canvas)
    
    def draw_stock_chart(self, symbol, prices, x_start, y_start, width, height, 
                        chart_type='filled', is_demo=False):
        """
        Draw a stock chart of the specified type.
        
        Args:
            symbol: Stock symbol (for logging)
            prices: List of price data
            x_start, y_start: Top-left corner of chart area
            width, height: Chart dimensions
            chart_type: 'filled' or 'line'
            is_demo: Whether this is demo data
        """
        # Track symbol changes for efficient clearing
        self.current_symbol = symbol
        
        try:
            if chart_type == 'filled':
                # Pass symbol information to time series chart
                self.time_series.current_symbol = symbol
                success = self.time_series.draw_filled_chart(
                    prices, x_start, y_start, width, height, is_demo=is_demo
                )
            elif chart_type == 'line':
                success = self.line_chart.draw_line_chart(
                    prices, x_start, y_start, width, height
                )
            else:
                print(f"Unknown chart type: {chart_type}")
                success = False
            
            if success:
                print(f"✓ Drew {chart_type} chart for {symbol}")
                # Store symbol for next comparison
                self.last_symbol = symbol
            else:
                print(f"✗ Failed to draw chart for {symbol}")
            
            return success
            
        except Exception as e:
            print(f"Chart rendering error for {symbol}: {e}")
            return False