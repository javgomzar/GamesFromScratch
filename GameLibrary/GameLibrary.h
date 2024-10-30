// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the GAMELIBRARY_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// GAMELIBRARY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GAMELIBRARY_EXPORTS
#define GAMELIBRARY_API __declspec(dllexport)
#else
#define GAMELIBRARY_API __declspec(dllimport)
#endif

//// This class is exported from the dll
//class GAMELIBRARY_API CGameLibrary {
//public:
//	CGameLibrary(void);
//	// TODO: add your methods here.
//};

extern GAMELIBRARY_API int nGameLibrary;

#pragma once
#include "GamePlatform.h"
#include "GameMath.h"
#include "FFMpeg.h"

#include "Assets.h"


// Platform independent structs and types
struct game_offscreen_buffer {
    void* Memory;
    uint32 Height;
    uint32 Width;
    uint32 Pitch;
    uint8 BytesPerPixel;
};

struct game_sound_buffer {
    uint32 SamplesPerSecond;
    uint16 BufferSize;
    int16* SampleOut;
};

struct game_button_state {
    bool IsDown;
    bool WasDown;
};


// Color
struct color {
    double Alpha;
    double R;
    double G;
    double B;
};

color Color(double R, double G, double B, double Alpha = 1.0) {
    return { Alpha, R, G, B };
}

color Color(color Color, double Alpha) {
    return { Alpha, Color.R, Color.G, Color.B };
}

color operator*(double Luminosity, color Color) {
    return {
        Color.Alpha,
        min(Luminosity * Color.R, 1.0),
        min(Luminosity * Color.G, 1.0),
        min(Luminosity * Color.B, 1.0),
    };
}

static int Attenuation = 100;
static color Black = { 1.0, 0.0, 0.0, 0.0 };
static color White = { 1.0, 1.0, 1.0, 1.0 };
static color Gray = { 1.0, 0.5, 0.5, 0.5 };
static color DarkGray = { 1.0, 0.1, 0.1, 0.1 };
static color Red = { 1.0, 1.0, 0.0, 0.0 };
static color Green = { 1.0, 0.0, 1.0, 0.0 };
static color Blue = { 1.0, 0.0, 0.0, 1.0 };
static color Magenta = { 1.0, 1.0, 0.0, 1.0 };
static color Yellow = { 1.0, 1.0, 1.0, 0.0 };
static color Cyan = { 1.0, 0.0, 1.0, 1.0 };
static color Orange = { 1.0, 1.0, 0.63, 0.0 };
static color BackgroundBlue = { 1.0, 0.4, 0.4, 0.8 };

uint32 GetColorBytes(color Color) {
    uint8 Alpha = Color.Alpha * 255.0;
    uint8 R = Color.R * 255.0;
    uint8 G = Color.G * 255.0;
    uint8 B = Color.B * 255.0;
    return (Alpha << 24) | (R << 16) | (G << 8) | B;
}

color GetColor(uint32 Bytes, uint32 RedMask, uint32 GreenMask, uint32 BlueMask) {
    uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
    
    uint32 RedShift;
    uint32 GreenShift;
    uint32 BlueShift;
    uint32 AlphaShift;
    _BitScanForward((DWORD*)&RedShift, RedMask);
    _BitScanForward((DWORD*)&GreenShift, GreenMask);
    _BitScanForward((DWORD*)&BlueShift, BlueMask);
    _BitScanForward((DWORD*)&AlphaShift, AlphaMask);

    color Color;
    Color.R = (double)((RedMask & Bytes) >> RedShift) / 255.0;
    Color.G = (double)((GreenMask & Bytes) >> GreenShift) / 255.0;
    Color.B = (double)((BlueMask & Bytes) >> BlueShift) / 255.0;
    Color.Alpha = (double)((AlphaMask & Bytes) >> AlphaShift) / 255.0;
    return Color;
}

color Blend(color Color, color Background) {
    color Result;
    double Alpha = (double)(Color.Alpha) / 255.0;
    Result.R = Background.R + (Alpha * (Color.R - Background.R) + 0.5);
    Result.G = Background.G + (Alpha * (Color.G - Background.G) + 0.5);
    Result.B = Background.B + (Alpha * (Color.B - Background.B) + 0.5);
    Result.Alpha = 255.0;
    return Result;
}

// Joysticks values should be floats between 0 and 1
/*struct joystick_coordinate {
    float Start;
    float Max;
    float Min;
    float End;
};*/

struct game_joystick_state {
    float X;
    float Y;
};

