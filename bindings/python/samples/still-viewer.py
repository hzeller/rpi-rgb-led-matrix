#!/usr/bin/env python
from rgbmatrix import RGBMatrix, RGBMatrixOptions
from PIL import Image
import urllib2 as urllib
import io
from samplebase import SampleBase

class StillViewer(SampleBase):
    def __init__(self, *args, **kwargs):
        super(StillViewer, self).__init__(*args, **kwargs)
        self.parser.add_argument("-i", "--image", help="Image Location or URL", default="../media/catnaps.jpg")
    
    def run(self):
        while True:
            sysArg = self.args.image
            if sysArg.find("http") >= 0 :
            	fd = urllib.urlopen(sysArg)
            	image_file = io.BytesIO(fd.read())
            	im = Image.open(image_file).convert('RGB')
            else:
            	im = Image.open(sysArg).convert('RGB')
            if im.size[0] > im.size[1]:
            	size = (self.matrix.width + self.matrix.width),self.matrix.width
                im.thumbnail(size)
            else:
        	    size = self.matrix.height,(self.matrix.height + self.matrix.height)
        	    im.thumbnail(size)
            width, height = im.size
            left = (width - self.matrix.width)/2
            top = (height - self.matrix.height)/2
            right = (width + self.matrix.width)/2
            bottom = (height + self.matrix.height)/2
            image = im.crop((left, top, right, bottom))

            image.thumbnail((self.matrix.width, self.matrix.height), Image.ANTIALIAS)
            self.matrix.SetImage(image)

# Main function
# e.g. call with
#  sudo ./still-viewer.py --chain=4
# if you have a chain of four
if __name__ == "__main__":
    still_viewer = StillViewer()
    if (not still_viewer.process()):
        still_viewer.print_help()

# Created By: Mr. CatNaps
# Date: 10/15/2019
# Description:
# StillViewer will automatically take a local or web image and display it on a LED Matrix. 
# StillViewer will also determine if the image is landscape or portrait and automatically resize and crop it.
# It has the latest option flags and supports U-mapping. 
# Set the '-i=' flag to a local path or a URL and it is ready to go.