import net.frozenbit.ledmatrix.driver.RGBMatrix;

/**
 * Can be used for a rough estimate of the overhead of the Java API when
 * compared to a C++ program.
 */
public class PerformanceTest {
    public static void main(String[] args) {
        RGBMatrix.Options options = new RGBMatrix.Options();
        options.cols = 64;
        options.rows = 64;
        try (RGBMatrix rgbMatrix = RGBMatrix.createFromOptions(options, null)) {
            long startTime = System.currentTimeMillis();
            int callCount = 1_000_000;
            for (int i = 0; i < callCount; i++) {
                rgbMatrix.setPixel(0, 0, 255, 255, 255);
            }
            long endTime = System.currentTimeMillis();
            float elapsedSeconds = (endTime - startTime) / 1000f;
            int callRate = Math.round(callCount / elapsedSeconds);
            System.out.println("setPixel: " + callRate + " calls/second, " + (callRate / (rgbMatrix.getWidth() * rgbMatrix.getHeight())) + " FPS");
        }
    }
}
