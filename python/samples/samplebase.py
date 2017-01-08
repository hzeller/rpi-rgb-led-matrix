import argparse
import time
import sys
import os

sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix, RGBMatrixOptions, RuntimeOptions


class SampleBase(object):
    def __init__(self, *args, **kwargs):
        self.parser = argparse.ArgumentParser()

        # TODO: could this fill RGBMatrix::Options instead ?
        self.parser.add_argument("-r", "--rows", action="store", help="Display rows. 16 for 16x32, 32 for 32x32. Default: 32", default=32, type=int)
        self.parser.add_argument("-P", "--parallel", action="store", help="For Plus-models or RPi2: parallel chains. 1..3. Default: 1", default=1, type=int)
        self.parser.add_argument("-c", "--chain", action="store", help="Daisy-chained boards. Default: 1.", default=1, type=int)
        self.parser.add_argument("-p", "--pwmbits", action="store", help="Bits used for PWM. Something between 1..11. Default: 11", default=11, type=int)
        self.parser.add_argument("-l", "--luminance", action="store_true", help="Don't do luminance correction (CIE1931)")
        self.parser.add_argument("-b", "--brightness", action="store", help="Sets brightness level. Default: 100. Range: 1..100", default=100, type=int)
        self.parser.add_argument("-m", "--hardware_mapping", help="Hardware Mapping: regular, adafruit-hat, adafruit-hat-pwm" , choices=['regular', 'adafruit-hat', 'adafruit-hat-pwm'], type=str)
        self.parser.add_argument("--pwm_lsb_nanoseconds", action="store", help="Base time-unit for the on-time in the lowest significant bit in nanoseconds. Default: 130", default=130, type=int)
        self.parser.add_argument("--show_refresh_rate", action="store_true", help="Shows the current refresh rate of the LED panel")
        self.parser.add_argument("--gpio_slowdown", action="store", help="Slow down writing to GPIO. Range: 1..100. Default: 1", type=int)

    def usleep(self, value):
        time.sleep(value / 1000000.0)

    def run(self):
        print("Running")

    def process(self):
        self.args = self.parser.parse_args()

        options = RGBMatrixOptions()

        if self.args.hardware_mapping != None:
          options.hardware_mapping = self.args.hardware_mapping
        options.rows = self.args.rows
        options.chain_length = self.args.chain
        options.parallel = self.args.parallel
        options.pwm_bits = self.args.pwmbits
        options.brightness = self.args.brightness
        options.pwm_lsb_nanoseconds = self.args.pwm_lsb_nanoseconds
        if self.args.show_refresh_rate:
          options.show_refresh_rate = 1

        runtime_options = RuntimeOptions()
        if self.args.gpio_slowdown != None:
            runtime_options.gpio_slowdown = self.args.gpio_slowdown

        self.matrix = RGBMatrix(options, runtime_options)

        if self.args.luminance:
            self.matrix.luminanceCorrect = False

        try:
            # Start loop
            print("Press CTRL-C to stop sample")
            self.run()
        except KeyboardInterrupt:
            print("Exiting\n")
            sys.exit(0)

        return True
