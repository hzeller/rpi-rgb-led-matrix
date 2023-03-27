import logging
import random

log = logging.getLogger(__name__)


class DVD:
    def __init__(self, offscreen_canvas):
        self.framerate = 10
        self.offscreen_canvas = offscreen_canvas

        self.bitmap = [
            [1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0],
            [0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1],
            [1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1],
            [1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1],
            [1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0],
            [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0],
            [1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1],
            [0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0],
        ]

        self.bitmap = [
            [1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0],
            [0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1],
            [1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1],
            [1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1],
            [1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0],
            [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0],
            [1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1],
            [0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0],
        ]

        self.x_start = 0
        self.y_start = 0

        self.x_pos = True
        self.y_pos = True

        self.x_bound = offscreen_canvas.width - len(self.bitmap[0])
        self.y_bound = offscreen_canvas.height - len(self.bitmap)

        self.r = random.randint(0, 255)
        self.g = random.randint(0, 255)
        self.b = random.randint(0, 255)

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        self.offscreen_canvas = matrix.SwapOnVSync(self.draw())

    def draw(self):
        self.offscreen_canvas.Clear()

        x_local = 0
        y_local = 0
        for row in self.bitmap:
            for col in row:
                if self.bitmap[y_local][x_local]:
                    self.offscreen_canvas.SetPixel(
                        self.x_start + x_local,
                        self.y_start + y_local,
                        self.r,
                        self.g,
                        self.b,
                    )
                x_local += 1
            y_local += 1
            x_local = 0

        if self.x_pos:
            if self.x_start < self.x_bound:
                self.x_start += 1
            else:
                self.x_start -= 1
                self.x_pos = False
                self.r = random.randint(0, 255)
                self.g = random.randint(0, 255)
                self.b = random.randint(0, 255)
        else:
            if self.x_start > 0:
                self.x_start -= 1
            else:
                self.x_start += 1
                self.x_pos = True
                self.r = random.randint(0, 255)
                self.g = random.randint(0, 255)
                self.b = random.randint(0, 255)

        if self.y_pos:
            if self.y_start < self.y_bound:
                self.y_start += 1
            else:
                self.y_start -= 1
                self.y_pos = False
                self.r = random.randint(0, 255)
                self.g = random.randint(0, 255)
                self.b = random.randint(0, 255)
        else:
            if self.y_start > 0:
                self.y_start -= 1
            else:
                self.y_start += 1
                self.y_pos = True
                self.r = random.randint(0, 255)
                self.g = random.randint(0, 255)
                self.b = random.randint(0, 255)

        return self.offscreen_canvas
