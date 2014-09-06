// Small example how to use the libray

#include "led-matrix.h"

#include <unistd.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

int main(int argc, char *argv[]) {
    /*
     * Set up GPIO pins. This fails when not running as root
     */
    GPIO io;
    if (!io.Init())
        return 1;

    /*
     * Set up the RGBMatrix. It implements a 'Canvas' interface.
     */
    int rows = 32;   // A 32x32 display. Use 16 when this is a 16x32 display.
    int chain = 1;   // Number of boards chained together.
    Canvas *canvas = new RGBMatrix(&io, rows, chain);

    /*
     * Now lets create a simple animation. We use the canvas to draw
     * pixels. We wait between each step.
     */
    for (int x = 0; x < canvas->width(); ++x) {
        for (int y = 0; y < canvas->height(); ++y) {
            // Wander three dots around.
            canvas->SetPixel(x, y, 255, 0, 0);
            canvas->SetPixel(x, canvas->height() - y, 0, 255, 0);
            canvas->SetPixel(y, x, 0, 255, 0);
            usleep(15 * 1000);  // Wait for 15ms
        }
    }
    
    canvas->ClearScreen();

    // Animation finished. Shut down the RGB matrix.
    delete canvas;

    return 0;
}
