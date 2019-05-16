package tsaarni.nativeeglexample;

import android.app.Activity;
import android.os.Bundle;
import android.util.DisplayMetrics;

public class DemoActivity extends Activity {

    RenderView mView;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new RenderView(getApplication());
        setContentView(mView);

        DisplayMetrics display = getResources().getDisplayMetrics();

        //TODO: hand screen width, height to renderer
        int width = display.widthPixels;
        int height = display.heightPixels;

        System.out.println("\n\n--------------------------"
                + "\nScreen sizes: "
                + "\nWidth: " + width
                + "\nHeight: " + height
                + "\n--------------------------\n\n");
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
