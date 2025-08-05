#include "GameVideo.h"

game_video LoadVideo(memory_arena* Arena, game_video_id ID, read_file_result File) {
    game_video Result = {};
    Result.ID = ID;
    void* Dest = PushSize(Arena, Asset->MemoryNeeded);
    memcpy(Dest, Asset->File.Content, Asset->File.ContentSize);
    return Result;
}

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

void CloseVideo(video_context* VideoContext) {
    avformat_close_input(&VideoContext->FormatContext);
    avformat_free_context(VideoContext->FormatContext);
    avcodec_free_context(&VideoContext->CodecContext);
    sws_freeContext(VideoContext->ScalerContext);
    av_frame_free(&VideoContext->Frame);
    av_packet_free(&VideoContext->Packet);

    return;
}

void WriteFrame(video_context* VideoContext, int TargetWidth, int TargetHeight) {
    auto& Frame = VideoContext->Frame;
    auto& CodecContext = VideoContext->CodecContext;
    auto& ScalerContext = VideoContext->ScalerContext;

    if (TargetWidth != VideoContext->TargetWidth || TargetHeight != VideoContext->TargetHeight) {
        VideoContext->TargetWidth = TargetWidth;
        VideoContext->TargetHeight = TargetHeight;
        if (VideoContext->TargetWidth || VideoContext->TargetHeight) {
            sws_freeContext(VideoContext->ScalerContext);
        }
        ScalerContext = sws_getContext(
            Frame->width, Frame->height, 
            CodecContext->pix_fmt, 
            TargetWidth, TargetHeight, 
            AV_PIX_FMT_RGB0,
            SWS_BILINEAR,
            NULL, 
            NULL, 
            NULL
        );
        if (!ScalerContext) {
            CloseVideo(VideoContext);
            VideoContext->Ended = true;
            return;
        }
    }

    uint8_t* Dest[4] = {(uint8_t*)VideoContext->VideoOut, 0, 0, 0};
    int DestLinesize[4] = { VideoContext->TargetWidth * 4, 0, 0, 0};
    int ScaleResult = sws_scale(ScalerContext, Frame->data, Frame->linesize, 0, Frame->height, Dest, DestLinesize);

    av_frame_unref(Frame);
}

static int ReadPacket(void* Opaque, uint8* Buffer, int BufferSize) {
    video_buffer* VideoBuffer = (video_buffer*)Opaque;
    int Remaining = VideoBuffer->FullSize - VideoBuffer->ReadSize;
    int ToRead = min(Remaining, BufferSize);
    if (ToRead == 0) return AVERROR_EOF;
    memcpy(Buffer, VideoBuffer->Start + VideoBuffer->ReadSize, ToRead);
    VideoBuffer->ReadSize += ToRead;
    return ToRead;
}

int64 SeekPacket(void* Opaque, int64 Offset, int Whence) {
    video_buffer* VideoBuffer = (video_buffer*)Opaque;
    int NewReadSize = 0;
    switch (Whence) {
        case AVSEEK_SIZE: return VideoBuffer->FullSize; break;
        case SEEK_SET: NewReadSize = Offset; break;
        case SEEK_CUR: NewReadSize = VideoBuffer->ReadSize + Offset; break;
        case SEEK_END: NewReadSize = VideoBuffer->FullSize + Offset; break;
        default: return EOF;
    }
    if (NewReadSize > VideoBuffer->FullSize) return EOF;
    VideoBuffer->ReadSize = NewReadSize;
    return NewReadSize;
}

void InitializeVideo(video_context* VideoContext) {
    auto& FormatContext = VideoContext->FormatContext;

    FormatContext = avformat_alloc_context();
    if (!FormatContext) {
        throw("Format context not allocated.");
    }

    int BufferSize = 0x1000;
    uint8* pBuffer = (uint8*)av_malloc(BufferSize);
    VideoContext->IOContext = avio_alloc_context(
        pBuffer,
        BufferSize,
        0,
        &VideoContext->Buffer,
        &ReadPacket,
        NULL,
        &SeekPacket
    );

    FormatContext->pb = VideoContext->IOContext;
    FormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
    FormatContext->iformat = av_find_input_format("mp4");

    // If loading video from file, change second parameter to URI
    int AVError = avformat_open_input(&FormatContext, NULL, NULL, NULL);
    if (AVError) {
        char StrError[256];
        av_strerror(AVError, StrError, 256);
        Log(Error, StrError);
        Assert(false);
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

    LoadFrame(VideoContext);
    int Width = VideoContext->Frame->width;
    int Height = VideoContext->Frame->height;
    VideoContext->VideoOut = VirtualAlloc(0, Width * Height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
