#!/usr/bin/env python
from samplebase import SampleBase
import math


def scale_col(val, lo, hi):
    if val < lo:
        return 0
    if val > hi:
        return 255
    return 255 * (val - lo) / (hi - lo)


class RotatingBlockGenerator(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RotatingBlockGenerator, self).__init__(*args, **kwargs)

    def run(self):
        cent_x = self.matrix.width / 2
        cent_y = self.matrix.height / 2

        rotate_square = min(self.matrix.width, self.matrix.height) * 1.41
        min_rotate = cent_x - rotate_square / 2
        max_rotate = cent_x + rotate_square / 2

        display_square = min(self.matrix.width, self.matrix.height) * 0.7
        min_display = cent_x - display_square / 2
        max_display = cent_x + display_square / 2

        deg_to_rad = 2 * 3.14159265 / 360
        rotation = 0

        # Pre calculate colors
        col_table = []
        for x in range(int(min_rotate), int(max_rotate)):
            col_table.insert(x, scale_col(x, min_display, max_display))

        # Pre calculate angles
        sin_table = []
        cos_table = []

        for rotation in range(0, 360):
            angle = rotation * deg_to_rad
            sin_table.insert(rotation, math.sin(angle))
            cos_table.insert(rotation, math.cos(angle))

        offset_canvas = self.matrix.CreateFrameCanvas()

        while True:
            rotation += 1
            rotation %= 360

            sin = sin_table[rotation]
            cos = cos_table[rotation]

            for x in range(int(min_rotate), int(max_rotate)):
                for y in range(int(min_rotate), int(max_rotate)):
                    # translate so that the origin is now (cent_x, cent_y)
                    temp_x = x - cent_x
                    temp_y = y - cent_x
                    # rotate about the new origin
                    new_x = temp_x * cos - temp_y * sin
                    new_y = temp_x * sin + temp_y * cos
                    # translate back
                    new_x += cent_x
                    new_y += cent_y

                    if x >= min_display and x < max_display and y >= min_display and y < max_display:
                        x_col = col_table[x]
                        y_col = col_table[y]
                        offset_canvas.SetPixel(new_x, new_y, x_col, 255 - y_col, y_col)
                    else:
                        offset_canvas.SetPixel(new_x, new_y, 0, 0, 0)

            offset_canvas = self.matrix.SwapOnVSync(offset_canvas)


# Main function
if __name__ == "__main__":
    rotating_block_generator = RotatingBlockGenerator()
    if (not rotating_block_generator.process()):
        rotating_block_generator.print_help()
