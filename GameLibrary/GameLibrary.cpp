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
void Initialize(game_state* pGameState, bool* pAlive) {
    // Initialization code
    srand(time(0));
    pGameState->Board.Width = BOARD_WIDTH;
    pGameState->Board.Height = BOARD_HEIGHT;
    pGameState->Board.IsAlive = pAlive;

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            // Empty initial board
            //*pAlive++ = false;
            
            // Random inital boards
            // Fair
            *pAlive++ = (rand() % 2) == 1;

            // Slightly favor empty
            //*pAlive++ = (rand() % 10) == 0;

            // Glider
            //*pAlive++ = (i == 3) && (j >= 1) && (j <=3) || (j == 3 && i == 2) || (i == 1 && j == 2); 

        }
    }
}

bool Collision(game_rect Rect, game_screen_position Cursor) {
    return Cursor.X > Rect.Left &&
        Cursor.X < Rect.Left + Rect.Width &&
        Cursor.Y > Rect.Top &&
        Cursor.Y < Rect.Top + Rect.Height;
}

bool IsAlive(bool* pAlive, int X, int Y, int Width) {
    return *pAlive + X + Y * Width;
}

void CopyBoard(game_board* Board, bool* NewBoard) {    
    bool* pOld = Board->IsAlive;
    for (int i = 0; i < Board->Height; i++) {
        for (int j = 0; j < Board->Width; j++) {
            *NewBoard++ = *pOld++;
        }
    }
}

void UpdateBoard(game_board* Board) {
    bool WasAlive[BOARD_HEIGHT][BOARD_WIDTH];

    bool* pIsAlive = Board->IsAlive;
    bool* pWasAlive = (bool*)WasAlive;
    CopyBoard(Board, pWasAlive);
    for (int i = 0; i < Board->Height; i++) {
        for (int j = 0; j < Board->Width; j++) {
            int AliveCount = 0;

            // Counting alive neighbors
            for (int DX = -1; DX <= 1; DX++) {
                for (int DY = -1; DY <= 1; DY++) {
                    if (DX != 0 || DY != 0) {
                        int X = j + DX;
                        int Y = i + DY;

                        if (X >= 0 && X < Board->Width && 
                            Y >= 0 && Y < Board->Height &&
                            WasAlive[Y][X]) {
                            AliveCount++;
                        }
                    }
                }
            }

            // Deciding new state
            if (*pIsAlive) {
                *pIsAlive = (AliveCount == 2) || (AliveCount == 3);
            }
            else if (AliveCount == 3) {
                *pIsAlive = true;
            }

            // Proceed to next square
            pIsAlive++;
            pWasAlive++;
        }
    }
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

void DrawBoard(game_offscreen_buffer* Buffer, game_board Board) {
    for (int i = 0; i < Board.Height; i++) {
        for (int j = 0; j < Board.Width; j++) {
            color Color = *Board.IsAlive++ ? ALIVE_COLOR : DEAD_COLOR;
            game_rect Rect = {i*SQUARE_LENGTH, j*SQUARE_LENGTH, SQUARE_LENGTH, SQUARE_LENGTH};
            DrawRectangle(Buffer, Rect, Color);
        }
    }
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

void GameOutputSound(game_offscreen_buffer* ScreenBuffer, game_sound_buffer* pSoundBuffer, game_state* pGameState) {
    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
}


// Main
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    static int FrameCount;
    static bool IsAlive[BOARD_HEIGHT][BOARD_WIDTH];
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    if (!Memory->IsInitialized) {
        Memory->IsInitialized = true;
        Initialize(pGameState, (bool*)IsAlive);
        FrameCount = 0;
    }

    DrawBoard(ScreenBuffer, pGameState->Board);

    if (FrameCount == 0) {
        UpdateBoard(&pGameState->Board);
        FrameCount = 0;
    }
    else {
        FrameCount++;
    }

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


