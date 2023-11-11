#!/usr/bin/env python
from models.words import get_positive_word

class PositiveWords:
    def get(self):
        word = get_positive_word()
        while len(word) <= 8:
            word = get_positive_word()
        return word
