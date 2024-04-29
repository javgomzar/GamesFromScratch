// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"
#include "render_group.h"


//// This is an example of an exported variable
//GAMELIBRARY_API int nGameLibrary=0;
//
//// This is an example of an exported function.
//GAMELIBRARY_API int fnGameLibrary(void)
//{
//    return 0;
//}
//
//// This is the constructor of a class that has been exported.
//CGameLibrary::CGameLibrary()
//{
//    return;
//}

// Game logic
bool Collision(game_rect Rect, game_screen_position Cursor) {
    return Cursor.X > Rect.Left &&
        Cursor.X < Rect.Left + Rect.Width &&
        Cursor.Y > Rect.Top &&
        Cursor.Y < Rect.Top + Rect.Height;
}

color Lighten(color RGB) {
    color Result;

    Result.R = min(RGB.R + Attenuation, 255);
    Result.G = min(RGB.G + Attenuation, 255);
    Result.B = min(RGB.B + Attenuation, 255);

    return Result;
}

void Plot(game_offscreen_buffer* Buffer, game_screen_position Position, color Color) {
    game_rect ScreenRect;
    ScreenRect.Left = 0;
    ScreenRect.Top = 0;
    ScreenRect.Width = Buffer->Width;
    ScreenRect.Height = Buffer->Height;

    if (Collision(ScreenRect, Position)) {
        uint8* PixelMemory = (uint8*)Buffer->Memory + (int)Position.X * Buffer->BytesPerPixel + (int)Position.Y * Buffer->Pitch;
        uint32* Pixel = (uint32*)PixelMemory;
        *Pixel = GetColorBytes(Color);
    }
}

// Asset loading
 // BMP
loaded_bmp LoadBMP(platform_read_entire_file* PlatformReadEntireFile, const char* FileName) {
    loaded_bmp Result = { 0 };
    Result.Handle = 0;
    read_file_result ReadResult = PlatformReadEntireFile(FileName);
    if (ReadResult.ContentSize != 0) {
        bitmap_header* Header = (bitmap_header*)ReadResult.Content;
        Result.Header = *Header;
        uint32 BytesPerPixel = Header->BitsPerPixel >> 3;
        Result.BytesPerPixel = BytesPerPixel;
        Result.Pitch = Header->Width * BytesPerPixel;
        Result.Content = (uint32*)((uint8*)ReadResult.Content + Header->BitmapOffset);
        
        bool HasAlpha = false;
        if (Result.Header.BitsPerPixel == 32 && Result.Header.Compression == 3) {
            uint32 AlphaMask = ~(Result.Header.RedMask | Result.Header.GreenMask | Result.Header.BlueMask);
            // If not all Alphas are zero, we need to use them
            uint32* Contents = Result.Content;
            for (int32 i = 0; i < Result.Header.Height * Result.Header.Width; i++) {
                if ((*Contents++ & AlphaMask) > 0) {
                    HasAlpha = true;
                    break;
                }
            }

            // If all alphas are zero, turn them to one
            Contents = Result.Content;
            if (!HasAlpha) {
                for (int32 j = 0; j < Result.Header.Height * Result.Header.Width; j++) {
                    *Contents = AlphaMask | (*Contents++ & ~AlphaMask);
                }
            }
            return Result;
        }
    }
    return Result;
}


// UI
void InitializeUI(memory_arena* Arena, game_assets* Assets, UI* UserInterface, platform_read_entire_file* Read) {
    // Initialize your UI elements here
}


// Sound
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

void DebugPlotSoundBuffer(game_offscreen_buffer* pScreenBuffer, game_sound_buffer* pSoundBuffer, game_screen_position PlotOrigin) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    uint16 PlotHeight = 200;
    uint16 PlotWidth = 400;
    uint16 Amplitude = 4000;

    int16* SampleOut = pSoundBuffer->SampleOut;
    for (int i = 0; i < SampleCount; i++) {
        game_screen_position Position = { PlotOrigin.X + (i * PlotWidth / SampleCount), (uint32)((*SampleOut * PlotHeight / Amplitude) + PlotOrigin.Y), 0 };
        Plot(pScreenBuffer, Position, White);
        SampleOut++;
        SampleOut++;
    }
}

void PlaySound(game_sound* Sound, game_sound_buffer* pSoundBuffer) {
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

    Sound->Played += 2*pSoundBuffer->BufferSize;
}

void GameOutputSound(game_assets* Assets, game_sound_buffer* pSoundBuffer, game_state* pGameState, game_input* Input) {
    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
    //WriteSineWave(pSoundBuffer, 480, 0);
}

