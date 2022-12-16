#!/usr/bin/env python

from base import Base
from rgbmatrix import graphics
from time import sleep
import json

import requests
from datetime import datetime, timedelta
import pytz
import numpy
from twelvedata import TDClient
import schedule

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

class Stocks(Base):
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

def handle_args(*args, **kwargs):
    parser = argparse.ArgumentParser()

    parser.add_argument("-r", "--led-rows", action="store", help="Display rows. 16 for 16x32, 32 for 32x32. Default: 32", default=32, type=int)
    parser.add_argument("--led-cols", action="store", help="Panel columns. Typically 32 or 64. (Default: 32)", default=64, type=int)
    parser.add_argument("-c", "--led-chain", action="store", help="Daisy-chained boards. Default: 1.", default=1, type=int)
    parser.add_argument("-P", "--led-parallel", action="store", help="For Plus-models or RPi2: parallel chains. 1..3. Default: 1", default=1, type=int)
    parser.add_argument("-p", "--led-pwm-bits", action="store", help="Bits used for PWM. Something between 1..11. Default: 11", default=11, type=int)
    parser.add_argument("-b", "--led-brightness", action="store", help="Sets brightness level. Default: 100. Range: 1..100", default=100, type=int)
    parser.add_argument("-m", "--led-gpio-mapping", help="Hardware Mapping: regular, adafruit-hat, adafruit-hat-pwm", default='adafruit-hat-pwm', choices=['regular', 'regular-pi1', 'adafruit-hat', 'adafruit-hat-pwm'], type=str)
    parser.add_argument("--led-scan-mode", action="store", help="Progressive or interlaced scan. 0 Progressive, 1 Interlaced (default)", default=1, choices=range(2), type=int)
    parser.add_argument("--led-pwm-lsb-nanoseconds", action="store", help="Base time-unit for the on-time in the lowest significant bit in nanoseconds. Default: 130", default=130, type=int)
    parser.add_argument("--led-show-refresh", action="store_true", help="Shows the current refresh rate of the LED panel")
    parser.add_argument("--led-slowdown-gpio", action="store", help="Slow down writing to GPIO. Range: 0..4. Default: 1", default=1, type=int)
    parser.add_argument("--led-no-hardware-pulse", action="store", help="Don't use hardware pin-pulse generation")
    parser.add_argument("--led-rgb-sequence", action="store", help="Switch if your matrix has led colors swapped. Default: RGB", default="RGB", type=str)
    parser.add_argument("--led-pixel-mapper", action="store", help="Apply pixel mappers. e.g \"Rotate:90\"", default="", type=str)
    parser.add_argument("--led-row-addr-type", action="store", help="0 = default; 1=AB-addressed panels; 2=row direct; 3=ABC-addressed panels; 4 = ABC Shift + DE direct", default=0, type=int, choices=[0,1,2,3,4])
    parser.add_argument("--led-multiplexing", action="store", help="Multiplexing type: 0=direct; 1=strip; 2=checker; 3=spiral; 4=ZStripe; 5=ZnMirrorZStripe; 6=coreman; 7=Kaler2Scan; 8=ZStripeUneven... (Default: 0)", default=0, type=int)
    parser.add_argument("--led-panel-type", action="store", help="Needed to initialize special panels. Supported: 'FM6126A'", default="", type=str)
    parser.add_argument("--led-no-drop-privs", dest="drop_privileges", help="Don't drop privileges from 'root' after initializing the hardware.", action='store_false')
    parser.set_defaults(drop_privileges=True)

    return parser.parse_args()

def create_matrix(args):
    options = RGBMatrixOptions()

    options.hardware_mapping = args.led_gpio_mapping
    options.rows = args.led_rows
    options.cols = args.led_cols
    options.chain_length = args.led_chain
    options.parallel = args.led_parallel
    options.row_address_type = args.led_row_addr_type
    options.multiplexing = args.led_multiplexing
    options.pwm_bits = args.led_pwm_bits
    options.brightness = args.led_brightness
    options.pwm_lsb_nanoseconds = args.led_pwm_lsb_nanoseconds
    options.led_rgb_sequence = args.led_rgb_sequence
    options.pixel_mapper_config = args.led_pixel_mapper
    options.panel_type = args.led_panel_type

    if args.led_show_refresh:
        options.show_refresh_rate = 1

    if args.led_slowdown_gpio != None:
        options.gpio_slowdown = args.led_slowdown_gpio
    if args.led_no_hardware_pulse:
        options.disable_hardware_pulsing = True
    if not args.drop_privileges:
        options.drop_privileges=False

    return RGBMatrix(options = options)

market = Market()
if __name__ == "__main__":
    matrix = create_matrix(handle_args())

    stocks = Stocks(matrix, "NVDA")

    schedule.every(1).minutes.do(stocks.refresh)
    while True:
        schedule.run_pending()
        sleep(5)
        matrix.SwapOnVSync(stocks.get_canvas())
