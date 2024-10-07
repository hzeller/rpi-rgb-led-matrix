#!/usr/bin/env python
import time
from samplebase import SampleBase
from PIL import Image


class GifViewer(SampleBase):
    def _init_(self, *args, **kwargs):
        super(GifViewer, self)._init_(*args, **kwargs)
        self.parser.add_argument("-g", "--gifs", help="The comma-separated list of GIF files to display", required=True)

    def run(self):
        # Load the gif file
        gif_paths = self.args.gif.split(",")

        # Ensure the file is a GIF by checking frames
        for gif_path in gif_paths:
            gif = Image.open(gif_path)
            try:
                num_frames = gif.n_frames
                print(num_frames)
            except Exception:
                sys.exit("Provided image is not a gif")

            # Preprocess the gif's frames into canvases to improve playback performance
            canvases = []
            print("Preprocessing gif, this may take a moment depending on the size of the gif...")
            for frame_index in range(0, num_frames):
                gif.seek(frame_index)
                frame = gif.copy()  # Copy the current frame
                frame.thumbnail((self.matrix.width, self.matrix.height), Image.LANCZOS)
                canvas = self.matrix.CreateFrameCanvas()
                canvas.SetImage(frame.convert("RGB"))
                canvases.append(canvas)

            # Close the gif file to save memory
            gif.close()
            print("Completed preprocessing, displaying gif")

            try:
                # Loop infinitely through the gif frames
                for i in range(num_frames):
                    self.matrix.SwapOnVSync(canvases[i], framerate_fraction=10)

            except KeyboardInterrupt:
                sys.exit(0)


# Main function
if __name__ == "_main_":
    gif_viewer = GifViewer()
    if not gif_viewer.process():
        gif_viewer.print_help()
