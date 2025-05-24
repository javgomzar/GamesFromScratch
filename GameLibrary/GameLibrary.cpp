// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"
#include "RenderGroup.h"


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

    double Time = pGameState->Time;
    camera* ActiveCamera = Group->Camera;

    bool firstFrame = false;
    if (!Memory->IsInitialized) {
        firstFrame = true;

        SetUpDebugArena(StringsArena, &Group->VertexBuffer.VertexArena[0], "Vertex arena (vec3)");
        SetUpDebugArena(StringsArena, &Group->VertexBuffer.VertexArena[1], "Vertex arena (vec3, vec2)");
        SetUpDebugArena(StringsArena, &Group->VertexBuffer.VertexArena[2], "Vertex arena (vec3, vec2, vec3)");
        SetUpDebugArena(StringsArena, &Group->VertexBuffer.VertexArena[3], "Vertex arena (mesh with bones)");
        SetUpDebugArena(StringsArena, TransientArena, "Transient arena");
        SetUpDebugArena(StringsArena, GeneralPurposeArena, "General purpose arena");
        SetUpDebugArena(StringsArena, StringsArena, "Strings arena");

        //TestPerformance();

        // Initialize entities
        game_entity_list* Entities = &pGameState->EntityList;
        AddCamera(Entities);
        AddCharacter(Entities, V3(0,0,0), 100);
        AddEnemy(Entities, V3(10,0,0), 100);
        AddProp(Entities, Mesh_Sphere_ID, Shader_Pipeline_Sphere_ID, Red, V3(0,0,10), Quaternion(1.0f, 0.0f, 0.0f, 0.0f), Scale(10,2,1));
        AddWeapon(Entities, Sword, White, V3(-5,0,0));
        AddWeapon(Entities, Shield, White, V3(-10,0,0));
        
        Memory->IsInitialized = true;
    }

    PushClear(Group, Orange, Target_None);
    PushClear(Group, { 0 }, Target_World);
    PushClear(Group, { 0 }, Target_Outline);
    PushClear(Group, { 0 }, Target_Postprocessing_Outline);
    PushClear(Group, Magenta, Target_PingPong);
    PushClear(Group, Color(BackgroundBlue, 1.0), Target_Output);

    Update(&Group->Camera, pGameState, Input, Group->Width, Group->Height);
    
    //GameOutputSound(Assets, SoundBuffer, pGameState, Input);
    
    PushEntities(Group, &pGameState->EntityList);
    
    // Debug info
    static double Alpha = 0.0;
    if (
        Input->Mode == Keyboard && Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown || 
        Input->Mode == Controller && Input->Controller.Start.IsDown && !Input->Controller.Start.WasDown
    ) {
        Group->Debug = !Group->Debug;
        if (!Group->Debug) {
            Alpha = 0.0;
        }
    }
    
    if (Group->Debug) {
        if (Alpha < 1.0) {
            double x = (pGameState->dt - 1.8) / 1.1;
            Alpha += exp(- x * x);
        }
        else Alpha = 1.0;
        
        if (Input->Keyboard.N.IsDown && !Input->Keyboard.N.WasDown) {
            Group->DebugNormals = !Group->DebugNormals;
        }
        
        if (Input->Keyboard.B.IsDown && !Input->Keyboard.B.WasDown) {
            Group->DebugBones = !Group->DebugBones;
        }
        
        if (Input->Keyboard.C.IsDown && !Input->Keyboard.C.WasDown) {
            Group->DebugColliders = !Group->DebugColliders;
        }
        
        PushDebugGrid(Group, Alpha);
        rectangle DebugInfoRect = { 0, 0, 400, 350 };
        
        PushRect(Group, DebugInfoRect, Color(Black, 0.5 * Alpha), SORT_ORDER_DEBUG_OVERLAY);
        PushRectOutline(Group, DebugInfoRect, Color(Gray, Alpha));
        
        // DebugInfo
        char DebugTextBuffer[128] = {};
        sprintf_s(
            DebugTextBuffer, 
            "%.02f ms/frame\n%.02f ms used\n%.02f fps\n%.02f Mcycles/frame\n%.02f time (s)", 
            Memory->DebugInfo.BudgetTime, 
            Memory->DebugInfo.UsedTime,
            Memory->DebugInfo.FPS,
            Memory->DebugInfo.UsedMCyclesPerFrame, 
            pGameState->Time
        );
        PushText(
            Group, 
            V2(10.0, 30.0), 
            Font_Menlo_Regular_ID, 
            128,
            DebugTextBuffer, 
            Color(White, Alpha), 
            12, 
            false, 
            SORT_ORDER_DEBUG_OVERLAY
        );
        
        // Debug arenas
        float DebugArenaStartHeight = 135.0f;
        PushDebugArena(Group, *TransientArena,                    V2(10.0f, DebugArenaStartHeight), Alpha);
        PushDebugArena(Group, *GeneralPurposeArena,               V2(10.0f, DebugArenaStartHeight + 30.0f), Alpha);
        PushDebugArena(Group, *StringsArena,                      V2(10.0f, DebugArenaStartHeight + 60.0f), Alpha);
        PushDebugArena(Group, Group->VertexBuffer.VertexArena[0], V2(10.0f, DebugArenaStartHeight + 90.0f), Alpha);
        PushDebugArena(Group, Group->VertexBuffer.VertexArena[1], V2(10.0f, DebugArenaStartHeight + 120.0f), Alpha);
        PushDebugArena(Group, Group->VertexBuffer.VertexArena[2], V2(10.0f, DebugArenaStartHeight + 150.0f), Alpha);
        PushDebugArena(Group, Group->VertexBuffer.VertexArena[3], V2(10.0f, DebugArenaStartHeight + 180.0f), Alpha);

        // Axes
        v2 XAxis = V2(cos(Group->Camera->Angle * Degrees), sin(Group->Camera->Angle * Degrees) * sin(Group->Camera->Pitch * Degrees));
        v2 YAxis = V2(0.0, -cos(Group->Camera->Pitch * Degrees));
        v2 ZAxis = V2(-sin(Group->Camera->Angle * Degrees), sin(Group->Camera->Pitch * Degrees) * cos(Group->Camera->Angle * Degrees));
        v2 AxisOrigin = V2(Group->Width - 0.08 * (float)Group->Height - 10.0, 0.1 * (float)Group->Height);
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, Red);
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, Green);
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, Blue);

        triangle Triangle = {
            V3(0,0,0),
            V3(5,0,0),
            V3(0,5,0)
        };

        // Debug camera basis
        // PushDebugVector(Group, Group->Camera.Basis.X, V3(0,0,0), World_Coordinates, Yellow);
        // PushDebugVector(Group, Group->Camera.Basis.Y, V3(0,0,0), World_Coordinates, Magenta);
        // PushDebugVector(Group, Group->Camera.Basis.Z, V3(0,0,0), World_Coordinates, Cyan);

        // Debug Framebuffer
        PushDebugFramebuffer(Group, Target_Postprocessing_Outline);
    }

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
            if (Group->Debug) PushText(Group, V2(20, Height), Font_Menlo_Regular_ID, String, White, 8, false, SORT_ORDER_DEBUG_OVERLAY);
            Height -= 17;
            DebugRecord->HitCount = 0;
            DebugRecord->CycleCount = 0;
        }
    }
}