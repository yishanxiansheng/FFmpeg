//
// Created by 一杉先生 on 2019-09-05.
//

#ifndef FFMPEG_ENCODE_H264_H
#define FFMPEG_ENCODE_H264_H


#include "Arguments.h"
#include "bade_include.h"
//c为c++的子集，这里定义类似C++的类,将采集的YUV数据编码为H264
class EncodeH264 {
    //构造函数
public:
    //构造函数
    EncodeH264(Arguments *args);
public:
    //析构函数
    ~EncodeH264(){};
public:
    //初始化编码器
    int initVedioCodec();
public:
    //开始编码
    void startEncodeH264();
public:
    //结束编码
    void endEncodecH264();
private:
    Arguments *arguments;
    //全局上下文
    AVFormatContext *av_format_context;
    //输出格式结构体
    AVOutputFormat *av_output_format;
    //音视频解码后的数据（原始数据）
    AVFrame *av_frame;
    //编码后视频帧数据（压缩）
    AVPacket *av_packet;
    //音视频数据流
    AVStream *av_stream;
    //编解码器上下文
    AVCodecContext *av_codec_context;
    //编解码器
    AVCodec *av_codec;
    //编码器参数
    AVCodecParameters *av_codec_param;
    //图像像素大小
    int picture_size;
};

#endif //FFMPEG_ENCODE_H264_H
