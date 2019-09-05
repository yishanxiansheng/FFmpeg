//
// Created by 一杉先生 on 2019-09-04.
//

#ifndef FFMPEG_ARGUMENTS_H
#define FFMPEG_ARGUMENTS_H

//参数结构体
struct Arguments {
    //文件存储地址
    const char *media_basepath;
    //视频名
    const char *media_name;
    //输出视频的全路径名
    char *vedio_path;
    //视频宽度
    int width;
    //视频高度
    int height;
    //帧率
    int frame_rate;
    //比特率
    long bit_rate;
};


#endif //FFMPEG_ARGUMENTS_H
