#!/usr/bin/env python
import random
import asyncio
import time
import sys
import os

from common import Common
from positiveword import PositiveWords
from positivephrase import PositivePhrases
from positivephrase import PositivePhrases
from commands import Command
from marquesine import Marquesine
from weather import Weather
from base import Base
from images import Images

class Index(Base):
    def __init__(self, *args, **kwargs):
        super(Index, self).__init__(*args, **kwargs)

    async def run(self):
        self.args = self.parser.parse_args()

        try:
            mainModule.log("Press CTRL-C to stop.")
            moods_count = 6
            randomList=[]
            while(True):

                action = random.randint(1,moods_count)
                action = 6

                mainModule.log("Selected by random: " + str(action))

                if action in randomList:
                    mainModule.log("Already exist: " + str(action))
                    if len(randomList) == moods_count:
                        randomList=[]
                else:
                    randomList.append(action)
                    mainModule.log("Clear selection: " + str(action))
                    mainModule.log(str(action))

                    if action == 1: #Positive Word
                        word_selected = PositiveWords.get(self)
                        word_selected = Common.center_word(self, word_selected)
                        Common.show_text(self, word_selected)
                    if action == 2: #Positive Phrase
                        phrase_selected = PositivePhrases.get(self)
                        Marquesine.show(self, phrase_selected)
                    elif action == 3: #Show Clock
                        word_selected = time.strftime('%H:%M')
                        word_selected = Common.center_word(self, word_selected)
                        Common.show_text(self, word_selected)
                    elif action == 4: #Weather
                        await Weather.show_async(self)
                    elif action == 5: #Command
                        Command.show(self)
                    elif action == 6: #Command
                        Images.show_random(self)
                    time.sleep(6)   # show display for 10 seconds before exit

        except IOError as e:
            print(e.strerror)
        except KeyboardInterrupt:
            sys.exit(0)

# Main function
if __name__ == "__main__":
    if os.name == 'nt':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    mainModule = Index()
    if (not asyncio.run(mainModule.process())):
        mainModule.print_help()

