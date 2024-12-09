#!/usr/bin/env python
# Display a runtext with double-buffering.
from samplebase import SampleBase
from rgbmatrix import graphics
import time


class RunText(SampleBase):
    def __init__(self, *args, **kwargs):
        super(RunText, self).__init__(*args, **kwargs)
        self.parser.add_argument("-t", "--text", help="The text to scroll on the RGB LED panel", default="How do actors kiss and not fall in love? #1604446337 7:56:00 AM What would the average single guy do if a girl falls asleep on his shoulder? #25400896 9:16:58 AM Has any couple met on Quora and fallen in love? #84000951 6:57:43 PM What do we do when everything falls apart? #49447933 5:26:25 AM How do you keep homemade burgers from falling apart? #1224633471 12:30:00 PM My mom is dying. How do I keep from falling apart? #588767159 8:59:23 PM Is it possible to fall in love online? #179982 1:03:29 AM What is life like for people who don’t know how to program a computer? Do they feel something significant is missing from their lives? #6358193 7:48:46 PM Is there a secret (a how to) to listen to classical music? #9325777 2:02:48 AM How do you kiss a 12-year-old boy? I’m also 12 years old #378636078 7:35:06 AM How do you kiss? Literally, what do I do with my lips? #1179095387 7:24:13 PM How do I delete my own question from quora #255971590 1:38:13 AM If someone were to get bit by a mouse and within 2-3 hours the bottom of their jaw hurts, is that something to be concerned about or is it just a coincidence #1897646074 1:25:26 AM What should I do with my girlfriend if we’re alone? #1941349688 4:04:51 PM I have an art degree should I retrain and become a hairdresser #1950807980 5:07:49 PM Exactly what do women use Tinder for? #338905360 12:09:58 AM How can I stop thinking so I can fall asleep when I go to bed? #52477585 2:18:24 PM How do I get up if I fall? #483495128 7:17:42 PM I want to break up with my gf because she’s repulsive and I didn’t want to talk to her on valentine’s day but I don’t know what to do. What should I do? #1979531314 2:35:30 PM Are people secretly calling you a dog when they say you’re barking up the wrong tree? #1976112190 7:01:38 PM Let’s pretend I’m a 13-year-old boy who has a crush on a younger 13-year-old girl. She likes me back but also has a bf. Wtf do I do? #1979022235 8:26:07 AM Why does god love Americans more than people from other countries? #1973713887 10:38:49 AM How do you make your girlfriend pay for what she did? #1966774790 1:33:31 AM")

    def run(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        font = graphics.Font()
        font.LoadFont("../../../fonts/10x20.bdf")
        textColor = graphics.Color(255, 0, 0)  # Red color
        pos = offscreen_canvas.width
        my_text = self.args.text
        
        # Centering text vertically (assuming the font height is 13 pixels)
        font_height = 35
        canvas_height = offscreen_canvas.height
        vertical_position = (canvas_height // 2) + (font_height // 2)  # Centered vertically

        while True:
            offscreen_canvas.Clear()
            text_length = graphics.DrawText(offscreen_canvas, font, pos, vertical_position, textColor, my_text)
            pos -= 1
            if (pos + text_length < 0):
                pos = offscreen_canvas.width

            time.sleep(0.05)
            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)


# Main function
if __name__ == "__main__":
    run_text = RunText()
    if (not run_text.process()):
        run_text.print_help()

