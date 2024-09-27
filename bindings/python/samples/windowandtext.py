#!/usr/bin/env python
import threading
import time
from samplebase import SampleBase
from rgbmatrix import graphics

class BasicWindow(SampleBase):
    def __init__(self, *args, **kwargs):
        super(BasicWindow, self).__init__(*args, **kwargs)

    def run(self):
        offset_canvas = self.matrix.CreateFrameCanvas()
        while True:
            # Draw top border
            for x in range(self.matrix.width):
                offset_canvas.SetPixel(x, 0, 255, 0, 0)  # Red color
            # Draw bottom border
            for x in range(self.matrix.width):
                offset_canvas.SetPixel(x, self.matrix.height - 1, 255, 0, 0)  # Red color
            # Draw left border
            for y in range(self.matrix.height):
                offset_canvas.SetPixel(0, y, 255, 0, 0)  # Red color
            # Draw right border
            for y in range(self.matrix.height):
                offset_canvas.SetPixel(self.matrix.width - 1, y, 255, 0, 0)  # Red color
            # Draw horizontal line in the center
            center_y = self.matrix.height // 2
            for x in range(self.matrix.width):
                offset_canvas.SetPixel(x, center_y, 255, 0, 0)  # Red color
            # Draw vertical line in the center
            center_x = self.matrix.width // 2
            for y in range(self.matrix.height):
                offset_canvas.SetPixel(center_x, y, 255, 0, 0)  # Red color

            offset_canvas = self.matrix.SwapOnVSync(offset_canvas)
            time.sleep(0.05)  # Add a small delay to control the frame rate


class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)
        self.parser.add_argument(
            "-t", "--text", 
            help="The text to scroll on the RGB LED panel", 
            default="I find myself living in times that profess to be three things simultaneously: the future, the precipice of the end and the aftermath. And I sit with the things I have seen filling up my pockets until I am no longer able to walk."
        )

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


def run_basic_window():
    basic_window = BasicWindow()
    if not basic_window.process():
        basic_window.print_help()


def run_text_scroller():
    run_text = RunText()
    if not run_text.process():
        run_text.print_help()


if __name__ == "__main__":
    # Create threads for the two tasks
    window_thread = threading.Thread(target=run_basic_window)
    text_thread = threading.Thread(target=run_text_scroller)

    # Start both threads
    window_thread.start()
    text_thread.start()

    # Keep the main thread alive while both are running
    window_thread.join()
    text_thread.join()
