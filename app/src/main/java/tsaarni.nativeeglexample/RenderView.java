package tsaarni.nativeeglexample;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.widget.Toast;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class RenderView extends GLSurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "DemoRender";
    private static final boolean DEBUG = true;
    private Renderer renderer;

    private float previousX;
    private float previousY;
    private float startX = 0;
    private float startY = 0;

    private Toast toast;

    public RenderView(Context context) {
        super(context);
        this.renderer = new Renderer();
        renderer.setScreenSizes(getWidth(), getHeight());

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
        float midStartX;
        float midStartY;
        float midEndX;
        float midEndY;

        switch (e.getAction()) {
            case MotionEvent.ACTION_MOVE:
                //FIXME: pause rotation while dragging

                midStartX = startX - getWidth() / 2.0f;
                midStartY = startY - getHeight() / 2.0f;
                midEndX = x - getWidth() / 2.0f;
                midEndY = y - getHeight() / 2.0f;

                renderer.setAngle((float) Math.toDegrees(
                        Math.atan2((midEndY - midStartY), (midEndX - midStartX))));
                break;

            case MotionEvent.ACTION_DOWN:
                startX = x;
                startY = y;
                break;

            case MotionEvent.ACTION_UP:
                if (startX == x && startY == y) {
                    toast = Toast.makeText(super.getContext(),
                            "This demo combines OpenGL ES 3.2 rendering and Java UI",
                            Toast.LENGTH_LONG);
                    toast.show();
                } else {
                    //TODO: speed up rotation when swiping faster
                    //from middle
                    midStartX = startX - getWidth() / 2.0f;
                    midStartY = startY - getHeight() / 2.0f;
                    midEndX = x - getWidth() / 2.0f;
                    midEndY = y - getHeight() / 2.0f;

                    renderer.setAngle((float) Math.toDegrees(Math.atan2((midEndY - midStartY), (midEndX - midStartX))));

                    /*System.out.println("\n\n---------------------------------"
                            + "\n              " + renderer.getAngle()
                            + "\n              X: " + x
                            + "\n              Y: " + y
                            + "\n---------------------------------\n\n");*/
                }
                break;

            default:
                break;
        }

        this.previousX = x;
        this.previousY = y;
        return true;
    }


    private static class Renderer implements GLSurfaceView.Renderer {
        private volatile float angle;
        private int width;
        private int height;

        public float getAngle() {
            return angle;
        }

        public void setAngle(float angle) {
            this.angle = angle % 360;
            DemoLIB.pushAngle(getAngle());
        }

        public void setScreenSizes(int width, int height) {
            this.width = width;
            this.height = height;
        }

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            DemoLIB.init(width, height);
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
