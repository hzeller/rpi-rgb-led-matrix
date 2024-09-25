#!/usr/bin/env python
from samplebase import SampleBase


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


# Main function
if __name__ == "__main__":
    basic_window = BasicWindow()
    if not basic_window.process():
        basic_window.print_help()
