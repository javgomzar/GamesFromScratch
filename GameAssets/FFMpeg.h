#ifndef FFMPEG
#define FFMPEG

#pragma once

    extern "C" {
        #include "libavcodec/avcodec.h"
        #include "libavformat/avformat.h"
        #include "libswscale/swscale.h"
        #include "libavformat/avio.h"
    }

    struct video_buffer {
        unsigned char* Pointer;
        unsigned char* Start;
        int Size;
        int FullSize;
    };

    struct video_context {
        int Width;
        int Height;
        int VideoStreamIndex;
        AVFormatContext* FormatContext;
        AVIOContext* IOContext;
        AVCodecContext* CodecContext;
        AVPacket* Packet;
        AVFrame* Frame;
        SwsContext* ScalerContext;
        video_buffer Buffer;
        void* VideoOut;
        bool Ended;
        double TimeBase;
        int64_t PTS;
    };

    void InitializeVideoFromFile(const char* Filename, video_context* VideoContext);
    void LoadFrame(video_context* VideoContext);
    void WriteFrame(video_context* VideoContext);
    void CloseVideo(video_context* VideoContext);
#endif

