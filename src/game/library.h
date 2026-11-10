#if _WIN32
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
#else
#define GAMELIBRARY_API
#endif

#pragma once

#include <platform.h>
#include "enum.h"
#include "math.h"
#include "input.h"
#include "asset.h"
//#include "render.h"
//#include "particles.h"
//#include "state.h"
//#include "debug.h"

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Game memory                                                                                                                     |
// +---------------------------------------------------------------------------------------------------------------------------------+

struct Memory {
    Asset_Manager asset_manager;
    Render_Group render_group;
    Input input;
    State* state;
    Arena permanent;
    Arena transient;
    Debug_Info debug_info;
    Time_Record time_records_library[MAX_TIME_RECORDS];
    Time_Record time_records_platform[MAX_TIME_RECORDS];
    uint16 time_records_library_count;
    uint16 time_records_platform_count;
    bool initialized;
    bool hot_reload;
    bool running;
    bool test;
    bool pause;
};

// #include "GameUI.h"

#define GAME_UPDATE_INPUTS Memory* memory, game_sound_buffer* PreviousSoundBuffer, game_sound_buffer* SoundBuffer
#define GAME_UPDATE(name) GAMELIBRARY_API void name(GAME_UPDATE_INPUTS)
typedef GAME_UPDATE(Game_Update);
