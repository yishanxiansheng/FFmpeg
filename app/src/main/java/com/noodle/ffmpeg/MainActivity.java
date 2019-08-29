package com.noodle.ffmpeg;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        MediaManager mediaManager = new MediaManager();
        mediaManager.cutAudio(Environment.getExternalStorageDirectory()+"/water.mp4",Environment.getExternalStorageDirectory()+"/water2.mp4");
    }
    public native String stringFromJNI();
}
