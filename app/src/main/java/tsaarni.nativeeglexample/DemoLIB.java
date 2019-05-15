package tsaarni.nativeeglexample;

public class DemoLIB {
    static {
        System.loadLibrary("nativeegl");
    }

    public static native void init();
    public static native void resize(int width, int height);
    public static native void render();
    public static native void pushAngle(float angle);
}
