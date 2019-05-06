package tsaarni.nativeeglexample;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

public class DemoActivity extends Activity {

    RenderView mView;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new RenderView(getApplication());
        setContentView(mView);

        mView.getHolder().addCallback(mView);
        mView.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view) {
                Toast toast = Toast.makeText(DemoActivity.this,
                        //"This demo combines Java UI and native EGL + OpenGL renderer",
                        "This demo combines gles3jni sample and Java UI",
                        Toast.LENGTH_LONG);
                toast.show();
            }});
    }

    @Override
    protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }
}
