//
// Created by 一杉先生 on 2019-09-05.
//

#include "encode_h264.h"

#define TAG "initVedioCodec"

EncodeH264::EncodeH264(Arguments *args) {
    arguments = args;
}

int EncodeH264::initVedioCodec() {
    int ret;
    size_t vedio_path_length = strlen(arguments->vedio_path);
    char *out_file = static_cast<char *>(malloc(vedio_path_length + 1));
    strcpy(out_file, arguments->vedio_path);

    //初始化输出码流的AVFormatContext
    avformat_alloc_output_context2(&av_format_context, NULL, NULL, out_file);
    av_output_format = av_format_context->oformat;

    //打开输出文件
    ret = avio_open(&av_format_context->pb, out_file, AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        LOGE(TAG, "fail to open %s", out_file);
        return -1;
    }
    //创建输出码流的AVStream
    av_stream = avformat_new_stream(av_format_context, 0);
    if (av_stream == NULL) {
        LOGE(TAG, "av_stream = null");
        return -1;
    }
    //查找编码器
    av_codec = avcodec_find_decoder(av_stream->codecpar->codec_id);
    //创建AVCodecContext并设置参数
    av_codec_context = avcodec_alloc_context3(av_codec);
    av_codec_context->codec_id = AV_CODEC_ID_H264;
    av_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    av_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;//像素格式
    av_codec_context->width = arguments->width;
    av_codec_context->height = arguments->height;
    av_codec_context->bit_rate = arguments->bit_rate;
    av_codec_context->gop_size = 50;
    av_codec_context->thread_count = 12;//线程数
    av_codec_context->time_base.den = arguments->frame_rate;//帧率
    av_codec_context->time_base.num = 1;//设置时间基
    av_codec_context->max_b_frames = 3;//最大b帧数，越大压缩率越高

    //存储ffmpeg中一些上下文的选项
    AVDictionary *av_dictionary = 0;
    if (av_codec_context->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&av_dictionary, "tune", "zerolatency", 0);
        av_opt_set(av_codec_context->priv_data, "preset", "ultrafast", 0);
        av_dict_set(&av_dictionary, "profile", "baseline", 0);

    }
    //将参数复制到avcodec_parameters
    ret = avcodec_parameters_from_context(av_stream->codecpar, av_codec_context);
    if (ret < 0) {
        LOGE(TAG, "faile to copy parameters to av_codec_context");
        return -1;
    }
    //打开编码器
    ret = avcodec_open2(av_codec_context, av_codec, &av_dictionary);
    if (ret < 0) {
        LOGE(TAG, "fail to open av_codec");
        return -1;
    }

    av_frame = av_frame_alloc();
    //1、通过指定像素格式、图像宽、图像高来计算所需的内存大小
    picture_size = av_image_get_buffer_size(av_codec_context->pix_fmt, av_codec_context->width,
                                            av_codec_context->height, 1);
    //2、计算的内存大小申请所需内存
    uint8_t *buf = (uint8_t *) av_malloc(picture_size);
    //3、对申请的内存进行格式化
    ret = av_image_fill_arrays(av_frame->data, av_frame->linesize, buf, av_codec_context->pix_fmt,
                               av_codec_context->width, av_codec_context->height, 1);
    if (ret < 0) {
        LOGE(TAG, "fail to allocate image source");
        return -1;
    }

    //写文件头
    avformat_write_header(av_format_context, NULL);
    av_new_packet(av_packet, picture_size);
    return 0;

}

void EncodeH264::startEncodeH264() {

}

void EncodeH264::endEncodecH264() {

}
