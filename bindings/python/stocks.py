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
        self.latest = datetime.now(pytz.timezone("America/New_York"))
        self.open = self.latest.replace(hour=9, minute=30, second=0, microsecond=0)
        self.exchange = "NYSE"
        self.length = 390 # minutes in stock day
        self.api_key = "d9142d27a31c40629d3e2347daa59c82"

    def updateDay(self):
        if self.isOpen():
            self.latest = datetime.now(pytz.timezone("America/New_York"))
            self.open = self.latest.replace(hour=9, minute=30, second=0, microsecond=0)
        elif (1):
            pass

    def isOpen(self):
        url = f"https://api.twelvedata.com/market_state?exchange={self.exchange}&apikey={self.api_key}"
        response = requests.get(url).json()
        return response[0]['is_market_open']

    def getStockData(self, ticker):
        url = f"https://api.twelvedata.com/time_series?symbol={ticker}&interval=1min&start_date={self.open}&end_date={self.latest}&apikey={self.api_key}"
        response = requests.get(url).json()
        print(response)
        return response

class Stocks(Base):
    def __init__(self, *args, **kwargs):
        super(Stocks, self).__init__(*args, **kwargs)
        self.graph = self.Graph()

    def run(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        font.LoadFont("../../../fonts/7x13.bdf")
        textColor = graphics.Color(255, 255, 0)
        pos = offscreen_canvas.width

        while True:
            offscreen_canvas.Clear()
            len = graphics.DrawText(offscreen_canvas, font, pos, 10, textColor, "NVDA")
            pos -= 1
            if (pos + len < 0):
                pos = offscreen_canvas.width

            sleep(0.05)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)

    class Graph:
        def __init__(self):
            self.height = 17
            self.width = 64
            self.length = 390 # minutes in stock day
            self.timestamps = list(numpy.rint(numpy.linspace(0,self.length-1,self.width)))
            self.y = list()

        def refresh(self, data):
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
            scale = self.height/(max_val-min_val)

            for x in samples:
                self.y.append(int((x-min_val)*scale))

        def draw():
            pass

# Main function
if __name__ == "__main__":
    market = Market()    
    stocks = Stocks()

    print(market.isOpen())
    data = market.getStockData(TICKER)
    stocks.graph.refresh(data)

    x = 0
    for y in stocks.graph.y:
        print(x, y, sep=', ')
        x += 1