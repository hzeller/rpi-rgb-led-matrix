import argparse
import time
import sys
import os

sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix


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

    def usleep(self, value):
        time.sleep(value / 1000000.0)

    def run(self):
        print("Running")

    def process(self):
        self.args = self.parser.parse_args()
        
        # TODO: validate values with RGBmatrix::Options::Validate().

        self.matrix = RGBMatrix(self.args.rows, self.args.chain, self.args.parallel)
        self.matrix.pwmBits = self.args.pwmbits
        self.matrix.brightness = self.args.brightness

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
