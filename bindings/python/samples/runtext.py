#!/usr/bin/env python
# Display a runtext with double-buffering.
from samplebase import SampleBase
from argparse import ArgumentTypeError
from rgbmatrix import graphics
import time

DEFAULT_FONT = "../../../fonts/7x13.bdf"

def color(value):
    try:
        return graphics.Color(*[int(v) for v in value.split(",")])
    except:
        raise ArgumentTypeError(f"{value} is an invalid color value. Expected R,G,B values between 0-255, ex: 255,255,255")

class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)
        self.parser.add_argument("-f", "--font", help="Path to *.bdf-font to be used", default=DEFAULT_FONT)
        self.parser.add_argument("-t", "--text", help="The text to scroll on the RGB LED panel", default="Hello world!")

        self.parser.add_argument("-y", type=int, help="Shift Y-Origin of displaying text (Default: 10)", default=10)
        self.parser.add_argument("-l", "--loop", type=int, help="Number of loops through the text")
        self.parser.add_argument("-k", "--blink", help="Blink while scrolling. Keep on and off for these amount of scrolled pixels. Ex: 10,5", default=None)

        self.parser.add_argument("-C", "--text-color", type=color, help="Text color. Default 255,255,255 (white)", default="255,255,255")
        self.parser.add_argument("-B", "--background-color", type=color, help="Background color. Default 0,0,0 (white)", default="0,0,0")


    def run(self):
        bg_color = self.args.background_color

        main_canvas = self.matrix.CreateFrameCanvas()
        bg_canvas = self.matrix.CreateFrameCanvas()
        bg_canvas.Fill(bg_color.red, bg_color.green, bg_color.blue)

        font = graphics.Font()
        font.LoadFont(self.args.font)
        x_pos = main_canvas.width

        # Looping params
        i = 0
        loop_max = self.args.loop or float("inf")

        # Blinking params
        blink_on_for, blink_off_for = [int(v) for v in self.args.blink.split(",")] if self.args.blink else [float("inf"), 0]
        blink_ct = 0
        blink_on = True

        while i < loop_max:
            x_pos -= 1

            if blink_on:
                if blink_ct >= blink_on_for:
                    blink_on = False
                    blink_ct = 0

                main_canvas.Fill(bg_color.red, bg_color.green, bg_color.blue)
                len = graphics.DrawText(main_canvas, font, x_pos, self.args.y, self.args.text_color, self.args.text)
                main_canvas = self.matrix.SwapOnVSync(main_canvas)

                if (x_pos + len < 0):
                    i += 1
                    x_pos = main_canvas.width
            else:
                if blink_ct >= blink_off_for:
                    blink_on = True
                    blink_ct = 0
                self.matrix.SwapOnVSync(bg_canvas)

            blink_ct += 1

            time.sleep(0.05)

# Main function
if __name__ == "__main__":
    run_text = RunText()
    if (not run_text.process()):
        run_text.print_help()
