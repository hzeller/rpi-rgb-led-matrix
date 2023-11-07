#!/usr/bin/env python
from models.phrases import get_positive_phrase

class PositivePhrases:
    def get(self):
        return get_positive_phrase()
