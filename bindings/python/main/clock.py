#!/usr/bin/env python
import argparse
from common import CommonBase
from rgbmatrix import graphics
import time
import sys
from PIL import Image

class Clock(CommonBase):
    def __init__(self, *args, **kwargs):
        super(Clock, self).__init__(*args, **kwargs)
        self.parser.add_argument("--image-path", action="store", help="Image path to show", default="/home/matias/ledtrix/bindings/python/img/domestic/WALWATCH.GIF", type=str)

    def run(self):
        self.args = self.parser.parse_args()
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)

        mainModule.log("Trying to draw the image from " + self.args.image_path)
        gif = Image.open(self.args.image_path)

        try:
            num_frames = gif.n_frames
        except Exception:
            sys.exit("provided image is not a gif")

        # Preprocess the gifs frames into canvases to improve playback performance
        canvases = []
        mainModule.log("Preprocessing gif, this may take a moment depending on the size of the gif...")
        for frame_index in range(0, num_frames):
            gif.seek(frame_index)
            # must copy the frame out of the gif, since thumbnail() modifies the image in-place
            frame = gif.copy()
            #frame.thumbnail((matrix.width, matrix.height), Image.ANTIALIAS)
            frame.thumbnail((10, 10), Image.ANTIALIAS)
            canvasImg = canvas.CreateFrameCanvas()
            canvasImg.SetImage(frame.convert("RGB"), 2, 10)
            canvases.append(canvasImg)
        # Close the gif file to save memory now that we have copied out all of the frames
        gif.close()

        white = graphics.Color(255, 255, 255)

        counter = 0
        while(True):
            counter = counter + 1
            timestamp = time.strftime('%H:%M:%S')
            timestamp = timestamp.center(self.args.padding)
            mainModule.log("|" + timestamp + "|")
            graphics.DrawText(canvas, font, 5, 21, white, timestamp)
            time.sleep(1)
            canvas.Clear()
            if counter == 10:
                sys.exit(0)


# Main function
if __name__ == "__main__":
    mainModule = Clock()
    if (not mainModule.process()):
        mainModule.print_help()
