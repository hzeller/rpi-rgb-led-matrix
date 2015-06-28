import argparse, time, sys
from rgbmatrix import RGBMatrix

class SampleBase(argparse.ArgumentParser):
    def __init__(self, *args, **kwargs):
        super(SampleBase, self).__init__(*args, **kwargs)

        self.add_argument("-r", "--rows", action = "store", help = "Display rows. 16 for 16x32, 32 for 32x32. Default: 32", default = 32, type = int)
        self.add_argument("-P", "--parallel", action = "store", help = "For Plus-models or RPi2: parallel chains. 1..3. Default: 1", default = 1, type = int)
        self.add_argument("-c", "--chain", action = "store", help = "Daisy-chained boards. Default: 1.", default = 1, type = int)
        self.add_argument("-p", "--pwmbits", action = "store", help = "Bits used for PWM. Something between 1..11. Default: 11", default = 11, type = int)
        self.add_argument("-l", "--luminance", action = "store_true", help = "Don't do luminance correction (CIE1931)")

        self.args = {}

    def usleep(self, value):
        time.sleep(value / 1000000.0)

    def Run(self):
        print("Running")

    def process(self):
        self.args = vars(self.parse_args())

        if self.args["rows"] != 16 and self.args["rows"] != 32:
            print("Rows can either be 16 or 32")
            return False

        if self.args["chain"] < 1:
            print("Chain outside usable range")
            return False

        if self.args["chain"] > 8:
            print("That is a long chain. Expect some flicker.")

        if self.args["parallel"] < 1 or self.args["parallel"] > 3:
            print("Parallel outside usable range.")
            return False

        self.matrix = RGBMatrix(self.args["rows"], self.args["chain"], self.args["parallel"])
        self.matrix.pwmBits = self.args["pwmbits"]

        if self.args["luminance"]:
            self.matrix.luminanceCorrect = False

        try:
            # Start loop
            print("Press CTRL-C to stop sample")
            self.Run()
        except KeyboardInterrupt:
            print "Exiting\n"
            sys.exit(0)

        return True