game_sound LoadWAV(platform_read_entire_file* PlatformReadEntireFile, const char* FileName) {
    read_file_result File = PlatformReadEntireFile(FileName);
    DWORD* Pointer = (DWORD*)File.Content;
    
    DWORD ChunkType = *Pointer++;
    if (ChunkType != 'FFIR') {
        Assert(false);
    }

    DWORD RIFFChunkSize = *Pointer++;
    DWORD FileType = *Pointer++;
    if (FileType != 'EVAW') {
        Assert(false);
    }

    ChunkType = *Pointer++;
    if (ChunkType != ' tmf') {
        Assert(false);
    }
    DWORD ChunkSize = *Pointer++;
    waveformat WaveFMT = *(waveformat*)Pointer;

    Pointer += 4;
    ChunkType = *Pointer++;
    if (ChunkType != 'atad') {
        Assert(false);
    }
    ChunkSize = *Pointer++;

    game_sound Result = {0};
    Result.SampleOut = (int16*)Pointer;
    Result.SampleCount = ChunkSize / 2;
    return Result;
}

// Video
game_video LoadVideo(memory_arena* Arena, const char* Filename) {
    game_video Result = { 0 };
    Result.VideoContext = PushStruct(Arena, video_context);

    InitializeVideo(Filename, Result.VideoContext);
    int Width = Result.VideoContext->Frame->width;
    int Height = Result.VideoContext->Frame->height;
    Result.VideoContext->VideoOut = PushSize(Arena, Width * Height * 4);

    return Result;
}

void PushVideo(render_group* Group, game_video* Video, game_rect Rect, int Z, double SecondsElapsed) {
    
    if (!Video->VideoContext->Ended) {
        Video->TimeElapsed += SecondsElapsed;
        char Text[256];
        sprintf_s(Text, "%.02f Time elapsed | %.02f Time played\n", Video->TimeElapsed, Video->VideoContext->PTS * Video->VideoContext->TimeBase);
        OutputDebugStringA(Text);

        if (Video->TimeElapsed > Video->VideoContext->PTS * Video->VideoContext->TimeBase) {
            LoadFrame(Video->VideoContext);
            Video->VideoContext->Width = Rect.Width;
            Video->VideoContext->Height = Rect.Height;
            WriteFrame(Video->VideoContext);
        }
    }
    else {
    }
    _PushVideo(Group, Video, Rect, Z);
}

void PushVideoLoop(render_group* Group, game_video* Video, game_rect Rect, int Z, double SecondsElapsed, int64_t StartOffset, int64_t EndOffset) {

    PushVideo(Group, Video, Rect, Z, SecondsElapsed);
    auto& VideoContext = Video->VideoContext;
    auto& FormatContext = VideoContext->FormatContext;
    auto& CodecContext = VideoContext->CodecContext;
    auto& StreamIndex = VideoContext->VideoStreamIndex;
    auto& PTS = VideoContext->PTS; // Presentation time-stamp (in time-base units)

    if (PTS >= EndOffset) {
        av_seek_frame(FormatContext, StreamIndex, StartOffset, AVSEEK_FLAG_BACKWARD);
        do { LoadFrame(Video->VideoContext); } while (Video->VideoContext->PTS < StartOffset - 1000);
        Video->TimeElapsed = Video->VideoContext->PTS * Video->VideoContext->TimeBase;
    }
}

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    //render_group* Group = Memory->Group;
    static video_context VideoContext = { 0 };
    bool firstFrame = false;

    if (!Memory->IsInitialized) {
        firstFrame = true;

        // Assets ----------------------------------------------------------------------------------------------------------------------------------------
        // Load your assets here

        // User Interface
        // InitializeUI();

        Memory->IsInitialized = true;
    }

    PushClear(Group, BackgroundBlue);

    // Controls
    // Put here your input code
        
    GameOutputSound(Assets, SoundBuffer, pGameState, Input);

    // Debug mouse position
    //if (Input->Mouse.LeftClick.IsDown) {
    //    game_rect Rect;
    //    Rect.Top = Input->Mouse.Cursor.Y;
    //    Rect.Left = Input->Mouse.Cursor.X;
    //    Rect.Height = 20;
    //    Rect.Width = 20;
    //    DrawRectangle(ScreenBuffer, Rect, White);
    //}

    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        pGameState->ShowDebugInfo = !pGameState->ShowDebugInfo;
    }

    if (pGameState->ShowDebugInfo) {
        game_rect DebugInfoRect = { 0, 0, 470, 150 };
        PushRect(Group, DebugInfoRect, {0.5, 0.0, 0.0, 0.0}, 999);
        PushRectOutline(Group, DebugInfoRect, Gray);
        text Text = { 0 };
        Text.Color = White;
        Text.Length = 71;
        Text.Points = 20;
        Text.Content = Memory->DebugInfo;
        Text.Characters = Assets->Characters;
        PushText(Group, { 0,30,999 }, Text);
    }

    // Render

    // Software renderer as a fallback (toggle with Space)
    //static bool SoftwareRenderer = false;
    //if (Input->Keyboard.Space.IsDown && !Input->Keyboard.Space.WasDown) {
    //    SoftwareRenderer = !SoftwareRenderer;
    //}
    //if (SoftwareRenderer) {
    //    RenderGroupToOutput(Group, &Target);
    //}
    //else {
    //    Platform.OpenGLRender(Group, &Target);
    //}
}

