#include "GamePlatform.h"
#include "GameBitmap.h"

#ifndef GAME_VIDEO
#define GAME_VIDEO

// FFMPEG
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavformat/avio.h"
}

enum game_video_id {
    //Video_Test_ID,

    game_video_id_count
};

struct video_buffer {
    uint8* Start;
    int ReadSize;
    int FullSize;
};

struct video_context {
    int TargetWidth;
    int TargetHeight;
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

struct game_video {
    game_video_id ID;
    int Width;
    int Height;
    game_bitmap Texture;
    video_context VideoContext;
    int Handle;
    bool Loop;
    double TimeElapsed;
};

#endif