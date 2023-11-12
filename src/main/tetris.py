import time
import lib.tetris_led as tetris

class TetrisClock():

    def show(self):

        double_buffer = self.matrix.CreateFrameCanvas()

        print('Canvas size W[%d] H[%d]'%(self.matrix.width, self.matrix.height))

        tetris.make_canvas(self.matrix.height, self.matrix.width , 0)

        now = time.strftime('%H %M', time.localtime(time.time()))

        tetris.set_scale(2)
        tetris.set_bottom_shift(0)
        tetris_str2 = tetris.TetrisString(1, -4,  now)
        tetris_str2.animate(self.matrix, double_buffer)

        time.sleep(10)
