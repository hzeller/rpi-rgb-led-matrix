import os
import logging
import time
import threading
import schedule
from rgbmatrix import RGBMatrix, RGBMatrixOptions
from welcome import Welcome
from dvd import DVD
from clock import Clock
from stocks import Stocks
from weather import Weather
from imageviewer import ImageViewer
from slack import SlackStatus
from secrets import SLACK_USER_ID, SLACK_TOKEN, LAT, LON

path = os.path.dirname(__file__) + "/"

logging.basicConfig(
    level=logging.INFO,
    filename=path + "log.txt",
    format="[%(asctime)s] %(levelname)-8s (%(name)s) %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)  # stream=sys.stdout)
log = logging.getLogger()


def create_matrix():
    options = RGBMatrixOptions()

    options.hardware_mapping = "adafruit-hat-pwm"
    options.rows = 32
    options.cols = 64
    options.chain_length = 1
    options.parallel = 1
    options.row_address_type = 0
    options.multiplexing = 0
    options.pwm_bits = 11
    options.brightness = 100
    options.pwm_lsb_nanoseconds = 130
    options.led_rgb_sequence = "RGB"
    options.pixel_mapper_config = ""
    options.panel_type = ""
    options.show_refresh_rate = 0
    options.gpio_slowdown = 4
    options.disable_hardware_pulsing = False
    options.drop_privileges = True

    return RGBMatrix(options=options)


if __name__ == "__main__":
    log.info("Server started.")

    matrix = create_matrix()

    loading_app = Welcome()
    t1 = threading.Thread(
        target=loading_app.show,
        args=(matrix,),
    )
    t1.start()
    main_app = SlackStatus(matrix.CreateFrameCanvas(), SLACK_USER_ID, SLACK_TOKEN)
    apps = list()
    apps.append(DVD(matrix.CreateFrameCanvas()))
    apps.append(Clock(matrix.CreateFrameCanvas()))
    apps.append(Stocks(matrix.CreateFrameCanvas(), "NVDA"))
    apps.append(Stocks(matrix.CreateFrameCanvas(), "VTI"))
    apps.append(Weather(matrix.CreateFrameCanvas(), LAT, LON))
    apps.append(ImageViewer(matrix.CreateFrameCanvas(), path + "images/nvidia.png"))
    t1.join()

    id = 0
    runtime = 0
    duration = 15
    while True:
        schedule.run_pending()
        if main_app.check_status():
            main_app.show(matrix)
            time.sleep(1 / main_app.get_framerate())
        else:
            framerate = apps[id].get_framerate()
            for sec in range(0, framerate):
                apps[id].show(matrix)
                time.sleep(1 / framerate)

            runtime += 1
            if runtime >= duration:
                runtime = 0
                id = 0 if id >= len(apps) - 1 else id + 1
