#!/usr/bin/env python
import time
from samplebase import SampleBase
from PIL import Image


class GifViewer(SampleBase):
    def __init__(self, *args, **kwargs):
        super(GifViewer, self).__init__(*args, **kwargs)
        self.parser.add_argument("-g", "--gif", help="The GIF file to display", required=True)

    def run(self):
        # Load the gif file
        gif = Image.open(self.args.gif)

        # Ensure the file is a GIF by checking frames
        try:
            num_frames = gif.n_frames
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
            print("Press CTRL-C to stop.")
            cur_frame = 0
            # Loop infinitely through the gif frames
            while cur_frame <= num_frames:
                self.matrix.SwapOnVSync(canvases[cur_frame], framerate_fraction=10)
                if cur_frame == num_frames - 1:
                    cur_frame = 0
                else:
                    cur_frame += 1
        except KeyboardInterrupt:
            sys.exit(0)


# Main function
if __name__ == "__main__":
    gif_viewer = GifViewer()
    if not gif_viewer.process():
        gif_viewer.print_help()
