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

// +------------------------------------------------------------------------------------------------------------------+
// |  Render graphics                                                                                                 |
// +------------------------------------------------------------------------------------------------------------------+
//void RenderWhiteNoise(game_offscreen_buffer* Buffer) {
//    uint8* Row = (uint8*)Buffer->Memory;
//    for (int Y = 0; Y < Buffer->Height; ++Y) {
//        uint32* Pixel = (uint32*)Row;
//        for (int X = 0; X < Buffer->Width; ++X) {
//            uint8 Gray = rand() % 255;
//            uint8 Red = Gray;
//            uint8 Green = Gray;
//            uint8 Blue = Gray;
//
//            uint32 RGB_color = (Red << 16) | (Green << 8) | Blue;
//
//            *Pixel++ = RGB_color;
//        }
//        Row += Buffer->Pitch;
//    }
//}

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

// Bitmaps
loaded_bmp LoadBMP(platform_read_entire_file* PlatformReadEntireFile, const char* FileName) {
    loaded_bmp Result = { 0 };
    read_file_result ReadResult = PlatformReadEntireFile(FileName);
    if (ReadResult.ContentSize != 0) {
        bitmap_header* Header = (bitmap_header*)ReadResult.Content;
        Result.Header = *Header;
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

void BlitBMP(loaded_bmp* BMP, game_offscreen_buffer* Buffer, game_screen_position Position) {
    int32 BMPWidth = BMP->Header.Width;
    int32 BMPHeight = BMP->Header.Height;

    int32 BufferWidth = (int32)Buffer->Width;
    int32 BufferHeight = (int32)Buffer->Height;
    
    if (Position.X + BMPWidth > 0 && Position.X < BufferWidth &&
        Position.Y + BMPHeight > 0 && Position.Y < BufferHeight) {
        int32 BlitWidth;
        int32 BlitHeight;
        game_screen_position SourcePosition;
        game_screen_position DestinationPosition;
        
        // Cropping
        if (Position.X < 0) {
            BlitWidth = BMPWidth + Position.X;
            SourcePosition.X = -Position.X;
            DestinationPosition.X = 0;
        }
        else {
            SourcePosition.X = 0;
            DestinationPosition.X = Position.X;
            if (Position.X + BMPWidth > Buffer->Width) {
                BlitWidth = Buffer->Width - Position.X;
            }
            else {
                BlitWidth = BMPWidth;
            }
        }

        if (Position.Y < 0) {
            BlitHeight = BMPHeight + Position.Y;
            SourcePosition.Y = -Position.Y;
            DestinationPosition.Y = 0;
        }
        else {
            SourcePosition.Y = 0;
            DestinationPosition.Y = Position.Y;
            if (Position.Y + BMPHeight > Buffer->Height) {
                BlitHeight = Buffer->Height - Position.Y;
            }
            else {
                BlitHeight = BMPHeight;
            }
        }

        // BMP starts on the last row
        uint32* SourceRow = BMP->Content + BMP->Header.Width * (BMP->Header.Height - SourcePosition.Y - 1) + SourcePosition.X;
        uint8* DestinationRow = (uint8*)((uint32*)Buffer->Memory + DestinationPosition.X) + DestinationPosition.Y * Buffer->Pitch;
        for (int32 Y = 0; Y < BlitHeight; Y++) {
            uint32* Destination = (uint32*)DestinationRow;
            uint32* Source = SourceRow;
            for (int32 X = 0; X < BlitWidth; X++) {
                color BMPColor = GetColor(*Source++, BMP->Header.RedMask, BMP->Header.GreenMask, BMP->Header.BlueMask);
                color BackgroundColor = GetColor(*Destination, 0x00ff0000, 0x0000ff00, 0x000000ff);

                // DEBUG: BMPColor.Alpha = 255;
                color BlitColor = BMP->HasAlpha ? Blend(BMPColor, BackgroundColor) : BMPColor;
                // DEBUG: BMP->HasAlpha = true;
                
                *Destination++ = GetColorBytes(BlitColor);
            }
            SourceRow -= BMP->Header.Width;
            DestinationRow += Buffer->Pitch;
        }
    }
}

void ClearBitmap(loaded_bmp* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = Bitmap->Header.Width * Bitmap->Header.Height * 32;
        ZeroSize(TotalBitmapSize, Bitmap->Content);
    }
}

loaded_bmp MakeEmptyBitmap(memory_arena* Arena, int32 Width, int32 Height, bool ClearToZero = true) {
    loaded_bmp Result;
    Result.Header = { 0 };
    Result.Header.Width = Width;
    Result.Header.Height = Height;
    Result.Header.BitsPerPixel = 32;
    int32 TotalBitmapSize = Width * Height * 32;
    Result.Header.FileSize = TotalBitmapSize;

    Result.Header.RedMask = 0x00ff0000;
    Result.Header.GreenMask = 0x0000ff00;
    Result.Header.BlueMask = 0x000000ff;
    Result.AlphaMask = 0xff000000;
    Result.HasAlpha = true;

    Result.Content = (uint32*)PushSize(Arena, TotalBitmapSize / 8);
    if (ClearToZero) {
        ClearBitmap(&Result);
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

void LoadFTBMP(FT_Bitmap* SourceBMP, loaded_bmp* DestBMP, text_options Options) {
    uint32* DestRow = DestBMP->Content + DestBMP->Header.Width*(DestBMP->Header.Height-1);
    uint8* Source = SourceBMP->buffer;
    for (int Y = 0; Y < SourceBMP->rows; Y++) {
        uint32* Pixel = DestRow;
        for (int X = 0; X < SourceBMP->width; X++) {
            color Color = { *Source++, Options.Color.R, Options.Color.G, Options.Color.B };
            uint32 Out = GetColorBytes(Color);
            *Pixel++ = Out;
        }
        DestRow -= SourceBMP->pitch;
    }
}

void RenderText(memory_arena* Arena, FT_Face* Face, game_offscreen_buffer* Buffer, game_screen_position Position, text_options Options, char* Text) {
    FT_Error error;

    error = FT_Set_Char_Size(*Face, 0, Options.Points * 64, 128, 128);
    if (error) {
        Assert(false);
    }
    else {
        FT_GlyphSlot Slot = (*Face)->glyph;
        int PenX = Position.X;
        int PenY = Position.Y;
        for (int i = 0; i < Options.Length; i++) {
            error = FT_Load_Char(*Face, Text[i], FT_LOAD_RENDER);
            if (error) {
                Assert(false);
            }
            
            // Line jumps
            if (Text[i] == '\n') {
                PenY += (int)(0.01875f * (float)Slot->metrics.height); // 0.01875 = 1.2 / 64 : height is in 64ths of pixel
                PenX = Position.X;
            }
            else {
                FT_Bitmap FTBMP = Slot->bitmap;
                loaded_bmp BMP = MakeEmptyBitmap(Arena, FTBMP.width, FTBMP.rows, true);
                LoadFTBMP(&FTBMP, &BMP, Options);
                BlitBMP(&BMP, Buffer, { PenX + Slot->bitmap_left, PenY - Slot->bitmap_top, 0 });
                PopSize(Arena, BMP.Header.FileSize / 8);
                PenX += Slot->advance.x >> 6;
            }
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
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets Assets = Memory->Assets;
    platform_api Platform = Memory->Platform;
    if (!Memory->IsInitialized) {
        pGameState->MaxCelerity = 20;
        pGameState->PlayerPosition = { 0, 290, 0 };

        // Memory arenas
        InitializeArena(&pGameState->TestArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8*)Memory->PermanentStorage + sizeof(game_state));

        // Load BMPs
        Memory->Assets.PlayerBMP = LoadBMP(Platform.ReadEntireFile, "..\\GameLibrary\\Media\\Bitmaps\\Player.bmp");
        Memory->Assets.BackgroundBMP = LoadBMP(Platform.ReadEntireFile, "..\\GameLibrary\\Media\\Bitmaps\\Background.bmp");

        // Fonts
        InitializeFonts(Memory);
        Memory->IsInitialized = true;

        Assets = Memory->Assets;
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
    BlitBMP(&Memory->Assets.BackgroundBMP, ScreenBuffer, {0, 0, 0});
    BlitBMP(&Memory->Assets.PlayerBMP, ScreenBuffer, pGameState->PlayerPosition);

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
        text_options Options = { 0 };
        Options.Color = White;
        Options.Length = 47;
        Options.Points = 20;
        RenderText(&pGameState->TestArena, &Assets.TestFont, ScreenBuffer, { 0,30,0 }, Options, Memory->DebugInfo);
    }
}