struct game_controller_input {
    bool Any;
    game_joystick_state LeftJoystick;
    game_joystick_state RightJoystick;
    union {
        game_button_state Buttons[16];
        struct {
            game_button_state AButton;
            game_button_state BButton;
            game_button_state XButton;
            game_button_state YButton;
            game_button_state PadUp;
            game_button_state PadDown;
            game_button_state PadLeft;
            game_button_state PadRight;
            game_button_state RB;
            game_button_state LB;
            game_button_state RS;
            game_button_state LS;
            game_button_state RT;
            game_button_state LT;
            game_button_state Start;
            game_button_state Back;
        };
    };
};

const int NUMBER_OF_KEYS = 57;

struct game_keyboard_input {
    bool Any;
    union
    {
        game_button_state Keys[NUMBER_OF_KEYS];
        struct {
            game_button_state One;
            game_button_state Two;
            game_button_state Three;
            game_button_state Four;
            game_button_state Five;
            game_button_state Six;
            game_button_state Seven;
            game_button_state Eight;
            game_button_state Nine;
            game_button_state Zero;
            game_button_state Q;
            game_button_state W;
            game_button_state E;
            game_button_state R;
            game_button_state T;
            game_button_state Y;
            game_button_state U;
            game_button_state I;
            game_button_state O;
            game_button_state P;
            game_button_state A;
            game_button_state S;
            game_button_state D;
            game_button_state F;
            game_button_state G;
            game_button_state H;
            game_button_state J;
            game_button_state K;
            game_button_state L;
            game_button_state Z;
            game_button_state X;
            game_button_state C;
            game_button_state V;
            game_button_state B;
            game_button_state N;
            game_button_state M;
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state Escape;
            game_button_state Space;
            game_button_state Enter;
            game_button_state Shift;
            game_button_state F1;
            game_button_state F2;
            game_button_state F3;
            game_button_state F4;
            game_button_state F5;
            game_button_state F6;
            game_button_state F7;
            game_button_state F8;
            game_button_state F9;
            game_button_state F10;
            game_button_state F11;
            game_button_state F12;
            game_button_state PageUp;
            game_button_state PageDown;
        };
    };
};

struct game_mouse_input {
    game_button_state LeftClick;
    game_button_state MiddleClick;
    game_button_state RightClick;
    v3 Cursor;
    v3 LastCursor;
    short Wheel;
};

enum game_input_mode {
    Keyboard,
    Controller
};

struct game_input {
    game_input_mode Mode;
    game_controller_input Controller;
    game_keyboard_input Keyboard;
    game_mouse_input Mouse;
};

struct record_and_playback {
    HANDLE RecordFile;
    int RecordIndex;
    HANDLE PlaybackFile;
    int PlaybackIndex;
    void* GameMemoryBlock;
    uint64 TotalSize;
};

// Colliders
struct rect_collider {
    v3 Center;
    double Width;
    double Height;
};

struct cube_collider {
    v3 Center;
    v3 Size;
};

struct sphere_collider {
    v3 Center;
    double Radius;
};

bool Collide(rect_collider Collider, v3 Position) {
    return fabs(Position.X - Collider.Center.X) < (double)Collider.Width / 2.0 &&
        fabs(Position.Y - Collider.Center.Y) < (double)Collider.Height / 2.0;
}

bool Collide(cube_collider Collider, v3 Position) {
    return fabs(Position.X - Collider.Center.X) < Collider.Size.X / 2.0 &&
           fabs(Position.Y - Collider.Center.Y) < Collider.Size.Y / 2.0 &&
           fabs(Position.Z - Collider.Center.Z) < Collider.Size.Z / 2.0;
}

bool Collide(sphere_collider Collider, v3 Position) {
    return module(Position - Collider.Center) < Collider.Radius;
}

// User Interface
struct slider {
    double Value;
    v3 Position;
    color Color;
    rect_collider Collider;
};

struct UI {
    slider Slider1;
    slider Slider2;
    slider Slider3;
    slider Slider4;
    slider Slider5;
    slider Slider6;
};

void InitSlider(slider* Slider, double Value, color Color) {
    *Slider = { 0 };
    
    if (Value > 1.0 || Value < 0.0) Assert(false);
    else Slider->Value = Value;

    Slider->Color = Color;
}

// Rubik's cube
struct face {
    color Color;
    transform Transform;
    cube_collider Collider;
};

face Face(color Color, transform Transform, cube_collider Collider) {
    face Result = { 0 };
    Result.Color = Color;
    Result.Transform = Transform;
    Result.Collider = Collider;
    return Result;
}

struct center_piece {
    face Face;
    transform Transform;
    iv3 Position;
};

struct edge_piece {
    face Faces[2];
    transform Transform;
    iv3 Position;
};

