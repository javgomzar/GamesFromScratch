// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"
#include "RenderGroup.h"


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

/*
    TODO:
        - Entity system
*/

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

//void Plot(game_offscreen_buffer* Buffer, v3 Position, color Color) {
//    game_rect ScreenRect;
//    ScreenRect.Left = 0;
//    ScreenRect.Top = 0;
//    ScreenRect.Width = Buffer->Width;
//    ScreenRect.Height = Buffer->Height;
//
//    if (Collision(ScreenRect, Position)) {
//        uint8* PixelMemory = (uint8*)Buffer->Memory + (int)Position.X * Buffer->BytesPerPixel + (int)Position.Y * Buffer->Pitch;
//        uint32* Pixel = (uint32*)PixelMemory;
//        *Pixel = GetColorBytes(Color);
//    }
//}

// UI
/*
    Initialize your UI elements here
*/
void InitializeUI(user_interface* UI) {
    InitializeSlider(&UI->Slider1, V3(900 - 200.0, 180, 0.0), 1.0, 1000);
    InitializeSlider(&UI->Slider2, V3(900 - 180.0, 180, 0.0));
    InitializeSlider(&UI->Slider3, V3(900 - 160.0, 180, 0.0));
    InitializeSlider(&UI->Slider4, V3(900 - 140.0, 180, 0.0));
    InitializeSlider(&UI->Slider5, V3(900 - 120.0, 180, 0.0));
    InitializeSlider(&UI->Slider6, V3(900 - 100.0, 180, 0.0));
}

// Sound
void GameOutputSound(game_assets* Assets, game_sound_buffer* pSoundBuffer, game_state* pGameState, game_input* Input) {
    
    Silence(pSoundBuffer);

    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
    //WriteSineWave(pSoundBuffer, 480, 0);
}

// Video
//void PushVideo(render_group* Group, game_video_id VideoID, game_rect Rect, double SecondsElapsed, double Order = SORT_ORDER_DEBUG_OVERLAY) {
//    game_video* Video = GetAsset(Group->Assets, VideoID);
//    if (!Video->VideoContext.Ended) {
//        Video->TimeElapsed += SecondsElapsed;
//        char Text[256];
//        sprintf_s(Text, "%.02f Time elapsed | %.02f Time played\n", Video->TimeElapsed, Video->VideoContext.PTS * Video->VideoContext.TimeBase);
//        OutputDebugStringA(Text);
//
//        if (Video->TimeElapsed > Video->VideoContext.PTS * Video->VideoContext.TimeBase) {
//            LoadFrame(&Video->VideoContext);
//            Video->VideoContext.Width = Rect.Width;
//            Video->VideoContext.Height = Rect.Height;
//            WriteFrame(&Video->VideoContext);
//        }
//    }
//
//    _PushVideo(Group, Video, Rect, Order);
//}

//void PushVideoLoop(render_group* Group, game_video_id VideoID, game_rect Rect, int Z, double SecondsElapsed, int64_t StartOffset, int64_t EndOffset) {
//
//    PushVideo(Group, VideoID, Rect, Z, SecondsElapsed);
//    game_video* Video = GetAsset(Group->Assets, VideoID);
//    auto& VideoContext = Video->VideoContext;
//    auto& FormatContext = VideoContext.FormatContext;
//    auto& CodecContext = VideoContext.CodecContext;
//    auto& StreamIndex = VideoContext.VideoStreamIndex;
//    auto& PTS = VideoContext.PTS; // Presentation time-stamp (in time-base units)
//
//    if (PTS >= EndOffset) {
//        av_seek_frame(FormatContext, StreamIndex, StartOffset, AVSEEK_FLAG_BACKWARD);
//        do { LoadFrame(&Video->VideoContext); } while (Video->VideoContext.PTS < StartOffset - 1000);
//        Video->TimeElapsed = Video->VideoContext.PTS * Video->VideoContext.TimeBase;
//    }
//}

// Updates

