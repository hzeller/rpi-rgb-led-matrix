#!/usr/bin/env python

from rgbmatrix import graphics
from time import sleep
import json

import requests
from datetime import datetime, timedelta
import pytz
import numpy
from twelvedata import TDClient

import argparse
from rgbmatrix import RGBMatrix, RGBMatrixOptions

class Market:
    def __init__(self):
        self.exchange = "NYSE"
        self.open_hour = 9
        self.open_min = 30
        self.open_time = 390 # minutes in stock day
        self.api_key = "d9142d27a31c40629d3e2347daa59c82"
        self.timezone = "America/New_York"
        self.update_interval = 6 # update every 6 mins when open
        self.is_open = False
        self.next_update = datetime.now()
        self.td = TDClient(apikey=self.api_key)
        self.symbols = list()

        self.check_market_state()

    def check_market_state(self):
        if (self.next_update <= datetime.now()):
            self.set_trading_day()

            url = f"https://api.twelvedata.com/market_state?exchange={self.exchange}&apikey={self.api_key}"
            response = requests.get(url).json()
            print("[STOCKS] URL:", url, "returned", response)

            if response[0]['is_market_open']:
                self.is_open = True
                time_to_close = response[0]['time_to_close'].split(":")
                self.next_update = datetime.now() + timedelta(minutes=int(time_to_close[0])*60+int(time_to_close[1])+1)
            else:
                self.is_open = False
                time_to_open = response[0]['time_to_open'].split(":")
                self.next_update = datetime.now() + timedelta(minutes=int(time_to_open[0])*60+int(time_to_open[1])+1)

            print("[STOCKS] next trading day update:", self.next_update.strftime('%Y-%m-%d %H:%M:%S'))

    def at_open(self, dt):
        return dt.replace(hour=self.open_hour, minute=self.open_min, second=0, microsecond=0)

    def is_trading_day(self, day):
        try:
            ts = self.td.time_series(
                symbol="NVDA",
                interval="1day",
                outputsize=1,
                start_date=start,
                end_date=day+timedelta(minutes=self.open_time),
                timezone=self.timezone
            )
            return True
        except:
            return False

        return ts.as_json()['status']

    def set_trading_day(self):
        day = self.at_open(datetime.now(pytz.timezone(self.timezone)))
        while self.is_trading_day(day):
            day -= timedelta(days=1)
        self.trading_day = self.at_open(day)
        
        day -= timedelta(days=1)
        while self.is_trading_day(day):
            day -= timedelta(days=1)
        self.previous_day = self.at_open(day)
        
        self.force_update = False
        self.last_update = datetime.now()

        print("[STOCKS] current trading day:", self.trading_day.strftime('%Y-%m-%d'))
        print("[STOCKS] previous trading day:", self.previous_day.strftime('%Y-%m-%d'))
    
    def get_last_close_price(self, symbols):
        self.check_market_state()
        ts = self.td.time_series(
            symbol=symbols,
            interval="1day",
            outputsize=1,
            start_date=self.previous_day,
            end_date=self.previous_day+timedelta(minutes=self.open_time),
            timezone=self.timezone
        )
        return float(ts.as_json()[0]['close'])
    
    def get_trading_day_data(self, symbols):
        self.check_market_state()
        ts = self.td.time_series(
            symbol=symbols,
            interval="1min",
            start_date=self.trading_day,
            outputsize=self.open_time,
            end_date=self.trading_day+timedelta(minutes=self.open_time),
            timezone=self.timezone
        )
        return ts.as_json()

