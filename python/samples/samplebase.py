import argparse, time, sys, os

sys.path.append(os.path.abspath(os.path.dirname(__file__) + '/..'))
from rgbmatrix import RGBMatrix

class SampleBase(argparse.ArgumentParser):
    def __init__(self, *args, **kwargs):
        super(SampleBase, self).__init__(*args, **kwargs)

        # TODO: could this fill RGBMatrix::Options instead ?
        self.add_argument("-r", "--rows", action = "store", help = "Display rows. 16 for 16x32, 32 for 32x32. Default: 32", default = 32, type = int)
        self.add_argument("-P", "--parallel", action = "store", help = "For Plus-models or RPi2: parallel chains. 1..3. Default: 1", default = 1, type = int)
        self.add_argument("-c", "--chain", action = "store", help = "Daisy-chained boards. Default: 1.", default = 1, type = int)
        self.add_argument("-p", "--pwmbits", action = "store", help = "Bits used for PWM. Something between 1..11. Default: 11", default = 11, type = int)
        self.add_argument("-l", "--luminance", action = "store_true", help = "Don't do luminance correction (CIE1931)")
        self.add_argument("-b", "--brightness", action = "store", help = "Sets brightness level. Default: 100. Range: 1..100", default = 100, type = int)
        self.add_argument("-t", "--text", help="The text to scroll on the RGB LED panel", default = "Hello world!")

        self.args = {}

    def usleep(self, value):
        time.sleep(value / 1000000.0)

    def Run(self):
        print("Running")

    def process(self):
        self.args = vars(self.parse_args())

        # TODO: validate values with RGBmatrix::Options::Validate().

        self.matrix = RGBMatrix(self.args["rows"], self.args["chain"], self.args["parallel"])
        self.matrix.pwmBits = self.args["pwmbits"]
        self.matrix.brightness = self.args["brightness"]

        if self.args["luminance"]:
            self.matrix.luminanceCorrect = False

        try:
            # Start loop
            print("Press CTRL-C to stop sample")
            self.Run()
        except KeyboardInterrupt:
            print("Exiting\n")
            sys.exit(0)

        return True
