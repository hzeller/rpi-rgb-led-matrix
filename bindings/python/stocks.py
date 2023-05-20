import os
import logging
import json
import time
import requests
import zoneinfo
from datetime import datetime, timedelta
import pytz
import numpy
from twelvedata import TDClient, exceptions
from apscheduler.schedulers.background import BackgroundScheduler
from rgbmatrix import graphics
from secrets import STOCKS_API_KEY, LOCAL_TZ

path = os.path.dirname(__file__) + "/"
log = logging.getLogger(__name__)

schedule = BackgroundScheduler(daemon=True)
schedule.start()


class API:
    def __init__(self):
        self.exchange = "NYSE"
        self.timezone = "America/New_York"
        self.symbol = "NVDA"
        self.open_hour = 9
        self.open_min = 30
        self.open_time = 390  # minutes in stock day

        self.api_key = STOCKS_API_KEY
        self.td = TDClient(apikey=self.api_key)

    def _try_api(self, func):
        tries = 5
        res = None
        while tries > 0:
            try:
                res = func.as_json()
                break
            except exceptions.BadRequestError:
                log.warning("API bad request using %s" % func.as_url())
                break
            except exceptions.TwelveDataError:
                timeout = 61 - datetime.now().second
                log.warning(
                    "API out of credits using %s trying again in %d seconds"
                    % (func.as_url(), timeout)
                )
                tries -= 1
                time.sleep(timeout)
            except:
                log.warning("API exception occured")
        if not res:
            log.error("API errors continue after several attempts")
        return res

    def _try_request(self, url):
        tries = 5
        res = None
        while tries > 0:
            try:
                json = requests.get(url).json()
                if isinstance(json, dict) and json["status"] == "error":
                    timeout = 61 - datetime.now().second
                    log.warning(
                        "URL bad request using %s trying again in %d seconds"
                        % (url, timeout)
                    )
                    tries -= 1
                    time.sleep(timeout)
                    continue
                res = json
                break
            except:
                log.warning("URL exception occured")
        if not res:
            log.error("URL errors continue after several attempts")
        return res

    def is_trading_day(self, day):
        ts = self.td.time_series(
            symbol=self.symbol,
            interval="1min",
            outputsize=1,
            start_date=day,
            end_date=day + timedelta(minutes=self.open_time),
            timezone=self.timezone,
        )
        res = self._try_api(ts)
        log.info("API _is_trading_day: %s -> %s" % (ts.as_url(), res))

        return res

    def get_market_state(self):
        url = f"https://api.twelvedata.com/market_state?exchange={self.exchange}&apikey={self.api_key}"
        res = self._try_request(url)
        log.info("API _update_market_state: %s -> %s" % (url, res))

        return res

    def get_last_close_price(self, day, symbols: list):
        assert len(symbols) > 0
        ts = self.td.time_series(
            symbol=symbols,
            interval="1day",
            outputsize=1,
            start_date=day,
            end_date=day + timedelta(minutes=self.open_time),
            timezone=self.timezone,
        )
        res = self._try_api(ts)
        log.info("API get_last_close_price: %s -> %s" % (ts.as_url(), res))

        data = dict()
        if res:
            if len(symbols) > 1:
                for symbol in symbols:
                    data[symbol] = float(res[symbol][0]["close"])
            else:
                data[symbols[0]] = float(res[0]["close"])

        return data

    def get_trading_day_data(self, day, symbols: list):
        assert len(symbols) > 0
        ts = self.td.time_series(
            symbol=symbols,
            interval="1min",
            start_date=day,
            outputsize=self.open_time,
            end_date=day + timedelta(minutes=self.open_time),
            timezone=self.timezone,
        )
        res = self._try_api(ts)
        log.info("API get_trading_day_data: %s -> %s" % (ts.as_url(), res))

        data = dict()
        if res:
            if len(symbols) > 1:
                data = res
            else:
                data[symbols[0]] = res

        return data


