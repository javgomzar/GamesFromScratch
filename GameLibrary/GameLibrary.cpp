// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"

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
    read_file_result ReadResult = PlatformReadEntireFile(FileName);
    if (ReadResult.ContentSize != 0) {
        bitmap_header* Header = (bitmap_header*)ReadResult.Content;
        Result.Header = *Header;
        uint32 BytesPerPixel = Header->BitsPerPixel >> 3;
        Result.BytesPerPixel = BytesPerPixel;
        Result.Pitch = Header->Width * BytesPerPixel;
        Result.Content = (uint32*)((uint8*)ReadResult.Content + Header->BitmapOffset);
        if (Result.Header.BitsPerPixel == 32 && Result.Header.Compression == 3) {
            uint32 AlphaMask = ~(Result.Header.RedMask | Result.Header.GreenMask | Result.Header.BlueMask);
            uint32* Contents = Result.Content;
            
            // If not all Alphas are zero, we need to use them
            for (int32 i = 0; i < Result.Header.Height * Result.Header.Width; i++) {
                if ((*Contents++ & AlphaMask) > 0) {
                    Result.HasAlpha = true;
                    break;
                }
            }
            return Result;
        }
    }
    return Result;
}


    // Fonts
void InitializeFonts(game_memory* Memory) {
    FT_Error error = FT_Init_FreeType(&Memory->FTLibrary);
    if (error) {
        Assert(false);
    }
    else {
        error = FT_New_Face(Memory->FTLibrary, "C:/Windows/Fonts/CascadiaMono.ttf", 0, &Memory->Assets.TestFont);
        if (error == FT_Err_Unknown_File_Format) {
            Assert(false);
        }
        else if (error) {
            Assert(false);
        }
    }
}


    // UI
