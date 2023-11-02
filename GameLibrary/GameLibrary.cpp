// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"
#include "render_group.h"
#include <gl/GL.h>


/*
    TODO:
        - Make it so when you change TileSize everything zooms in (possibly control this with mouse wheel).
        - Support float values in game_screen_position.
*/


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
        uint8* PixelMemory = (uint8*)Buffer->Memory + Position.X * Buffer->BytesPerPixel + Position.Y * Buffer->Pitch;
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

void GameOutputSound(game_offscreen_buffer* ScreenBuffer, game_sound_buffer* pSoundBuffer, game_state* pGameState) {
    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
}

void ResetRoom(room* Room) {
    Room->Width = 10;
    Room->Height = 10;
    Room->Doors[0] = true;
    Room->Doors[1] = true;
    Room->Doors[2] = true;
    Room->Doors[3] = true;
}

// Main
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    render_group* Group = Memory->Group;

    if (!Memory->IsInitialized) {
        // Memory arenas
        InitializeArena(&pGameState->RenderArena, Megabytes(5), (uint8*)Memory->PermanentStorage + sizeof(game_state) + pGameState->TextArena.Size);
        InitializeArena(&pGameState->MapArena, Megabytes(1), (uint8*)Memory->PermanentStorage + sizeof(game_state) + pGameState->TextArena.Size + pGameState->RenderArena.Size);

        // Assets ----------------------------------------------------------------------------------------------------------------------------------------
        // Load your assets here

        Memory->Assets.PlayerBMP = LoadBMP(Platform->ReadEntireFile, "..\\GameLibrary\\RogueMedia\\Player.bmp");
        Memory->Assets.FloorBMP = LoadBMP(Platform->ReadEntireFile, "..\\GameLibrary\\RogueMedia\\Floor.bmp");
        Memory->Assets.DoorBMP = LoadBMP(Platform->ReadEntireFile, "..\\GameLibrary\\RogueMedia\\Door.bmp");

        // User Interface
        // InitializeUI();

        // Initialize game state
        pGameState->PlayerPosition = { 5, 5 };
        pGameState->TileSize = 30;
        pGameState->Camera = { {0,0,0}, {0,0,0} };
        pGameState->TestRoom = {0};
        pGameState->TestRoom.FloorBMP = &Memory->Assets.FloorBMP;
        pGameState->TestRoom.DoorBMP = &Memory->Assets.DoorBMP;
        pGameState->TestRoom.Position = { 5,5 };
        pGameState->TestRoom.Height = 3;
        pGameState->TestRoom.Width = 3;
        pGameState->TestRoom.Doors[0] = true;
        pGameState->TestRoom.Doors[1] = true;
        pGameState->TestRoom.Doors[2] = true;
        pGameState->TestRoom.Doors[3] = true;

        // Renderer --------------------------------------------------------------------------------------------------------------------------------------
        Memory->Group = AllocateRenderGroup(&pGameState->RenderArena, Megabytes(4));
        Memory->Group->Camera = &pGameState->Camera;
        Group = Memory->Group;

        Memory->IsInitialized = true;
    }

    PushClear(Group, Black);

    // Controls
    // Put here your input code

    // Character movement
    room CurrentRoom = pGameState->TestRoom;
    if (Input->Keyboard.D.IsDown && !Input->Keyboard.D.WasDown && pGameState->PlayerPosition.Col < CurrentRoom.Position.Col + CurrentRoom.Width - 1) {
        pGameState->PlayerPosition.Col += 1;
    }
    if (Input->Keyboard.A.IsDown && !Input->Keyboard.A.WasDown && pGameState->PlayerPosition.Col > CurrentRoom.Position.Col) {
        pGameState->PlayerPosition.Col -= 1;
    }
    if (Input->Keyboard.S.IsDown && !Input->Keyboard.S.WasDown && pGameState->PlayerPosition.Row < CurrentRoom.Position.Row + CurrentRoom.Height - 1) {
        pGameState->PlayerPosition.Row += 1;
    }
    if (Input->Keyboard.W.IsDown && !Input->Keyboard.W.WasDown && pGameState->PlayerPosition.Row > CurrentRoom.Position.Row) {
        pGameState->PlayerPosition.Row -= 1;
    }

    // Camera movement
    v3 Direction = { 0 };

    if (Input->Keyboard.Right.IsDown) {
        Direction.X += 1;
    }
    if (Input->Keyboard.Left.IsDown) {
        Direction.X -= 1;
    }
    if (Input->Keyboard.Up.IsDown) {
        Direction.Y -= 1;
    }
    if (Input->Keyboard.Down.IsDown) {
        Direction.Y += 1;
    }
    Direction = normalize(Direction);

    pGameState->Camera.Velocity = 10 * Direction;

    UpdateCamera(&pGameState->Camera);

    // Reset room
    if (Input->Keyboard.Space.IsDown) {
        ResetRoom(&pGameState->TestRoom);
    }

    // Debug camera position and velocity
    /*
    char TextBuffer[256];
    sprintf_s(TextBuffer, " %.02f,%.02f,%.02f Position\n %.02f,%.02f,%.02f Velocity\n", 
        pGameState->Camera.Position.X, pGameState->Camera.Position.Y, pGameState->Camera.Position.Z,
        pGameState->Camera.Velocity.X, pGameState->Camera.Velocity.Y, pGameState->Camera.Velocity.Z);
    OutputDebugStringA(TextBuffer);
    */

    GameOutputSound(ScreenBuffer, SoundBuffer, pGameState);

    // Debug mouse position
    //if (Input->Mouse.LeftClick.IsDown) {
    //    game_rect Rect;
    //    Rect.Top = Input->Mouse.Cursor.Y;
    //    Rect.Left = Input->Mouse.Cursor.X;
    //    Rect.Height = 20;
    //    Rect.Width = 20;
    //    DrawRectangle(ScreenBuffer, Rect, White);
    //}

    // Render
    loaded_bmp Target = { 0 };
    Target.Header.Width = ScreenBuffer->Width;
    Target.Header.Height = ScreenBuffer->Height;
    Target.Pitch = ScreenBuffer->Pitch;
    Target.Content = (uint32*)ScreenBuffer->Memory;

    PushRoom(Group, &pGameState->TestRoom, pGameState->TileSize);
    
    PushBMP(Group, &Memory->Assets.PlayerBMP, ToScreenCoord({pGameState->PlayerPosition.Row - 1, pGameState->PlayerPosition.Col}, pGameState->TileSize));

    static bool ShowDebugInfo = false;
    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        ShowDebugInfo = !ShowDebugInfo;
    }

    if (ShowDebugInfo) {
        game_rect DebugInfoRect = { pGameState->Camera.Position.X, pGameState->Camera.Position.Y, 450, 120 };
        PushDebugLattice(Group, pGameState->TileSize, {0.5f, 1.0f, 1.0f, 0.0f });
        PushRect(Group, DebugInfoRect, {0.5f, 0.0f, 0.0f, 0.0f});
        PushRectOutline(Group, DebugInfoRect, Gray);
        text Text = { 0 };
        Text.Color = White;
        Text.Length = 48;
        Text.Points = 20;
        Text.Content = Memory->DebugInfo;
        PushText(Group, Assets->Characters, { (int)pGameState->Camera.Position.X, (int)pGameState->Camera.Position.Y + 30,0 }, Text);
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
    
    Platform->OpenGLRender(Group, Target.Header.Width, Target.Header.Height);

    // Clear render group
    ClearEntries(Group);}

