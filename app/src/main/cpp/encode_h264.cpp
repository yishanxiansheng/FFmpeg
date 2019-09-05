//
// Created by 一杉先生 on 2019-09-05.
//

#include "encode_h264.h"

EncodeH264::EncodeH264(Arguments *args) {
    arguments = args;
}

int EncodeH264::initVedioCodec() {
    int ret;
    size_t vedio_path_length = strlen(arguments->vedio_path);
    char *out_file = static_cast<char *>(malloc(vedio_path_length + 1));
    strcpy(out_file, arguments->vedio_path);
    //创建av_format_context
    avformat_alloc_output_context2(&av_format_context, NULL, NULL, out_file);
    av_output_format = av_format_context->oformat;
    ret = avio_open(&av_format_context->pb, out_file, AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        LOGE("fail to open %s",out_file);
        return -1;
    }
    av_stream = avformat_new_stream(av_format_context,0);
    if (av_stream == NULL){
        LOGE("av_stream = null");
        return -1;
    }

    av_codec_param = av_stream->codecpar;
    av_codec_param->codec_id = AV_CODEC_ID_H264;
    av_codec_param->codec_type = AVMEDIA_TYPE_VIDEO;
    return 0;

}

void EncodeH264::startEncode() {

}

void EncodeH264::endEncodec() {

}
