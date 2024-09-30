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
bool Collision(game_rect Rect, v3 Cursor) {
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

void Plot(game_offscreen_buffer* Buffer, v3 Position, color Color) {
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

void DebugPlotSoundBuffer(game_offscreen_buffer* pScreenBuffer, game_sound_buffer* pSoundBuffer, v3 PlotOrigin) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    uint16 PlotHeight = 200;
    uint16 PlotWidth = 400;
    uint16 Amplitude = 4000;

    int16* SampleOut = pSoundBuffer->SampleOut;
    for (int i = 0; i < SampleCount; i++) {
        v3 Position = { PlotOrigin.X + (i * PlotWidth / SampleCount), (uint32)((*SampleOut * PlotHeight / Amplitude) + PlotOrigin.Y), 0 };
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
        LoadAssets(Assets, Platform, 
            &pGameState->RenderArena, 
            &pGameState->StringsArena, 
            &pGameState->FontsArena, 
            &pGameState->MeshArena, 
            &pGameState->VideoArena
        );

        // Faces
        pGameState->Cube = { 0 };
            // Distance between faces
        double D = 2.2;
            // Cube length
        double L = 3.3;

        for (int i = 0; i < 9; i++) {
            double sub_x = D * (double)(i % 3 - 1);
            double sub_y = D * (double)(i / 3 - 1);
            pGameState->Cube.Top[i] = { White, V3(sub_x, L, sub_y), Quaternion(1.0) };
            pGameState->Cube.Bottom[i] = { Red, V3(sub_x, -L, sub_y), Quaternion(Tau / 2.0, V3(1.0, 0.0, 0.0)) };
            pGameState->Cube.Left[i] = { Blue, V3(sub_x, sub_y, -L), Quaternion(Tau / 4.0, V3(1.0, 0.0, 0.0)) };
            pGameState->Cube.Right[i] = { Green, V3(sub_x, sub_y, L), Quaternion(-Tau / 4.0, V3(1.0, 0.0, 0.0)) };
            pGameState->Cube.Front[i] = { Yellow, V3(L, sub_y, sub_x), Quaternion(Tau / 4.0, V3(0.0, 0.0, 1.0)) };
            pGameState->Cube.Back[i] = { Orange, V3(-L, sub_y, sub_x), Quaternion(-Tau / 4.0, V3(0.0, 0.0, 1.0)) };
        }

        // User Interface
        // InitializeUI();

        Camera->Position = V3(0, 0, -15.0);

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
    if (Input->Keyboard.Space.IsDown) {
        Camera->Velocity.Y -= 1.0;
    }
    else if (Input->Keyboard.Shift.IsDown) {
        Camera->Velocity.Y += 1.0;
    }

    v3 Direction = normalize(Camera->Velocity);
    v3 X2 = V3(cos(Camera->Angle * Pi / 180.0), 0, sin(Camera->Angle * Pi / 180.0));
    v3 Z2 = V3(-sin(Camera->Angle * Pi / 180.0), 0, cos(Camera->Angle * Pi / 180.0));
    Camera->Velocity = 0.2 * (Direction.X * X2 + Direction.Y * V3(0.0, 1.0, 0.0) + Direction.Z * Z2);
    Camera->Position += Camera->Velocity;

    // Render
    light Light = { 0 };
    Light.Ambient = 0.2;
    Light.Direction = normalize(V3(-0.5,-1,1));

    for (int i = 0; i < 54; i++) {
        face Face = pGameState->Cube.Faces[i];
        Light.Color = Face.Color;
        transform FaceTransform = Transform(Face.Rotation, Face.Position + V3(0.0, 0.0, 5.0));
        PushMesh(Group, &Assets->FaceMesh, FaceTransform, Light, &Assets->TextureShader);
    }

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
        game_rect DebugInfoRect = { 0, 0, 350, 250 };
        PushRect(Group, DebugInfoRect, { 0.5, 0.0, 0.0, 0.0 }, 0);
        PushRectOutline(Group, DebugInfoRect, Gray);
        PushText(Group, { 0, 30, 0.5 }, Assets->Characters, White, 12, Memory->DebugInfo, false);

        // Strings Arena
        PushDebugArena(Group, Assets->Characters, pGameState->StringsArena, V3(20.0, 120.0, 0.5));

        // Fonts Arena
        PushDebugArena(Group, Assets->Characters, pGameState->FontsArena, V3(20.0, 150.0, 0.5));

        // Mesh Arena
        PushDebugArena(Group, Assets->Characters, pGameState->MeshArena, V3(20.0, 180.0, 0.5));

        // Video Arena
        PushDebugArena(Group, Assets->Characters, pGameState->VideoArena, V3(20.0, 210.0, 0.5));
    }
}

