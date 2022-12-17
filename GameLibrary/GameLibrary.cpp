// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"
#include <time.h>


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
//    return;s
//}


#include "stdio.h"
#include "math.h"
#include "stdlib.h" // for rand()


// Game logic
void InitializeSquares(game_squares* pGameSquares) {
    color Colors[MAX_DIFFICULTY] = { Red, Orange, Yellow, Green, Cyan, Blue, Magenta };

    float Tone = 220;
    int Padding = 20;
    int Length = 100;
    for (int i = 0; i < MAX_DIFFICULTY; i++) {
        pGameSquares->Colors[i] = Colors[i];
        pGameSquares->Rects[i].Left = Padding + i * (Padding + Length);
        pGameSquares->Rects[i].Top = Padding;
        pGameSquares->Rects[i].Width = Length;
        pGameSquares->Rects[i].Height = Length;
        pGameSquares->Tones[i] = Tone;
        Tone *= twroot;
        if (i != 2 && i != 6) {
            Tone *= twroot;
        }
    }
}

void GenerateSequence(int nColors, game_state* pGameState) {
    srand(time(NULL));
    for (int i = 0; i < MAX_SEQUENCE_LENGTH; i++) {
        int Random = rand() % nColors;
        pGameState->Sequence[i] = Random;
    }
}

bool Collision(game_rect Rect, game_screen_position Cursor) {
    return Cursor.X > Rect.Left &&
        Cursor.X < Rect.Left + Rect.Width &&
        Cursor.Y > Rect.Top &&
        Cursor.Y < Rect.Top + Rect.Height;
}


// Render graphics
void RenderWhiteNoise(game_offscreen_buffer* Buffer) {
    uint8* Row = (uint8*)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; ++Y) {
        uint32* Pixel = (uint32*)Row;
        for (int X = 0; X < Buffer->Width; ++X) {
            uint8 Gray = rand() % 255;
            uint8 Red = Gray;
            uint8 Green = Gray;
            uint8 Blue = Gray;

            uint32 RGB_color = (Red << 16) | (Green << 8) | Blue;

            *Pixel++ = RGB_color;
        }
        Row += Buffer->Pitch;
    }
}

color Lighten(color RGB) {
    color Result;

    Result.R = min(RGB.R + Attenuation, 255);
    Result.G = min(RGB.G + Attenuation, 255);
    Result.B = min(RGB.B + Attenuation, 255);

    return Result;
}

void DrawRectangle(game_offscreen_buffer* Buffer, game_rect Rect, color Color) {
    // Crop extra pixels
    int MinX = Rect.Left;
    if (Rect.Left < 0) {
        MinX = 0;
    }
    else if (MinX > Buffer->Width) {
        return;
    }

    int MaxX = MinX + Rect.Width;
    if (MaxX > Buffer->Width) {
        MaxX = Buffer->Width;
    }

    int MinY = Rect.Top;
    if (MinY < 0) {
        MinY = 0;
    }
    else if (MinY > Buffer->Height) {
        return;
    }

    int MaxY = MinY + Rect.Height;
    if (MaxY > Buffer->Height) {
        MaxY = Buffer->Height;
    }

    uint32 ColorBytes = GetColorBytes(Color);
    uint8* Row = (uint8*)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch;
    for (int Y = MinY; Y < MaxY; Y++) {
        uint32* Pixel = (uint32*)Row;
        for (int X = MinX; X < MaxX; X++) {
            *Pixel++ = ColorBytes;
        }
        Row += Buffer->Pitch;
    }
}

void Plot(game_offscreen_buffer* Buffer, game_screen_position Position, color Color) {
    game_rect ScreenRect;
    ScreenRect.Left = 0;
    ScreenRect.Top = 0;
    ScreenRect.Width = Buffer->Width;
    ScreenRect.Height = Buffer->Height;

    if (Collision(ScreenRect, Position)) {
        uint8* PixelMemory = (uint8*)Buffer->Memory + Position.X * Buffer->BytesPerPixel + Position.Y * Buffer->Pitch;
        uint32* Pixel = (uint32*)PixelMemory;
        *Pixel = GetColorBytes(Color);
    }
}