class Data:
    def __init__(self):
        self.api = API()
        self.data_file = "data.json"
        self.data = dict()
        self.symbols = list()

        self.timestamps = list(
            numpy.rint(numpy.linspace(0, self.api.open_time - 1, 64))
        )
        self.graph = Graph(17, 64, self.timestamps)

        self.trade_day = None
        self.previous_day = None
        self.next_update = None

    def _update_data(self, previous_day, trading_day, symbols):
        api = API()
        # prepare output stucture
        data = dict()
        for symbol in symbols:
            data[symbol] = dict()

        # update closing price
        res = api.get_last_close_price(previous_day, symbols)
        for symbol in res:
            data[symbol]["close"] = round(res[symbol], 2)

        # update trading day data
        res = api.get_trading_day_data(trading_day, symbols)
        for symbol in res:
            data[symbol]["current"] = round(float(res[symbol][0]["close"]), 2)
            data[symbol]["difference"] = round(
                float(res[symbol][0]["close"]) - data[symbol]["close"], 2
            )
            data[symbol]["percent"] = round(
                data[symbol]["difference"] / data[symbol]["close"] * 100, 2
            )
            data[symbol]["graph"] = self.graph.parse(res[symbol], data[symbol]["close"])
            data[symbol]["updated"] = datetime.now().timestamp()

        self._save(data)

    def _update_market_state(self):
        # update trading day
        trading_day = datetime.now(pytz.timezone(self.api.timezone))
        # is it before trading hours
        log.info("now: %i, open: %i" % (trading_day.hour * 60 + trading_day.minute, self.api.open_hour * 60 + self.api.open_min))
        if (trading_day.hour * 60 + trading_day.minute) < (
            self.api.open_hour * 60 + self.api.open_min
        ):
            log.info("before open!")
            trading_day = (trading_day - timedelta(days=1)).replace(
                hour=self.api.open_hour,
                minute=self.api.open_min,
                second=0,
                microsecond=0,
            )
        else:
            trading_day = trading_day.replace(
                hour=self.api.open_hour,
                minute=self.api.open_min,
                second=0,
                microsecond=0,
            )
        while trading_day.weekday() > 4 or not self.api.is_trading_day(trading_day):
            trading_day -= timedelta(days=1)
        log.info("current trading day: %s" % trading_day.strftime("%Y-%m-%d"))

        # update previous trading day
        previous_day = trading_day - timedelta(days=1)
        while previous_day.weekday() > 4 or not self.api.is_trading_day(previous_day):
            previous_day -= timedelta(days=1)
        log.info("previous trading day: %s" % previous_day.strftime("%Y-%m-%d"))

        self._update_data(previous_day, trading_day, self.symbols)

        # update market status
        market_state = self.api.get_market_state()[0]
        if market_state["is_market_open"]:
            time_to_close = market_state["time_to_close"].split(":")
            next_update = datetime.now().replace(
                tzinfo=zoneinfo.ZoneInfo(LOCAL_TZ)
            ) + timedelta(
                minutes=int(time_to_close[0]) * 60 + int(time_to_close[1]) + 5
            )

            exists = False
            for job in schedule.get_jobs():
                if job.id == "update_data":
                    exists = True
            if not exists:
                schedule.add_job(
                    self._update_data,
                    "interval",
                    args=[previous_day, trading_day, self.symbols],
                    minutes=3,
                    id="update_data",
                )
        else:
            time_to_open = market_state["time_to_open"].split(":")
            next_update = datetime.now().replace(
                tzinfo=zoneinfo.ZoneInfo(LOCAL_TZ)
            ) + timedelta(minutes=int(time_to_open[0]) * 60 + int(time_to_open[1]) + 5)

            exists = False
            for job in schedule.get_jobs():
                if job.id == "update_data":
                    exists = True
            if exists:
                schedule.remove_job("update_data")

        log.info(
            "next trading day update: %s" % (next_update).strftime("%Y-%m-%d %H:%M")
        )
        schedule.add_job(
            self._update_market_state,
            "date",
            run_date=next_update,
            id="_update_market_state",
        )

        data = dict()
        data["trading_day"] = trading_day.strftime("%Y-%m-%d")
        data["previous_day"] = previous_day.strftime("%Y-%m-%d")
        data["next_update"] = next_update.strftime("%Y-%m-%d %H:%M")
        self._save(data)

    def _save(self, data):
        # add timestamp to data
        data["updated"] = datetime.now().timestamp()

        with open("stocks.json", "r") as file:
            load_data = json.load(file)
        load_data.update(data)
        with open("stocks.json", "w") as file:
            file.write(json.dumps(load_data))

        # read in only necessary symbols
        for symbol in self.symbols:
            if symbol in load_data:
                self.data[symbol] = load_data[symbol]

    def add_symbol(self, symbol):
        self.symbols.append(symbol)
        if len(self.symbols) == 1:
            schedule.add_job(self._update_market_state)

    def remove_symbol(self, symbol):
        self.symbols.remove(symbol)

    def get_close_price(self, symbol):
        return self.data[symbol]["close"]

    def get_current_price(self, symbol):
        return self.data[symbol]["current"]

    def get_current_difference(self, symbol):
        return self.data[symbol]["difference"]

    def get_current_percent(self, symbol):
        return self.data[symbol]["percent"]

    def get_graph_data(self, symbol):
        return self.data[symbol]["graph"]

    def has_data(self, symbol):
        return True if symbol in self.data else False


