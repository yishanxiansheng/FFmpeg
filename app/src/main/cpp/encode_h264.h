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
    ~EncodeH264() {};
public:
    //初始化编码器
    int initVedioCodec();

public:
    //将一帧数据存入到一个线程中进行处理
    int startSendOnFrame(uint8_t *buf);

public:
    //开始编码 void *obj 为无类型指针，可以指向任何类型
    static void *startEncodeH264(void *obj);

public:
    //结束编码
    void endEncodecH264();

public:
    //自定义滤镜，将数据拷入到AVFrame中
    void custom_filter(const EncodeH264 *encode_h264,uint8_t *buf,int y_size);

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
    //是否暂停录制
    int is_pause = 0;
    //是否结束录制
    int is_release = 0;
    //以帧数据为单位的存储数据的队列
    threadsafe_queue<uint8_t *> frame_queue;
    //记录视频的帧数
    int frame_count = 0;
};

#endif //FFMPEG_ENCODE_H264_H