/* 
    Updates Slider. If slider is clicked and dragged, the value of the slider will fluctuate accordingly. Min and max values
    of slider will be enforced.
*/
void Update(slider* Slider, game_input* Input) {
    Slider->Collider = {
        Slider->Position + V3(0.0, 30.0, 0.0),
        20.0,
        80.0
    };

    if (
        Input->Mouse.LeftClick.IsDown && 
        Collide(Slider->Collider, V3(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y, 0.0))
    ) {
        double Range = (Slider->MaxValue - Slider->MinValue) / 60.0;
        Slider->Value = Slider->MaxValue - Range * (Input->Mouse.Cursor.Y - Slider->Position.Y);

        // Min and max values shall not be surpassed
        if (Slider->Value < Slider->MinValue) Slider->Value = Slider->MinValue;
        else if (Slider->Value > Slider->MaxValue) Slider->Value = Slider->MaxValue;
    }
};

void Update(user_interface* UI, game_input* Input) {
    Update(&UI->Slider1, Input);
    Update(&UI->Slider2, Input);
    Update(&UI->Slider3, Input);
    Update(&UI->Slider4, Input);
    Update(&UI->Slider5, Input);
    Update(&UI->Slider6, Input);
}

void Update(camera* Camera, game_input* Input) {
    // Zoom
    if (Input->Mouse.Wheel > 0) Camera->Distance /= 1.2;
    else if (Input->Mouse.Wheel < 0) Camera->Distance *= 1.2;

    // Orbit around Camera->Position
    if (Input->Mouse.MiddleClick.IsDown && Input->Mouse.MiddleClick.WasDown) {
        v3 Offset = Input->Mouse.Cursor - Input->Mouse.LastCursor;
        double AngularVelocity = 0.5;

        Camera->Angle -= AngularVelocity * Offset.X;
        Camera->Pitch += AngularVelocity * Offset.Y;
    }

    v2 Joystic = V2(Input->Controller.RightJoystick.X, Input->Controller.RightJoystick.Y);

    if (modulus(Joystic) > 0.1) {
        Camera->Angle -= 3.0 * Joystic.X;
        Camera->Pitch -= 3.0 * Joystic.Y;
    }

    // Translation
    Camera->Velocity = V3(0, 0, 0);
    if (Input->Keyboard.D.IsDown) {
        Camera->Velocity.X += 1.0;
    }
    else if (Input->Keyboard.A.IsDown) {
        Camera->Velocity.X -= 1.0;
    }
    if (Input->Keyboard.W.IsDown) {
        Camera->Velocity.Z += 1.0;
    }
    else if (Input->Keyboard.S.IsDown) {
        Camera->Velocity.Z -= 1.0;
    }
    if (Input->Keyboard.Space.IsDown) {
        Camera->Velocity.Y += 1.0;
    }
    else if (Input->Keyboard.Shift.IsDown) {
        Camera->Velocity.Y -= 1.0;
    }

    Camera->Basis = GetCameraBasis(Camera->Angle, Camera->Pitch);

    basis HorizontalBasis = GetCameraBasis(Camera->Angle, 0);

    v3 Direction = normalize(Camera->Velocity);
    float Speed = 0.01 * Camera->Distance;
    Camera->Velocity = Speed * (Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z);
    Camera->Position += Camera->Velocity;
}

void LogGameDebugRecords();

#define FourierN 4096

