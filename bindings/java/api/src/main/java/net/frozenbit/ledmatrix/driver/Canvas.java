package net.frozenbit.ledmatrix.driver;

public class Canvas {
    @SuppressWarnings({"FieldCanBeLocal", "unused"}) // accessed in native code
    private final long nativePtr;

    public Canvas(long nativePtr) {
        this.nativePtr = nativePtr;
    }

    public native int getWidth();

    public native int getHeight();

    public native void setPixel(int x, int y, int red, int green, int blue);

    public native void clear();

    public native void fill(int red, int green, int blue);
}
