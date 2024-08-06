'''
Have fun using this simple program.

Logic: Varga "vsumpi" Zsombor
Base-lib: https://github.com/hzeller/rpi-rgb-led-matrix
Other: 
    gpiozero: https://github.com/gpiozero
'''

import time # type: ignore
import datetime # type: ignore
from gpiozero import Button # type: ignore
from signal import pause # type: ignore
from samplebase import SampleBase # type: ignore
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics # type: ignore


# Matrix and Graphics Configuration (moved outside loop for clarity)
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'adafruit-hat'
options.drop_privileges = False
options.gpio_slowdown = 2
options.pwm_bits = 1
matrix = RGBMatrix(options=options)
font = graphics.Font()
font.LoadFont("~/rpi-rgb-led-matrix/fonts/6x10.bdf")
font1 = graphics.Font()
font1.LoadFont("~/rpi-rgb-led-matrix/fonts/8x13B.bdf")
font2 = graphics.Font()
font2.LoadFont("~/rpi-rgb-led-matrix/fonts/6x9.bdf")
red = graphics.Color(255, 0, 0)
green = graphics.Color(0, 255, 0)
blue = graphics.Color(0, 0, 255)
white = graphics.Color(255, 255, 255)
button = Button(19) #gpiozero BCM 19


# Utility Functions
def clear_screen():
    matrix.Clear()


def display_clock():
    clear_screen()
    graphics.DrawText(matrix, font, 3, 10, red, clockwork) #This is the clock render
    #This is the countdown
    if countdown > 5:
        graphics.DrawText(matrix, font, 15, 26, white, f": {countdown}")
    if countdown <= 5:
        graphics.DrawText(matrix, font, 15, 26, red, f": {countdown}")
    if countdown == 1:
        clear_screen()
        graphics.DrawText(matrix, font1, 12, 21, green, "START")
        time.sleep(1)
    time.sleep(1)

def on_press():
    #Buttton press logic
    print(clockwork)
    time_buffer.append(clockwork)
    #Make a file and sabe lap time
    with open("~/laptime.txt", "a") as f:
        f.write("\n".join(time_buffer) + "\n")
        time_buffer.clear()
    button.when_pressed = None #Used to stop the infinite loop of gpiozero


# Main Loop
countdown_length = 10 #Set the countdown length here
time_buffer = []

while True:
    clockwork = datetime.datetime.now().strftime("%H:%M:%S") #This is your clock
    countdown = abs((int(clockwork[-2:]) % countdown_length) - countdown_length) # This is the logic for the countdown
    if not button.when_pressed: #Button press listener
        button.when_pressed = on_press #DO NOT USE as a function() it will break the code, beacuse it's just referring to the function!
    display_clock()