#include <jni.h>
#include <string>
#include "mylog.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_noodle_ffmpeg_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
} extern "C"
JNIEXPORT void JNICALL
Java_com_noodle_ffmpeg_FfmpegManager_cutAudio(JNIEnv *env, jobject instance, jstring srcFile_,
                                             jstring desFile_) {
    const char *in_filename = env->GetStringUTFChars(srcFile_, 0);
    const char *out_filename = env->GetStringUTFChars(desFile_, 0);
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", in_filename);
        return;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        return;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        LOGE("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            LOGE("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
            return;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("Could not open output file '%s'", out_filename);
            return;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE("Error occurred when opening output file\n");
        return;
    }

    ret = av_seek_frame(ifmt_ctx, -1, 2 * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        LOGE("Error seek\n");
        return;
    }

    int64_t *dts_start_from = static_cast<int64_t *>(malloc(
            sizeof(int64_t) * ifmt_ctx->nb_streams));
    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    int64_t *pts_start_from = static_cast<int64_t *>(malloc(
            sizeof(int64_t) * ifmt_ctx->nb_streams));
    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);

    while (1) {
        AVStream *in_stream, *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];


        if (av_q2d(in_stream->time_base) * pkt.pts > 9) {
            break;
        }

        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
            printf("dts_start_from: %s\n", av_ts2str(dts_start_from[pkt.stream_index]));
        }
        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
            printf("pts_start_from: %s\n", av_ts2str(pts_start_from[pkt.stream_index]));
        }

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base,
                                   out_stream->time_base, AV_ROUND_NEAR_INF);
        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base,
                                   out_stream->time_base, AV_ROUND_NEAR_INF);
        if (pkt.pts < 0) {
            pkt.pts = 0;
        }
        if (pkt.dts > 0) {
            pkt.dts = 0;
        }
        pkt.duration = (int) av_rescale_q(pkt.duration, in_stream->time_base,
                                          out_stream->time_base);
        pkt.pos = -1;
        printf("\n");

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
    free(dts_start_from);
    free(pts_start_from);

    av_write_trailer(ofmt_ctx);
    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    if (ret < 0 && ret != AVERROR_EOF) {
        LOGE("Error occurred: %s\n", av_err2str(ret));
        return;
    }

    env->ReleaseStringUTFChars(srcFile_, in_filename);
    env->ReleaseStringUTFChars(desFile_, out_filename);
}