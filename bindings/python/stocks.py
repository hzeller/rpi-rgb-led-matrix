#!/usr/bin/env python
# Display a runtext with double-buffering.
from base import Base
from rgbmatrix import graphics
from time import sleep

import requests
from datetime import datetime, timedelta
import pytz
import numpy

TICKER = "NVDA"

class Market:
    def __init__(self):
        self.exchange = "NYSE"
        self.open_hour = 9
        self.open_min = 30
        self.open_time = 390 # minutes in stock day
        self.api_key = "d9142d27a31c40629d3e2347daa59c82"
        self.timezone = "America/New_York"
        self.update_interval = 6 # update every 6 mins when open
        self.last_update = datetime.now() - timedelta(minutes=self.update_interval)
        self.is_open = True

    def market_state(self):
        url = f"https://api.twelvedata.com/market_state?exchange={self.exchange}&apikey={self.api_key}"
        response = requests.get(url).json()
        if response[0]['is_market_open'] == 'true':
            res = True
        else:
            res = False
        return res

    def at_open(self, dt):
        return dt.replace(hour=self.open_hour, minute=self.open_min, second=0, microsecond=0)

    def get_time_series(self, ticker, start, end):
        url = f"https://api.twelvedata.com/time_series?symbol={ticker}&interval=1day&start_date={start}&end_date={end}&apikey={self.api_key}"
        return requests.get(url).json()

    def update_status(self):
        if (self.last_update + timedelta(minutes=self.update_interval) <= datetime.now()):
            if self.is_open:
                day = self.at_open(datetime.now(pytz.timezone(self.timezone)))
                data = self.get_time_series("NVDA", day, day + timedelta(days=1))
                while (data['status'] == 'error'):
                    day -= timedelta(days=1)
                    data = self.get_time_series("NVDA", day, day + timedelta(days=1))
                self.trading_day = self.at_open(day)
                
                day -= timedelta(days=1)
                data = self.get_time_series( "NVDA", day, day + timedelta(days=1))
                while (data['status'] == 'error'):
                    day -= timedelta(days=1)
                    data = self.get_time_series("NVDA", day, day + timedelta(days=1))
                self.previous_day = self.at_open(day)
                
                self.force_update = False
                self.last_update = datetime.now()

                print("current trading day:", self.trading_day)
                print("previous trading day:", self.previous_day)

            self.is_open = self.market_state()
    
    def get_last_close_price(self, ticker):
        self.update_status()
        url = f"https://api.twelvedata.com/time_series?symbol={ticker}&interval=1day&start_date={self.previous_day}&end_date={self.previous_day+timedelta(minutes=self.open_time)}&apikey={self.api_key}"
        response = requests.get(url).json()
        return float(response['values'][0]['close'])
    
    def get_trading_day_data(self, ticker):
        self.update_status()
        url = f"https://api.twelvedata.com/time_series?symbol={ticker}&interval=1min&start_date={self.trading_day}&end_date={self.trading_day+timedelta(minutes=self.open_time)}&apikey={self.api_key}"
        return requests.get(url).json()

class Stocks(Base):
    def __init__(self, *args, **kwargs):
        super(Stocks, self).__init__(*args, **kwargs)
        self.graph = self.Graph()
        self.market = Market()

    def run(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        font.LoadFont("../../fonts/7x13.bdf")
        white = graphics.Color(255, 255, 255)

        self.graph.refresh(self.market.get_trading_day_data(TICKER),self.market.get_last_close_price(TICKER))

        while True:
            offscreen_canvas.Clear()
            graphics.DrawText(offscreen_canvas, font, 1, 10, white, TICKER)

            x = 0
            y_start = 31
            green = (graphics.Color(0, 25, 0),graphics.Color(0, 255, 0))
            red = (graphics.Color(25, 0, 0), graphics.Color(255, 0, 0))
            for y in self.graph.y:
                if y >= self.graph.inflection_pt:
                    graphics.DrawLine(offscreen_canvas, x, y_start-y, x, y_start-self.graph.inflection_pt, green[0])
                    if x == len(self.graph.y)-1:
                        graphics.DrawLine(offscreen_canvas, x, y_start-y, x, y_start-y, green[1])
                    else:
                        next_y = y_start-self.graph.y[x+1]
                        if next_y <= self.graph.inflection_pt:
                            graphics.DrawLine(offscreen_canvas, x, self.graph.inflection_pt, x+1, next_y, red[1])
                            graphics.DrawLine(offscreen_canvas, x, y_start-y, x+1, self.graph.inflection_pt, green[1])
                        else:
                            graphics.DrawLine(offscreen_canvas, x, y_start-y, x+1, next_y, green[1])
                else:
                    graphics.DrawLine(offscreen_canvas, x, y_start-y, x, y_start-self.graph.inflection_pt, red[0])
                    if x == len(self.graph.y)-1:
                        graphics.DrawLine(offscreen_canvas, x, y_start-y, x, y_start-y, red[1])
                    else:
                        next_y = y_start-self.graph.y[x+1]
                        if next_y < self.graph.inflection_pt:
                            graphics.DrawLine(offscreen_canvas, x, y_start-y, x+1, self.graph.inflection_pt, red[1])
                            graphics.DrawLine(offscreen_canvas, x, self.graph.inflection_pt, x+1, next_y, green[1])
                        else:
                            graphics.DrawLine(offscreen_canvas, x, y_start-y, x+1, next_y, red[1])  

                x += 1

            sleep(1)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)

    class Graph:
        def __init__(self):
            self.height = 17
            self.width = 64
            self.open_time = 390 # minutes in stock day
            self.timestamps = list(numpy.rint(numpy.linspace(0,self.open_time-1,self.width)))
            self.y = list()
            self.x = list()

        def refresh(self, data, close_price):
            samples = list()
            for delta in self.timestamps:
                time = datetime.strptime(data['values'][-1]['datetime'], '%Y-%m-%d %H:%M:%S') + timedelta(minutes=delta)
                sample = list(filter(lambda values: values['datetime'] == time.strftime('%Y-%m-%d %H:%M:%S'), data['values']))
                if sample:
                    if delta == 0: # first data point is at open
                        samples.append(float(sample[0]['open']))
                    else:
                        samples.append(float(sample[0]['close']))
                else:
                    break

            max_val = max(samples)
            min_val = min(samples)
            
            if close_price > max_val:
                max_val = close_price
            elif close_price < min_val:
                min_val = close_price
            
            scale = self.height/(max_val-min_val)
            self.inflection_pt = int((close_price-min_val)*scale)

            idx = 0
            for sample in samples:
                self.x.append(idx)
                idx += 1

                self.y.append(int((sample-min_val)*scale))

        def draw():
            pass

# Main function
if __name__ == "__main__":
    stocks = Stocks()

    stocks.process()
