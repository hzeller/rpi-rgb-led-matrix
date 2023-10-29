# import argparse
# import sys
# import random
# from word import get_positive_word
# import plaintext

# #from common import CommonBase

# class Ledtrix():
#     # def __init__(self, *args, **kwargs):
#     #     self.parser = argparse.ArgumentParser()
#         #super(Ledtrix, self).__init__(*args, **kwargs)

#     # def run(self):
#     #     self.args = self.parser.parse_args()
#     def run(self):
#         try:
#             print("Press CTRL-C to stop.")
#             action = self.get_random();
#             print(str(action));

#             action = 1

#             if action == 1:
#                 word_selected = get_positive_word()
#                 print(word_selected);
#                 plaintext

#             sys.exit(0)

#         except IOError as e:
#             print(e.strerror)
#         except KeyboardInterrupt:
#             sys.exit(0)

#     def get_random(self):
#         return random.randint(0,9)

#     def print_help(self):
#         print("Help")

# # Main function
# if __name__ == "__main__":
#     mainModule = Ledtrix()
#     if (not Ledtrix.run(mainModule)):
#         mainModule.print_help()