market = Market()
class Stocks:
    def __init__(self, matrix, symbol):
        self.graph = self.Graph()
        self.canvas = matrix.CreateFrameCanvas()
        self.offscreen_canvas = matrix.CreateFrameCanvas()
        self.next_update = None
        self.symbol = symbol
        market.symbols.append(self.symbol)

        self.refresh()
    
    def __del__(self):
        market.symbols.remove(self.symbol)

    def save_state(self):
        print("[STOCKS] Saving current state ...")

        save_symbol = dict()
        save_symbol["last_update"] = self.last_update.strftime('%Y-%m-%d %H:%M:%S')
        save_symbol["closing_price"] = self.closing_price
        save_symbol["curr_price"] = self.curr_price
        save_symbol["curr_diff"] = self.curr_diff
        save_symbol["curr_percent"] = self.curr_percent

        save_symbol["graph_data"] = self.graph.data
        save_symbol["graph_inflection_pt"] = self.graph.inflection_pt

        save_symbol["next_update"] = market.next_update.strftime('%Y-%m-%d %H:%M:%S')
        
        file = dict()
        try:
            with open('data/stocks.json', 'r') as json_file:
                file = json.load(json_file)
        except FileNotFoundError:
            open('data/stocks.json', 'x')

        file[self.symbol] = save_symbol
        
        with open('data/stocks.json', 'w') as json_file:
            json.dump(file, json_file)
        
        print(file[self.symbol])

    def load_state(self):
        print("[STOCKS] Load current state ...")

        try:
            with open('data/stocks.json', 'r') as json_file:
                file = json.load(json_file)

            print(file[self.symbol])

            self.last_update = file[self.symbol]["last_update"]
            self.closing_price = file[self.symbol]["closing_price"]
            self.curr_price = file[self.symbol]["curr_price"]
            self.curr_diff = file[self.symbol]["curr_diff"]
            self.curr_percent = file[self.symbol]["curr_percent"]

            self.graph.data = file[self.symbol]["graph_data"]
            self.graph.inflection_pt = file[self.symbol]["graph_inflection_pt"]

            self.next_update = datetime.strptime(file[self.symbol]["next_update"], '%Y-%m-%d %H:%M:%S')
        except:
            print("[STOCKS] No stock data found ...")

    def get_canvas(self):
        return self.canvas

    def update_from_market(self):
        self.last_update = datetime.now()

        self.closing_price = market.get_last_close_price(self.symbol)
        self.data = market.get_trading_day_data(self.symbol)
        self.graph.parse(self.data, self.closing_price)
        
        self.curr_price = round(float(self.data[0]['close']),2)
        self.curr_diff = round(float(self.data[0]['close'])-self.closing_price,2)
        self.curr_percent = round(self.curr_diff/self.closing_price*100,2)
        
        self.save_state()

    def refresh(self):
        font = graphics.Font()
        font.LoadFont("../../fonts/5x6.bdf")
        text_font = graphics.Font()
        white = graphics.Color(255, 255, 255)
        red = graphics.Color(255, 0, 0)
        green = graphics.Color(0, 255, 0)

        if market.is_open:
            self.update_from_market()
        else:
            self.load_state()
            
            if self.next_update == None or self.next_update <= market.next_update: # load is stale?
                self.update_from_market() 

        self.offscreen_canvas.Clear()
        graphics.DrawText(self.offscreen_canvas, font, 1, 6, white, self.symbol)
        graphics.DrawText(self.offscreen_canvas, font, 1, 13, white, str(self.curr_price))
        graphics.DrawText(self.offscreen_canvas, font, 34, 6, red, str(self.curr_diff))
        graphics.DrawText(self.offscreen_canvas, font, 34, 13, red, str(self.curr_percent) + '%')

        self.graph.draw(self.offscreen_canvas, 0, 31)

        self.canvas = self.offscreen_canvas

    class Graph:
        def __init__(self):
            self.height = 17
            self.width = 64
            self.open_time = 390 # minutes in stock day
            self.timestamps = list(numpy.rint(numpy.linspace(0,self.open_time-1,self.width)))
            self.data = list()

        def parse(self, raw, close_price):
            samples = list()
            for delta in self.timestamps:
                time = datetime.strptime(raw[-1]['datetime'], '%Y-%m-%d %H:%M:%S') + timedelta(minutes=delta)
                sample = list(filter(lambda values: values['datetime'] == time.strftime('%Y-%m-%d %H:%M:%S'), raw))
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

            x = 0
            self.data.clear()
            for sample in samples:
                self.data.append((x,int((sample-min_val)*scale)))
                x += 1
        
        def draw(self, canvas, x_offset, y_offset):
            green = (graphics.Color(0, 25, 0),graphics.Color(0, 255, 0))
            red = (graphics.Color(25, 0, 0), graphics.Color(255, 0, 0))
            for idx in range(0,len(self.data)):
                x = self.data[idx][0]
                y = self.data[idx][1]
                if y >= self.inflection_pt:
                    graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset, y_offset-self.inflection_pt, green[0])
                    if x == len(self.data)-1:
                        graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset, y_offset-y, green[1])
                    else:
                        next_y = y_offset-self.data[idx+1][1]
                        if next_y <= self.inflection_pt:
                            graphics.DrawLine(canvas, x+x_offset, self.inflection_pt, x+x_offset+1, next_y, red[1])
                            graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset+1, self.inflection_pt, green[1])
                        else:
                            graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset+1, next_y, green[1])
                else:
                    graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset, y_offset-self.inflection_pt, red[0])
                    if x == len(self.data)-1:
                        graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset, y_offset-y, red[1])
                    else:
                        next_y = y_offset-self.data[idx+1][1]
                        if next_y < self.inflection_pt:
                            graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset+1, self.inflection_pt, red[1])
                            graphics.DrawLine(canvas, x+x_offset, self.inflection_pt, x+x_offset+1, next_y, green[1])
                        else:
                            graphics.DrawLine(canvas, x+x_offset, y_offset-y, x+x_offset+1, next_y, red[1])