void InitializeUI(memory_arena* Arena, game_assets* Assets, UI* UserInterface, platform_read_entire_file* Read) {
    UserInterface->TestButton = { 0 };
    UserInterface->TestButton.Collider = {150, 0, 100, 100};
    UserInterface->TestButton.Image = LoadBMP(Read, "..\\GameLibrary\\Media\\Bitmaps\\Button.bmp");
    UserInterface->TestButton.ClickedImage = LoadBMP(Read, "..\\GameLibrary\\Media\\Bitmaps\\ButtonClicked.bmp");
    UserInterface->TestButton.Text.Color = White;
    UserInterface->TestButton.Text.Length = 1;
    UserInterface->TestButton.Text.Points = 16;
    
    UserInterface->TestButton.Face = &Assets->TestFont;

    UserInterface->TestButton.Text.Content = PushArray(Arena, 1, char);
    *UserInterface->TestButton.Text.Content = 'A';
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
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets Assets = Memory->Assets;
    platform_api Platform = Memory->Platform;
    render_group* Group = Memory->Group;
    if (!Memory->IsInitialized) {
        pGameState->MaxCelerity = 20;
        pGameState->PlayerPosition = { 0, 290, 0 };

        // Memory arenas
        InitializeArena(&pGameState->TestArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8*)Memory->PermanentStorage + sizeof(game_state));

        // Assets ----------------------------------------------------------------------------------------------------------------------------------------
        // Load BMPs
        Memory->Assets.PlayerBMP = LoadBMP(Platform.ReadEntireFile, "..\\GameLibrary\\Media\\Bitmaps\\Player.bmp");
        Memory->Assets.BackgroundBMP = LoadBMP(Platform.ReadEntireFile, "..\\GameLibrary\\Media\\Bitmaps\\Background.bmp");

        // Fonts
        InitializeFonts(Memory);
        Memory->IsInitialized = true;

        // User Interface
        pGameState->UserInterface = { 0 };
        InitializeUI(&pGameState->TestArena, &Memory->Assets, &pGameState->UserInterface, Platform.ReadEntireFile);

        Assets = Memory->Assets;

        // Renderer --------------------------------------------------------------------------------------------------------------------------------------
        Memory->Group = AllocateRenderGroup(&pGameState->TestArena, Megabytes(4));
        Group = Memory->Group;
    }

    // Controls
    double dt = 1;
    v3 Position = ToV3(pGameState->PlayerPosition);
    v3 Velocity = pGameState->PlayerVelocity;
    v3 Acceleration = { 0 };
    double AccelerationModule = 0;
    v3 Direction = { 0 };

    if (Input->Keyboard.D.IsDown) {
        Direction.X += 1;
    }
    if (Input->Keyboard.A.IsDown) {
        Direction.X -= 1;
    }
    if (Input->Keyboard.W.IsDown) {
        Direction.Y -= 1;
    }
    if (Input->Keyboard.S.IsDown) {
        Direction.Y += 1;
    }
    Direction = normalize(Direction);

    if (module(Direction) > 0.9) {
        AccelerationModule = 2;
    }
    else {
        if (module(Velocity) > 2.5) {
            AccelerationModule = 5;
            Direction = -normalize(Velocity);
        }
        else {
            Velocity = { 0 };
            AccelerationModule = 0;
        }
    }

    Acceleration = AccelerationModule * Direction;

    Velocity = Velocity + dt * Acceleration;
    Position = Position + dt * Velocity;

    if (module(Velocity) > pGameState->MaxCelerity) {
        Velocity = pGameState->MaxCelerity * Direction;
    }
    
    // Border detection
    if (Position.X < 0) {
        Position.X = 0;
        Velocity.X = 0;
    }
    if (Position.X > (double)ScreenBuffer->Width - Memory->Assets.PlayerBMP.Header.Width) {
        Position.X = (double)ScreenBuffer->Width - Memory->Assets.PlayerBMP.Header.Width;
        Velocity.X = 0;
    }
    if (Position.Y < 0) {
        Position.Y = 0;
        Velocity.Y = 0;
    }
    if (Position.Y > (double)ScreenBuffer->Height - Memory->Assets.PlayerBMP.Header.Height) {
        Position.Y = (double)ScreenBuffer->Height - Memory->Assets.PlayerBMP.Header.Height;
        Velocity.Y = 0;
    }
    pGameState->PlayerPosition = ToScreenPosition(Position);
    pGameState->PlayerVelocity = Velocity;
        
    //GameOutputSound(ScreenBuffer, SoundBuffer, pGameState);
    PushBMP(Group, &Memory->Assets.BackgroundBMP, {0, 0, 0});
    PushBMP(Group, &Memory->Assets.PlayerBMP, pGameState->PlayerPosition);

    // Debug mouse position
    //if (Input->Mouse.LeftClick.IsDown) {
    //    game_rect Rect;
    //    Rect.Top = Input->Mouse.Cursor.Y;
    //    Rect.Left = Input->Mouse.Cursor.X;
    //    Rect.Height = 20;
    //    Rect.Width = 20;
    //    DrawRectangle(ScreenBuffer, Rect, White);
    //}

    static bool ShowDebugInfo = false;
    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        ShowDebugInfo = !ShowDebugInfo;
    }

    if (ShowDebugInfo) {
        text Text = { 0 };
        Text.Color = White;
        Text.Length = 47;
        Text.Points = 20;
        Text.Content = Memory->DebugInfo;
        PushText(Group, &pGameState->TestArena, &Assets.TestFont, { 0,30,0 }, Text);
    }

    char TextBuffer[124];
    sprintf_s(TextBuffer, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
    static int Length = 1;
    static int Counter = 0;

    if (Counter == 124) {
        Counter = 0;
        Length = 0;
    }
    else {
        if (Length < 123) {
            Length++;
        }
        Counter++;
    }

    text Text = { Length, Black, 20, true, TextBuffer };
    PushText(Group, &pGameState->TestArena, &Assets.TestFont, { 400,150,0 }, Text);

    // Update clicks
    pGameState->UserInterface.TestButton.Clicked = Input->Mouse.LeftClick.IsDown && Collision(pGameState->UserInterface.TestButton.Collider, Input->Mouse.Cursor);
    PushButton(Group, &pGameState->TestArena, pGameState->UserInterface.TestButton);

    // Render
    loaded_bmp Target = { 0 };
    Target.Header.Width = ScreenBuffer->Width;
    Target.Header.Height = ScreenBuffer->Height;
    Target.Pitch = ScreenBuffer->Pitch;
    Target.Content = (uint32*)ScreenBuffer->Memory;
    RenderGroupToOutput(Group, &Target);

    // Clear render group
    ClearEntries(Group);
}

