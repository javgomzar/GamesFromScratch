#include "pch.h"
#include "GameLibrary.h"

// Debug
void LogGameDebugRecords(render_group* Group);

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    if (Memory->HotReload) {
        RNG.Seed = SeedRNG();
        RNG.State = RNG.Seed;

        TimeRecords = (time_record*)&Memory->TimeRecordsLibrary;

#ifdef _WIN32
        LARGE_INTEGER PerfCountFrequencyResult;
        QueryPerformanceFrequency(&PerfCountFrequencyResult);
        Platform.PerformanceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
        
        Memory->HotReload = false;
    }

    render_group* Group = &Memory->RenderGroup;
    game_input* Input = &Memory->Input;
    game_state* State = Memory->GameState;
    game_assets* Assets = &Memory->Assets;
    game_entity_manager* EntityManager = &State->EntityManager;
    debug_info* DebugInfo = &Memory->DebugInfo;
{
    TIMED_BLOCK;

    float Time = State->Time;
    camera* ActiveCamera = State->ActiveCamera;

    bool FirstFrame = false;
    if (!Memory->IsInitialized) {
        FirstFrame = true;

        //TestPerformance();

        Transition(State, Game_State_Main_Menu);

        // Initialize camera
        ActiveCamera = AddCamera(EntityManager, V3(0, 3.2f, 0), -45.0f, 22.5f);

        Memory->IsInitialized = true;
    }

    PushClear(Group, Black, Target_None);
    PushClear(Group, { 0 }, Target_World);
    PushClear(Group, { 0 }, Target_Outline);
    PushClear(Group, { 0 }, Target_Postprocessing_Outline);
    PushClear(Group, Magenta, Target_PingPong);
    PushClear(Group, Black, Target_Output);

    UpdateGameState(Group, State, Input);
    
    // GameOutputSound(Assets, SoundBuffer, State, Input);

    PushEntities(Group, ActiveCamera, State, Input, Time);

    DEBUG_VALUE(State->Type, game_state_type);

    UpdateUI(Memory, Input);

    PushSky(Group);

    PushRenderTarget(Group, Target_World, Target_Output);

    static bool Screenshot = false;
    if (Input->Keyboard.F10.WasDown && !Input->Keyboard.F10.IsDown) {
        Screenshot = true;
    }

    static double ScreenRectAlpha = 1.0;
    if (Screenshot) {
        ScreenRectAlpha -= 0.05;
        if (ScreenRectAlpha < 0.0) {
            Screenshot = false;
            ScreenRectAlpha = 1.0;
        }
        else {
            rectangle ScreenRect = { 0, 0, (float)Group->Width, (float)Group->Height };
            PushRect(Group, ScreenRect, ChangeAlpha(White, ScreenRectAlpha), SORT_ORDER_PUSH_RENDER_TARGETS - 5.0);
        }
    }
    PushRenderTarget(Group, Target_Output, Target_None, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
}
    Memory->nTimeRecordsLibrary = __COUNTER__;
    if (Group->Debug) {
        PushTimeRecords(
            Group, 
            Memory->nTimeRecordsLibrary, Memory->TimeRecordsLibrary, 
            Memory->nTimeRecordsPlatform, Memory->TimeRecordsPlatform
        );
    }
}
