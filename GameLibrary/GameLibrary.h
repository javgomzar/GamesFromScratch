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
//	// Add your methods here.
//};

extern GAMELIBRARY_API int nGameLibrary;

#ifndef GAME_LIBRARY
#define GAME_LIBRARY

#pragma once
#include "GamePlatform.h"
#include "GameMath.h"
#include "GameInput.h"
#include "GameAssets.h"
#include "GameSound.h"

#include "RenderGroup.h"

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Game memory                                                                                                                     |
// +---------------------------------------------------------------------------------------------------------------------------------+

/*
    This struct will manage all necessary memory for the game. The current layout of this memory will be
        - First, the `game_state`.
        - Second, vertex and element buffers for rendering.
        - Then, memory that will be managed by memory arenas. Strings, transient memory (erased each memory) and general purpose arena.
*/
struct game_memory {
    game_assets Assets;
    platform_api Platform;
    render_group RenderGroup;
    memory_arena StringsArena;
    memory_arena GeneralPurposeArena;
    memory_arena TransientArena;
    memory_arena TurnsArena;
    debug_info DebugInfo;
    uint64 PermanentStorageSize;
    void* PermanentStorage;
    bool IsInitialized;
};

#define GAME_UPDATE_INPUTS game_memory* Memory, game_sound_buffer* PreviousSoundBuffer, game_sound_buffer* SoundBuffer, game_input* Input
#define GAME_UPDATE(name) GAMELIBRARY_API void name(GAME_UPDATE_INPUTS)
typedef GAME_UPDATE(game_update);

#endif