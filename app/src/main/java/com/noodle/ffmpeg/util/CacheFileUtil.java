package com.noodle.ffmpeg.util;

import android.os.Environment;

import java.io.File;

/**
 * @author heshufan
 * @date 2019-09-04
 */
public class CacheFileUtil {

    /**
     * 视频缓存的文件夹
     */
    private static String mCacheFilePath;

    public static String getCacheFilePath() {
        return mCacheFilePath;
    }

    public static void setCacheFilePath(String cacheFilePath) {
        File file = new File(cacheFilePath);
        if (!file.exists()){
            file.mkdir();
        }
        mCacheFilePath = cacheFilePath;
    }
}
