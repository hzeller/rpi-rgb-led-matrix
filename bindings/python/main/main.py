#!/usr/bin/env python
import random
import sys
from word import get_positive_word
from common import CommonBase
from rgbmatrix import graphics
import time


class PlainText(CommonBase):
    def __init__(self, *args, **kwargs):
        super(PlainText, self).__init__(*args, **kwargs)

    def prepare_word(self, word):
        if self.args.centered :
            word = word.center(self.args.padding)
        return word

    def get_random(self):
        return random.randint(1,2)

    def show_text(self, word):
        canvas = self.matrix
        font = graphics.Font()
        font.LoadFont("../../../fonts/" + self.args.font)
        white = graphics.Color(255, 255, 255)
        mainModule.log("Fade:" + str(self.args.fade))

        x = 0
        y = 0

        #word = mainModule.getTextToShow();

        mainModule.log("word:|" + word + "|")

        if self.args.fade == 'top':
            y = -21
            while(y <= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, white, word)
                time.sleep(0.200)
                y = y + 2
        elif self.args.fade == 'bottom':
            y = 41
            while(y >= 21):
                canvas.Clear()
                graphics.DrawText(canvas, font, 0, y, white, word)
                time.sleep(0.200)
                y = y - 2
        elif self.args.fade == 'left':
            x = -60
            while(x <= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, white, word)
                time.sleep(0.200)
                x = x + 2
        elif self.args.fade == 'right':
            x = 60
            while(x >= 0):
                canvas.Clear()
                graphics.DrawText(canvas, font, x, 21, white, word)
                time.sleep(0.200)
                x = x - 2
        else:
            graphics.DrawText(canvas, font, 0, 21, white, word)

    def run(self):
        self.args = self.parser.parse_args()

        try:
            mainModule.log("Press CTRL-C to stop.")


            while(True):

                action = 0
                action = self.get_random()
                mainModule.log(str(action))

                if action == 1: #Positive Word
                    word_selected = get_positive_word()
                    word_selected = mainModule.prepare_word(word_selected)
                    mainModule.show_text(word_selected);
                elif action == 2: #Show Clock
                    word_selected = time.strftime('%H:%M')
                    mainModule.log("antes:" + word_selected);
                    word_selected = mainModule.prepare_word(word_selected)
                    mainModule.log("despues:" + word_selected);
                    mainModule.show_text(word_selected);

                time.sleep(6)   # show display for 10 seconds before exit

        except IOError as e:
            print(e.strerror)
        except KeyboardInterrupt:
            sys.exit(0)

# Main function
if __name__ == "__main__":
    mainModule = PlainText()
    if (not mainModule.process()):
        mainModule.print_help()
