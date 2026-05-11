#include "Tests.h"

void RunOnceTests(game_memory* Memory) {
    render_group* Group = &Memory->RenderGroup;
    game_state* State = Memory->GameState;
    memory_arena* Permanent = &Memory->Permanent;
    game_input* Input = &Memory->Input;
    light* Light = &Memory->RenderGroup.Light;
    float Time = State->Time;
    bool Result = false;

    try { Result = TestFormat(); }
    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }
    catch(...) { Log(log_level::Error, "Unknown test error."); }
    if(Result) { Log(log_level::Test, "Test 'TestFormat' was correctly executed."); }
    else       { Log(log_level::Error, "Test 'TestFormat' failed."); }

    try { Result = TestFloatingPoint(); }
    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }
    catch(...) { Log(log_level::Error, "Unknown test error."); }
    if(Result) { Log(log_level::Test, "Test 'TestFloatingPoint' was correctly executed."); }
    else       { Log(log_level::Error, "Test 'TestFloatingPoint' failed."); }

    try { Result = TestDataFileManager(); }
    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }
    catch(...) { Log(log_level::Error, "Unknown test error."); }
    if(Result) { Log(log_level::Test, "Test 'TestDataFileManager' was correctly executed."); }
    else       { Log(log_level::Error, "Test 'TestDataFileManager' failed."); }

    try { Result = TestData(); }
    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }
    catch(...) { Log(log_level::Error, "Unknown test error."); }
    if(Result) { Log(log_level::Test, "Test 'TestData' was correctly executed."); }
    else       { Log(log_level::Error, "Test 'TestData' failed."); }

    try { Result = TestEntities(State); }
    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }
    catch(...) { Log(log_level::Error, "Unknown test error."); }
    if(Result) { Log(log_level::Test, "Test 'TestEntities' was correctly executed."); }
    else       { Log(log_level::Error, "Test 'TestEntities' failed."); }
}

void RunReloadTests(game_memory* Memory) {
    render_group* Group = &Memory->RenderGroup;
    game_state* State = Memory->GameState;
    memory_arena* Permanent = &Memory->Permanent;
    game_input* Input = &Memory->Input;
    light* Light = &Memory->RenderGroup.Light;
    float Time = State->Time;
    bool Result = false;
}

void RunEveryFrameTests(game_memory* Memory) {
    render_group* Group = &Memory->RenderGroup;
    game_state* State = Memory->GameState;
    memory_arena* Permanent = &Memory->Permanent;
    game_input* Input = &Memory->Input;
    light* Light = &Memory->RenderGroup.Light;
    float Time = State->Time;
    bool Result = false;
}