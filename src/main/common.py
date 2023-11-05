class Common():
    def center_word(self, word: str):
        if self.args.centered :
            word = word.center(self.args.padding)
        return word
