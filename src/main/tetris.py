import time
import lib.tetris_led as tetris

class TetrisClock():


    def show(self):

        # parser = argparse.ArgumentParser(description="RGB LED matrix Example")
        # parser.add_argument("--horizontal", type=int, default = 1, help="horizontal count")
        # parser.add_argument("--vertical", type=int, default = 1, help="vertical count")
        # args = parser.parse_args()

        #revisionm, version = getrevision()
        # Configuration for the matrix
        # options = RGBMatrixOptions()
        # options.cols = 64
        # options.rows = 64
        # options.chain_length = args.horizontal * args.vertical
        # options.parallel = 1
        # options.brightness = 80

        # if(version == 4):
        #     options.gpio_slowdown =  4
        # elif(version == 3):
        #     options.gpio_slowdown =  1.0
        # else:
        #     options.gpio_slowdown =  1

        # options.show_refresh_rate = 1
        # options.hardware_mapping = 'regular'  # If you have an Adafruit HAT: 'adafruit-hat'
        # options.pwm_dither_bits = 0

        # matrix = RGBMatrix(options = options)
        double_buffer = self.matrix.CreateFrameCanvas()

        canvas_w = 64
        canvas_h = 32
        print('Canvas size W[%d] H[%d]'%(canvas_w, canvas_h))

        tetris.make_canvas(canvas_h, canvas_w , 0)

        # tetris.set_scale(1)
        # tetris_str = tetris.TetrisString(1, tetris.CHAR_HEIGHT * 2, "TETRIS")
        # tetris_str.animate(self.matrix, double_buffer)

        tetris.set_scale(1)
        tetris_str = tetris.TetrisString(1, tetris.CHAR_HEIGHT * 3, "CLOCK")
        tetris_str.animate(self.matrix, double_buffer)

        now = time.strftime('%H %M', time.localtime(time.time()))

        tetris.set_scale(2)
        tetris.set_bottom_shift(0)
        tetris_str2 = tetris.TetrisString(1, 0,  now)
        tetris_str2.animate(self.matrix, double_buffer)

        time.sleep(10)
