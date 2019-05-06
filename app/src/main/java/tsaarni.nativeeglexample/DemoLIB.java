package tsaarni.nativeeglexample;

import android.view.Surface;

public class DemoLIB {
    static {
        System.loadLibrary("nativeegl");
    }

    public static native void init();
    public static native void resize(int width, int height);
    public static native void step();
}
