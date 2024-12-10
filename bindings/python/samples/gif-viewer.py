#!/usr/bin/env python
import time
from samplebase import SampleBase
from PIL import Image
import sys


class GifViewer(SampleBase):
    def __init__(self, *args, **kwargs):
        super(GifViewer, self).__init__(*args, **kwargs)
        self.parser.add_argument("-g", "--gif", help="The GIF file(s) to display (comma-separated)", required=True)

    def run(self):
        # Load the GIF file paths
        gif_paths = self.args.gif.split(",")

        # Preprocess all GIFs
        gif_frames = []
        print("Preprocessing gifs, this may take a moment depending on the size of the gifs...")
        for gif_path in gif_paths:
            try:
                gif = Image.open(gif_path)
                num_frames = gif.n_frames
                canvases = []
                for frame_index in range(num_frames):
                    gif.seek(frame_index)
                    frame = gif.copy()  # Copy the current frame
                    frame.thumbnail((self.matrix.width, self.matrix.height), Image.LANCZOS)
                    canvas = self.matrix.CreateFrameCanvas()
                    canvas.SetImage(frame.convert("RGB"))
                    canvases.append(canvas)
                gif_frames.append(canvases)
                gif.close()
            except Exception as e:
                sys.exit(f"Error processing gif {gif_path}: {e}")

        print("Completed preprocessing, displaying gifs")

        # Display the GIFs in a loop
        try:
            while True:  # Infinite loop to play all gifs repeatedly
                for canvases in gif_frames:
                    for canvas in canvases:
                        self.matrix.SwapOnVSync(canvas, framerate_fraction=10)
        except KeyboardInterrupt:
            sys.exit(0)


# Main function
if __name__ == "__main__":
    gif_viewer = GifViewer()
    if not gif_viewer.process():
        gif_viewer.print_help()
