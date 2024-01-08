package net.frozenbit.ledmatrix.driver;

import java.io.IOException;

public class RGBMatrix extends Canvas implements AutoCloseable {

    static {
        try {
            NativeUtils.loadLibraryFromJar("/resources/librgbmatrix.so.1");
            NativeUtils.loadLibraryFromJar("/resources/libapi.so");
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @SuppressWarnings({"FieldCanBeLocal", "unused"}) // accessed in native code
    private final long nativePtr;

    public RGBMatrix(long nativePtr) {
        super(nativePtr);
        this.nativePtr = nativePtr;
    }

    public native void delete();

    @Override
    public void close() {
        delete();
    }

    public native FrameCanvas createFrameCanvas();

    public native void swapOnVSync(FrameCanvas other, int frameRateFraction);

    public void swapOnVSync(FrameCanvas other) {
        swapOnVSync(other, 1);
    }

    public static native RGBMatrix createFromOptions(Options options, RuntimeOptions runtimeOptions);

    public static class Options {
        public String hardwareMapping;
        public Integer rows;
        public Integer cols;
        public Integer chainLength;
        public Integer parallel;
        public Integer pwmBits;
        public Integer pwmLsbNanoseconds;
        public Integer pwmDitherBits;
        public Integer brightness;
        public Integer scanMode;
        public Integer rowAddressType;
        public Integer multiplexing;
        public Boolean disableHardwarePulsing;
        public Boolean showRefreshRate;
        public Boolean inverseColors;
        public String ledRgbSequence;
        public String pixelMapperConfig;
        public String panelType;
        public Integer limitRefreshRateHz;
    }

    public static class RuntimeOptions {
        public Integer gpioSlowdown;
        public Integer daemon;
        public Integer dropPrivileges;
        public Boolean doGpioInit;
        public String dropPrivUser;
        public String dropPrivGroup;
    }
}