struct corner_piece {
    face Faces[3];
    transform Transform;
    iv3 Position;
};

enum direction {
    Clockwise,
    AntiClockwise
};

enum turn {
    Top,
    Bottom,
    Left,
    Right,
    Front,
    Back
};

turn Opposite(turn Turn) {
    switch (Turn) {
        case Top: {
            return Bottom;
        } break;
        case Bottom: {
            return Top;
        } break;
        case Left: {
            return Right;
        } break;
        case Right: {
            return Left;
        } break;
        case Front: {
            return Back;
        } break;
        case Back: {
            return Front;
        } break;
        default: {
            Assert(false);
        }
    }
}

struct movement {
    turn Turn;
    direction Direction;
};

const int MOVEMENT_BUFFER_SIZE = 10;
struct rubiks_cube {
    center_piece Centers[6];
    edge_piece Edges[12];
    corner_piece Corners[8];
    v3 Position;
    double Time;
    movement Movement;
    movement MovementBuffer[MOVEMENT_BUFFER_SIZE];
    bool Animating;
};

v3 GetRotationVector(turn Turn, direction Direction) {
    v3 Result = { 0 };
    switch (Turn) {
        case Top: {
            Result = V3(0.0, 1.0, 0.0);
        } break;
        case Bottom: {
            Result = V3(0.0, -1.0, 0.0);
        } break;
        case Left: {
            Result = V3(-1.0, 0.0, 0.0);
        } break;
        case Right: {
            Result = V3(1.0, 0.0, 0.0);
        } break;
        case Front: {
            Result = V3(0.0, 0.0, -1.0);
        } break;
        case Back: {
            Result = V3(0.0, 0.0, 1.0);
        } break;
        default: {
            Assert(false);
        }
    }

    return Direction == AntiClockwise ? Result : -Result;
}

bool ShouldMove(iv3 Position, turn Turn) {
    switch (Turn) {
        case Top: {
            return Position.Y == 1;
        } break;
        case Bottom: {
            return Position.Y == -1;
        } break;
        case Left: {
            return Position.X == -1;
        } break;
        case Right: {
            return Position.X == 1;
        } break;
        case Front: {
            return Position.Z == 1;
        } break;
        case Back: {
            return Position.Z == -1;
        } break;
        default: {
            Assert(false);
        }
    }
    return false;
}

void Move(iv3* Position, turn Turn, direction Direction) {
    iv3 OldPosition = *Position;

    if (Direction == Clockwise) Turn = Opposite(Turn);

    switch (Turn) {
        case Top: {
            Position->X = OldPosition.Z;
            Position->Z = -OldPosition.X;
        } break;
        case Bottom: {
            Position->X = -OldPosition.Z;
            Position->Z = OldPosition.X;
        } break;
        case Left: {
            Position->Y = OldPosition.Z;
            Position->Z = -OldPosition.Y;
        } break;
        case Right: {
            Position->Y = -OldPosition.Z;
            Position->Z = OldPosition.Y;
        } break;
        case Front: {
            Position->Y = OldPosition.X;
            Position->X = -OldPosition.Y;
        } break;
        case Back: {
            Position->Y = -OldPosition.X;
            Position->X = OldPosition.Y;
        } break;
        default: {
            Assert(false);
        }
    }
}

