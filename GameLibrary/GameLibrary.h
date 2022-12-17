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
#include "stdint.h"

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// Utility macros
#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value)*1024)

// Platform independent constants
static float Pi = 3.14159265359f;
static float Tau = 2.0f * Pi;
static float twroot = 1.05946309436f;

// Platform independent structs and types
struct game_rect {
    int Top;
    int Left;
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
};

struct game_screen_position {
    uint32 X;
    uint32 Y;
    uint32 Z;
};

// Color
struct color {
    uint8 R;
    uint8 G;
    uint8 B;
};

static int Attenuation = 100;
static color Black = { 0, 0, 0 };
static color White = { 255 - Attenuation, 255 - Attenuation, 255 - Attenuation };
static color Gray = { 127 - Attenuation, 127 - Attenuation, 127 - Attenuation };
static color Red = { 255 - Attenuation, 0, 0 };
static color Green = { 0, 220 - Attenuation, 0 };
static color Blue = { 0, 0, 255 - Attenuation };
static color Magenta = { 255 - Attenuation, 0, 255 - Attenuation };
static color Yellow = { 255 - Attenuation, 255 - Attenuation, 0 };
static color Cyan = { 0, 220 - Attenuation, 255 - Attenuation };
static color Orange = { 255 - Attenuation, 160 - Attenuation, 0 };
static color LightGray = {200, 200, 200};
static color DarkGray = {100, 100, 100};

uint32 GetColorBytes(color Color) {
    return (Color.R << 16) | (Color.G << 8) | Color.B;
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
    game_button_state Up;
    game_button_state Down;
    game_button_state Left;
    game_button_state Right;
    game_button_state Escape;
    game_button_state Space;
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

struct game_board {
    int Width;
    int Height;
    bool* IsAlive;
};

const int BOARD_WIDTH = 60;
const int BOARD_HEIGHT = 40;
uint8 SQUARE_LENGTH = 20;
color ALIVE_COLOR = Green;
color DEAD_COLOR = Magenta;

struct game_state {
    game_board Board;
};

struct record_and_playback {
    HANDLE RecordFile;
    int RecordIndex;
    HANDLE PlaybackFile;
    int PlaybackIndex;
    void* GameMemoryBlock;
    uint64 TotalSize;
};

// Services that the platform layer provides for the game
struct read_file_result {
    uint32 ContentSize;
    void* Content;
};

#define PLATFORM_READ_ENTIRE_FILE(name) read_file_result name(char* Filename)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_WRITE_ENTIRE_FILE(name) bool name(char* Filename, uint64 MemorySize, void* Memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

// Services that the game provides to the platform layer
struct game_memory {
    bool IsInitialized;
    uint64 PermanentStorageSize;
    void* PermanentStorage;
    platform_read_entire_file* PlatformReadEntireFile;
    platform_free_file_memory* PlatformFreeFileMemory;
    platform_write_entire_file* PlatformWriteEntireFile;
};

#define GAME_UPDATE_AND_RENDER(name) void GAMELIBRARY_API name(game_memory* Memory, game_sound_buffer* PreviousSoundBuffer, game_sound_buffer* SoundBuffer, game_offscreen_buffer* ScreenBuffer, game_input* Input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender);