#pragma once
#include "pch.h"
#include "FFMpeg.h"


void LoadFrame(video_context* VideoContext) {
    auto& FormatContext = VideoContext->FormatContext;
    auto& CodecContext = VideoContext->CodecContext;
    auto& StreamIndex = VideoContext->VideoStreamIndex;
    auto& Frame = VideoContext->Frame;
    auto& Packet = VideoContext->Packet;

    while (av_read_frame(FormatContext, Packet) >= 0) {
        if (Packet->stream_index != StreamIndex) {
            av_packet_unref(Packet);
            continue;
        }

        int result = avcodec_send_packet(CodecContext, Packet);
        if (result < 0) throw("Packet sending failed.");

        result = avcodec_receive_frame(CodecContext, Frame);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            av_packet_unref(Packet);
            av_frame_unref(Frame);
            continue;
        }
        else if (result < 0) {
            throw("Failed to decode packet.");
        }

        av_packet_unref(Packet);
        break;
    }

    if (!Frame) {
        throw("No frame was found.");
    }

    VideoContext->PTS = Frame->pts;

    return;
}

void InitializeVideo(const char* Filename, video_context* VideoContext) {
    // Opening file
    auto& FormatContext = VideoContext->FormatContext;

    FormatContext = avformat_alloc_context();
    if (!FormatContext) {
        throw("Format context not allocated.");
    }
    else if (avformat_open_input(&FormatContext, Filename, NULL, NULL)) {
        throw("File could not be opened.");
    }

    // Finding video stream (not audio stream or others)
    AVStream* VideoStream = 0;
    for (unsigned int i = 0; i < FormatContext->nb_streams; i++) {
        AVStream* Stream = FormatContext->streams[i];
        if (Stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            VideoStream = Stream;
            break;
        }
    }

    if (VideoStream == 0) throw("VideoStream not found.");

    VideoContext->VideoStreamIndex = VideoStream->index;
    VideoContext->TimeBase = (double)VideoStream->time_base.num / (double)VideoStream->time_base.den;

    // Allocating resources
        // Codec context & params
    AVCodecParameters* CodecParams = VideoStream->codecpar;
    const AVCodec* Codec = avcodec_find_decoder(CodecParams->codec_id);
    if (!Codec) throw("Decoder not found.");

    VideoContext->CodecContext = avcodec_alloc_context3(Codec);
    if (!VideoContext->CodecContext) throw("Codec context could not be allocated.");

    if (avcodec_parameters_to_context(VideoContext->CodecContext, CodecParams) < 0) throw("Codec params could not be loaded to context.");
    if (avcodec_open2(VideoContext->CodecContext, Codec, NULL) < 0) throw("Codec could not be opened.");

    // Packet & Frame
    VideoContext->Packet = av_packet_alloc();
    if (!VideoContext->Packet) throw("Packet could not be allocated.");

    VideoContext->Frame = av_frame_alloc();
    if (!VideoContext->Frame) throw("Frame could not be allocated.");

    return;
}

void CloseVideo(video_context* VideoContext) {
    avformat_close_input(&VideoContext->FormatContext);
    avformat_free_context(VideoContext->FormatContext);
    avcodec_free_context(&VideoContext->CodecContext);
    sws_freeContext(VideoContext->ScalerContext);
    av_frame_free(&VideoContext->Frame);
    av_packet_free(&VideoContext->Packet);

    return;
}

void WriteFrame(video_context* VideoContext) {
    auto& Frame = VideoContext->Frame;
    auto& CodecContext = VideoContext->CodecContext;
    auto& ScalerContext = VideoContext->ScalerContext;

    static int Width = 0;
    static int Height = 0;
    if (VideoContext->Width != Width || VideoContext->Height != Height) {
        if (Width != 0 && Height != 0) {
            sws_freeContext(VideoContext->ScalerContext);
        }
        ScalerContext = sws_getContext(Frame->width, Frame->height, CodecContext->pix_fmt, VideoContext->Width, VideoContext->Height, AV_PIX_FMT_BGR0, SWS_BILINEAR, NULL, NULL, NULL);
        if (!ScalerContext) {
            CloseVideo(VideoContext);
            VideoContext->Ended = true;
            return;
        }

        Width = VideoContext->Width;
        Height = VideoContext->Height;
    }

    uint8_t* Dest[4] = {(uint8_t*)VideoContext->VideoOut, 0, 0, 0};
    int DestLinesize[4] = { VideoContext->Width * 4, 0, 0, 0};
    int ScaleResult = sws_scale(ScalerContext, Frame->data, Frame->linesize, 0, Frame->height, Dest, DestLinesize);

    av_frame_unref(Frame);
}