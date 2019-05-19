package tsaarni.nativeeglexample;

import android.app.Activity;
import android.os.Bundle;

public class DemoActivity extends Activity {

    RenderView mView;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new RenderView(getApplication());
        setContentView(mView);
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