class Graph:
    def __init__(self, height, width, timestamps):
        self.height = height
        self.width = width
        self.timestamps = timestamps

    def parse(self, raw, close_price):
        data = dict()
        samples = list()
        prev = datetime.strptime(
            raw[-1]["datetime"], "%Y-%m-%d %H:%M:%S"
        ) - timedelta(minutes=1)
        for delta in self.timestamps:
            time = datetime.strptime(
                raw[-1]["datetime"], "%Y-%m-%d %H:%M:%S"
            ) + timedelta(minutes=delta)
            sample = None
            tries = 5
            while not sample and tries > 0 and time > prev:
                sample = list(
                    filter(
                        lambda values: values["datetime"]
                        == time.strftime("%Y-%m-%d %H:%M:%S"),
                        raw,
                    )
                )
                if not sample: time -= timedelta(minutes=1)
                tries -= 1
            if sample:
                prev = time
                if delta == 0:  # first data point is at open
                    samples.append(float(sample[0]["open"]))
                else:
                    samples.append(float(sample[0]["close"]))
            else:
                break

        max_val = max(samples)
        min_val = min(samples)

        if close_price > max_val:
            max_val = close_price
        elif close_price < min_val:
            min_val = close_price

        scale = self.height / (max_val - min_val)
        data["inflection_pt"] = round((close_price - min_val) * scale)

        x = 0
        data["values"] = list()
        for sample in samples:
            data["values"].append((x, int((sample - min_val) * scale)))
            x += 1

        return data

    def draw(self, data, canvas, x_offset, y_offset):
        green = (graphics.Color(0, 25, 0), graphics.Color(0, 255, 0))
        red = (graphics.Color(25, 0, 0), graphics.Color(255, 0, 0))
        # draw area
        for idx in range(0, len(data["values"])):
            x = data["values"][idx][0]
            y = data["values"][idx][1]
            if y >= data["inflection_pt"]:
                graphics.DrawLine(
                    canvas,
                    x + x_offset,
                    y_offset - y,
                    x + x_offset,
                    y_offset - data["inflection_pt"],
                    green[0],
                )
            else:
                graphics.DrawLine(
                    canvas,
                    x + x_offset,
                    y_offset - y,
                    x + x_offset,
                    y_offset - data["inflection_pt"],
                    red[0],
                )
        # draw line
        for idx in range(0, len(data["values"])):
            x = data["values"][idx][0]
            y = data["values"][idx][1]
            if y > data["inflection_pt"]:  # in the green
                if x == len(data["values"]) - 1:  # last data point
                    graphics.DrawLine(
                        canvas,
                        x + x_offset,
                        y_offset - y,
                        x + x_offset,
                        y_offset - y,
                        green[1],
                    )
                else:
                    next_y = data["values"][idx + 1][1]
                    if next_y < data["inflection_pt"]:  # transition below the line
                        graphics.DrawLine(
                            canvas,
                            x + x_offset,
                            y_offset - y,
                            x + x_offset,
                            y_offset - data["inflection_pt"],
                            green[1],
                        )
                        graphics.DrawLine(
                            canvas,
                            x + x_offset,
                            y_offset - data["inflection_pt"],
                            x + x_offset + 1,
                            y_offset - next_y,
                            red[1],
                        )
                    else:
                        graphics.DrawLine(
                            canvas,
                            x + x_offset,
                            y_offset - y,
                            x + x_offset + 1,
                            y_offset - next_y,
                            green[1],
                        )
            else:  # in the red
                if x == len(data["values"]) - 1:  # last data point
                    graphics.DrawLine(
                        canvas,
                        x + x_offset,
                        y_offset - y,
                        x + x_offset,
                        y_offset - y,
                        red[1],
                    )
                else:
                    next_y = data["values"][idx + 1][1]
                    if next_y > data["inflection_pt"]:  # transition above the line
                        graphics.DrawLine(
                            canvas,
                            x + x_offset,
                            y_offset - y,
                            x + x_offset,
                            y_offset - data["inflection_pt"],
                            red[1],
                        )
                        graphics.DrawLine(
                            canvas,
                            x + x_offset,
                            y_offset - data["inflection_pt"],
                            x + x_offset + 1,
                            y_offset - next_y,
                            green[1],
                        )
                    else:
                        graphics.DrawLine(
                            canvas,
                            x + x_offset,
                            y_offset - y,
                            x + x_offset + 1,
                            y_offset - next_y,
                            red[1],
                        )