//void RenderWeirdGradient(game_offscreen_buffer* Buffer, game_state* pGameState) {
//    //int XOffset = pGameState->XOffset;
//    //int YOffset = pGameState->YOffset;
//
//    uint8* Row = (uint8*)Buffer->Memory;
//    for (int Y = 0; Y < Buffer->Height; ++Y) {
//        uint32* Pixel = (uint32*)Row;
//        for (int X = 0; X < Buffer->Width; ++X) {
//            uint8 Red = 0xff;
//            //uint8 Green = X + XOffset;
//            uint8 Blue = Y + YOffset;
//
//            uint32 RGB_color = (Red << 16) | (Green << 8) | Blue;
//
//            *Pixel++ = RGB_color;
//        }
//        Row += Buffer->Pitch;
//    }
//}


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

    int16* SampleOut = pSoundBuffer->SampleOut;
    for (int i = 0; i < SampleCount; i++) {
        game_screen_position Position = { PlotOrigin.X + (i * PlotWidth / SampleCount), (uint32)((*SampleOut * PlotHeight / Amplitude) + PlotOrigin.Y), 0 };
        Plot(pScreenBuffer, Position, White);
        SampleOut++;
        SampleOut++;
    }
}

void GameOutputSound(game_offscreen_buffer* ScreenBuffer, game_sound_buffer* PreviousSoundBuffer, game_sound_buffer* pSoundBuffer, game_state* pGameState, int SelectedSquare) {
    static float Phase;
    static float Tone;
    static uint8 TransitionCount;
    static bool SoundOn = false;
    uint8 TransitionMax = 3;

    game_screen_position PreviousOrigin = { 20, 400, 0 };
    game_screen_position Origin = { 420, 400, 0 };

    if (SelectedSquare < 0) {
        if (SoundOn) {
            if (TransitionCount == TransitionMax) {
                SoundOn = false;
                //DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
                //DebugPlotSoundBuffer(ScreenBuffer, pSoundBuffer, Origin);
            }
            else {
                Phase = WriteSineWave(pSoundBuffer, Tone, Phase);
                FadeOut(pSoundBuffer, TransitionCount);
                TransitionCount++;
                //DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
                //DebugPlotSoundBuffer(ScreenBuffer, pSoundBuffer, Origin);
            }
        }
        if (!SoundOn) {
            TransitionCount = 0;
            Phase = 0;
            Silence(pSoundBuffer);
        }
    }
    else {
        if (!SoundOn) {
            if (TransitionCount == TransitionMax) {
                SoundOn = true;
            }
            else {
                Tone = pGameState->GameSquares.Tones[SelectedSquare];
                Phase = WriteSineWave(pSoundBuffer, Tone, Phase);
                FadeIn(pSoundBuffer, TransitionCount);
                TransitionCount++;
            }
        }
        if (SoundOn) {
            TransitionCount = 0;
            Phase = WriteSineWave(pSoundBuffer, Tone, Phase);
        }
    }
}


// Main
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    if (!Memory->IsInitialized) {
        pGameState->Difficulty = 7;
        Memory->IsInitialized = true;
        GenerateSequence(pGameState->Difficulty, pGameState);
        InitializeSquares(&(pGameState->GameSquares));
    }

    //static int Count = 0;
    //static int BlippedSquare = -1;
    //static int SquareIndex = 0;
    //if (Count == 60) {
    //    Count = 0;
    //    if (SquareIndex >= MAX_DIFFICULTY) {
    //        SquareIndex = 0;
    //    }
    //    else {
    //        SquareIndex++;
    //    }
    //    BlippedSquare = pGameState->Sequence[SquareIndex];
    //}
    //else {
    //    Count++;
    //}

    int SelectedSquare = -1;
    for (int i = 0; i < pGameState->Difficulty; i++) {
        game_rect Rect = pGameState->GameSquares.Rects[i];
        color Color = pGameState->GameSquares.Colors[i];
        if (Collision(Rect, Input->Mouse.Cursor) && Input->Mouse.LeftClick.IsDown) {
            SelectedSquare = i;
            Color = Lighten(Color);
        }
        DrawRectangle(ScreenBuffer, Rect, Color);
    }

    GameOutputSound(ScreenBuffer, PreviousSoundBuffer, SoundBuffer, pGameState, SelectedSquare);

    // Debug mouse position
    //if (Input->Mouse.LeftClick.IsDown) {
    //    game_rect Rect;
    //    Rect.Top = Input->Mouse.Cursor.Y;
    //    Rect.Left = Input->Mouse.Cursor.X;
    //    Rect.Height = 20;
    //    Rect.Width = 20;
    //    DrawRectangle(ScreenBuffer, Rect, White);
    //}
}


