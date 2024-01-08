import net.frozenbit.ledmatrix.driver.RGBMatrix;

import java.awt.*;
import java.awt.image.BufferedImage;

/**
 * Demonstrates how to use java.awt to render a string onto the LED matrix.
 * Of course, you may use any framework you like to create images.
 * <p>
 * To make it more interesting, the text is colorfully animated.
 */
public class DrawString {
    public static void main(String[] args) {
        RGBMatrix.Options options = new RGBMatrix.Options();
        options.cols = 64;
        options.rows = 64;

        // Initialize the LED matrix. Using try-with-resources ensures that
        // it is destroyed when no longer needed.
        try (RGBMatrix rgbMatrix = RGBMatrix.createFromOptions(options, null)) {
            BufferedImage image = new BufferedImage(64, 64, BufferedImage.TYPE_INT_RGB);
            Graphics g = image.getGraphics();
            Graphics2D graphics = (Graphics2D) g;
            Font arial = new Font("Arial", Font.BOLD, 10);
            int frameCounter = 0;
            while (true) {
                graphics.setColor(Color.BLACK);
                graphics.fillRect(0, 0, image.getWidth(), image.getHeight());
                graphics.setFont(arial);
                graphics.setColor(Color.getHSBColor((frameCounter / 120f), 1f, 1f));
                graphics.drawString("Hello", 0, 10);
                graphics.drawString("from", 0, 20);
                graphics.drawString("Java!", 0, 30);
                for (int x = 0; x < image.getWidth(); x++) {
                    for (int y = 0; y < image.getHeight(); y++) {
                        int rgb = image.getRGB(x, y);
                        int red = (rgb & 0x00ff0000) >> 16;
                        int green = (rgb & 0x0000ff00) >> 8;
                        int blue = rgb & 0x000000ff;
                        rgbMatrix.setPixel(x, y, red, green, blue);
                    }
                }
                frameCounter++;
                Thread.sleep(13);
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}

