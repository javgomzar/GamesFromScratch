#pragma once

#include "GamePlatform.h"
#include "GameMath.h"

#ifndef GAME_SOUND
#define GAME_SOUND

/*
    TODO:
        - Mixer
        - Some effects (low pass, high pass)
        - Volume control
        - Implement circular buffer with VirtualAlloc2 and MapViewOfFile3
*/

enum game_sound_id {
    Sound_Test_ID,

    game_sound_id_count
};

struct waveformat {
    unsigned short    wFormatTag;        /* format type */
    unsigned short    nChannels;         /* number of channels (i.e. mono, stereo...) */
    unsigned long     nSamplesPerSec;    /* sample rate */
    unsigned long     nAvgBytesPerSec;   /* for buffer estimation */
    unsigned short    nBlockAlign;       /* block size of data */
    unsigned short    wBitsPerSample;    /* Number of bits per sample of mono data */
    unsigned short    cbSize;            /* The count in bytes of the size of extra information (after cbSize) */
};

struct game_sound {
    game_sound_id ID;
    waveformat WaveFormat;
    uint32 SampleCount;
    uint32 Played;
    int16* SampleOut;
};

struct preprocessed_sound {
    waveformat WaveFormat;
    uint64 Size;
    int16* Data;
};

preprocessed_sound PreprocessSound(read_file_result File) {
    unsigned long* Pointer = (unsigned long*)File.Content;

    unsigned long ChunkType = *Pointer++;
    if (ChunkType != 'FFIR') {
        Assert(false);
    }

    unsigned long RIFFChunkSize = *Pointer++;
    unsigned long FileType = *Pointer++;
    if (FileType != 'EVAW') {
        Assert(false);
    }

    preprocessed_sound Result;
    ChunkType = *Pointer++;
    if (ChunkType != ' tmf') {
        Assert(false);
    }
    unsigned long ChunkSize = *Pointer++;
    Result.WaveFormat = *(waveformat*)Pointer;

    Pointer += 4;
    ChunkType = *Pointer++;
    if (ChunkType != 'atad') {
        Assert(false);
    }
    ChunkSize = *Pointer++;

    Result.Size = ChunkSize;
    Result.Data = (int16*)Pointer;

    return Result;
}

game_sound LoadSound(memory_arena* Arena, preprocessed_sound* Sound) {
    game_sound Result = {};
    Result.SampleOut = (int16*)PushSize(Arena, Sound->Size);
    Result.SampleCount = Sound->Size / 2;
    Result.WaveFormat = Sound->WaveFormat;

    memcpy(Result.SampleOut, Sound->Data, Sound->Size);

    return Result;
}

struct game_sound_buffer {
    uint32 SamplesPerSecond;
    uint16 BufferSize;
    int16* SampleOut;
};

int16 Amplitude = 4000;

float WriteSineWave(game_sound_buffer* pSoundBuffer, float Hz, float InitialPhase) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    float SineWavePeriod = (float)(pSoundBuffer->SamplesPerSecond) / Hz;
    float PhaseIncrement = (2.0f * Pi) / (float)SineWavePeriod;

    float Phase = InitialPhase;
    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        float SineValue = sinf(Phase);
        Phase += PhaseIncrement;
        while (Phase > Tau) {
            Phase -= Tau;
        }
        uint16 SampleValue = (uint16)(SineValue * Amplitude);
        *SampleOut++ = SampleValue; // LEFT
        *SampleOut++ = SampleValue; // RIGHT
    }
    return Phase;
}

void FadeIn(game_sound_buffer* pSoundBuffer, uint8 TransitionCount) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    uint16 HalfLife = (uint16)(SampleCount / 2);
    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        float Exp = (1 - expf(-(float)((SampleIndex + TransitionCount * SampleCount) / (float)HalfLife)));
        float LeftValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(LeftValue); // LEFT
        SampleOut++;
        float RightValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(RightValue); // RIGHT
        SampleOut++;
    }
}

void FadeOut(game_sound_buffer* pSoundBuffer, uint8 TransitionCount) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    uint16 HalfLife = (uint16)(SampleCount / 2);
    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        float Exp = expf(-(float)((SampleIndex + TransitionCount * SampleCount) / (float)HalfLife));
        float LeftValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(LeftValue); // LEFT
        SampleOut++;
        float RightValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(RightValue); // RIGHT
        SampleOut++;
    }
}

void Silence(game_sound_buffer* pSoundBuffer) {
    uint32 SampleCount = pSoundBuffer->BufferSize;

    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        *SampleOut++ = 0; // LEFT
        *SampleOut++ = 0; // RIGHT
    }
}

//void DebugPlotSoundBuffer(game_offscreen_buffer* pScreenBuffer, game_sound_buffer* pSoundBuffer, v3 PlotOrigin) {
//    uint32 SampleCount = pSoundBuffer->BufferSize;
//    uint16 PlotHeight = 200;
//    uint16 PlotWidth = 400;
//    uint16 Amplitude = 4000;
//
//    int16* SampleOut = pSoundBuffer->SampleOut;
//    for (int i = 0; i < SampleCount; i++) {
//        v3 Position = { PlotOrigin.X + (i * PlotWidth / SampleCount), (uint32)((*SampleOut * PlotHeight / Amplitude) + PlotOrigin.Y), 0 };
//        Plot(pScreenBuffer, Position, White);
//        SampleOut++;
//        SampleOut++;
//    }
//}

void PlayGameSound(game_sound* Sound, game_sound_buffer* pSoundBuffer) {
    if (Sound->Played + pSoundBuffer->BufferSize > Sound->SampleCount) {
        Sound->Played = 0;
    }

    uint32 SampleCount = pSoundBuffer->BufferSize;
    int16* SampleOut = pSoundBuffer->SampleOut;
    int16* SampleIn = Sound->SampleOut + Sound->Played;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        *SampleOut++ = *SampleIn++; // LEFT
        *SampleOut++ = *SampleIn++; // RIGHT
    }

    Sound->Played += 2 * pSoundBuffer->BufferSize;
}

#endif