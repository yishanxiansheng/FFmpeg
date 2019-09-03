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
    private Camera mCamera;

    /**
     * 后置摄像头
     */
    protected int mCameraId = Camera.CameraInfo.CAMERA_FACING_BACK;

    /**
     * 相机参数
     */
    protected Camera.Parameters mParameters = null;
    /**
     * 最大帧率
     */
    protected static int MAX_FRAME_RATE = 30;
    /**
     * 支持的预览尺寸
     */
    protected List<Size> mSupportedPreviewSizes;
    /**
     * 帧率
     */
    protected int mFrameRate = MAX_FRAME_RATE;
    /**
     * 是否全屏
     */
    public static  boolean NEED_FULL_SCREEN = true;
    /**
     * 小视频高度
     */
    public static int SMALL_VIDEO_HEIGHT = 1080;
    /**
     * 小视频宽度
     */
    public static int SMALL_VIDEO_WIDTH = 2160;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        hideStatusAndNavigation();
        initView();


        //MediaManager mediaManager = new MediaManager();
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
        mSurfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                //连接camera与holder
                if (mCamera != null) {
                    try {
                        mCamera.setPreviewDisplay(holder);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                if (mCamera == null) {
                    return;
                }
                //设置相机的属性：对焦、闪光灯等
                mParameters = mCamera.getParameters();
                //获取相机支持的尺寸
                mSupportedPreviewSizes = mParameters.getSupportedPreviewSizes();
                prepareCameraParaments();
                mCamera.setParameters(mParameters);
                try {
                    mCamera.startPreview();
                } catch (Exception e) {
                    /**
                     * 启动失败时，我们通过异常控制机制释放了相机资源。任何时候，打开相机并完成任务后，必须记得及时释放它，
                     * 即使是在发生异常时。
                     */
                    e.printStackTrace();
                    mCamera.release();
                    mCamera = null;
                }
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                if (mCamera != null) {
                    mCamera.stopPreview();
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mCameraId == Camera.CameraInfo.CAMERA_FACING_BACK){
            mCamera = Camera.open();
        }
        else{
            mCamera = Camera.open(mCameraId);
        }
        mCamera.setDisplayOrientation(90);
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mCamera != null) {
            mCamera.release();
            mCamera = null;
        }
    }

    /**
     * 设置预览的属性
     */
    private void prepareCameraParaments() {
        if (mParameters == null) {
            return;

        }
        //预览的帧率
        List<Integer> rates = mParameters.getSupportedPreviewFrameRates();
        if (rates != null) {
            if (rates.contains(MAX_FRAME_RATE)) {
                mFrameRate = MAX_FRAME_RATE;
            } else {
                boolean findFrame = false;
                Collections.sort(rates);
                for (int i = rates.size() - 1; i >= 0; i--) {
                    if (rates.get(i) <= MAX_FRAME_RATE) {
                        mFrameRate = rates.get(i);
                        findFrame = true;
                        break;
                    }
                }
                if (!findFrame) {
                    mFrameRate = rates.get(0);
                }
            }
        }
        mParameters.setPreviewFrameRate(mFrameRate);

        //设置浏览尺寸
        mParameters.setPreviewSize(SMALL_VIDEO_WIDTH, SMALL_VIDEO_HEIGHT);

        // 设置输出视频流尺寸，采样率
        mParameters.setPreviewFormat(ImageFormat.YV12);

        //设置自动连续对焦
        String mode = getAutoFocusMode();
        if (StringUtils.isNotEmpty(mode)) {
            mParameters.setFocusMode(mode);
        }

        //设置人像模式，用来拍摄人物相片，如证件照。数码相机会把光圈调到最大，做出浅景深的效果。而有些相机还会使用能够表现更强肤色效果的色调、对比度或柔化效果进行拍摄，以突出人像主体。
        //		if (mCameraId == Camera.CameraInfo.CAMERA_FACING_FRONT && isSupported(mParameters.getSupportedSceneModes(), Camera.Parameters.SCENE_MODE_PORTRAIT))
        //			mParameters.setSceneMode(Camera.Parameters.SCENE_MODE_PORTRAIT);

        if (isSupported(mParameters.getSupportedWhiteBalance(), "auto")) {
            mParameters.setWhiteBalance("auto");
        }

        //是否支持视频防抖
        if ("true".equals(mParameters.get("video-stabilization-supported"))) {
            mParameters.set("video-stabilization", "true");

        }
        if (!DeviceUtils.isDevice("GT-N7100", "GT-I9308", "GT-I9300")) {
            mParameters.set("cam_mode", 1);
        }
    }

    /**
     * 连续自动对焦
     */
    private String getAutoFocusMode() {
        if (mParameters != null) {
            //持续对焦是指当场景发生变化时，相机会主动去调节焦距来达到被拍摄的物体始终是清晰的状态。
            List<String> focusModes = mParameters.getSupportedFocusModes();
            if ((Build.MODEL.startsWith("GT-I950") || Build.MODEL.endsWith("SCH-I959") || Build.MODEL.endsWith("MEIZU MX3")) && isSupported(focusModes, "continuous-picture")) {
                return "continuous-picture";
            } else if (isSupported(focusModes, "continuous-video")) {
                return "continuous-video";
            } else if (isSupported(focusModes, "auto")) {
                return "auto";
            }
        }
        return null;
    }

    /**
     * 检测是否支持指定特性
     */
    private boolean isSupported(List<String> list, String key) {
        return list != null && list.contains(key);
    }

    public native String stringFromJNI();
}
