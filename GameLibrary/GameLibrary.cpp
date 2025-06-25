// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "GameLibrary.h"
#include "GameUI.h"

//// This is an example of an exported variable
//GAMELIBRARY_API int nGameLibrary=0;
//
//// This is an example of an exported function.
//GAMELIBRARY_API int fnGameLibrary(void)
//{
//    return 0;
//}
//
//// This is the constructor of a class that has been exported.
//CGameLibrary::CGameLibrary()
//{
//    return;
//}

// Game logic
bool Collision(rectangle Rect, v3 Cursor) {
    return Cursor.X > Rect.Left &&
        Cursor.X < Rect.Left + Rect.Width &&
        Cursor.Y > Rect.Top &&
        Cursor.Y < Rect.Top + Rect.Height;
}

// Sound
void GameOutputSound(game_assets* Assets, game_sound_buffer* pSoundBuffer, game_state* pGameState, game_input* Input) {
    
    Silence(pSoundBuffer);

    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
    //WriteSineWave(pSoundBuffer, 480, 0);
}

// Debug
void LogGameDebugRecords(render_group* Group, memory_arena* TransientArena);

void TestPerformance() {
    //TIMED_BLOCK;
    
}

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    memory_arena* StringsArena = &Memory->StringsArena;
    memory_arena* TransientArena = &Memory->TransientArena;
    memory_arena* GeneralPurposeArena = &Memory->GeneralPurposeArena;

    render_group* Group = &Memory->RenderGroup;
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    game_entity_list* EntityList = &pGameState->EntityList;
    {
    TIMED_BLOCK;

    float Time = pGameState->Time;
    camera* ActiveCamera = Group->Camera;

    bool firstFrame = false;
    if (!Memory->IsInitialized) {
        firstFrame = true;

        //TestPerformance();

        // Initialize entities
        game_entity_list* Entities = &pGameState->EntityList;
        pGameState->Combat.Entities = Entities;
        Group->Camera = AddCamera(Entities, V3(0, 3.2f, 0), -45.0f, 22.5f);
        character* Character = AddCharacter(Entities, V3(0,0,0), 100);
        enemy* Enemy1 = AddEnemy(Entities, V3(10,0,5), 30);
        enemy* Enemy2 = AddEnemy(Entities, V3(10,0,0), 30);
        enemy* Enemy3 = AddEnemy(Entities, V3(10,0,-5), 30);
        weapon* Sword = AddWeapon(Entities, Weapon_Sword, White, V3(-5,0,0));
        weapon* Shield = AddWeapon(Entities, Weapon_Shield, White, V3(-10,0,0));
        Equip(Sword, Character);
        Equip(Shield, Character);

        Memory->IsInitialized = true;
    }

    PushClear(Group, Orange, Target_None);
    PushClear(Group, { 0 }, Target_World);
    PushClear(Group, { 0 }, Target_Outline);
    PushClear(Group, { 0 }, Target_Postprocessing_Outline);
    PushClear(Group, Magenta, Target_PingPong);
    PushClear(Group, BackgroundBlue, Target_Output);

    Update(&Group->Camera, pGameState, Input, Group->Width, Group->Height);
    
    //GameOutputSound(Assets, SoundBuffer, pGameState, Input);
    
    UpdateUI(Memory, Input);

    PushEntities(Group, &pGameState->EntityList, Input, Time);

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
            PushRect(Group, ScreenRect, Color(White, ScreenRectAlpha), SORT_ORDER_PUSH_RENDER_TARGETS - 5.0);
        }
    }
    PushRenderTarget(Group, Target_Output, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
    }

    LogGameDebugRecords(Group, TransientArena);
}

debug_record DebugRecordArray[__COUNTER__];

void LogGameDebugRecords(render_group* Group, memory_arena* TransientArena) {
    char Buffer[512];
    int Height = Group->Height - 20;
    for (int i = 0; i < ArrayCount(DebugRecordArray); i++) {
        debug_record* DebugRecord = DebugRecordArray + i;

        if (DebugRecord->HitCount) {
            if (DebugRecord->HitCount == 1) {
                sprintf_s(Buffer, "%s: (%i hit) %.02f Mcycles (%s:%i)\n", 
                    DebugRecord->FunctionName, DebugRecord->HitCount, DebugRecord->CycleCount / 1000000.0f, DebugRecord->FileName, DebugRecord->LineNumber);
            }
            else {
                sprintf_s(Buffer, "%s: (%i hits) Total: %.02f Mcycles, Average: %.02f (%s:%i)\n", 
                    DebugRecord->FunctionName, DebugRecord->HitCount, DebugRecord->CycleCount / 1000000.0f, 
                    (float)DebugRecord->CycleCount / (1000000.0f * (float)DebugRecord->HitCount), DebugRecord->FileName, DebugRecord->LineNumber);
            }
            string String = PushString(TransientArena, 512, Buffer);
            if (Group->Debug) PushText(Group, V2(250, Height), Font_Menlo_Regular_ID, String, White, 8, false, SORT_ORDER_DEBUG_OVERLAY);
            Height -= 17;
            DebugRecord->HitCount = 0;
            DebugRecord->CycleCount = 0;
        }
    }
}