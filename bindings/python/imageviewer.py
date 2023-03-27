import logging
from PIL import Image

log = logging.getLogger(__name__)


class ImageViewer:
    def __init__(self, offscreen_canvas, path):
        self.framerate = 100
        self.offscreen_canvas = offscreen_canvas

        image = Image.open(path)
        image.thumbnail(
            (offscreen_canvas.width, offscreen_canvas.height), Image.ANTIALIAS
        )
        self.image = image.convert("RGB")

    def get_framerate(self):
        return self.framerate

    def show(self, matrix):
        self.offscreen_canvas.SetImage(self.image)
        self.offscreen_canvas = matrix.SwapOnVSync(self.offscreen_canvas)