void InitializeCube(rubiks_cube* Cube) {
    // Piece half length
    double D = 1.0;
    // Cube half length
    double L = 2.2;

    Cube->Position = V3(0.0, 0.0, 5.0);
    Cube->Time = 0.0;
    Cube->Animating = false;

    color Colors[6] = { White, Yellow, Blue, Green, Orange, Red };
    iv3 Positions[6] = {
        IV3( 0,  1,  0),
        IV3( 0, -1,  0),
        IV3( 1,  0,  0),
        IV3(-1,  0,  0),
        IV3( 0,  0, -1),
        IV3( 0,  0,  1),
    };
    quaternion Rotations[6] = {
        Quaternion(1.0),
        Quaternion(Tau / 2.0, V3(1.0, 0.0, 0.0)),
        Quaternion(Tau / 4.0, V3(0.0, 0.0, 1.0)),
        Quaternion(-Tau / 4.0, V3(0.0, 0.0, 1.0)),
        Quaternion(-Tau / 4.0, V3(1.0, 0.0, 0.0)),
        Quaternion(Tau / 4.0, V3(1.0, 0.0, 0.0)),
    };
    v3 ColliderSizes[6] = {
        V3(2.0, 0.1, 2.0),
        V3(2.0, 0.1, 2.0),
        V3(0.1, 2.0, 2.0),
        V3(0.1, 2.0, 2.0),
        V3(2.0, 2.0, 0.1),
        V3(2.0, 2.0, 0.1),
    };

    int CurrentEdge = 0;
    int CurrentCorner = 0;
    for (int i = 0; i < 6; i++) {
        // Centers
        center_piece* CenterPiece = &Cube->Centers[i];
        CenterPiece->Transform.Scale = Scale();
        CenterPiece->Transform.Translation = L * V3(Positions[i].X, Positions[i].Y, -Positions[i].Z);
        CenterPiece->Position = Positions[i];
        CenterPiece->Transform.Rotation = Quaternion(1.0);
        face* CenterFace = &CenterPiece->Face;
        CenterFace->Color = Colors[i];
        CenterFace->Transform = { 0 };
        CenterFace->Transform.Scale = Scale();
        CenterFace->Transform.Rotation = Rotations[i];
        CenterFace->Transform.Translation = Rotate(V3(0, D, 0), Rotations[i]);
        CenterFace->Collider = {
            Cube->Position + CenterPiece->Transform.Translation + CenterFace->Transform.Translation,
            ColliderSizes[i]
        };

        // Edges
            // Skip first second color if i is even (opposite faces)
        int first_j = i % 2 == 0 ? i + 2 : i + 1;
        for (int j = first_j; j < 6; j++) {
            edge_piece* EdgePiece = &Cube->Edges[CurrentEdge];
            EdgePiece->Transform.Scale = Scale();
            EdgePiece->Position = Positions[i] + Positions[j];
            EdgePiece->Transform.Translation = L * V3(EdgePiece->Position.X, EdgePiece->Position.Y, -EdgePiece->Position.Z);
            EdgePiece->Transform.Rotation = Quaternion(1.0);
            CurrentEdge++;
            int Index[2] = { i, j };
            for (int n = 0; n < 2; n++) {
                EdgePiece->Faces[n].Color = Colors[Index[n]];
                EdgePiece->Faces[n].Transform.Scale = Scale();
                EdgePiece->Faces[n].Transform.Rotation = Rotations[Index[n]];
                EdgePiece->Faces[n].Transform.Translation = Rotate(V3(0, D, 0), Rotations[Index[n]]);
                EdgePiece->Faces[n].Collider = {
                    Cube->Position + EdgePiece->Transform.Translation + EdgePiece->Faces[n].Transform.Translation,
                    ColliderSizes[Index[n]]
                };
            }

            // Corners
            int first_k = j % 2 == 0 ? j + 2 : j + 1;
            for (int k = first_k; k < 6; k++) {
                int CornerIndex[3] = { i, j, k };
                corner_piece* CornerPiece = &Cube->Corners[CurrentCorner];
                CornerPiece->Transform.Scale = Scale();
                CornerPiece->Position = Positions[i] + Positions[j] + Positions[k];
                CornerPiece->Transform.Translation = L * V3(CornerPiece->Position.X, CornerPiece->Position.Y, -CornerPiece->Position.Z);
                CornerPiece->Transform.Rotation = Quaternion(1.0);
                CurrentCorner++;
                for (int n = 0; n < 3; n++) {
                    CornerPiece->Faces[n].Color = Colors[CornerIndex[n]];
                    CornerPiece->Faces[n].Transform.Scale = Scale();
                    CornerPiece->Faces[n].Transform.Rotation = Rotations[CornerIndex[n]];
                    CornerPiece->Faces[n].Transform.Translation = Rotate(V3(0, D, 0), Rotations[CornerIndex[n]]);
                    CornerPiece->Faces[n].Collider = {
                        Cube->Position + CornerPiece->Transform.Translation + CornerPiece->Faces[n].Transform.Translation,
                        ColliderSizes[CornerIndex[n]]
                    };
                }
            }
        }        
    }
}


// Game State: Persistent (between frames) values
struct game_state {
    memory_arena RenderArena;
    memory_arena StringsArena;
    memory_arena FontsArena;
    memory_arena VideoArena;
    memory_arena MeshArena;
    rubiks_cube Cube;
    UI UserInterface;
    double dt;
    double Time;
};


// Game Memory
struct game_memory {
    bool IsInitialized;
    uint64 PermanentStorageSize;
    void* PermanentStorage;
    platform_api Platform;
    game_assets Assets;
    render_group* Group;
    string DebugInfo;
};

#define GAME_UPDATE(name) void GAMELIBRARY_API name(game_memory* Memory, game_sound_buffer* PreviousSoundBuffer, game_sound_buffer* SoundBuffer, render_group* Group, game_input* Input)
typedef GAME_UPDATE(game_update);
