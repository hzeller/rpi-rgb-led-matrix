import net.frozenbit.ledmatrix.driver.FrameCanvas;
import net.frozenbit.ledmatrix.driver.RGBMatrix;

/**
 * Demonstrates the usage of offscreen buffers to prepare the next
 * frame for an animation.
 * <p>
 * Shows a 1x1 pixel ball bouncing around the matrix.
 */
public class BallBounce {
    public static void main(String[] args) {
        RGBMatrix.Options options = new RGBMatrix.Options();
        options.cols = 64;
        options.rows = 64;
        options.limitRefreshRateHz = 60;

        RGBMatrix.RuntimeOptions runtimeOptions = new RGBMatrix.RuntimeOptions();
        /*
        runtimeOptions.dropPrivileges = 1;
        runtimeOptions.dropPrivUser = "someuser";
        runtimeOptions.dropPrivGroup = "somegroup";
        */

        // Initialize the LED matrix. Using try-with-resources ensures that
        // it is destroyed when no longer needed.
        try (RGBMatrix rgbMatrix = RGBMatrix.createFromOptions(options, runtimeOptions)) {
            FrameCanvas frameCanvas1 = rgbMatrix.createFrameCanvas();
            FrameCanvas frameCanvas2 = rgbMatrix.createFrameCanvas();

            float ballX = rgbMatrix.getWidth() / 2f,
                    ballY = rgbMatrix.getHeight() / 2f,
                    velX = (float) Math.random(),
                    velY = (float) Math.random();

            // normalize velocity
            float velSpeed = (float) Math.sqrt(velX * velX + velY * velY);
            velX /= velSpeed;
            velY /= velSpeed;

            int frameCounter = 0;
            while (true) {
                FrameCanvas canvasToUse = frameCounter % 2 == 0 ? frameCanvas1 : frameCanvas2;
                canvasToUse.clear();
                canvasToUse.setPixel(
                        (int) ballX,
                        (int) ballY,
                        255,
                        0,
                        255
                );
                rgbMatrix.swapOnVSync(canvasToUse);

                boolean bounceX = false, bounceY = false;
                if (ballX + velX < 0 || ballX + velX >= rgbMatrix.getWidth()) {
                    bounceX = true;
                }
                if (ballY + velY < 0 || ballY + velY >= rgbMatrix.getHeight()) {
                    bounceY = true;
                }
                if (bounceX) {
                    velX = -velX;
                }
                if (bounceY) {
                    velY = -velY;
                }
                ballX += velX;
                ballY += velY;

                frameCounter++;
            }
        }
    }
}

