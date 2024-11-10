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

        if (cos(Camera->Pitch * Degrees) > 0) {
            Camera->Angle -= AngularVelocity * Offset.X;
        }
        else {
            Camera->Angle += AngularVelocity * Offset.X;
        }
        Camera->Pitch += AngularVelocity * Offset.Y;
    }

    v2 Joystic = V2(Input->Controller.RightJoystick.X, Input->Controller.RightJoystick.Y);

    if (module(Joystic) > 0.1) {
        Camera->Angle -= 3.0 * Joystic.X;
        Camera->Pitch -= 3.0 * Joystic.Y;
    }

    Camera->Basis = GetCameraBasis(Camera->Angle, Camera->Pitch);
}

movement RandomMovement() {
    movement Result;
    Result.Turn = (turn)(rand() % 6);
    Result.Direction = (direction)(rand() % 2);
    return Result;
}

void RandomizeQueue(movement_queue* Queue, int n) {
    Assert(n > 0);

    if (n > MOVEMENT_BUFFER_SIZE) n = MOVEMENT_BUFFER_SIZE;

    for (int i = 0; i < n; i++) Queue->Queue[i] = RandomMovement();

    Queue->Length = n;
}

void Update(rubiks_cube* Cube, game_input* Input, camera* Camera, double Width, double Height, double dt) {
    if (Input->Keyboard.R.IsDown && !Input->Keyboard.R.WasDown) {
        RandomizeQueue(&Cube->Queue, MOVEMENT_BUFFER_SIZE);
    }

    direction Direction = Input->Controller.LB.IsDown ? Clockwise : AntiClockwise;

    if (Input->Controller.RB.IsDown && !Input->Controller.RB.WasDown) {
        AddMovement(&Cube->Queue, { Top, Direction });        
    }
    if (Input->Controller.RT.IsDown && !Input->Controller.RT.WasDown) {
        AddMovement(&Cube->Queue, { Bottom, Direction });
    }
    if (Input->Controller.XButton.IsDown && !Input->Controller.XButton.WasDown) {
        AddMovement(&Cube->Queue, { Right, Direction });
    }
    if (Input->Controller.BButton.IsDown && !Input->Controller.BButton.WasDown) {
        AddMovement(&Cube->Queue, { Front, Direction });
    }
    if (Input->Controller.YButton.IsDown && !Input->Controller.YButton.WasDown) {
        AddMovement(&Cube->Queue, { Back, Direction });
    }
    if (Input->Controller.AButton.IsDown && !Input->Controller.AButton.WasDown) {
        AddMovement(&Cube->Queue, { Left, Direction });
    }

    if (Cube->Queue.Length > 0) {
        Cube->Animating = true;
        Cube->Movement = Cube->Queue.Queue[Cube->Queue.Offset];
    }

    face* Face;
    turn Turns[6] = {
        Top,
        Bottom,
        Right,
        Left,
        Back,
        Front,
    };
    for (int i = 0; i < 12; i++) {
        if (i < 6) {
            Face = &Cube->Centers[i].Face;
            Face->Selected = ShouldCheck(Camera->Angle, Camera->Pitch, Cube->Centers[i].Position, Face->Offset) &&
                Raycast(Camera, Width, Height, V2(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y), Face->Collider);
            if (Face->Selected) {
                if (Input->Mouse.LeftClick.IsDown && !Input->Mouse.LeftClick.WasDown) {
                    AddMovement(&Cube->Queue, { Turns[i], AntiClockwise });
                }
                else if (Input->Mouse.RightClick.IsDown && !Input->Mouse.RightClick.WasDown) {
                    AddMovement(&Cube->Queue, { Turns[i], Clockwise });
                }
            }
        }
        for (int n = 0; n < 2; n++) {
            Face = &Cube->Edges[i].Faces[n];
            Face->Selected = ShouldCheck(Camera->Angle, Camera->Pitch, Cube->Edges[i].Position, Face->Offset) &&
                Raycast(Camera, Width, Height, V2(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y), Face->Collider);
        }
        if (i < 8) {
            for (int n = 0; n < 3; n++) {
                Face = &Cube->Corners[i].Faces[n];
                Face->Selected = ShouldCheck(Camera->Angle, Camera->Pitch, Cube->Corners[i].Position, Face->Offset) &&
                    Raycast(Camera, Width, Height, V2(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y), Face->Collider);
            }
        }
    }

    if (Cube->Animating) {
        double Duration = 0.3;
        
        if (Cube->Time + dt > Duration) {
            Cube->Animating = false;
            dt = Duration - Cube->Time;
        }
        else Cube->Time += dt;

        // Rotate pieces
        quaternion Rotation = Quaternion(Tau / 4.0 * dt / Duration, GetRotationVector(Cube->Movement.Turn, Cube->Movement.Direction));

        for (int i = 0; i < 12; i++) {
            if (i < 6) {
                center_piece* CenterPiece = &Cube->Centers[i];
                if (ShouldMove(CenterPiece->Position, Cube->Movement.Turn)) {
                    CenterPiece->Transform = CenterPiece->Transform * Transform(V3(0.0, 0.0, 0.0), Rotation);
                    if (!Cube->Animating) Move(&CenterPiece->Position, Cube->Movement.Turn, Cube->Movement.Direction);
                }
            }
            edge_piece* EdgePiece = &Cube->Edges[i];
            if (ShouldMove(EdgePiece->Position, Cube->Movement.Turn)) {
                EdgePiece->Transform = EdgePiece->Transform * Transform(V3(0.0, 0.0, 0.0), Rotation);
                if (!Cube->Animating) Move(&EdgePiece->Position, Cube->Movement.Turn, Cube->Movement.Direction);
            }
            if (i < 8) {
                corner_piece* CornerPiece = &Cube->Corners[i];
                if (ShouldMove(CornerPiece->Position, Cube->Movement.Turn)) {
                    CornerPiece->Transform = CornerPiece->Transform * Transform(V3(0.0, 0.0, 0.0), Rotation);
                    if (!Cube->Animating) Move(&CornerPiece->Position, Cube->Movement.Turn, Cube->Movement.Direction);
                }
            }
        }

        if (!Cube->Animating) {
            Cube->Time = 0.0;
            if (Cube->Queue.Length > 1) {
                Cube->Movement = NextMovement(&Cube->Queue);
            }
            else {
                Cube->Queue.Offset = 0;
                Cube->Queue.Length = 0;
            }
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
            &pGameState->VideoArena,
            &pGameState->MeshArena
        );

        // Faces
        pGameState->Cube = { 0 };
        InitializeCube(&pGameState->Cube);

        // User Interface
        InitSlider(&pGameState->UserInterface.Slider1, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider2, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider3, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider4, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider5, 0.5, Black);
        InitSlider(&pGameState->UserInterface.Slider6, 0.5, Black);

        Camera->Position = V3(0.0, 0.0, 5.0);
        Camera->Angle = 45;
        Camera->Pitch = 45;

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

    Update(&pGameState->Cube, Input, Camera, Group->Width, Group->Height, pGameState->dt);
    
    // Render
    light Light = { 0 };
    Light.Ambient = 0.2;
    Light.Direction = normalize(V3(-1,-1,1));

    PushCube(Group, Input, &pGameState->Cube);
    PushMeshOutline(Group, Assets, Group->Width / 100.0, Black, 12, (1 << 11), pGameState->Time);

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
    // PushUI(Group, &pGameState->UserInterface);

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

        // Debug normals and colliders
        transform Transform = { 0 };
        color Color = White;
        for (int i = 0; i < 12; i++) {
            if (i < 6) {
                center_piece CenterPiece = pGameState->Cube.Centers[i];
                Transform = CenterPiece.Face.Transform * CenterPiece.Transform;
                Transform.Translation += pGameState->Cube.Position;
                PushDebugVector(Group, Rotate(V3(0.0, 1.0, 0.0), Transform.Rotation), Transform.Translation);
                Color = Raycast(
                    Camera, 
                    (double)Group->Width, (double)Group->Height, 
                    V2(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y), 
                    CenterPiece.Face.Collider
                ) ? Red : White;
                PushCubeOutline(Group, CenterPiece.Face.Collider, Color);
            }

            edge_piece EdgePiece = pGameState->Cube.Edges[i];
            for (int j = 0; j < 2; j++) {
                Transform = EdgePiece.Faces[j].Transform * EdgePiece.Transform;
                Transform.Translation += pGameState->Cube.Position;
                PushDebugVector(Group, Rotate(V3(0.0, 1.0, 0.0), Transform.Rotation), Transform.Translation);
                Color = Raycast(
                    Camera, 
                    (double)Group->Width, (double)Group->Height, 
                    V2(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y), 
                    EdgePiece.Faces[j].Collider
                ) ? Red : White;
                PushCubeOutline(Group, EdgePiece.Faces[j].Collider, Color);
            }

            if (i < 8) {
                corner_piece CornerPiece = pGameState->Cube.Corners[i];
                for (int j = 0; j < 3; j++) {
                    Transform = CornerPiece.Faces[j].Transform * CornerPiece.Transform;
                    Transform.Translation += pGameState->Cube.Position;
                    PushDebugVector(Group, Rotate(V3(0.0, 1.0, 0.0), Transform.Rotation), Transform.Translation);
                    Color = Raycast(
                        Camera,
                        (double)Group->Width, (double)Group->Height,
                        V2(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y),
                        CornerPiece.Faces[j].Collider
                    ) ? Red : White;
                    PushCubeOutline(Group, CornerPiece.Faces[j].Collider, Color);
                }
            }
        }
    }

    PushRenderTarget(Group, Background, &Assets->FramebufferShader, 500);
    PushRenderTarget(Group, World, &Assets->FramebufferShader, 500);
    //PushShaderPass(Group, &Assets->EdgeDetectionShader, Postprocessing_World, 501);
    PushRenderTarget(Group, Screen, &Assets->FramebufferShader, 500);
    PushRenderTarget(Group, Outline, &Assets->FramebufferShader, 500);
    PushRenderTarget(Group, Postprocessing_Background, &Assets->FramebufferShader, 1020);
    PushRenderTarget(Group, Postprocessing_Outline, &Assets->FramebufferShader, 1021);
    PushRenderTarget(Group, Postprocessing_World, &Assets->FramebufferShader, 1022);
    PushRenderTarget(Group, Postprocessing_Screen, &Assets->FramebufferShader, 1023);
}
