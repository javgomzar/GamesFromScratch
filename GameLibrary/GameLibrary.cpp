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

// Video
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
    double* Time = &pGameState->Time;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    camera* Camera = &Group->Camera;
    static video_context VideoContext = { 0 };
    bool firstFrame = false;

    if (!Memory->IsInitialized) {
        firstFrame = true;

        // Assets ----------------------------------------------------------------------------------------------------------------------------------------
        // Load your assets here
        LoadAssets(Assets, Platform, &pGameState->RenderArena, &pGameState->MeshArena, &pGameState->VideoArena, &pGameState->TextArena);

        // User Interface
        // InitializeUI();

        Camera->Position = V3(0, 0, -10.0);

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

    // Camera
    if (Input->Mouse.Wheel > 0) Camera->Distance /= 1.2;
    else if (Input->Mouse.Wheel < 0) Camera->Distance *= 1.2;

    if (Input->Mouse.LeftClick.IsDown && Input->Mouse.LeftClick.WasDown) {
        v3 Offset = Input->Mouse.Cursor - Input->Mouse.LastCursor;
        double AngularVelocity = 0.5;

        Camera->Pitch -= AngularVelocity * Offset.Y;
        Camera->Angle -= AngularVelocity * Offset.X;
    }

    Camera->Velocity = V3(0, 0, 0);
    if (Input->Keyboard.D.IsDown) {
        Camera->Velocity.X -= 1.0;
    }
    else if (Input->Keyboard.A.IsDown) {
        Camera->Velocity.X += 1.0;
    }
    if (Input->Keyboard.W.IsDown) {
        Camera->Velocity.Z -= 1.0;
    }
    else if (Input->Keyboard.S.IsDown) {
        Camera->Velocity.Z += 1.0;
    }

    v3 Direction = normalize(Camera->Velocity);
    v3 X2 = V3(cos(Camera->Angle * Pi / 180.0), 0, sin(Camera->Angle * Pi / 180.0));
    v3 Z2 = V3(-sin(Camera->Angle * Pi / 180.0), 0, cos(Camera->Angle * Pi / 180.0));
    Camera->Velocity = 0.2 * (Direction.X * X2 + Direction.Z * Z2);
    Camera->Position += Camera->Velocity;

    // Render
    light Light = { 0 };
    Light.Ambient = 0.2;
    Light.Direction = normalize(V3(-0.5,-1,1));

    Light.Color = Red;
    //transform Transform1 = Transform(Quaternion(pGameState->Time, V3(0.0, -1.0, 0.0)));
    transform Transform1 = Transform(Quaternion(1.0, 0.0, 0.0, 0.0));
    PushMesh(Group, &Assets->TestMesh, Transform1, Light, &Assets->TestShader);

    Light.Color = Green;
    transform Transform2 = Transform(Quaternion(pGameState->Time, V3(0.0, 1.0, 0.0)), V3(3.0, 0.0, 0.0));
    PushMesh(Group, &Assets->TestMesh2, Transform2, Light, &Assets->TestShader);

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

        // Debug info
    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        pGameState->ShowDebugInfo = !pGameState->ShowDebugInfo;
    }

    if (pGameState->ShowDebugInfo) {
        game_rect DebugInfoRect = { 0, 0, 350, 220 };
        PushRect(Group, DebugInfoRect, { 0.5, 0.0, 0.0, 0.0 }, 0);
        PushRectOutline(Group, DebugInfoRect, Gray);
        PushText(Group, { 0, 30, 0.5 }, Assets->Characters, White, 12, Memory->DebugInfo, false);

        // Render Arena
        double RenderArenaPercentage = (double)pGameState->RenderArena.Used / (double)pGameState->RenderArena.Size;
        double RenderGroupPercentage = (double)Group->PushBufferSize / (double)Group->MaxPushBufferSize;
        PushRect(Group, { 20.0, 120.0, 100.0, 20.0 }, DarkGray, 0.1);
        PushRect(Group, { 20.0, 120.0, 100.0 * RenderArenaPercentage, 20.0 }, Gray, 0.2);
        PushRect(Group, { 20.0, 120.0, 100.0 * RenderGroupPercentage * RenderArenaPercentage, 20.0 }, Red, 0.3);
        PushText(Group, { 20.0, 135.0, 0.5 }, Assets->Characters, White, 8, Assets->RenderArenaStr, false);
        sprintf_s(Assets->RenderPercentageStr.Content, 7, "%.02f%%", RenderGroupPercentage);
        PushText(Group, { 125.0, 135.0, 0.5 }, Assets->Characters, White, 8, Assets->RenderPercentageStr, false);

        // Video Arena
        double VideoArenaPercentage = (double)pGameState->VideoArena.Used / (double)pGameState->VideoArena.Size;
        PushRect(Group, { 20.0, 150.0, 100.0, 20.0 }, DarkGray, 0.1);
        PushRect(Group, { 20.0, 150.0, 100.0 * VideoArenaPercentage, 20.0 }, Red, 0.2);
        PushText(Group, { 20.0, 165.0, 0.5 }, Assets->Characters, White, 8, Assets->VideoArenaStr, false);
        sprintf_s(Assets->VideoPercentageStr.Content, 7, "%.02f%%", VideoArenaPercentage * 100.0);
        PushText(Group, { 125.0, 135.0 + 30.0, 0.5 }, Assets->Characters, White, 8, Assets->VideoPercentageStr, false);

        // Text Arena
        double TextArenaPercentage = (double)pGameState->TextArena.Used / (double)pGameState->TextArena.Size;
        PushRect(Group, { 20.0, 180.0, 100.0, 20.0 }, DarkGray, 0.1);
        PushRect(Group, { 20.0, 180.0, 100.0 * TextArenaPercentage, 20.0 }, Red, 0.2);
        PushText(Group, { 20.0, 195.0, 0.5 }, Assets->Characters, White, 8, Assets->TextArenaStr, false);
        sprintf_s(Assets->TextPercentageStr.Content, 7, "%.02f%%", TextArenaPercentage * 100.0);
        PushText(Group, { 125.0, 135.0 + 60.0, 0.5 }, Assets->Characters, White, 8, Assets->TextPercentageStr, false);
    }
}

