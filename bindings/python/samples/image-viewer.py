#!/usr/bin/env python
import time
import sys

from samplebase import SampleBase
from rgbmatrix import graphics
from PIL import Image


class ImageWithText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(ImageWithText, self).__init__(*args, **kwargs)
        self.parser.add_argument("-i", "--image", help="The image file to display", required=True)
        self.parser.add_argument("-s", "--song", help="The song name to display", default="Song Name")
        self.parser.add_argument("-a", "--artist", help="The artist name to display", default="Artist Name")

    def run(self):
        # Load and prepare the image
        image = Image.open(self.args.image)
        
        # Resize image to fit in 26x26 lower left area
        # Pillow ≥10 compatibility
        resample_mode = getattr(Image, "Resampling", Image).LANCZOS
        image.thumbnail((26, 26), resample=resample_mode)
        
        # Load font for text
        font = graphics.Font()
        font.LoadFont("../../../fonts/7x13.bdf")
        
        # Define colors
        text_color = graphics.Color(255, 255, 255)  # White text
        
        # Create off-screen canvas for double buffering
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        
        try:
            print("Press CTRL-C to stop.")
            while True:
                # Clear the canvas
                offscreen_canvas.Clear()
                
                # Draw the image in the lower left (26x26)
                # Position: bottom of matrix - image height
                image_y = offscreen_canvas.height - image.height
                offscreen_canvas.SetImage(image.convert('RGB'), 0, image_y)
                
                # Draw song name spanning full width above image area
                song_y = image_y - 15  # Place above image with some spacing
                graphics.DrawText(offscreen_canvas, font, 0, song_y, text_color, self.args.song)
                
                # Draw artist name to the right of the image
                artist_x = 28  # Start just to the right of the 26px wide image
                artist_y = offscreen_canvas.height - 10  # Align roughly with image center
                graphics.DrawText(offscreen_canvas, font, artist_x, artist_y, text_color, self.args.artist)
                
                # Swap buffers
                offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)
                
                time.sleep(0.1)
                
        except KeyboardInterrupt:
            sys.exit(0)


# Main function
if __name__ == "__main__":
    image_viewer = ImageWithText()
    if (not image_viewer.process()):
        image_viewer.print_help()
