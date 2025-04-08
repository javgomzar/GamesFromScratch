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

        memory_arena* StringsArena = &pGameState->StringsArena;
        pGameState->TransientArena.Name = PushString(StringsArena, 16, "Transient Arena");
        pGameState->TransientArena.Percentage = PushString(StringsArena, 7, "0.0%");
        pGameState->GeneralPurposeArena.Name = PushString(StringsArena, 24, "General purpose Arena");
        pGameState->GeneralPurposeArena.Percentage = PushString(StringsArena, 7, "0.0%");
        pGameState->StringsArena.Name = PushString(StringsArena, 16, "Strings Arena");
        pGameState->StringsArena.Percentage = PushString(StringsArena, 7, "0.0%");
        pGameState->RenderArena.Name = PushString(StringsArena, 16, "Render Arena");
        pGameState->RenderArena.Percentage = PushString(StringsArena, 7, "0.0%");

        //TestPerformance();

        // Initialize entities
        
        Memory->IsInitialized = true;
    }

    PushClear(Group, { 0 }, World);
    PushClear(Group, { 0 }, Outline);
    PushClear(Group, { 0 }, Postprocessing_Outline);
    PushClear(Group, Color(BackgroundBlue, 1.0), Output);

    Update(&Group->Camera, pGameState, Input);

    GameOutputSound(Assets, SoundBuffer, pGameState, Input);

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

        rectangle DebugInfoRect = { 0, 0, 350, 250 };

        PushRect(Group, DebugInfoRect, Color(Black, 0.5 * Alpha), SORT_ORDER_DEBUG_OVERLAY);
        PushRectOutline(Group, DebugInfoRect, Color(Gray, Alpha));
        
    // DebugInfo
        string Buffer = PushString(&pGameState->TransientArena, 128, " %.02f ms/frame\n %.02f fps\n %.02f Mcycles/frame\n %.02f time (s)");
        sprintf_s(Buffer.Content, 128, " %.02f ms/frame\n %.02f fps\n %.02f Mcycles/frame\n %.02f time (s)", Memory->DebugInfo.msPerFrame, Memory->DebugInfo.FPS, Memory->DebugInfo.MCyclesPerFrame, pGameState->Time);

        PushText(Group, V2(0, 30.0), GetAsset(Assets, Font_Cascadia_Mono_ID), Buffer, Color(White, Alpha), 12, false, SORT_ORDER_DEBUG_OVERLAY);

        // Debug arenas
        PushDebugArena(Group, pGameState->TransientArena, V2(20.0, 120.0), Alpha);
        PushDebugArena(Group, pGameState->GeneralPurposeArena, V2(20.0, 150.0), Alpha);
        PushDebugArena(Group, pGameState->RenderArena, V2(20.0, 180.0), Alpha);
        PushDebugArena(Group, pGameState->StringsArena, V2(20.0, 210.0), Alpha);

        // Axes
        v3 XAxis = V3(cos(Group->Camera->Angle * Degrees), sin(Group->Camera->Angle * Degrees) * sin(Group->Camera->Pitch * Degrees), 0.0);
        v3 YAxis = V3(0.0, -cos(Group->Camera->Pitch * Degrees), 0.0);
        v3 ZAxis = V3(-sin(Group->Camera->Angle * Degrees), sin(Group->Camera->Pitch * Degrees) * cos(Group->Camera->Angle * Degrees), 0.0);
        v3 AxisOrigin = V3(Group->Width - 0.08 * (double)Group->Height - 10.0, 0.1 * (double)Group->Height, 0);
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, Screen_Coordinates, Red);
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, Screen_Coordinates, Green);
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, Screen_Coordinates, Blue);

        // Debug camera basis
        // PushDebugVector(Group, Group->Camera.Basis.X, V3(0,0,0), World_Coordinates, Yellow);
        // PushDebugVector(Group, Group->Camera.Basis.Y, V3(0,0,0), World_Coordinates, Magenta);
        // PushDebugVector(Group, Group->Camera.Basis.Z, V3(0,0,0), World_Coordinates, Cyan);

        // Debug Framebuffer
        PushDebugFramebuffer(Group, Postprocessing_Outline);
    }

    PushRenderTarget(Group, World);

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
            rectangle ScreenRect = { 0, 0, Group->Width, Group->Height };
            PushRect(Group, ScreenRect, Color(White, ScreenRectAlpha), SORT_ORDER_PUSH_RENDER_TARGETS - 5.0);
        }
    }
    PushRenderTarget(Group, Output, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
    }

    LogGameDebugRecords(Group, &pGameState->TransientArena);
}

debug_record DebugRecordArray[__COUNTER__];

void LogGameDebugRecords(render_group* Group, memory_arena* TransientArena) {
    char Buffer[512];
    int Height = Group->Height - 20;
    game_font* Font = GetAsset(Group->Assets, Font_Cascadia_Mono_ID);
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
            if (Group->Debug) PushText(Group, V2(20, Height), Font, String, White, 8, false, SORT_ORDER_DEBUG_OVERLAY);
            Height -= 17;
            DebugRecord->HitCount = 0;
            DebugRecord->CycleCount = 0;
        }
    }
}