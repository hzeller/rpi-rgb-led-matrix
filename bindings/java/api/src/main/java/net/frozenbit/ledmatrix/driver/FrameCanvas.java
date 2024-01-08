package net.frozenbit.ledmatrix.driver;

public class FrameCanvas extends Canvas {
    public FrameCanvas(long nativePtr) {
        super(nativePtr);
    }

    public native boolean setPwmBits(int value);

    public native int getPwmBits();

    public native void setLuminanceCorrect(boolean on);

    public native boolean isLuminanceCorrect();

    public native void setBrightness(int brightness);

    public native int getBrightness();

    public native void copyFrom(FrameCanvas other);
}
