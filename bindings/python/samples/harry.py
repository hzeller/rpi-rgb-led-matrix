#!/usr/bin/env python

import time
import sys
import requests
import json
from datetime import datetime

from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics

# Configuration for the matrix
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 2
options.gpio_slowdown = 4
options.brightness = 75
options.parallel = 1
options.hardware_mapping = "adafruit-hat"  # If you have an Adafruit HAT: 'adafruit-hat'

matrix = RGBMatrix(options = options)

big_font = graphics.Font()
big_font.LoadFont("../../../fonts/10x20.bdf")

medium_big_font = graphics.Font()
medium_big_font.LoadFont("../../../fonts/7x13B.bdf")

medium_font = graphics.Font()
medium_font.LoadFont("../../../fonts/6x9.bdf")

small_font = graphics.Font()
small_font.LoadFont("../../../fonts/5x7.bdf")

mta_g_green = graphics.Color(108, 190, 69)
white = graphics.Color(255, 255, 255)
half_white = graphics.Color(126, 126, 126)
another_white = graphics.Color(50, 50, 50)
orange = graphics.Color(194, 89, 29)
purple = graphics.Color(78, 0, 130)
red = graphics.Color(213, 0, 0)
yellow = graphics.Color(201, 181, 0)

def run():
    transit_times = get_transit()
    matrix.Clear()
    if transit_times:
        draw_dividing_lines()
        draw_trains(transit_times["train"])
        draw_bus(transit_times["bus"])
    else:
        draw_error(95, 16)
    # draw_trains({ 'n': [4, 13], 's': [2, 18] })
    # draw_bus([3, 10, 29])

def draw_error(x, y):
    draw_face(17, 16)
    graphics.DrawText(matrix, big_font, 39, 23, half_white, "ERROR")
    draw_face(110, 16)

def draw_face(x, y):
    graphics.DrawCircle(matrix, x, y, 14, yellow)
    graphics.DrawLine(matrix, x - 5, y - 4, x - 4, y - 4, half_white)
    graphics.DrawLine(matrix, x + 4, y - 4, x + 5, y - 4, half_white)
    graphics.DrawLine(matrix, x - 5, y + 5, x + 5, y + 5, half_white)

def draw_trains(train_times):
    draw_mta_g()
    draw_station_names()
    if len(train_times) > 0:
        draw_court_sq_times(str(train_times["n"][0]), str(train_times["n"][1]))
        draw_church_ave_times(str(train_times["s"][0]), str(train_times["s"][1]))
    else:
        new_draw_no_transit(78, 7)
        new_draw_no_transit(78, 24)

def new_draw_no_transit(x, y):
    graphics.DrawCircle(matrix, x, y, 6, red)
    graphics.DrawLine(matrix, x - 4, y + 4, x + 4, y - 4, red)

def draw_bus(bus_times):
    draw_bus_name()
    if len(bus_times) > 0:
        draw_bus_times(bus_times)
    else:
        new_draw_no_transit(120, 24)

def get_transit():
    transit_times = requests.get('https://funmirror-server.herokuapp.com/led-sign-transit')
    if transit_times.status_code == 200:
        return transit_times.json()
    else:
        return False

def draw_bus_name():
    graphics.DrawText(matrix, medium_big_font, 107, 10, purple, "B62")

def draw_bus_times(b62_times):
    time_base_x_coord = 100
    time_base_y_coord = 31
    y_coords = [24, 17, 10]
    y_coord_base = y_coords[len(b62_times) - 1]

    for i, mins in enumerate(b62_times, start=1):
        mins_str = str(mins)
        graphics.DrawText(matrix, medium_font,time_base_x_coord + x_coord_time_offset(mins_str), y_coord_base + (i * 7), white, mins_str)
        graphics.DrawText(matrix, small_font, time_base_x_coord + 13, y_coord_base + (i * 7), orange, "min")

def draw_mta_g():
    graphics.DrawText(matrix, big_font, 0, 14, mta_g_green, "G")
    graphics.DrawText(matrix, big_font, 0, 31, mta_g_green, "G")

def draw_station_names():
    graphics.DrawText(matrix, medium_big_font, 13, 12, half_white, "Court")
    graphics.DrawText(matrix, medium_big_font, 13, 29, half_white, "Church")

def draw_dividing_lines():
    graphics.DrawLine(matrix, 0, 16, 88, 16, another_white)
    graphics.DrawLine(matrix, 88, 0, 88, 32, another_white)

def draw_court_sq_times(first_court_g, second_court_g):
    time_base_x_coord = 60

    graphics.DrawText(matrix, medium_font, time_base_x_coord + x_coord_time_offset(first_court_g), 7, white, first_court_g)
    graphics.DrawText(matrix, medium_font, time_base_x_coord + x_coord_time_offset(second_court_g), 15, white, second_court_g)

    graphics.DrawText(matrix, small_font, time_base_x_coord + 13, 7, orange, "min")
    graphics.DrawText(matrix, small_font, time_base_x_coord + 13, 15, orange, "min")

def draw_church_ave_times(first_church_g, second_church_g):
    time_base_x_coord = 60

    graphics.DrawText(matrix, medium_font, time_base_x_coord + x_coord_time_offset(first_church_g), 23, white, first_church_g)
    graphics.DrawText(matrix, medium_font, time_base_x_coord + x_coord_time_offset(second_church_g), 31, white, second_church_g)

    graphics.DrawText(matrix, small_font, time_base_x_coord + 13, 23, orange, "min")
    graphics.DrawText(matrix, small_font, time_base_x_coord + 13, 31, orange, "min")

def x_coord_time_offset(mins):
    if len(mins) == 1:
        return 6
    else:
        return 0

try:
    print("Press CTRL-C to stop.")
    while True:
        run()
        time.sleep(10)

except KeyboardInterrupt:
    sys.exit(0)

