import time  # type: ignore
import datetime  # type: ignore
from signal import pause  # type: ignore
from samplebase import SampleBase  # type: ignore
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics  # type: ignore
import subprocess  # For system audio playback

# Matrix and Graphics Configuration
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 1
options.parallel = 1
options.hardware_mapping = 'adafruit-hat'
options.drop_privileges = False
options.gpio_slowdown = 3 # set 3 to decrease glitches (works well with pi4b)
options.pwm_bits = 1
matrix = RGBMatrix(options=options)

# Fonts
font = graphics.Font()
font.LoadFont("~/rpi-rgb-led-matrix/fonts/6x10.bdf")
font1 = graphics.Font()
font1.LoadFont("~/rpi-rgb-led-matrix/fonts/8x13B.bdf")
font2 = graphics.Font()
font2.LoadFont("~/rpi-rgb-led-matrix/fonts/6x9.bdf")

# Colors
red = graphics.Color(255, 0, 0)
green = graphics.Color(0, 255, 0)
blue = graphics.Color(0, 0, 255)
white = graphics.Color(255, 255, 255)

# Audio Configuration
red_alert_sound = "~/rpi-rgb-led-matrix/bindings/python/samples/REPLACE_WITH_YOUR_AUDIO.wav"  # Replace with your WAV file path

# Utility Functions
def clear_screen():
    matrix.Clear()

def play_audio(file_path):
    # Play the .wav file using system command
    subprocess.Popen(["aplay", file_path])

def display_clock():
    global sound_played
    clear_screen()
    graphics.DrawText(matrix, font, 3, 10, red, clockwork)  # Render clock

    # Countdown logic
    if countdown > 5:
        sound_played = False  # Reset the flag when not in the red zone
        graphics.DrawText(matrix, font, 15, 26, white, f": {countdown}")
    elif countdown <= 5:
        graphics.DrawText(matrix, font, 15, 26, red, f": {countdown}")
        if countdown == 4 and not sound_played:
            play_audio(red_alert_sound)  # Play audio when countdown is 4
            sound_played = True
    if countdown == 1:
        graphics.DrawText(matrix, font, 15, 26, red, f": {countdown}")
        time.sleep(1)
        clear_screen()
        graphics.DrawText(matrix, font1, 12, 21, green, "START")
        time.sleep(1)

# Main Loop
countdown_length = 10  # Set the countdown length
time_buffer = []
sound_played = False  # Audio flag for 4-second countdown

while True:
    start_time = datetime.datetime.now()  # Record start of loop
    clockwork = start_time.strftime("%H:%M:%S")  # Current time

    # Countdown calculation
    countdown = abs((int(start_time.second) % countdown_length) - countdown_length)

    # Display the clock and countdown
    display_clock()

    # Calculate time spent in this loop iteration
    elapsed_time = (datetime.datetime.now() - start_time).total_seconds()

    # Adjust sleep to ensure each loop runs precisely for 1 second
    time.sleep(max(0, 1 - elapsed_time))
