#include "..\..\GameLibrary\GamePlatform.h"

#ifndef FFMPEG
#define FFMPEG

#pragma once
    extern "C" {
        #include "libavcodec/avcodec.h"
        #include "libavformat/avformat.h"
        #include "libswscale/swscale.h"
    }

    struct video_context {
        int Width;
        int Height;
        int VideoStreamIndex;
        AVFormatContext* FormatContext;
        AVCodecContext* CodecContext;
        AVPacket* Packet;
        AVFrame* Frame;
        SwsContext* ScalerContext;
        void* VideoOut;
        bool Ended;
        double TimeBase;
        int64_t PTS;
    };

    void InitializeVideo(const char* Filename, video_context* VideoContext);
    void LoadFrame(video_context* VideoContext);
    void WriteFrame(video_context* VideoContext);
    void CloseVideo(video_context* VideoContext);
#endif

