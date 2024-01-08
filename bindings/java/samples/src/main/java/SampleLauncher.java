import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.List;

public class SampleLauncher {
    private static final List<Class<?>> AVAILABLE_SAMPLES = Arrays.asList(
            PerformanceTest.class,
            DrawString.class,
            BallBounce.class
    );

    public static void main(String[] args) {
        if (args.length < 1) {
            showHelp();
            System.exit(1);
            return;
        }

        int sampleIndex;
        try {
            sampleIndex = Integer.parseInt(args[0]);
            if (sampleIndex < 0 || sampleIndex >= AVAILABLE_SAMPLES.size()) {
                showHelp();
                System.exit(1);
                return;
            }
        } catch (NumberFormatException e) {
            showHelp();
            System.exit(1);
            return;
        }

        Class<?> sample = AVAILABLE_SAMPLES.get(sampleIndex);

        System.out.println("Running " + sample.getSimpleName());
        try {

            sample.getDeclaredMethod("main", String[].class)
                    .invoke(null, (Object) args);
        } catch (NoSuchMethodException | IllegalAccessException |
                 InvocationTargetException e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }

    private static void showHelp() {
        System.err.println("usage: ./sample <number>");
        for (int i = 0; i < AVAILABLE_SAMPLES.size(); i++) {
            Class<?> allowedClass = AVAILABLE_SAMPLES.get(i);
            System.err.println(" " + i + " - " + allowedClass.getSimpleName());
        }
    }
}
