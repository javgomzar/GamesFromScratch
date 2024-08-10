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

// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H


// Platform independent structs and types
struct game_triangle {
    union {
        v3 Points[3];
        struct {
            v3 Point0;
            v3 Point1;
            v3 Point2;
        };
    };
};

struct game_rect {
    double Left;
    double Top;
    double Width;
    double Height;
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
    double X;
    double Y;
    double Z;
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
    double Alpha;
    double R;
    double G;
    double B;
};

bool operator==(color A, color B) {
    return A.Alpha == B.Alpha && A.R == B.R && A.G == B.G && A.B == B.B;
}

bool operator!=(color A, color B) {
    return A.Alpha != B.Alpha || A.R != B.R || A.G != B.G || A.B != B.B;
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

const int NUMBER_OF_KEYS = 55;

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
        };
    };
};

struct game_mouse_input {
    game_button_state LeftClick;
    game_button_state RightClick;
    game_screen_position Cursor;
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


// Sounds
struct game_sound {
    uint32 SampleCount;
    uint32 Played;
    int16* SampleOut;
};

struct waveformat {
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
    WORD    cbSize;            /* The count in bytes of the size of extra information (after cbSize) */
};

// Video
struct game_video {
    video_context* VideoContext;
    int Handle;
    bool Loop;
    double TimeElapsed;
};

// Game Assets
struct game_assets {
    character* Characters;
    loaded_bmp HeadBMP;
    loaded_bmp ArmBMP;
    loaded_bmp LegBMP;
    loaded_bmp TorsoBMP;
    loaded_bmp SwordBMP;
    loaded_bmp EnemyBMP1;
    loaded_bmp EnemyBMP2;
    loaded_bmp EnemyBMP3;
    loaded_bmp EnemyBMP4;
    game_sound TestSound;
    game_video TestVideo;
    string RenderArenaStr;
    string RenderPercentageStr;
    string VideoArenaStr;
    string VideoPercentageStr;
    string TextArenaStr;
    string TextPercentageStr;
};

// Game specific structs
struct entity {
    v3 Position;
    v3 Velocity;
    double Time;
};

// Bones
const int MAX_BONE_CHILD = 8;

struct bone {
    bone* Parent;
    int nChildren;
    bone* Children[MAX_BONE_CHILD];
    loaded_bmp* BMP;
    v3 BMPOffset;
    v3 Start;
    v3 Finish;
    double Length;
    basis Basis;
    bool FlipX;
    bool FlipY;
};

void AddChild(bone* Parent, bone* Child) {
    Parent->Children[Parent->nChildren] = Child;
    Parent->nChildren++;
    Child->Parent = Parent;
}

void RemoveChild(bone* Parent, bone* Child) {
    bool Removed = false;
    for (int i = 0; i < Parent->nChildren; i++) {
        if (Parent->Children[i] == Child) {
            Removed = true;
        }
        if (Removed) {
            if (i == Parent->nChildren - 1) {
                Parent->nChildren--;
                Child->Parent = 0;
                break;
            }
            else {
                Parent->Children[i] = Parent->Children[i + 1];
            }
        }
    }
}

void Bone(bone* Bone, loaded_bmp* BMP, bone* Parent, v3 BMPOffset, v3 Start, v3 Finish, basis Basis, bool FlipX = false, bool FlipY = false) {
    Bone->BMP = BMP;
    if (Parent) {
        AddChild(Parent, Bone);
    }
    Bone->BMPOffset = BMPOffset;
    Bone->Start = Start;
    Bone->Finish = Finish;
    Bone->Length = module(Finish - Start);
    Bone->Basis = Basis;
    Bone->FlipX = FlipX;
    Bone->FlipY = FlipY;
}

void Rotate(bone* Bone, double Angle, v3 RotationCenter) {
    Bone->Start = Rotate(Bone->Start, Angle, RotationCenter);
    Bone->Finish = Rotate(Bone->Finish, Angle, RotationCenter);
    Bone->BMPOffset = Rotate(Bone->BMPOffset, Angle);
    Bone->Basis = Rotate(Bone->Basis, Angle);
    for (int i = 0; i < Bone->nChildren; i++) {
        Rotate(Bone->Children[i], Angle, RotationCenter);
    }
}

void Rotate(bone* Bone, double Angle) {
    Bone->Basis = Rotate(Bone->Basis, Angle);
    Bone->Finish = Bone->Start + Rotate(Bone->Finish - Bone->Start, Angle);
    Bone->BMPOffset = Rotate(Bone->BMPOffset, Angle);
    for (int i = 0; i < Bone->nChildren; i++) {
        Rotate(Bone->Children[i], Angle, Bone->Start);
    }
}

void Flip(bone* Bone, bool FlipX = false, bool FlipY = false) {
    if (FlipX) {
        Bone->FlipX = !Bone->FlipX;
    }
    if (FlipY) {
        Bone->FlipY = !Bone->FlipY;
    }
}

enum player_bone_id {
    Head,
    Spine,
    LeftShoulder,
    RightShoulder,
    LeftArm,
    RightArm,
    LeftHip,
    RightHip,
    LeftLeg,
    RightLeg,
    Sword
};

struct player_bone {
    player_bone_id Id;
    bone Bone;
};

void PlayerBone(
    player_bone_id Id, 
    player_bone* PlayerBone, 
    loaded_bmp* BMP, 
    player_bone* Parent, 
    v3 BMPOffset, 
    v3 Start, v3 Finish, 
    basis Basis, 
    bool FlipX = false, bool FlipY = false
) {
    PlayerBone->Id = Id;
    bone* ParentBone = Parent ? &Parent->Bone : 0;
    Bone(&PlayerBone->Bone, BMP, ParentBone, BMPOffset, Start, Finish, Basis, FlipX, FlipY);
}

enum combatant_type {
    Player,
    Enemy
};

struct stats {
    combatant_type Type;
    int ATB;
    int HP;
    int MaxHP;
    int Strength;
    int Defense;
    int Speed;
    bool Busy;
};

enum player_animation {
    Player_Idle,
    Player_Defending,
    Player_Attacking,
    Player_Walking
};

struct player {
    stats Stats;
    entity Entity;
    player_animation Animation;
    union {
        player_bone Skeleton[12];
        struct {
            player_bone LeftHip;
            player_bone RightHip;
            player_bone LeftLeg;
            player_bone RightLeg;
            player_bone Spine;
            player_bone Head;
            player_bone LeftShoulder;
            player_bone RightShoulder;
            player_bone LeftArm;
            player_bone RightArm;
            player_bone Sword;
        };
    };
};

enum enemy_animation {
    Enemy_Idle,
    Enemy_Defending,
    Enemy_Attacking
};

struct enemy {
    stats Stats;
    entity Entity;
    loaded_bmp* BMP;
    enemy_animation Animation;
};

// User Interface
struct button {
    bool Clicked;
    bool Active;
    game_rect Collider;
    loaded_bmp Image;
    loaded_bmp ClickedImage;
    string Text;
};

struct combat_menu {
    bool Active;
    int Cursor;
    string AttackText;
    string TechniqueText;
    string MagicText;
    string ItemsText;
};

const int MAX_TURN_QUEUE_LENGTH = 10;
const int MAX_COMBATANTS = 10;
struct turn_queue {
    int CurrentTurn;
    int Queue[MAX_TURN_QUEUE_LENGTH];
    int ShowTurns;
    int Combatants;
    stats* Stats[MAX_COMBATANTS];
    loaded_bmp* BMPs[MAX_COMBATANTS];
    v3 BMPOffsets[MAX_COMBATANTS];
    double Scales[MAX_COMBATANTS];
    v3 RectOffsets[MAX_COMBATANTS];
};

void NextTurn(turn_queue* TurnQueue) {
    for (int i = 0; i < TurnQueue->Combatants; i++) {
        TurnQueue->Stats[i]->ATB += TurnQueue->Stats[i]->Speed;
        if (TurnQueue->Stats[i]->ATB > 100) TurnQueue->Stats[i]->ATB = 100;
    }
    if (++TurnQueue->CurrentTurn == MAX_TURN_QUEUE_LENGTH) TurnQueue->CurrentTurn = 0;
}

void InitializeQueue(turn_queue* TurnQueue) {
    int Filled = 0;
    int Speeds[MAX_COMBATANTS] = { 0 };
    int ATBs[MAX_COMBATANTS] = { 0 };
    int FullATBs[MAX_COMBATANTS] = { 0 };
    for (int i = 0; i < TurnQueue->Combatants; i++) {
        Speeds[i] = TurnQueue->Stats[i]->Speed;
        ATBs[i] = TurnQueue->Stats[i]->ATB;
    }

    int Actions = 0;
    while (Filled < MAX_TURN_QUEUE_LENGTH) {
        Actions = 0;
        for (int i = 0; i < TurnQueue->Combatants; i++) {
            if (ATBs[i] == 100) {
                FullATBs[Actions++] = i;
            }
        }

        if (Actions > 0) {
            int Action = FullATBs[0];
            int MaxSpeed = Speeds[Action];
            for (int i = 1; i < Actions; i++) {
                if (Speeds[FullATBs[i]] > MaxSpeed) {
                    Action = FullATBs[i];
                    MaxSpeed = Speeds[Action];
                }
            }
            TurnQueue->Queue[Filled++] = Action;
            ATBs[Action] = 0;
        }
        else {
            for (int i = 0; i < TurnQueue->Combatants; i++) {
                ATBs[i] += Speeds[i];
                if (ATBs[i] > 100) ATBs[i] = 100;
            }
        }
    }
}

struct UI {
    combat_menu CombatMenu;
    turn_queue TurnQueue;
};

// Game State: Persistent (between frames) values
struct game_state {
    memory_arena TextArena;
    memory_arena RenderArena;
    memory_arena VideoArena;
    UI UserInterface;
    bool ShowDebugInfo;
    double dt;
    double Time;
    player Player;
    enemy Enemy;
    stats* Turn;
    character Characters[];
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