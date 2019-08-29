//
// Created by 一杉先生 on 2019-08-28.
//

#ifndef FFMPEG_LOG_H
#define FFMPEG_LOG_H

#endif //FFMPEG_LOG_H

#include <android/log.h>
#define TAG "MyFFmpeg"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
