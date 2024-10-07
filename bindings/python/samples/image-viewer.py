#!/usr/bin/env python
import time
from samplebase import SampleBase
from PIL import Image


class ImageViewer(SampleBase):
    def __init__(self, *args, **kwargs):
        super(ImageViewer, self).__init__(*args, **kwargs)
        self.parser.add_argument("-i", "--image", help="The image to display", required=True)

    def run(self):
        # Load image
        image = Image.open(self.args.image).convert('RGB')
        image.thumbnail((self.matrix.width, self.matrix.height), Image.ANTIALIAS)

        # Set the image on the matrix
        self.matrix.SetImage(image)

        try:
            print("Press CTRL-C to stop.")
            while True:
                time.sleep(100)
        except KeyboardInterrupt:
            sys.exit(0)


# Main function
if __name__ == "__main__":
    image_viewer = ImageViewer()
    if not image_viewer.process():
        image_viewer.print_help()

