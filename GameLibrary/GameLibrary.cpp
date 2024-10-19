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


/*
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/
bool HitBoundingBox(double minB[3], double maxB[3], double origin[3], double dir[3], double coord[3])
/* double minB[NUMDIM], maxB[NUMDIM];		box */
/* double origin[NUMDIM], dir[NUMDIM];		ray */
/* double coord[NUMDIM];			hit point */
{
    bool inside = true;
    char quadrant[3];
    register int i;
    int whichPlane;
    double maxT[3];
    double candidatePlane[3];
    char LEFT = 1;
    char RIGHT = 0;
    char MIDDLE = 2;

    /* Find candidate planes; this loop can be avoided if
    rays cast all from the eye(assume perpsective view) */
    for (i = 0; i < 3; i++)
        if (origin[i] < minB[i]) {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = false;
        }
        else if (origin[i] > maxB[i]) {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = false;
        }
        else {
            quadrant[i] = MIDDLE;
        }

    /* Ray origin inside bounding box */
    if (inside) {
        coord = origin;
        return true;
    }


    /* Calculate T distances to candidate planes */
    for (i = 0; i < 3; i++)
        if (quadrant[i] != MIDDLE && dir[i] != 0.)
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        else
            maxT[i] = -1.;

    /* Get largest of the maxT's for final choice of intersection */
    whichPlane = 0;
    for (i = 1; i < 3; i++)
        if (maxT[whichPlane] < maxT[i])
            whichPlane = i;

    /* Check final candidate actually inside box */
    if (maxT[whichPlane] < 0.) return false;
    for (i = 0; i < 3; i++)
        if (whichPlane != i) {
            coord[i] = origin[i] + maxT[whichPlane] * dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i])
                return false;
        }
        else {
            coord[i] = candidatePlane[i];
        }
    return true;				/* ray hits box */
}


bool Raycast(v3 Origin, v3 Direction, cube_collider Collider) {
    double minB[3] = { 0 };
    minB[0] = Collider.Center.X - Collider.Size.X / 2.0;
    minB[1] = Collider.Center.Y - Collider.Size.Y / 2.0;
    minB[2] = Collider.Center.Z - Collider.Size.Z / 2.0;
    double maxB[3] = { 0 };
    maxB[0] = Collider.Center.X + Collider.Size.X / 2.0;
    maxB[1] = Collider.Center.Y + Collider.Size.Y / 2.0;
    maxB[2] = Collider.Center.Z + Collider.Size.Z / 2.0;
    double origin[3] = { Origin.X, Origin.Y, Origin.Z };
    double dir[3] = { Direction.X, Direction.Y, Direction.Z };
    double coord[3] = { 0,0,0 };

    return HitBoundingBox(minB, maxB, origin, dir, coord);
}

