package tsaarni.nativeeglexample;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class RenderView extends GLSurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "DemoRender";
    private static final boolean DEBUG = true;

    public RenderView(Context context) {
        super(context);

        setEGLConfigChooser(8, 8, 8, 8, 8, 0);
        setEGLContextClientVersion(3);
        setRenderer(new Renderer());
    }

    private static class Renderer implements GLSurfaceView.Renderer {

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            DemoLIB.init();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            DemoLIB.resize(width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            DemoLIB.step();
        }
    }
}
