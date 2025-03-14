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
#include "GameInput.h"

#include "GameAssets.h"

#include "GameSound.h"

#include "RenderGroup.h"

// Game State: Persistent (between frames) values
struct game_state {
    memory_arena RenderArena;
    memory_arena StringsArena;
    memory_arena GeneralPurposeArena;
    memory_arena TransientArena;
    user_interface UI;
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