class Stocks:
    def __init__(self, offscreen_canvas, symbol):
        self.framerate = 1
        self.offscreen_canvas = offscreen_canvas

        self.symbol = symbol

        data_store.add_symbol(self.symbol)

    def __del__(self):
        data_store.remove_symbol(self.symbol)

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        self.offscreen_canvas = matrix.SwapOnVSync(self.draw())

    def draw(self):
        self.offscreen_canvas.Clear()
        font = graphics.Font()
        font.LoadFont(path + "../../fonts/5x6.bdf")
        white = graphics.Color(255, 255, 255)
        grey = graphics.Color(155, 155, 155)
        red = graphics.Color(255, 0, 0)
        green = graphics.Color(0, 255, 0)
        black = graphics.Color(0, 0, 0)

        graphics.DrawText(self.offscreen_canvas, font, 1, 6, white, self.symbol)
        try:
            self.closing_price = data_store.get_close_price(self.symbol)
            self.curr_price = data_store.get_current_price(self.symbol)
            self.graph_data = data_store.get_graph_data(self.symbol)
            self.curr_diff = data_store.get_current_difference(self.symbol)
            self.curr_percent = data_store.get_current_percent(self.symbol)

            graphics.DrawText(
                self.offscreen_canvas, font, 1, 13, grey, str("%0.2f" % self.curr_price)
            )
            line1_width = graphics.DrawText(
                self.offscreen_canvas, font, 0, 0, black, str("%0.2f" % self.curr_diff)
            )
            line2_width = graphics.DrawText(
                self.offscreen_canvas,
                font,
                0,
                0,
                black,
                str("%0.2f" % self.curr_percent) + "%",
            )
            width = self.offscreen_canvas.width
            if self.curr_diff >= 0:
                graphics.DrawText(
                    self.offscreen_canvas,
                    font,
                    width - line1_width,
                    6,
                    green,
                    str("%0.2f" % self.curr_diff),
                )
                graphics.DrawText(
                    self.offscreen_canvas,
                    font,
                    width - line2_width,
                    13,
                    green,
                    str("%0.2f" % self.curr_percent) + "%",
                )
            else:
                graphics.DrawText(
                    self.offscreen_canvas,
                    font,
                    width - line1_width,
                    6,
                    red,
                    str("%0.2f" % self.curr_diff),
                )
                graphics.DrawText(
                    self.offscreen_canvas,
                    font,
                    width - line2_width,
                    13,
                    red,
                    str("%0.2f" % self.curr_percent) + "%",
                )

            data_store.graph.draw(self.graph_data, self.offscreen_canvas, 0, 31)
        except KeyError:
            graphics.DrawText(self.offscreen_canvas, font, 1, 13, grey, "-.--")
            graphics.DrawText(self.offscreen_canvas, font, 50, 6, grey, "-.--")
            graphics.DrawText(self.offscreen_canvas, font, 45, 13, grey, "-.--%")
            graphics.DrawText(self.offscreen_canvas, font, 13, 25, grey, "No data")

        return self.offscreen_canvas


data_store = Data()
