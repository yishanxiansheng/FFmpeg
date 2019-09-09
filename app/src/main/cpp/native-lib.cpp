
#include "Arguments.h"
#include "bade_include.h"
#include "encode_h264.h"

#define VEDIO_FORMAT ".264"

EncodeH264 *encode_h264;

extern "C"
JNIEXPORT jint JNICALL
Java_com_noodle_ffmpeg_camera_FFmpegBridge_prepareFFmpegEncoder(JNIEnv *env, jclass type,
                                                                  jstring mediaBasePath_,
                                                                  jstring mediaName_, jint width,
                                                                  jint height, jint frameRate,
                                                                  jlong bitRate) {
    const char *mediaBasePath = env->GetStringUTFChars(mediaBasePath_, 0);
    const char *mediaName = env->GetStringUTFChars(mediaName_, 0);
    Arguments *arguments = static_cast<Arguments *>(malloc(sizeof(Arguments)));
    arguments->media_basepath = mediaBasePath;
    arguments->media_name = mediaName;
    arguments->bit_rate = bitRate;
    arguments->frame_rate = frameRate;
    arguments->height = height;
    arguments->width = width;

    size_t vedio_path_length = strlen(mediaBasePath) + strlen(mediaName) +
                               strlen(VEDIO_FORMAT) + 2;//"/"的长度为2 size_t 为unsigned int
    arguments->vedio_path = (char *) (malloc(vedio_path_length));

    strcpy(arguments->vedio_path,mediaBasePath);
    strcat(arguments->vedio_path,"/");
    strcat(arguments->vedio_path,mediaName);
    strcat(arguments->vedio_path,VEDIO_FORMAT);

    // TODO
    EncodeH264  encode_H264(arguments) ;
    encode_H264.initVedioCodec();

    env->ReleaseStringUTFChars(mediaBasePath_, mediaBasePath);
    env->ReleaseStringUTFChars(mediaName_, mediaName);
    return 1;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_noodle_ffmpeg_camera_FFmpegBridge_encodeFrame2AAC(JNIEnv *env, jclass type,
                                                           jbyteArray data_) {
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    int i = encode_h264->startSendOnFrame((uint8_t *) data);
    env->ReleaseByteArrayElements(data_, data, 0);
    return i;
}