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

// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H


// Platform independent structs and types
struct game_rect {
    int Left;
    int Top;
    int Width;
    int Height;
};

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

struct game_screen_position {
    int32 X;
    int32 Y;
    int32 Z;
};

game_screen_position ToScreenPosition(v3 V) {
    game_screen_position Result;
    Result.X = CustomRound(V.X);
    Result.Y = CustomRound(V.Y);
    Result.Z = CustomRound(V.Z);
    return Result;
}

v3 ToV3(game_screen_position Position) {
    v3 Result;
    double X = Position.X;
    double Y = Position.Y;
    double Z = Position.Z;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    return Result;
}


// Color
struct color {
    float Alpha;
    float R;
    float G;
    float B;
};

static int Attenuation = 100;
static color Black = { 1.0f, 0.0f, 0.0f, 0.0f };
static color White = { 1.0f, 1.0f, 1.0f, 1.0f };
static color Gray = { 1.0f, 0.5f, 0.5f, 0.5f };
static color Red = { 1.0f, 1.0f, 0.0f, 0.0f };
static color Green = { 1.0f, 0.0f, 1.0f, 0.0f };
static color Blue = { 1.0f, 0.0f, 0.0f, 1.0f };
static color Magenta = { 1.0f, 1.0f, 0.0f, 1.0f };
static color Yellow = { 1.0f, 1.0f, 1.0f, 0.0f };
static color Cyan = { 1.0f, 0.0f, 1.0f, 1.0f };
static color Orange = { 1.0f, 1.0f, 0.63f, 0.0f };
static color BackgroundBlue = { 1.0f, 0.4f, 0.4f, 0.8f };

uint32 GetColorBytes(color Color) {
    uint8 Alpha = Color.Alpha * 255.0f;
    uint8 R = Color.R * 255.0f;
    uint8 G = Color.G * 255.0f;
    uint8 B = Color.B * 255.0f;
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
    Color.R = (float)((RedMask & Bytes) >> RedShift) / 255.0f;
    Color.G = (float)((GreenMask & Bytes) >> GreenShift) / 255.0f;
    Color.B = (float)((BlueMask & Bytes) >> BlueShift) / 255.0f;
    Color.Alpha = (float)((AlphaMask & Bytes) >> AlphaShift) / 255.0f;
    return Color;
}

color Blend(color Color, color Background) {
    color Result;
    float Alpha = (float)(Color.Alpha) / 255.0f;
    Result.R = Background.R + (Alpha * (Color.R - Background.R) + 0.5f);
    Result.G = Background.G + (Alpha * (Color.G - Background.G) + 0.5f);
    Result.B = Background.B + (Alpha * (Color.B - Background.B) + 0.5f);
    Result.Alpha = 255;
    return Result;
}

// Joysticks values should be floats between 0 and 1
/*struct joystick_coordinate {
    float Start;
    float Max;
    float Min;
    float End;
};*/

// Fonts
struct text {
    int Length;
    color Color;
    int Points;
    bool Wrapped;
    char* Content;
};

struct text_options {
    int Length;
    color Color;
    int Points;
    bool Wrapped;
};

struct game_joystick_state {
    float X;
    float Y;
};

struct game_controller_input {
    game_joystick_state LeftJoystick;
    game_joystick_state RightJoystick;
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

struct game_keyboard_input {
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
    game_button_state W;
    game_button_state A;
    game_button_state S;
    game_button_state D;
    game_button_state Q;
    game_button_state E;
    game_button_state F;
    game_button_state Up;
    game_button_state Down;
    game_button_state Left;
    game_button_state Right;
    game_button_state Escape;
    game_button_state Space;
    game_button_state Enter;
    game_button_state F1;
};

struct game_mouse_input {
    game_button_state LeftClick;
    game_button_state RightClick;
    game_screen_position Cursor;
};

struct game_input {
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



// User Interface
struct button {
    bool Clicked;
    bool Active;
    game_rect Collider;
    loaded_bmp Image;
    loaded_bmp ClickedImage;
    text Text;
    FT_Face* Face;
};

struct UI {

};


// Game Assets
struct game_assets {
    Character* Characters;
    loaded_bmp PlayerBMP;
    loaded_bmp FloorBMP;
    loaded_bmp DoorBMP;
};


// Game structs
struct camera {
    v3 Position;
    v3 Velocity;
};


void UpdateCamera(camera* Camera) {
    Camera->Position = Camera->Position + Camera->Velocity;
}

// Tiles
struct tile_position {
    int Row;
    int Col;
    int Z;
};

game_screen_position ToScreenCoord(tile_position Position, int TileSize) {
    return { TileSize * Position.Col, TileSize * Position.Row, Position.Z };
}

struct tile_direction {
    int Row;
    int Col;
    int Z;
};

tile_position operator+(tile_position P, tile_direction D) {
    return { P.Row + D.Row, P.Col + D.Col, P.Z + D.Z};
}

tile_position operator+(tile_direction D, tile_position P) {
    return { P.Row + D.Row, P.Col + D.Col, P.Z + D.Z };
}

enum tile_type {
    Floor,
    Door,
    Wall
};

struct tile {
    tile_type Type;
};

struct tile_pointer {
    int Row;
    int Col;
    tile_direction Direction;
    int IdleSteps;
};

struct room {
    int ID;
    int Top;
    int Left;
    int Width;
    int Height;
    bool Explored;
};

const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 100;

const int MAX_ROOMS = 500;

// Game State: Persistent (between frames) values
struct game_state {
    memory_arena TextArena;
    memory_arena RenderArena;
    memory_arena MapArena;
    UI UserInterface;
    camera Camera;
    int TileSize;
    tile_position PlayerPosition;
    tile Map[MAP_HEIGHT][MAP_WIDTH];
    int nRooms;
    room Rooms[MAX_ROOMS];
};

// Game Memory
struct game_memory {
    bool IsInitialized;
    uint64 PermanentStorageSize;
    void* PermanentStorage;
    platform_api Platform;
    game_assets Assets;
    FT_Library FTLibrary;
    render_group* Group;
    char* DebugInfo;
};

#define GAME_UPDATE_AND_RENDER(name) void GAMELIBRARY_API name(game_memory* Memory, game_sound_buffer* PreviousSoundBuffer, game_sound_buffer* SoundBuffer, game_offscreen_buffer* ScreenBuffer, game_input* Input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);