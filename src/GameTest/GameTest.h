#include "Tests.h"

void RunOnceTests(Memory* memory) {
    Render_Group* group = &memory->render_group;
    Asset_Manager* asset_manager = &memory->asset_manager;
    State* state = memory->state;
    Arena* permanent = &memory->permanent;
    Arena* transient = &memory->transient;
    Input* input = &memory->input;
    Light* light = &memory->render_group.light;
    float time = state->time;
    bool result = false;

    try { result = TestFormat(); }
    catch(const char* error_message) { Log(Log_Level::Error, error_message); }
    catch(...) { Log(Log_Level::Error, "Unknown test error."); }
    if(result) { Log(Log_Level::Test, "Test 'TestFormat' was correctly executed."); }
    else       { Log(Log_Level::Error, "Test 'TestFormat' failed."); }
}

void RunReloadTests(Memory* memory) {
    Render_Group* group = &memory->render_group;
    Asset_Manager* asset_manager = &memory->asset_manager;
    State* state = memory->state;
    Arena* permanent = &memory->permanent;
    Arena* transient = &memory->transient;
    Input* input = &memory->input;
    Light* light = &memory->render_group.light;
    float time = state->time;
    bool result = false;
}

void RunEveryFrameTests(Memory* memory) {
    Render_Group* group = &memory->render_group;
    Asset_Manager* asset_manager = &memory->asset_manager;
    State* state = memory->state;
    Arena* permanent = &memory->permanent;
    Arena* transient = &memory->transient;
    Input* input = &memory->input;
    Light* light = &memory->render_group.light;
    float time = state->time;
    bool result = false;
}