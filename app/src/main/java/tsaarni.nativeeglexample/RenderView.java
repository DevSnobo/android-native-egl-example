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

    private final float TOUCH_SCALE_FACTOR = 180.0f / 320;
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

        switch (e.getAction()) {
            case MotionEvent.ACTION_MOVE:
                //FIXME: redo this section and do it correctly




                //------------------------------------------------------------------
                //TODO: modulo screen width/height or whatever
                /*float dx = x - previousX;
                float dy = y - previousY;

                // reverse direction of rotation above the mid-line
                if (y > getHeight() / 2.0) {
                    dx = dx * -1;
                }

                // reverse direction of rotation to left of the mid-line
                if (x < getWidth() / 2.0) {
                    dy = dy * -1;
                }

                renderer.setAngle(
                        //renderer.getAngle() +
                        ((dx + dy) * TOUCH_SCALE_FACTOR));*/
                /*System.out.println("\n\n---------------------------------"
                        + "\n              " + renderer.getAngle()
                        + "\n              X: " + x
                        + "\n              Y: " + y
                        + "\n---------------------------------\n\n");*/
                //FIXME: need to figure out proper manipulation of rotation
                //DemoLIB.pushAngle(renderer.getAngle());
                break;
            //------------------------------------------------------------------

            case MotionEvent.ACTION_DOWN:
                startX = x;
                startY = y;
                break;

            case MotionEvent.ACTION_UP:
                if (startX == x && startY == y) {
                    toast = Toast.makeText(super.getContext(),
                            //"This demo combines Java UI and native EGL + OpenGL renderer",
                            "This demo combines OpenGL ES 3.2 rendering and Java UI",
                            Toast.LENGTH_LONG);
                    toast.show();
                } else {
                    //from middle
                    float midStartX = startX - getWidth() / 2.0f;
                    float midStartY = startY - getHeight() / 2.0f;
                    float midEndX = x - getWidth() / 2.0f;
                    float midEndY = y - getHeight() / 2.0f;

                    renderer.setAngle((float) Math.toDegrees(Math.atan2((midEndY - midStartY), (midEndX - midStartX))));

                    System.out.println("\n\n---------------------------------"
                            + "\n              " + renderer.getAngle()
                            + "\n              X: " + x
                            + "\n              Y: " + y
                            + "\n---------------------------------\n\n");
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
        public volatile float angle;
        private int width;
        private int height;

        public float getAngle() {
            return angle;
        }

        public void setAngle(float angle) {
            this.angle = angle % 360;
            //FIXME:
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
