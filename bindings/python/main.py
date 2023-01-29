import argparse
from rgbmatrix import RGBMatrix, RGBMatrixOptions
import schedule
from time import sleep
from datetime import datetime, timedelta
import logging
import sys
import os
import requests

from clock import Clock
from stocks import Stocks, Market
from weather import Weather
from imageviewer import ImageViewer
from slack import SlackStatus
from secrets import SLACK_USER_ID, SLACK_TOKEN, LAT, LON

log = logging.getLogger()
path = os.path.dirname(__file__) + '/'

def handle_args(*args, **kwargs):
    parser = argparse.ArgumentParser()

    parser.add_argument("-d", "--debug", action="store", help="Sets debug output level: info, warning, error", default='warning', choices=['info', 'warning', 'error'], type=str)

    # matrix specific
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
    parser.add_argument("--led-slowdown-gpio", action="store", help="Slow down writing to GPIO. Range: 0..4. Default: 1", default=4, type=int)
    parser.add_argument("--led-no-hardware-pulse", action="store", help="Don't use hardware pin-pulse generation")
    parser.add_argument("--led-rgb-sequence", action="store", help="Switch if your matrix has led colors swapped. Default: RGB", default="RGB", type=str)
    parser.add_argument("--led-pixel-mapper", action="store", help="Apply pixel mappers. e.g \"Rotate:90\"", default="", type=str)
    parser.add_argument("--led-row-addr-type", action="store", help="0 = default; 1=AB-addressed panels; 2=row direct; 3=ABC-addressed panels; 4 = ABC Shift + DE direct", default=0, type=int, choices=[0,1,2,3,4])
    parser.add_argument("--led-multiplexing", action="store", help="Multiplexing type: 0=direct; 1=strip; 2=checker; 3=spiral; 4=ZStripe; 5=ZnMirrorZStripe; 6=coreman; 7=Kaler2Scan; 8=ZStripeUneven... (Default: 0)", default=0, type=int)
    parser.add_argument("--led-panel-type", action="store", help="Needed to initialize special panels. Supported: 'FM6126A'", default="", type=str)
    parser.add_argument("--led-no-drop-privs", dest="drop_privileges", help="Don't drop privileges from 'root' after initializing the hardware.", action='store_false')
    parser.set_defaults(drop_privileges=True)

    res = parser.parse_args()
    cmd = res.debug
    if res.debug == 'info':
        logging.basicConfig(level=logging.INFO, \
                            filename=path + 'log.txt', \
                            format='[%(asctime)s] %(levelname)-8s (%(name)s) %(message)s', datefmt='%H:%M:%S') # stream=sys.stdout)
    elif res.debug == 'warning':
        logging.basicConfig(level=logging.WARNING, \
                            filename=path + 'log.txt', \
                            format='[%(asctime)s] %(levelname)-8s (%(name)s) %(message)s', datefmt='%H:%M:%S') # stream=sys.stdout)
    elif res.debug == 'error':
        logging.basicConfig(level=logging.ERROR, \
                            filename=path + 'log.txt', \
                            format='[%(asctime)s] %(levelname)-8s (%(name)s) %(message)s', datefmt='%H:%M:%S') # stream=sys.stdout)

    return res

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

def log_schedule():
    log.info("Scheduled jobs: %s" % schedule.get_jobs())

if __name__ == "__main__":
    schedule.every(5).minutes.do(log_schedule).tag('system')
    log_schedule()

    matrix = create_matrix(handle_args())

    main_app = SlackStatus(matrix, SLACK_USER_ID, SLACK_TOKEN)

    apps = list()
    apps.append(Stocks(matrix, "NVDA"))
    apps.append(Stocks(matrix, "VTI"))
    apps.append(Clock(matrix))
    apps.append(Weather(matrix, LAT, LON))
    apps.append(ImageViewer(matrix, path + "images/nvidia.png"))

    id = 0
    runtime = 0
    duration = 6
    while True:
        schedule.run_pending()
        if main_app.check_status():
            matrix.SwapOnVSync(main_app.show())
            sleep(1/main_app.get_framerate())
        else:
            framerate = apps[id].get_framerate()
            for sec in range(0,framerate):
                matrix.SwapOnVSync(apps[id].show())
                sleep(1/framerate)
            
            runtime += 1
            if runtime >= duration:
                runtime = 0
                id = 0 if id >= len(apps)-1 else id + 1
