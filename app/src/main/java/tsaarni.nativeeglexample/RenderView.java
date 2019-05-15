package tsaarni.nativeeglexample;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class RenderView extends GLSurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "DemoRender";
    private static final boolean DEBUG = true;
    private Renderer renderer;

    private final float TOUCH_SCALE_FACTOR = 180.0f / 320;
    private float previousX;
    private float previousY;



    public RenderView(Context context) {
        super(context);
        this.renderer = new Renderer();
        previousX = 0;
        previousY = 0;

        setEGLConfigChooser(8, 8, 8, 8, 8, 0);
        setEGLContextClientVersion(3);
        setRenderer(renderer);
    }


    @Override
    public boolean onTouchEvent(MotionEvent e) {
        // MotionEvent reports input details from the touch screen
        // and other input controls. In this case, you are only
        // interested in events where the touch position changed.

        float x = e.getRawX();
        float y = e.getRawY();

        switch (e.getAction()) {
            case MotionEvent.ACTION_MOVE:

                float dx = x - previousX;
                float dy = y - previousY;

                // reverse direction of rotation above the mid-line
                if (y > getHeight() / 2.0) {
                    dx = dx * -1 ;
                }

                // reverse direction of rotation to left of the mid-line
                if (x < getWidth() / 2.0) {
                    dy = dy * -1 ;
                }

                renderer.setAngle(
                        renderer.getAngle() +
                                ((dx + dy) * TOUCH_SCALE_FACTOR));
                System.out.println("---------------------------------");
                System.out.println(renderer.getAngle());
                System.out.println("X direction: " + dx);
                System.out.println("Y direction: " + dy);
                System.out.println("---------------------------------");
                //FIXME: need to figure out proper manipulation of rotation
                //DemoLIB.pushAngle(renderer.getAngle());
                break;
        }

        this.previousX = x;
        this.previousY = y;
        return true;
    }


    private static class Renderer implements GLSurfaceView.Renderer {

        public volatile float mAngle;

        public float getAngle() {
            return mAngle;
        }

        public void setAngle(float angle) {
            mAngle = angle;
        }


        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            DemoLIB.init();
            DemoLIB.pushAngle(getAngle());
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            DemoLIB.resize(width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            DemoLIB.render();
        }
    }
}
