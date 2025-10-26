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
        
        Memory->HotReload = false;
    }

    render_group* Group = &Memory->RenderGroup;
    game_input* Input = &Memory->Input;
    game_state* pGameState = Memory->GameState;
    game_assets* Assets = &Memory->Assets;
    game_entity_state* EntityState = &pGameState->Entities;
    debug_info* DebugInfo = &Memory->DebugInfo;
{
    TIMED_BLOCK;

    float Time = pGameState->Time;
    camera* ActiveCamera = pGameState->ActiveCamera;

    bool firstFrame = false;
    if (!Memory->IsInitialized) {
        firstFrame = true;

        //TestPerformance();

        // Initialize entities
        ActiveCamera = AddCamera(EntityState, V3(0, 3.2f, 0), -45.0f, 22.5f);
        ActiveCamera->OnAir = true;
        character* Character = AddCharacter(Assets, EntityState, V3(0,0,0), 100);
        prop* Prop = AddProp(EntityState, Mesh_Sphere_ID, Shader_Pipeline_Mesh_ID, Red, V3(0,0,5), Quaternion(1.0), Scale(10,1,1));
        enemy* Enemy = AddEnemy(EntityState, V3(10,0,5));
        weapon* Sword = AddWeapon(EntityState, Weapon_Sword, White, V3(-5,0,0));
        weapon* Shield = AddWeapon(EntityState, Weapon_Shield, White, V3(-10,0,0));
        Equip(Sword, Character);
        Equip(Shield, Character);

        pGameState->Emitter = AllocateParticleEmitter(&Memory->Permanent, 200);
        SetParticleEmitterCircle(pGameState->Emitter, V3(0,0,0), 1.0f, V3(0,1,0));
        pGameState->Emitter->ParticleLifetime = 2.0f;

        Memory->IsInitialized = true;
    }

    PushClear(Group, Orange, Target_None);
    PushClear(Group, { 0 }, Target_World);
    PushClear(Group, { 0 }, Target_Outline);
    PushClear(Group, { 0 }, Target_Postprocessing_Outline);
    PushClear(Group, Magenta, Target_PingPong);
    PushClear(Group, BackgroundBlue, Target_Output);

    UpdateGameState(Assets, pGameState, Input, Group->Width, Group->Height);
    
    //GameOutputSound(Assets, SoundBuffer, pGameState, Input);

    // PushEntities(Group, &pGameState->Entities, Input, Time);

    TestRendering(Group, Input, Time);

    Update(Group, ActiveCamera->Position, pGameState->Emitter, pGameState->dt);
    
    UpdateUI(Memory, Input);

    PushRenderTarget(Group, Target_World);

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
    PushRenderTarget(Group, Target_Output, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
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
