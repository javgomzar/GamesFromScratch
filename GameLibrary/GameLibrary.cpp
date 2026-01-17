#include "pch.h"
#include "GameLibrary.h"

// Sound
void GameOutputSound(game_assets* Assets, game_sound_buffer* pSoundBuffer, game_state* pGameState, game_input* Input) {
    
    Silence(pSoundBuffer);

    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
    //WriteSineWave(pSoundBuffer, 480, 0);
}

// Debug
void LogGameDebugRecords(render_group* Group);

void TestPerformance() {
    //TIMED_BLOCK;
    
}

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
    game_entity_state* EntityState = &State->Entities;
    debug_info* DebugInfo = &Memory->DebugInfo;
{
    TIMED_BLOCK;

    float Time = State->Time;
    camera* ActiveCamera = State->ActiveCamera;

    bool FirstFrame = false;
    if (!Memory->IsInitialized) {
        FirstFrame = true;

        //TestPerformance();

        // Initialize entities
        ActiveCamera = AddCamera(EntityState, V3(0, 3.2f, 0), -45.0f, -90.0f);
        ActiveCamera->OnAir = true;

        Initialize(State, &Memory->Permanent);

        Memory->IsInitialized = true;
    }

    PushClear(Group, Black, Target_None);
    PushClear(Group, { 0 }, Target_World);
    PushClear(Group, { 0 }, Target_Outline);
    PushClear(Group, { 0 }, Target_Postprocessing_Outline);
    PushClear(Group, Magenta, Target_PingPong);
    PushClear(Group, Black, Target_Output);

    UpdateGameState(Assets, State, Input, Group->Width, Group->Height);

    PushSky(Group);
    
    // GameOutputSound(Assets, SoundBuffer, State, Input);

    PushSky(Group);
    PushStars(Group, State);

    DEBUG_VALUE(State->ActiveCamera->Pitch, float);
    
    UpdateUI(Memory, Input);

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