void FourierTransform(
    memory_arena* TransientArena,
    memory_arena* PermanentArena,
    double Freq
) {
    TIMED_BLOCK;

    static double* PlotBuffer = 0;
    static double* DFTModuleBuffer = 0;
    static double* DFTPhaseBuffer = 0;

    if (PlotBuffer == 0) {
        PlotBuffer = PushArray(PermanentArena, FourierN, double);
        DFTModuleBuffer = PushArray(PermanentArena, FourierN, double);
        DFTPhaseBuffer = PushArray(PermanentArena, FourierN, double);
    }
    
    int w = ceil(Freq);
    double Step = w * Tau / (double)(FourierN);
    for (int i = 0; i < FourierN; i ++) {
        PlotBuffer[i] = i % (FourierN / w) < (FourierN / (2*w)) ? 50.0 : -50.0; // Square wave
        //PlotBuffer[i] = 50.0 * sin(i * Step) + 100.0; // Sine wave
        //PlotBuffer[i] = 50.0*w;
    }

    complex Transform[FourierN] = {};

    FFT(TransientArena, Transform, FourierN, PlotBuffer);
    for (int i = 0; i < FourierN; i++) {
        DFTModuleBuffer[i] = 0.02 * modulus(Transform[i]);
        DFTPhaseBuffer[i] = 10.0 * atan(Transform[i].i / Transform[i].r);
    }
}

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    {
    TIMED_BLOCK;

    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    user_interface* UI = &pGameState->UI;
    
    double Time = pGameState->Time;
    camera* Camera = &Group->Camera;

    bool firstFrame = false;
    if (!Memory->IsInitialized) {
        firstFrame = true;

        memory_arena* StringsArena = &pGameState->StringsArena;
        pGameState->TransientArena.Name = PushString(StringsArena, 16, "Transient Arena");
        pGameState->TransientArena.Percentage = PushString(StringsArena, 7, "0.0%");
        pGameState->GeneralPurposeArena.Name = PushString(StringsArena, 24, "General purpose Arena");
        pGameState->GeneralPurposeArena.Percentage = PushString(StringsArena, 7, "0.0%");
        pGameState->StringsArena.Name = PushString(StringsArena, 16, "Strings Arena");
        pGameState->StringsArena.Percentage = PushString(StringsArena, 7, "0.0%");
        pGameState->RenderArena.Name = PushString(StringsArena, 16, "Render Arena");
        pGameState->RenderArena.Percentage = PushString(StringsArena, 7, "0.0%");

        const int N = 8;
        double Test[N] = {50,50,50,50};
        complex Out[N] = {0};
        FFT(&pGameState->TransientArena, Out, N, Test);
        
        // User Interface
        InitializeUI(&pGameState->UI);
        
        // Camera
        Camera->Position = V3(0.0, 3.2, 0.0);
        Camera->Angle = 45;
        Camera->Pitch = 45;
        Camera->Distance = 9.0;
        
        Memory->IsInitialized = true;
    }
    FourierTransform(&pGameState->TransientArena, &pGameState->GeneralPurposeArena, pGameState->UI.Slider1.Value);

    PushClear(Group, { 0 }, World);
    PushClear(Group, { 0 }, Outline);
    PushClear(Group, { 0 }, Postprocessing_Outline);
    PushClear(Group, Color(BackgroundBlue, 1.0), Output);

// Controls
    // Put here your input code

    GameOutputSound(Assets, SoundBuffer, pGameState, Input);

    // Updates
    Update(Camera, Input);

    Update(&pGameState->UI, Input);
    
    // Render
    light LightSource = Light(V3(-0.5, -1, 1), White);

    v3 MeshPosition = V3(0, 0, Group->Camera.Position.Z);
    transform Transform1 = Transform(Quaternion((Group->Camera.Angle + 90) * Degrees, V3(0,1,0)), MeshPosition);
    transform Transform2 = Transform(Quaternion(1.0, 0.0, 0.0, 0.0), V3(-5.0, 0.0, 20.0), Scale(10,1,1));

    PushMesh(Group, Mesh_Body_ID, Transform1, LightSource, Mesh_Shader_Pipeline_ID, Bitmap_Empty_ID, White, SORT_ORDER_MESHES, false);
    PushMesh(Group, Mesh_Sphere_ID, Transform2, LightSource, Sphere_Shader_Pipeline_ID, Bitmap_Empty_ID, Red, SORT_ORDER_MESHES, false);

    //PushHeightmap(Group, Heightmap_Spain_ID, Trochoidal_Shader_Pipeline_ID);

    //PushUI(Group, UI);

    // Debug info
    static double Alpha = 0.0;
    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        Group->Debug = !Group->Debug;
        if (!Group->Debug) {
            Alpha = 0.0;
        }
    }

    if (Group->Debug) {
        if (Alpha < 1.0) {
            double x = (pGameState->dt - 1.8) / 1.1;
            Alpha += exp(- x * x);
        }
        else Alpha = 1.0;

        PushDebugGrid(Group, Alpha);

        game_rect DebugInfoRect = { 0, 0, 350, 250 };

        PushRect(Group, DebugInfoRect, Color(Black, 0.5 * Alpha), SORT_ORDER_DEBUG_OVERLAY);
        PushRectOutline(Group, DebugInfoRect, Color(Gray, Alpha));
        PushText(Group, V2(0, 30.0), GetAsset(Assets, Font_Cascadia_Mono_ID), Memory->DebugInfo, Color(White, Alpha), 12, false, SORT_ORDER_DEBUG_OVERLAY);

        // Debug arenas
        PushDebugArena(Group, pGameState->TransientArena, V2(20.0, 120.0), Alpha);
        PushDebugArena(Group, pGameState->GeneralPurposeArena, V2(20.0, 150.0), Alpha);
        PushDebugArena(Group, pGameState->RenderArena, V2(20.0, 180.0), Alpha);
        PushDebugArena(Group, pGameState->StringsArena, V2(20.0, 210.0), Alpha);

        // Axes
        v3 XAxis = V3(cos(Group->Camera.Angle * Degrees), sin(Group->Camera.Angle * Degrees) * sin(Group->Camera.Pitch * Degrees), 0.0);
        v3 YAxis = V3(0.0, -cos(Group->Camera.Pitch * Degrees), 0.0);
        v3 ZAxis = V3(-sin(Group->Camera.Angle * Degrees), sin(Group->Camera.Pitch * Degrees) * cos(Group->Camera.Angle * Degrees), 0.0);
        v3 AxisOrigin = V3(Group->Width - 0.08 * (double)Group->Height - 10.0, 0.1 * (double)Group->Height, 0);
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, Screen_Coordinates, Red);
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, Screen_Coordinates, Green);
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, Screen_Coordinates, Blue);

        // Debug camera basis
        // PushDebugVector(Group, Group->Camera.Basis.X, V3(0,0,0), World_Coordinates, Yellow);
        // PushDebugVector(Group, Group->Camera.Basis.Y, V3(0,0,0), World_Coordinates, Magenta);
        // PushDebugVector(Group, Group->Camera.Basis.Z, V3(0,0,0), World_Coordinates, Cyan);

        // Debug Framebuffer
        PushDebugFramebuffer(Group, Postprocessing_Outline);
    }

    PushRenderTarget(Group, World);

    static bool Screenshot = false;
    if (Input->Keyboard.F10.WasDown && !Input->Keyboard.F10.IsDown) {
        Screenshot = true;
    }

    static double ScreenRectAlpha = 1.0;
    if (Screenshot) {
        ScreenRectAlpha -= 0.05;
        if (ScreenRectAlpha < 0.0) {
            Screenshot = false;
            ScreenRectAlpha = 1.0;
        }
        else {
            game_rect ScreenRect = { 0, 0, Group->Width, Group->Height };
            PushRect(Group, ScreenRect, Color(White, ScreenRectAlpha), SORT_ORDER_PUSH_RENDER_TARGETS - 5.0);
        }
    }
    PushRenderTarget(Group, Output, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
    }

    LogGameDebugRecords();
}

