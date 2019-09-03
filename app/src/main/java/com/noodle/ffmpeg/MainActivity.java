package com.noodle.ffmpeg;

import android.graphics.Color;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.os.Build;
import android.os.Environment;
import android.support.constraint.ConstraintLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.TextView;

import java.io.IOException;
import java.util.Collections;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private HolderCallBack mHolderCallBack;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        hideStatusAndNavigation();
        initView();


        //FfmpegManager mediaManager = new FfmpegManager();
        //mediaManager.cutAudio(Environment.getExternalStorageDirectory()+"/water.mp4",Environment.getExternalStorageDirectory()+"/water2.mp4");
    }

    /**
     * 隐藏导航栏和状态栏
     */
    private void hideStatusAndNavigation() {
        if (Build.VERSION.SDK_INT >= 21) {
            View decorView = getWindow().getDecorView();
            int option = View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
            decorView.setSystemUiVisibility(option);
            getWindow().setNavigationBarColor(Color.TRANSPARENT);
            getWindow().setStatusBarColor(Color.TRANSPARENT);
        }
    }

    private void initView() {
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        mSurfaceView = findViewById(R.id.mSurfaceView);
        mSurfaceHolder = mSurfaceView.getHolder();
        // 为了兼容Honeycomb之前版本的设备。
        mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mHolderCallBack = new HolderCallBack();
        mSurfaceHolder.addCallback(mHolderCallBack);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mHolderCallBack.openCamera();
    }

    @Override
    public void onPause() {
        super.onPause();
        mHolderCallBack.releaseCamera();
    }

    public native String stringFromJNI();
}
