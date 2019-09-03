package com.noodle.ffmpeg;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Build;
import android.view.SurfaceHolder;

import java.io.IOException;
import java.util.Collections;
import java.util.List;

/**
 * @author heshufan
 * @date 2019-09-03
 */
public class HolderCallBack implements SurfaceHolder.Callback {
    /**
     * 小视频高度
     */
    private int maxVideoHeight = 1080;
    /**
     * 小视频宽度
     */
    private int smallVideoWidth = 2160;

    /**
     * 最大帧率
     */
    private static int MAX_FRAME_RATE = 30;

    /**
     * 帧率
     */
    private int mFrameRate = MAX_FRAME_RATE;

    /**
     * 后置摄像头
     */
    private int mCameraId = Camera.CameraInfo.CAMERA_FACING_BACK;

    private Camera mCamera;

    /**
     * 相机参数
     */
    protected Camera.Parameters mParameters = null;

    public HolderCallBack() {
    }

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
        smallVideoWidth = width;
        maxVideoHeight = height;
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
        mParameters.setPreviewSize(smallVideoWidth, maxVideoHeight);

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

    public void openCamera() {
        if (mCameraId == Camera.CameraInfo.CAMERA_FACING_BACK) {
            mCamera = Camera.open();
        } else {
            mCamera = Camera.open(mCameraId);
        }
        mCamera.setDisplayOrientation(90);
    }

    public void releaseCamera(){
        if (mCamera != null) {
            mCamera.release();
            mCamera = null;
        }
    }
}
