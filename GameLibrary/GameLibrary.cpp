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
void InitializeUI(memory_arena* Arena, game_assets* Assets, UI* UserInterface, platform_read_entire_file* Read) {
    // Initialize your UI elements here
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
void Update(slider* Slider, game_input* Input, v3 Position) {
    Slider->Position = Position;
    Slider->Collider = {
        Slider->Position + V3(0.0, 30.0, 0.0),
        20.0,
        80.0
    };

    if (Collide(Slider->Collider, V3(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y, 0.0)) &&
        Input->Mouse.LeftClick.IsDown) {
        Slider->Value = 1 - (Input->Mouse.Cursor.Y - Slider->Position.Y) / 60.0;
        if (Slider->Value < 0.0) Slider->Value = 0.0;
        else if (Slider->Value > 1.0) Slider->Value = 1.0;
    }
};

void Update(UI* UserInterface, game_input* Input, int Width, int Height) {
    Update(&UserInterface->Slider1, Input, V3(Width - 200.0, 0.4 * Height, 0.0));
    Update(&UserInterface->Slider2, Input, V3(Width - 180.0, 0.4 * Height, 0.0));
    Update(&UserInterface->Slider3, Input, V3(Width - 160.0, 0.4 * Height, 0.0));
    Update(&UserInterface->Slider4, Input, V3(Width - 140.0, 0.4 * Height, 0.0));
    Update(&UserInterface->Slider5, Input, V3(Width - 120.0, 0.4 * Height, 0.0));
    Update(&UserInterface->Slider6, Input, V3(Width - 100.0, 0.4 * Height, 0.0));
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

    if (module(Joystic) > 0.1) {
        Camera->Angle -= 3.0 * Joystic.X;
        Camera->Pitch -= 3.0 * Joystic.Y;
    }

    Camera->Basis = GetCameraBasis(Camera->Angle, Camera->Pitch);
    Camera->Plane.Base[0] = Camera->Basis.X;
    Camera->Plane.Base[1] = Camera->Basis.Y;

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
    Camera->Velocity = 0.2 * (Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z);
    Camera->Position += Camera->Velocity;
}


// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    UI* UserInterface = &pGameState->UserInterface;
    
    double Time = pGameState->Time;
    camera* Camera = &Group->Camera;

    bool firstFrame = false;
    if (!Memory->IsInitialized) {
        firstFrame = true;

        memory_arena* StringsArena = &pGameState->StringsArena;
        pGameState->StringsArena.Name = PushString(StringsArena, 13, "Strings Arena");
        pGameState->StringsArena.Percentage = PushString(StringsArena, 7, "0.0%");
        pGameState->RenderArena.Name = PushString(StringsArena, 13, "Render Arena");
        pGameState->RenderArena.Percentage = PushString(StringsArena, 7, "0.0%");

        // User Interface

        // Camera
        Camera->Position = V3(0.0, 2.2, 5.0);
        Camera->Angle = 45;
        Camera->Pitch = 45;

        Memory->IsInitialized = true;
    }

    PushClear(Group, { 0 }, World);
    PushClear(Group, { 0 }, Outline);
    PushClear(Group, { 0 }, Postprocessing_Outline);
    PushClear(Group, Color(BackgroundBlue, 1.0), Output);

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

    // Updates
    Update(Camera, Input);

    Update(&pGameState->UserInterface, Input, Group->Width, Group->Height);
    
    // Render
    light LightSource = Light(V3(-0.5, -1, 1), White);

    transform Transform1 = Transform(Quaternion(Pi / 2, V3(0.0, -1.0, 0.0)), V3(0.0, 0.0, 5.0));
    transform Transform2 = Transform(Quaternion(1.0, 0.0, 0.0, 0.0), V3(4.0, 0.0, 5.0));
    //PushMesh(Group, Mesh_Enemy_ID, Transform1, LightSource, Shader_Texture_ID, Bitmap_Enemy_ID, White, SORT_ORDER_MESHES, true);

    PushMesh(Group, Mesh_Body_ID, Transform1, LightSource, Mesh_Shader_Pipeline_ID, Bitmap_Empty_ID, White, SORT_ORDER_MESHES, false);
    PushMesh(Group, Mesh_Sphere_ID, Transform2, LightSource, Sphere_Shader_Pipeline_ID, Bitmap_Empty_ID, Red, SORT_ORDER_MESHES, false);

    //PushCircle(Group, V3(0.0, 0.0, 0.0), V3(1.0, 1.0, 0.0), 1.0, Magenta, SORT_ORDER_MESHES);

    // PushVideo(Group, &Assets->TestVideo, {0, 0, (double)Group->Width, (double)Group->Height}, pGameState->dt);

    // Render

    // Software renderer as a fallback
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

    // PushHeightmap(Group, Heightmap_Spain_ID);

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
        else {
            Alpha = 1.0;
        }

        PushDebugGrid(Group, Alpha);

        game_rect DebugInfoRect = { 0, 0, 350, 200 };

        PushRect(Group, DebugInfoRect, Color(Black, 0.5 * Alpha), World, SORT_ORDER_DEBUG_OVERLAY);
        PushRectOutline(Group, DebugInfoRect, Color(Gray, Alpha));
        PushText(Group, V2(0, 30.0), GetAsset(Assets, Font_Cascadia_Mono_ID), Memory->DebugInfo, Color(White, Alpha), 12, false, SORT_ORDER_DEBUG_OVERLAY);

        // Render Arena
        PushDebugArena(Group, pGameState->RenderArena, V2(20.0, 120.0), Alpha);

        // Strings Arena
        PushDebugArena(Group, pGameState->StringsArena, V2(20.0, 150.0), Alpha);

        // Axes
        v3 XAxis = V3(cos(Group->Camera.Angle * Degrees), sin(Group->Camera.Angle * Degrees) * sin(Group->Camera.Pitch * Degrees), 0.0);
        v3 YAxis = V3(0.0, -cos(Group->Camera.Pitch * Degrees), 0.0);
        v3 ZAxis = V3(-sin(Group->Camera.Angle * Degrees), sin(Group->Camera.Pitch * Degrees) * cos(Group->Camera.Angle * Degrees), 0.0);
        v3 AxisOrigin = V3(Group->Width - 0.08 * (double)Group->Height - 10.0, 0.1 * (double)Group->Height, 0);
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, Screen_Coordinates, Red);
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, Screen_Coordinates, Green);
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, Screen_Coordinates, Blue);
    }

    PushRenderTarget(Group, World);

    static bool Screenshot = false;
    if (Input->Keyboard.F10.IsDown && !Input->Keyboard.F10.WasDown) {
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
            PushRect(Group, ScreenRect, Color(White, ScreenRectAlpha), Output, SORT_ORDER_PUSH_RENDER_TARGETS + 50.0);
        }
    }
    PushRenderTarget(Group, Output, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
}
