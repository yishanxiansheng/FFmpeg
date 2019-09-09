//
// Created by 一杉先生 on 2019-09-05.
//

#include <pthread.h>
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
        LOGE(TAG, "failed to open %s", out_file);
        return -1;
    }
    //创建输出码流的AVStream
    av_stream = avformat_new_stream(av_format_context, 0);
    if (av_stream == NULL) {
        LOGE(TAG, "av_stream = null");
        return -1;
    }
    //查找编码器
    av_codec = avcodec_find_decoder(av_format_context->oformat->video_codec);

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
        LOGE(TAG, "failed to copy parameters to av_codec_context");
        return -1;
    }
    //打开编码器
    ret = avcodec_open2(av_codec_context, av_codec, &av_dictionary);
    if (ret < 0) {
        LOGE(TAG, "failed to open av_codec");
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
        LOGE(TAG, "failed to allocate image source");
        return -1;
    }

    //写文件头
    avformat_write_header(av_format_context, NULL);
    av_new_packet(av_packet, picture_size);

    //创建一个新的线程
    pthread_t p_thread;
    //开启线程 参数依次是：创建的线程id，线程参数，调用的函数，传入的函数参数
    pthread_create(&p_thread, NULL, EncodeH264::startEncodeH264, this);
    return 0;
}

int EncodeH264::startSendOnFrame(uint8_t *buf) {
    int in_y_size = arguments->height * arguments->width;
    uint8_t *new_buf = static_cast<uint8_t *>(malloc(in_y_size * 3 / 2));
    memcpy(new_buf, buf, in_y_size * 3 / 2);
    //将数据存入队列
    frame_queue.push(new_buf);
    return 0;
}


void *EncodeH264::startEncodeH264(void *obj) {

    EncodeH264 *encode_h264 = static_cast<EncodeH264 *>(obj);
    while (!encode_h264->is_pause || !encode_h264->frame_queue.empty()) {
        if (encode_h264->frame_queue.empty()) {
            continue;
        }
        //队列取出数据
        uint8_t *picture_buf = reinterpret_cast<uint8_t *>(encode_h264->frame_queue.wait_and_pop().get());
        int y_size = encode_h264->arguments->width * encode_h264->arguments->height;
        //给AVFrame赋值
        encode_h264->custom_filter(encode_h264, picture_buf, y_size);
        //todo?
        encode_h264->av_frame->pts = encode_h264->frame_count;
        encode_h264->frame_count++;
        int got_size = 0;

        //进行编码，将AVFrame转化为AVPacket
        int ret = avcodec_send_frame(encode_h264->av_codec_context, encode_h264->av_frame);
        if (ret < 0) {
            LOGE(TAG, "failed to send frame to avcodeccontexr");
            return nullptr;
        }
        ret = avcodec_receive_packet(encode_h264->av_codec_context, encode_h264->av_packet);
        if (ret < 0) {
            LOGE(TAG, "failed to receive packet");
            return nullptr;
        }
        encode_h264->frame_cnt++;
        encode_h264->av_packet->stream_index = encode_h264->av_stream->index;
        //av_write_frame()：将编码后的视频码流写入文件。
        ret = av_write_frame(encode_h264->av_format_context, encode_h264->av_packet);
        //编码完成释放AVPacket
        av_packet_unref(encode_h264->av_packet);
        if (ret < 0) {
            LOGE(TAG, "failed to write frame");
            return nullptr;
        }
        delete (picture_buf);
        if (encode_h264->is_pause) {
            encode_h264->endEncodecH264();
            delete encode_h264;
        }
    }
    return nullptr;
}

int EncodeH264::endEncodecH264() {
    //Flush Encoder
    int ret_1 = flush_encoder(av_format_context, 0);
    if (ret_1 < 0) {
        LOGE(TAG, "Flushing encoder failed\n");
        return -1;
    }

    //写入尾部
    av_write_trailer(av_format_context);

    //Clean
    if (av_stream) {
        av_free(av_frame);
    }
    avio_close(av_format_context->pb);
    avformat_free_context(av_format_context);
    LOGI(TAG, "视频编码结束");
    //arguments->handler->setup_video_state(END_STATE);
    //arguments->handler->try_encode_over(arguments);
    return 1;

}

int EncodeH264::flush_encoder(AVFormatContext *av_format_context, int stream_index) {
    int ret;
    AVPacket enc_pkt;
    if (!(av_codec->capabilities &
          AV_CODEC_CAP_DELAY))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        avcodec_send_frame(av_codec_context, av_frame);
        ret = avcodec_receive_packet(av_codec_context, &enc_pkt);
        av_frame_free(NULL);
        if (ret < 0) {
            LOGE(TAG, "failed to receive packet");
            break;
        }
        LOGI(TAG, "_Flush Encoder: Succeed to encode 1 frame video!\tsize:%5d\n", enc_pkt.size);
        ret = av_write_frame(av_format_context, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

void EncodeH264::custom_filter(const EncodeH264 *encode_h264, uint8_t *buf, int y_size) {

    int y_height_start_index = 0;
    int uv_height_start_index = 0;
    //给Y赋值
    for (int i = y_height_start_index; i < encode_h264->arguments->height; i++) {
        for (int j = 0; j < encode_h264->arguments->width; j++) {
            int index = encode_h264->arguments->width * i + j;
            uint8_t value = *(buf + index);
            //将Y值赋值给AVFrame
            *(encode_h264->av_frame->data[0] +
              (i - y_height_start_index) * encode_h264->arguments->width + j) = value;
        }
    }
    //给UV赋值
    for (int i = uv_height_start_index; i < encode_h264->arguments->height / 2; i++) {
        for (int j = 0; j < encode_h264->arguments->width / 2; j++) {

            int index = encode_h264->arguments->width / 2 * i + j;
            uint8_t v = *(buf + y_size + index);
            uint8_t u = *(buf + y_size * 5 / 4 + index);
            *(encode_h264->av_frame->data[2] +
              ((i - uv_height_start_index) * encode_h264->arguments->width / 2 + j)) = v;
            *(encode_h264->av_frame->data[1] +
              ((i - uv_height_start_index) * encode_h264->arguments->width / 2 + j)) = u;
        }
    }
}
