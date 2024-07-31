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

// Game code
void Attack(stats Attacker, stats* Defender) {
    if (Attacker.Strength > Attacker.Defense) {
        int NewHP = Defender->HP - Attacker.Strength + Attacker.Defense;
        if (NewHP < 0) {
            Defender->HP = 0;
        }
        else {
            Defender->HP = NewHP;
        }
    }
}

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    UI* UserInterface = &pGameState->UserInterface;
    //render_group* Group = Memory->Group;
    static video_context VideoContext = { 0 };
    bool firstFrame = false;

    if (!Memory->IsInitialized) {
        firstFrame = true;

        // Assets ----------------------------------------------------------------------------------------------------------------------------------------
        // Load your assets here
        Assets->RenderArenaStr = PushString(&pGameState->RenderArena, 13, "Render Arena");
        Assets->RenderPercentageStr = PushString(&pGameState->TextArena, 7, "0.0%");
        Assets->VideoArenaStr = PushString(&pGameState->VideoArena, 13, "Video Arena");
        Assets->VideoPercentageStr = PushString(&pGameState->TextArena, 7, "0.0%");
        Assets->TextArenaStr = PushString(&pGameState->TextArena, 13, "Text Arena");
        Assets->TextPercentageStr = PushString(&pGameState->TextArena, 7, "0.0%");
        Assets->PlayerBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Player.bmp");
        Assets->EnemyBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Enemy.bmp");

        // Game state
            // Player
        pGameState->Player.BMP = &Assets->PlayerBMP;
        stats* PlayerStats = &pGameState->Player.Stats;
        PlayerStats->HP = 100;
        PlayerStats->MaxHP = 100;
        PlayerStats->Strength = 10;
        PlayerStats->Defense = 5;
        PlayerStats->Speed = 10;

            // Enemy
        pGameState->Enemy.BMP = &Assets->EnemyBMP;
        pGameState->Enemy.Attacking = false;
        stats* EnemyStats = &pGameState->Enemy.Stats;
        EnemyStats->HP = 100;
        EnemyStats->MaxHP = 100;
        EnemyStats->Strength = 10;
        EnemyStats->Defense = 5;
        EnemyStats->Speed = 10;
        

        // User Interface
            // Initialize your UI elements here
        UserInterface->CombatMenu.Active = true;
        UserInterface->CombatMenu.Cursor = 0;
        UserInterface->CombatMenu.AttackText = PushString(&pGameState->TextArena, 7, "Attack");
        UserInterface->CombatMenu.TechniqueText = PushString(&pGameState->TextArena, 10, "Technique");
        UserInterface->CombatMenu.MagicText = PushString(&pGameState->TextArena, 6, "Magic");

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

    // Input
        // Debug Info
    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        pGameState->ShowDebugInfo = !pGameState->ShowDebugInfo;
    }

        // Combat menu
    if (Input->Keyboard.Down.IsDown && !Input->Keyboard.Down.WasDown) {
        if (pGameState->UserInterface.CombatMenu.Cursor < 3) {
            pGameState->UserInterface.CombatMenu.Cursor += 1;
        }
    }
    if (Input->Keyboard.Up.IsDown && !Input->Keyboard.Up.WasDown) {
        if (pGameState->UserInterface.CombatMenu.Cursor > 0) {
            pGameState->UserInterface.CombatMenu.Cursor -= 1;
        }
    }

    static int Counter = 0;
    if (Input->Keyboard.Enter.IsDown && !Input->Keyboard.Enter.WasDown && 
        UserInterface->CombatMenu.Cursor == 0 &&
        !pGameState->Enemy.Attacking) {
        Attack(pGameState->Player.Stats, &pGameState->Enemy.Stats);
        pGameState->Enemy.Attacking = true;
    }

    if (pGameState->Enemy.Attacking) {
        Counter++;
        if (Counter == 100) {
            Attack(pGameState->Enemy.Stats, &pGameState->Player.Stats);
            Counter = 0;
            pGameState->Enemy.Attacking = false;
        }
    }

    if (pGameState->ShowDebugInfo) {
        game_rect DebugInfoRect = { 0, 0, 350, 220 };
        PushRect(Group, DebugInfoRect, {0.5, 0.0, 0.0, 0.0}, 980);
        PushRectOutline(Group, DebugInfoRect, Gray);
        PushText(Group, { 0,30,981 }, Assets->Characters, White, 12, Memory->DebugInfo, false);

        // Render Arena
        double RenderArenaPercentage = (double)pGameState->RenderArena.Used / (double)pGameState->RenderArena.Size;
        double RenderGroupPercentage = (double)Group->PushBufferSize / (double)Group->MaxPushBufferSize;
        PushRect(Group, { 20.0, 120.0, 100.0, 20.0 }, DarkGray, 981);
        PushRect(Group, { 20.0, 120.0, 100.0 * RenderArenaPercentage, 20.0 }, Gray, 982);
        PushRect(Group, { 20.0, 120.0, 100.0 * RenderGroupPercentage * RenderArenaPercentage, 20.0 }, Red, 983);
        PushText(Group, { 20.0, 135.0, 985}, Assets->Characters, White, 8, Assets->RenderArenaStr, false);
        sprintf_s(Assets->RenderPercentageStr.Content, 7, "%.02f%%", RenderGroupPercentage);
        PushText(Group, { 125.0, 135.0, 983 }, Assets->Characters, White, 8, Assets->RenderPercentageStr, false);

        // Video Arena
        double VideoArenaPercentage = (double)pGameState->VideoArena.Used / (double)pGameState->VideoArena.Size;
        PushRect(Group, { 20.0, 150.0, 100.0, 20.0 }, DarkGray, 981);
        PushRect(Group, { 20.0, 150.0, 100.0 * VideoArenaPercentage, 20.0 }, Red, 982);
        PushText(Group, { 20.0, 165.0, 983}, Assets->Characters, White, 8, Assets->VideoArenaStr, false);
        sprintf_s(Assets->VideoPercentageStr.Content, 7, "%.02f%%", VideoArenaPercentage*100.0);
        PushText(Group, { 125.0, 135.0 + 30.0, 983 }, Assets->Characters, White, 8, Assets->VideoPercentageStr, false);

        // Text Arena
        double TextArenaPercentage = (double)pGameState->TextArena.Used / (double)pGameState->TextArena.Size;
        PushRect(Group, { 20.0, 180.0, 100.0, 20.0 }, DarkGray, 981);
        PushRect(Group, { 20.0, 180.0, 100.0 * TextArenaPercentage, 20.0 }, Red, 982);
        PushText(Group, { 20.0, 195.0, 983}, Assets->Characters, White, 8, Assets->TextArenaStr, false);
        sprintf_s(Assets->TextPercentageStr.Content, 7, "%.02f%%", TextArenaPercentage*100.0);
        PushText(Group, { 125.0, 135.0 + 60.0, 983 }, Assets->Characters, White, 8, Assets->TextPercentageStr, false);
    }

    // Render
        // Player
    game_screen_position PlayerPosition = { 300, 150, 0 };
    PushTexturedRect(Group, { PlayerPosition.X, PlayerPosition.Y, 64, 188 }, &Assets->PlayerBMP, 0);
    PushHealthBar(Group, { PlayerPosition.X - 15, PlayerPosition.Y - 20, 0 }, pGameState->Player.Stats.HP, pGameState->Player.Stats.MaxHP);

        // Enemy
    game_screen_position EnemyPosition = { 700, 70, 0 };
    PushTexturedRect(Group, { EnemyPosition.X, EnemyPosition.Y + 20 * sin(4.0 * pGameState->Time), 200, 200}, &Assets->EnemyBMP, 0);
    PushHealthBar(Group, { EnemyPosition.X + 50, EnemyPosition.Y - 20, 0 }, pGameState->Enemy.Stats.HP, pGameState->Enemy.Stats.MaxHP);

        // Combat menu
    PushCombatMenu(Group, Assets->Characters, &pGameState->UserInterface.CombatMenu);

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

