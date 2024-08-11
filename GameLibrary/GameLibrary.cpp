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

void Initialize(entity* Entity, v3 Position = V3(0,0,0), v3 Velocity = V3(0,0,0)) {
    Entity->Position = Position;
    Entity->Velocity = Velocity;
    Entity->Time = 0;
}

void RestoreDefault(player_bone* Bone) {
    basis Basis = Identity(1.0);
    switch (Bone->Id) {
        case Head:
        {
            Bone->Bone.BMPOffset = V3(-28.0, -66.0, 0.0);
            Bone->Bone.Start = V3(30, 70, 12);
            Bone->Bone.Finish = V3(30, 35, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case Spine:
        {
            Bone->Bone.BMPOffset = V3(-28.0, -68.0, 0.0);
            Bone->Bone.Start = V3(30, 120, 10);
            Bone->Bone.Finish = V3(30, 70, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case LeftShoulder:
        {
            Bone->Bone.Start = V3(30, 70, 0);
            Bone->Bone.Finish = V3(10, 70, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case RightShoulder:
        {
            Bone->Bone.Start = V3(30, 70, 0);
            Bone->Bone.Finish = V3(50, 70, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case LeftArm:
        {
            Bone->Bone.BMPOffset = V3(-8.0, -6.0, 0.0);
            Bone->Bone.Start = V3(10, 70, 15);
            Bone->Bone.Finish = V3(10, 135, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case RightArm:
        {
            Bone->Bone.BMPOffset = V3(-8.0, -6.0, 0.0);
            Bone->Bone.Start = V3(50, 70, 12);
            Bone->Bone.Finish = V3(50, 135, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case LeftHip:
        {
            Bone->Bone.Start = V3(30, 120, 0);
            Bone->Bone.Finish = V3(24, 120, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case RightHip:
        {
            Bone->Bone.Start = V3(30, 120, 0);
            Bone->Bone.Finish = V3(36, 120, 0);
            Bone->Bone.Basis = Basis;

        } break;
        case LeftLeg:
        {
            Bone->Bone.BMPOffset = V3(-10.0, 6.0, 0.0);
            Bone->Bone.Start = V3(24, 122, 12);
            Bone->Bone.Finish = V3(24, 180, 0);
            Bone->Bone.Basis = Basis;

        } break;
        case RightLeg:
        {
            Bone->Bone.BMPOffset = V3(-10.0, 6.0, 0.0);
            Bone->Bone.Start = V3(36, 122, 6);
            Bone->Bone.Finish = V3(36, 180, 0);
            Bone->Bone.Basis = Basis;
        } break;
        case Sword:
        {
            Bone->Bone.BMPOffset = V3(154, -20, 0);
            Bone->Bone.Start = V3(50, 135, 5);
            Bone->Bone.Finish = V3(200, 135, 0);
            Bone->Bone.Basis = Rotate(Basis, -Tau / 4.0);
        } break;
    }
}

void InitializeSkeleton(player* Player, basis Basis, game_assets* Assets) {
    PlayerBone(LeftHip, &Player->LeftHip, 0, 0, V3(0, 0, 0), V3(30, 120, 0), V3(24, 120, 0), Basis);
    PlayerBone(RightHip, &Player->RightHip, 0, &Player->LeftHip, V3(0, 0, 0), V3(30, 120, 0), V3(36, 120, 0), Basis);
    PlayerBone(LeftLeg, &Player->LeftLeg, &Assets->LegBMP, &Player->LeftHip, V3(-10.0, 6.0, 0), V3(24, 122, 12), V3(24, 180, 0), Basis);
    PlayerBone(RightLeg, &Player->RightLeg, &Assets->LegBMP, &Player->RightHip, V3(-10.0, 6.0, 0), V3(36, 122, 6), V3(36, 180, 0), Basis);
    PlayerBone(Spine, &Player->Spine, &Assets->TorsoBMP, &Player->LeftHip, V3(-28.0, -68.0, 0), V3(30, 120, 10), V3(30, 70, 0), Basis);
    PlayerBone(Head, &Player->Head, &Assets->HeadBMP, &Player->Spine, V3(-28.0, -66.0, 12.0), V3(30, 70, 0), V3(30, 35, 0), Basis);
    PlayerBone(LeftShoulder, &Player->LeftShoulder, 0, &Player->Spine, V3(0, 0, 0), V3(30, 70, 0), V3(10, 70, 0), Basis);
    PlayerBone(RightShoulder, &Player->RightShoulder, 0, &Player->Spine, V3(0, 0, 0), V3(30, 70, 0), V3(50, 70, 0), Basis);
    PlayerBone(LeftArm, &Player->LeftArm, &Assets->ArmBMP, &Player->LeftShoulder, V3(-8.0, -6.0, 0), V3(10, 70, 15), V3(10, 135, 0), Basis);
    PlayerBone(RightArm, &Player->RightArm, &Assets->ArmBMP, &Player->RightShoulder, V3(-8.0, -6.0, 0), V3(50, 70, 12), V3(50, 135, 0), Basis, true);
    PlayerBone(Sword, &Player->Sword, &Assets->SwordBMP, &Player->RightArm, V3(154, -20, 0), V3(50, 135, 5), V3(200, 135, 0), Rotate(Basis, -Tau / 4.0), true);
}

void Animate(player* Player, game_input* Input, double dt) {
    double* Time = &Player->Entity.Time;
    player_animation* CurrentAnimation = &Player->Animation;

    switch (*CurrentAnimation) {
        case Player_Idle:
        {
            *Time = 0;
        } break;

        case Player_Attacking:
        {
            if (*Time < 1.0) {
                Rotate(&Player->Spine.Bone, 0.02 * cos(10.0 * *Time));
                Rotate(&Player->LeftLeg.Bone, 0.02 * -cos(10.0 * *Time));
                Rotate(&Player->RightLeg.Bone, 0.02 * cos(10.0 * *Time));
                Rotate(&Player->LeftArm.Bone, 0.02 * cos(10.0 * *Time));
                Rotate(&Player->RightArm.Bone, 0.03 * -cos(10.0 * *Time));
                Player->Entity.Position = Player->Entity.Position + 300.0 * dt * V3(1.0, 0.0, 0.0);
            }
            
            static bool Swinging = false;
            if (1.0 < *Time && *Time < 2.0) {
                if (Swinging) Rotate(&Player->RightArm.Bone, 0.05);
                else { 
                    Swinging = true;
                    RestoreDefault(&Player->Head);
                    RestoreDefault(&Player->Spine);
                    RestoreDefault(&Player->LeftArm);
                    RestoreDefault(&Player->RightArm);
                    RestoreDefault(&Player->LeftLeg);
                    RestoreDefault(&Player->RightLeg);
                    RestoreDefault(&Player->LeftShoulder);
                    RestoreDefault(&Player->RightShoulder);
                }
            }

            if (2.0 < *Time && *Time < 2.3) {
                Rotate(&Player->RightArm.Bone, -0.15);
            }

            static bool Returning = false;
            if (*Time > 2.5) {
                if (Returning) {
                    Rotate(&Player->Spine.Bone, 0.02 * cos(10.0 * *Time));
                    Rotate(&Player->LeftLeg.Bone, 0.02 * -cos(10.0 * *Time));
                    Rotate(&Player->RightLeg.Bone, 0.02 * cos(10.0 * *Time));
                    Rotate(&Player->LeftArm.Bone, 0.02 * cos(10.0 * *Time));
                    Rotate(&Player->RightArm.Bone, 0.03 * -cos(10.0 * *Time));
                    Player->Entity.Position = Player->Entity.Position - 300.0 * dt * V3(1.0, 0.0, 0.0);
                }
                else {
                    Returning = true;
                    RestoreDefault(&Player->RightArm);
                    RestoreDefault(&Player->Sword);
                    Flip(&Player->Head.Bone, true);
                    Flip(&Player->LeftLeg.Bone, true);
                    Flip(&Player->RightLeg.Bone, true);
                    Flip(&Player->Sword.Bone, false, true);
                    Flip(&Player->Spine.Bone, true);
                    RemoveChild(&Player->RightArm.Bone, &Player->Sword.Bone);
                    AddChild(&Player->LeftArm.Bone, &Player->Sword.Bone);
                    Player->Sword.Bone.Start = Player->LeftArm.Bone.Finish;
                    Player->Sword.Bone.Finish = Player->LeftArm.Bone.Finish - Player->Sword.Bone.Length * V3(1,0,0);
                    Player->Sword.Bone.BMPOffset.X = 24.0;
                }
            }

            *Time += dt;

            if (*Time > 3.5) {
                RestoreDefault(&Player->Head);
                RestoreDefault(&Player->Spine);
                RestoreDefault(&Player->LeftArm);
                RestoreDefault(&Player->RightArm);
                RestoreDefault(&Player->LeftLeg);
                RestoreDefault(&Player->RightLeg);
                RestoreDefault(&Player->Sword);
                RestoreDefault(&Player->LeftShoulder);
                RestoreDefault(&Player->RightShoulder);
                RemoveChild(&Player->LeftArm.Bone, &Player->Sword.Bone);
                AddChild(&Player->RightArm.Bone, &Player->Sword.Bone);
                Flip(&Player->Head.Bone, true);
                Flip(&Player->LeftLeg.Bone, true);
                Flip(&Player->RightLeg.Bone, true);
                Flip(&Player->Sword.Bone, false, true);
                Flip(&Player->Spine.Bone, true);

                Player->Entity.Position = V3(300, 150, 1);
  
                *CurrentAnimation = Player_Idle;
                Returning = false;
                Swinging = false;
                Player->Busy = false;
            }
        } break;

        case Player_Walking:
        {
            Rotate(&Player->Spine.Bone, 0.02 * cos(10.0 * *Time));
            Rotate(&Player->LeftLeg.Bone, 0.02 * -cos(10.0 * *Time));
            Rotate(&Player->RightLeg.Bone, 0.02 * cos(10.0 * *Time));
            Rotate(&Player->LeftArm.Bone, 0.02 * cos(10.0 * *Time));
            Rotate(&Player->RightArm.Bone, 0.02 * -cos(10.0 * *Time));
            *Time += dt;
        } break;
    }
}

void Animate(enemy* Enemy, double dt) {
    double* Time = &Enemy->Entity.Time;
    switch (Enemy->Animation) {
        case Enemy_Idle:
        {
            static int Count = 0;
            int Frames = 8;
            if (Count > 3 * Frames) {
                Count = 0;
            }
            else if (Count > 2 * Frames) {
                Enemy->BMP = Enemy->BMP3;
            }
            else if (Count > Frames) {
                Enemy->BMP = Enemy->BMP2;
            }
            else {
                Enemy->BMP = Enemy->BMP1;
            }
            Count++;
            Enemy->Entity.Position.Y = 80 + 20 * sin(Tau / 2.0 * *Time);
            *Time += dt;
        } break;

        case Enemy_Attacking:
        {
            if (*Time > 1.0) {
                Enemy->Entity.Position.X = 700;
                Enemy->Entity.Time = 0;
                Enemy->Animation = Enemy_Idle;
                Enemy->Busy = false;
            }
            if (*Time > 0.5) {
                Enemy->Entity.Position.X += 800.0 * dt;
            }
            else {
                Enemy->Entity.Position.Y = 80;
                Enemy->Entity.Position.X -= 800.0 * dt;
            }

            *Time += dt;
        } break;

        case Enemy_Defending:
        {

        } break;
    }
}

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    double* Time = &pGameState->Time;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    UI* UserInterface = &pGameState->UserInterface;
    player* Player = &pGameState->Player;
    enemy* Enemy = &pGameState->Enemy;
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

        // BMPs
        Assets->HeadBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Head.bmp");
        Assets->ArmBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Arm.bmp");
        Assets->TorsoBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Torso.bmp");
        Assets->LegBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Leg.bmp");
        Assets->EnemyBMP1 = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Enemy1.bmp");
        Assets->EnemyBMP2 = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Enemy2.bmp");
        Assets->EnemyBMP3 = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Enemy3.bmp");
        Assets->EnemyBMP4 = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Enemy4.bmp");
        Assets->SwordBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Sword.bmp");

        // Game state
            // Player
        Initialize(&pGameState->Player.Entity, V3(300, 150, 1));
        InitializeSkeleton(&pGameState->Player, Group->DefaultBasis, Assets);
        Player->Busy = false;
        stats* PlayerStats = &pGameState->Player.Stats;
        PlayerStats->HP = 100;
        PlayerStats->MaxHP = 100;
        PlayerStats->Strength = 10;
        PlayerStats->Defense = 5;
        PlayerStats->Speed = 20;
        PlayerStats->ATB = 100;

            // Enemy
        enemy* Enemy = &pGameState->Enemy;
        Enemy->Busy = false;
        Enemy->BMP1 = &Assets->EnemyBMP1;
        Enemy->BMP2 = &Assets->EnemyBMP2;
        Enemy->BMP3 = &Assets->EnemyBMP3;
        Enemy->BMP4 = &Assets->EnemyBMP4;
        Enemy->BMP = Enemy->BMP1;
        Enemy->Entity.Position = V3(700, 70, 0);
        Enemy->Entity.Time = 0;
        stats* EnemyStats = &Enemy->Stats;
        EnemyStats->HP = 100;
        EnemyStats->MaxHP = 100;
        EnemyStats->Strength = 10;
        EnemyStats->Defense = 5;
        EnemyStats->Speed = 12;
        EnemyStats->ATB = 100;

        // User Interface
            // Initialize your UI elements here
        // Combat menu
        combat_menu* CombatMenu = &UserInterface->CombatMenu;
        CombatMenu->Active = true;
        CombatMenu->Cursor = 0;
        CombatMenu->AttackText = PushString(&pGameState->TextArena, 7, "Attack");
        CombatMenu->TechniqueText = PushString(&pGameState->TextArena, 10, "Technique");
        CombatMenu->MagicText = PushString(&pGameState->TextArena, 6, "Magic");
        CombatMenu->ItemsText = PushString(&pGameState->TextArena, 6, "Items");

        // Turn queue
        turn_queue* TurnQueue = &UserInterface->TurnQueue;
        TurnQueue->Time = 0;
        TurnQueue->Combatants = 2;
        TurnQueue->ShowTurns = 6;
        TurnQueue->BMPs[0] = &Assets->HeadBMP;
        TurnQueue->BMPs[1] = &Assets->EnemyBMP1;
        TurnQueue->BMPOffsets[0] = V3(0.0, 3.0, 0.0);
        TurnQueue->BMPOffsets[1] = V3(0, 15.0, 0);
        TurnQueue->Scales[0] = 2.0;
        TurnQueue->Scales[1] = 1.0;
        TurnQueue->RectOffsets[0] = V3(15.0, 0.0, 0.0);
        TurnQueue->RectOffsets[1] = V3(6.0, 0.0, 0.0);
        TurnQueue->Stats[0] = &Player->Stats;
        TurnQueue->Stats[1] = &Enemy->Stats;

        UpdateQueue(&UserInterface->TurnQueue);
        TurnQueue->CurrentTurn = TurnQueue->Queue[0];

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

    if (Input->Keyboard.PageDown.IsDown && !Input->Keyboard.PageDown.WasDown) {
        NextTurn(&UserInterface->TurnQueue);
    }

    // Attack if it's the enemy's turn
    if (UserInterface->TurnQueue.CurrentTurn == 0) {
        Player->Busy = true;
    }
    if ((UserInterface->TurnQueue.CurrentTurn == 1) && !pGameState->Enemy.Busy) {
        pGameState->Enemy.Busy = true;
        pGameState->Enemy.Entity.Time = 0;
        pGameState->Enemy.Animation = Enemy_Attacking;
        Attack(pGameState->Enemy.Stats, &Player->Stats);
    }

    if (Input->Keyboard.Enter.IsDown && !Input->Keyboard.Enter.WasDown && 
        UserInterface->CombatMenu.Cursor == 0 &&
        !Enemy->Busy) {
        Attack(pGameState->Player.Stats, &pGameState->Enemy.Stats);
        Player->Busy = true;
        Player->Animation = Player_Attacking;
    }

    if (pGameState->ShowDebugInfo) {
        game_rect DebugInfoRect = { 0, 0, 300, 220 };
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
    PushHealthBar(Group, { Player->Entity.Position.X - 15, Player->Entity.Position.Y - 20, 0 }, pGameState->Player.Stats.HP, pGameState->Player.Stats.MaxHP);
    Animate(Player, Input, pGameState->dt);
    PushSkeleton(Group, 11, Player->Skeleton, Player->Entity.Position, pGameState->ShowDebugInfo);

        // Enemy
    PushHealthBar(Group, { 750, 50, 0 }, pGameState->Enemy.Stats.HP, pGameState->Enemy.Stats.MaxHP);
    Animate(&pGameState->Enemy, pGameState->dt);
    game_rect EnemyRect;
    EnemyRect.Left = pGameState->Enemy.Entity.Position.X;
    EnemyRect.Top = pGameState->Enemy.Entity.Position.Y;
    EnemyRect.Width = 200;
    EnemyRect.Height = 200;
    PushTexturedRectClamp(Group, Enemy->BMP, EnemyRect);
    color ShadowColor;
    ShadowColor.R = 0.1;
    ShadowColor.G = 0.1;
    ShadowColor.B = 0.1;
    ShadowColor.Alpha = 0.5;
    PushCircle(Group, V3(pGameState->Enemy.Entity.Position.X + 100, 320, 0), 50, ShadowColor, Scale(Identity(), 1.0, 0.3, 1.0));

    if (!Player->Busy && !pGameState->Enemy.Busy) {
        NextTurn(&UserInterface->TurnQueue);
    }

        // Combat menu
    PushCombatMenu(Group, Assets->Characters, &pGameState->UserInterface.CombatMenu);

        // Turn queue
    PushTurnQueue(Group, &UserInterface->TurnQueue);

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

