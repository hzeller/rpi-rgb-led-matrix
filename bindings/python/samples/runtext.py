#!/usr/bin/env python
# Display a runtext with double-buffering.
from samplebase import SampleBase
from rgbmatrix import graphics
import time


class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)
        self.parser.add_argument("-t", "--text", help="The text to scroll on the RGB LED panel", default="I find myself living in times that profess to be three things simultaneously: the future, the precipice of the end and the aftermath. And I sit with the things I have seen filling up my pockets until I am no longer able to walk. For how is one supposed to hold the future, the end and the aftermath in one pocket (or even two)? As I write this it becomes clear that the world has ended many times and continues to end in front of my eyes.")

    def run(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        font.LoadFont("../../../fonts/7x13.bdf")
        textColor = graphics.Color(255, 0, 0)  # Red color
        pos = offscreen_canvas.width
        my_text = self.args.text
        
        # Centering text vertically (assuming the font height is 13 pixels)
        font_height = 13
        canvas_height = offscreen_canvas.height
        vertical_position = (canvas_height // 2) + (font_height // 2)  # Centered vertically

        while True:
            offscreen_canvas.Clear()
            text_length = graphics.DrawText(offscreen_canvas, font, pos, vertical_position, textColor, my_text)
            pos -= 1
            if (pos + text_length < 0):
                pos = offscreen_canvas.width

            time.sleep(0.05)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)


# Main function
if __name__ == "__main__":
    run_text = RunText()
    if (not run_text.process()):
        run_text.print_help()

