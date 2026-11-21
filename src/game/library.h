#pragma once

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

#include <platform.h>
#include "enum.h"
#include "math.h"
#include "geometry.h"
#include "input.h"
#include "asset.h"
#include "render.h"
//#include "particles.h"
#include "entity.h"
#include "debug.h"
#include "renderer.h"


/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Game memory                                                                                                                     |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

struct Context {
    Asset_Manager asset_manager;
    Render_Group render_group;
    Input input;
    State* state;
    // Time_Record time_records_library[MAX_TIME_RECORDS];
    // Time_Record time_records_platform[MAX_TIME_RECORDS];
    // uint16 time_records_library_count;
    // uint16 time_records_platform_count;
    float fps;
    float used_time;
    float used_mcycles_per_frame;
    float used_memory;
    bool initialized;
    bool hot_reload;
    bool running;
    bool test;
    bool pause;
};

// #include "GameUI.h" 

extern "C" GAMELIBRARY_API void GameUpdate(Memory& memory);