debug_record DebugRecordArray_GameLibrary[__COUNTER__];

void LogGameDebugRecords() {
    char Buffer[512];
    for (int i = 0; i < ArrayCount(DebugRecordArray_GameLibrary); i++) {
        debug_record* DebugRecord = DebugRecordArray_GameLibrary + i;

        if (DebugRecord->HitCount) {
            if (DebugRecord->HitCount == 1) {
                sprintf_s(Buffer, "%s: (%i hit) %.02f Mcycles (%s:%i)\n", 
                    DebugRecord->FunctionName, DebugRecord->HitCount, DebugRecord->CycleCount / 1000000.0f, DebugRecord->FileName, DebugRecord->LineNumber);
            }
            else {
                sprintf_s(Buffer, "%s: (%i hits) Total: %.02f Mcycles, Average: %.02f (%s:%i)\n", 
                    DebugRecord->FunctionName, DebugRecord->HitCount, DebugRecord->CycleCount / 1000000.0f, 
                    (float)DebugRecord->CycleCount / (1000000.0f * (float)DebugRecord->HitCount), DebugRecord->FileName, DebugRecord->LineNumber);
            }
            Log(Info, Buffer);
            DebugRecord->HitCount = 0;
            DebugRecord->CycleCount = 0;
        }
    }
}