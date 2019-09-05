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
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.noodle.ffmpeg.camera.MediaObject;
import com.noodle.ffmpeg.camera.callback.OnPrepareCallback;
import com.noodle.ffmpeg.util.CacheFileUtil;

import java.io.File;
import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.Timer;

public class MainActivity extends AppCompatActivity implements View.OnClickListener,OnPrepareCallback{
    private static final String TAG = "MainActivity.class";
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private HolderCallBack mHolderCallBack;
    private Button mStartBtn;
    private MediaObject mMediaObject;
    public CameraStatus mCameraStatus = CameraStatus.START;

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
        mStartBtn = findViewById(R.id.start_record);
        mStartBtn.setOnClickListener(this);
        mSurfaceView = findViewById(R.id.mSurfaceView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        initCacheFile();
        initMediaRecord();
    }

    @Override
    public void onPause() {
        super.onPause();
        mHolderCallBack.releaseCamera();
        mCameraStatus = CameraStatus.FINISH;
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start_record:
                if (mCameraStatus == CameraStatus.READY){
                    mCameraStatus= CameraStatus.RECORDING;
                    startRecord();
                }
                break;
            default:
                break;
        }
    }

    /**
     * 开始录制
     */
    private void startRecord() {
        MediaObject.MediaPart part = mHolderCallBack.startRecord();
    }

    /**
     * 初始化音视频录制
     */
    private void initMediaRecord(){
        mSurfaceHolder = mSurfaceView.getHolder();
        // 为了兼容Honeycomb之前版本的设备。
        mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mHolderCallBack = new HolderCallBack();
        mSurfaceHolder.addCallback(mHolderCallBack);
        mHolderCallBack.setPrepareCallback(this);
        if (mCameraStatus == CameraStatus.START){
            mHolderCallBack.openCamera();
        }

        String key = String.valueOf(System.currentTimeMillis());
        mMediaObject = mHolderCallBack.setOutputDirectory(key,
                CacheFileUtil.getCacheFilePath() + key);
    }

    /**
     * 初始化存视频的文件夹
     */
    private void initCacheFile(){
        //多媒体文件
        File dcim = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM);
        CacheFileUtil.setCacheFilePath(dcim + "/yishanxiansheng/");
    }

    @Override
    public void onPrepared() {
        Log.d(TAG,"Camera prepared");
        mCameraStatus = CameraStatus.READY;
    }
}