bool Raycast(camera Camera, game_input* Input, double Width, double Height, cube_collider Collider) {
    basis B = Camera.Basis;

    v3 ScreenOffset = 
        (2.0 * (double)Input->Mouse.Cursor.X / Width - 1.0)    * B.X +
        (Height - 2.0 * (double)Input->Mouse.Cursor.Y) / Width * B.Y
                                                               - B.Z;
    return Raycast(Camera.Position + Camera.Distance * B.Z, ScreenOffset, Collider);
}


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

        Camera->Pitch += AngularVelocity * Offset.Y;
        Camera->Angle -= AngularVelocity * Offset.X;
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

    static video_context VideoContext = { 0 };
    
    double Time = pGameState->Time;
    camera* Camera = &Group->Camera;

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

            quaternion TopRotation = Quaternion(1.0);
            quaternion BottomRotation = Quaternion(Tau / 2.0, V3(1.0, 0.0, 0.0));
            quaternion LeftRotation = Quaternion(-Tau / 4.0, V3(1.0, 0.0, 0.0));
            quaternion RightRotation = Quaternion(Tau / 4.0, V3(1.0, 0.0, 0.0));
            quaternion FrontRotation = Quaternion(-Tau / 4.0, V3(0.0, 0.0, 1.0));
            quaternion BackRotation = Quaternion(Tau / 4.0, V3(0.0, 0.0, 1.0));

            pGameState->Cube.Top[i]    = { White,  Transform(V3(sub_x, L, sub_y),  TopRotation) };
            pGameState->Cube.Bottom[i] = { Yellow, Transform(V3(sub_x, -L, sub_y), BottomRotation) };
            pGameState->Cube.Left[i] = { Blue,   Transform(V3(sub_x, sub_y, -L), LeftRotation) };
            pGameState->Cube.Right[i] = { Green,  Transform(V3(sub_x, sub_y, L),  RightRotation) };
            pGameState->Cube.Front[i] = { Orange, Transform(V3(L, sub_y, sub_x),  FrontRotation) };
            pGameState->Cube.Back[i] = { Red,    Transform(V3(-L, sub_y, sub_x), BackRotation) };
        }

        // User Interface
        InitSlider(&pGameState->UserInterface.Slider1, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider2, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider3, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider4, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider5, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider6, 0.5, Black);

        Camera->Position = V3(0, 0, 0.0);
        Camera->Angle = 0;
        Camera->Pitch = 0;

        Memory->IsInitialized = true;
    }

    PushClear(Group, BackgroundBlue, Background);
    PushClear(Group, { 0 }, Outline);
    PushClear(Group, { 0 }, Postprocessing_Outline);
    PushClear(Group, Color(BackgroundBlue, 0), World);
    PushClear(Group, { 0 }, Screen);

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
    Update(Camera, Input);
    
    // Render
    light Light = { 0 };
    Light.Ambient = 0.2;
    Light.Direction = normalize(V3(-1,-1,1));

    for (int i = 0; i < 9; i++) {
        face* Face = &pGameState->Cube.Top[i];
        quaternion Rotation = Quaternion(0.01, V3(0.0, 1.0, 0.0));
        Face->Transform.Translation = Rotate(Face->Transform.Translation, Rotation);
        Face->Transform.Rotation = Conjugate(Rotation) * Face->Transform.Rotation;
    }

    for (int i = 0; i < 3; i++) {
        face* FaceLeft = &pGameState->Cube.Left[6+i];
        face* FaceRight = &pGameState->Cube.Right[6+i];
        face* FaceFront = &pGameState->Cube.Front[6+i];
        face* FaceBack = &pGameState->Cube.Back[6+i];

        quaternion Rotation = Quaternion(0.01, V3(0.0, 1.0, 0.0));
        FaceLeft->Transform.Translation = Rotate(FaceLeft->Transform.Translation, Rotation);
        FaceRight->Transform.Translation = Rotate(FaceRight->Transform.Translation, Rotation);
        FaceFront->Transform.Translation = Rotate(FaceFront->Transform.Translation, Rotation);
        FaceBack->Transform.Translation = Rotate(FaceBack->Transform.Translation, Rotation);
        FaceLeft->Transform.Rotation = Conjugate(Rotation) * FaceLeft->Transform.Rotation;
        FaceRight->Transform.Rotation = Conjugate(Rotation) * FaceRight->Transform.Rotation;
        FaceFront->Transform.Rotation = Conjugate(Rotation) * FaceFront->Transform.Rotation;
        FaceBack->Transform.Rotation = Conjugate(Rotation) * FaceBack->Transform.Rotation;
    }

    for (int i = 0; i < 54; i++) {
        face Face = pGameState->Cube.Faces[i];
        Light.Color = Face.Color;
        PushMesh(Group, &Assets->FaceMesh, Face.Transform, Light, &Assets->TextureShader);
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

    Update(&pGameState->UserInterface, Input, Group->Width, Group->Height);
    PushUI(Group, &pGameState->UserInterface);

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

        game_rect DebugInfoRect = { 0, 0, 350, 270 };

        PushRect(Group, DebugInfoRect, Color(Black, 0.5 * Alpha), 0);
        PushRectOutline(Group, DebugInfoRect, Color(Gray, Alpha));
        PushText(Group, { 0, 30, 0.5 }, Assets->Characters, Color(White, Alpha), 12, Memory->DebugInfo, false);

        // Render Arena
        PushDebugArena(Group, Assets->Characters, pGameState->RenderArena, V3(20.0, 120.0, 0.5), Alpha);

        // Strings Arena
        PushDebugArena(Group, Assets->Characters, pGameState->StringsArena, V3(20.0, 150.0, 0.5), Alpha);

        // Fonts Arena
        PushDebugArena(Group, Assets->Characters, pGameState->FontsArena, V3(20.0, 180.0, 0.5), Alpha);

        // Mesh Arena
        PushDebugArena(Group, Assets->Characters, pGameState->MeshArena, V3(20.0, 210.0, 0.5), Alpha);

        // Video Arena
        PushDebugArena(Group, Assets->Characters, pGameState->VideoArena, V3(20.0, 240.0, 0.5), Alpha);

        // Debug normals
        for (int i = 0; i < 54; i++) {
            face Face = pGameState->Cube.Faces[i];
            PushDebugVector(Group, Rotate(V3(0.0, 1.0, 0.0), Conjugate(Face.Transform.Rotation)), Face.Transform.Translation);
        }
    }

    PushRenderTarget(Group, Background, &Assets->FramebufferShader, 500);
    PushRenderTarget(Group, World, &Assets->FramebufferShader, 500);
    PushRenderTarget(Group, Screen, &Assets->FramebufferShader, 500);
    PushRenderTarget(Group, Outline, &Assets->FramebufferShader, 500);
    PushRenderTarget(Group, Postprocessing_Background, &Assets->FramebufferShader, 1020);
    PushRenderTarget(Group, Postprocessing_Outline, &Assets->FramebufferShader, 1021);
    PushRenderTarget(Group, Postprocessing_World, &Assets->FramebufferShader, 1022);
    PushRenderTarget(Group, Postprocessing_Screen, &Assets->FramebufferShader, 1023);
}
