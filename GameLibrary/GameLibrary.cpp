// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "GameLibrary.h"

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

// Debug
void LogGameDebugRecords(render_group* Group);

void TestPerformance() {
    //TIMED_BLOCK;
    
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Rooms                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(room_type,
    Room_Type_Combat,
    Room_Type_Camp,
    Room_Type_Merchant,
    Room_Type_Blacksmith,
    Room_Type_Wizard,
    Room_Type_Quest,
    Room_Type_Miniboss,
    Room_Type_Boss
);

struct room {
    room_type Type;
    uint8 nNext;
    uint8 nPrevious;
    room* Next[4];
    room* Previous[4];
};

room RandomRoom(room_type Type) {
    room Result = {};
    Result.Type = Type;
    switch(Type) {
        case Room_Type_Combat: {
            
        } break;

        case Room_Type_Merchant: {

        } break;
    }

    return Result;
}

void PushRoom(render_group* Group, room Room, v2 Position) {
    game_bitmap_id Bitmap;

    switch(Room.Type) {
        case Room_Type_Combat: {
            Bitmap = Bitmap_Combat_ID;
        } break;
        case Room_Type_Camp: {
            Bitmap = Bitmap_Fire_ID;
        } break;
        case Room_Type_Merchant: {
            Bitmap = Bitmap_Coin_ID;
        } break;
        case Room_Type_Blacksmith: {
            Bitmap = Bitmap_Anvil_ID;
        } break;
        case Room_Type_Wizard: {
            Bitmap = Bitmap_Wizard_ID;
        } break;
        case Room_Type_Quest: {
            Bitmap = Bitmap_Quest_ID;
        } break;
        case Room_Type_Miniboss: {
            Bitmap = Bitmap_Miniboss_ID;
        } break;
        case Room_Type_Boss: {
            Bitmap = Bitmap_Boss_ID;
        } break;
    }

    PushBitmap(Group, Bitmap, {Position.X, Position.Y, 100, 100});
}

struct level {
    room Rooms[16];
    uint32 nRooms;
};

level RandomizeLevel() {
    level Result = {};
    Result.nRooms = 0;

    room* FirstRoom = &Result.Rooms[Result.nRooms++];
    *FirstRoom = RandomRoom(Room_Type_Combat);
    
    int nNextRow = RandInt(1, 4);
    
    for (int i = 0; i < nNextRow; i++) {
        room_type RoomType = RandomEnum(room_type);
        room* Room = &Result.Rooms[Result.nRooms++];
        *Room = RandomRoom(RoomType);
        Room->nPrevious = 1;
        Room->Previous[0] = FirstRoom;
        FirstRoom->Next[i] = Room;
    }
    FirstRoom->nNext = nNextRow;

    return Result;
}

void PushLevel(render_group* Group, level Level) {
    room First = Level.Rooms[0];

    v2 Center = V2(0.5f * (float)Group->Width, 0.5f * (float)Group->Height);

    PushRoom(Group, First, Center + V2(-50, 50));

    for (int i = 0; i < First.nNext; i++) {
        PushRoom(Group, Level.Rooms[i + 1], Center - V2(50, 50) + i * V2(100, 0));
    }
}

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    render_group* Group = &Memory->RenderGroup;
    game_state* pGameState = Memory->GameState;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    game_entity_state* EntityState = &pGameState->Entities;
    debug_info* DebugInfo = &Memory->DebugInfo;
    {
    TIMED_BLOCK;

    float Time = pGameState->Time;
    camera* ActiveCamera = Group->Camera;

    static level Level = {};

    bool firstFrame = false;
    if (!Memory->IsInitialized) {
        firstFrame = true;

        RNG.Seed = Platform->SeedRNG();
        RNG.State = RNG.Seed;

        //TestPerformance();

        Transition(pGameState, Game_State_Main_Menu);

        // Initialize combat
        pGameState->Combat.State = EntityState;

        // Initialize camera
        Group->Camera = AddCamera(EntityState, V3(0, 3.2f, 0), -45.0f, 22.5f);
        Group->Camera->OnAir = true;

        // UI
        uint32 MaxUIIDStack = 32;
        uint32* StackMemory = PushArray(&Memory->Permanent, MaxUIIDStack, uint32);

        Memory->IsInitialized = true;

        Level = RandomizeLevel();
    }

    PushClear(Group, Orange, Target_None);
    PushClear(Group, { 0 }, Target_World);
    PushClear(Group, { 0 }, Target_Outline);
    PushClear(Group, { 0 }, Target_Postprocessing_Outline);
    PushClear(Group, Magenta, Target_PingPong);
    PushClear(Group, BackgroundBlue, Target_Output);

    UpdateEntities(Group, pGameState, Input);
    
    //GameOutputSound(Assets, SoundBuffer, pGameState, Input);

    PushEntities(Group, pGameState, Input, Time);

    PushLevel(Group, Level);

    // UpdateUI(Memory, Input);

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

    LogGameDebugRecords(Group);
}

time_record TimeRecordArray[__COUNTER__];

void LogGameDebugRecords(render_group* Group) {
    if (Group->Debug) {
        char Buffer[512];
        const float Points = DEBUG_ENTRIES_TEXT_POINTS;
        game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);

        float Width = 0, Height = 0;
        GetTextWidthAndHeight("Function", Font, Points, &Width, &Height);
        float FunctionHeaderWidth = Width;
        float FunctionColWidth = Width;
        GetTextWidthAndHeight("Hits", Font, Points, &Width, &Height);
        float HitsHeaderWidth = Width;
        float HitsColWidth = Width;
        GetTextWidthAndHeight("MCycles", Font, Points, &Width, &Height);
        float MCyclesHeaderWidth = Width;
        float MCyclesColWidth = Width;
        GetTextWidthAndHeight("File", Font, Points, &Width, &Height);
        float FileHeaderWidth = Width;
        float FileColWidth = Width;

        uint32 nTimeRecords = 0;
        uint32 TimeRecordArrayLength = ArrayCount(TimeRecordArray);

        for (int i = 0; i < TimeRecordArrayLength; i++) {
            time_record* Record = TimeRecordArray + i;
            if (Record->HitCount > 0) {
                nTimeRecords += 1;
                GetTextWidthAndHeight(Record->FunctionName, Font, Points, &Width, &Height);
                if (Width > FunctionColWidth) FunctionColWidth = Width;
                sprintf_s(Buffer, "%d", Record->HitCount);
                GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
                if (Width > HitsColWidth) HitsColWidth = Width;
                sprintf_s(Buffer, "%.2f", Record->CycleCount / 1000000.0f);
                GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
                if (Width > MCyclesColWidth) MCyclesColWidth = Width;
                sprintf_s(Buffer, "%s:%d", Record->FileName, Record->LineNumber);
                GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
                if (Width > FileColWidth) FileColWidth = Width;
            }
        }

        float HMargin = 10.0f;
        float VMargin = 10.0f;

        float TotalWidth = FunctionColWidth + HitsColWidth + MCyclesColWidth + FileColWidth + 5 * HMargin;
        float RecordHeight = GetCharMaxHeight(Font, Points);
        float TotalHeight = RecordHeight * (nTimeRecords + 1) + 2 * VMargin;

        PushRect(Group, { Group->Width - TotalWidth, Group->Height - TotalHeight, TotalWidth, TotalHeight }, ChangeAlpha(Black, 0.7f));

        float RecordX = Group->Width - TotalWidth + HMargin;
        float RecordY = Group->Height - TotalHeight + RecordHeight + 0.5f * VMargin;

        PushText(
            Group, 
            V2(RecordX + 0.5f * (FunctionColWidth - FunctionHeaderWidth), RecordY), 
            Font_Menlo_Regular_ID,
            "Function",
            White,
            Points
        );
        RecordX += FunctionColWidth + HMargin;

        PushText(
            Group, 
            V2(RecordX + 0.5f * (HitsColWidth - HitsHeaderWidth), RecordY), 
            Font_Menlo_Regular_ID,
            "Hits",
            White,
            Points
        );
        RecordX += HitsColWidth + HMargin;

        PushText(
            Group, 
            V2(RecordX + 0.5f * (MCyclesColWidth - MCyclesHeaderWidth), RecordY), 
            Font_Menlo_Regular_ID,
            "MCycles",
            White,
            Points
        );
        RecordX += MCyclesColWidth + HMargin;

        PushText(
            Group, 
            V2(RecordX + 0.5f * (FileColWidth - FileHeaderWidth), RecordY), 
            Font_Menlo_Regular_ID,
            "File",
            White,
            Points
        );

        RecordX = Group->Width - TotalWidth + HMargin;
        RecordY += RecordHeight + 0.5f * VMargin;

        for (int i = 0; i < TimeRecordArrayLength; i++) {
            time_record* Record = TimeRecordArray + i;

            if (Record->HitCount > 0) {
                PushText(Group, V2(RecordX, RecordY), Font_Menlo_Regular_ID, Record->FunctionName, White, Points);
                RecordX += FunctionColWidth + HMargin;

                sprintf_s(Buffer, "%d", Record->HitCount);
                GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
                RecordX += HitsColWidth - Width;
                PushText(Group, V2(RecordX, RecordY), Font_Menlo_Regular_ID, Buffer, White, Points);
                RecordX += Width + HMargin;
                
                sprintf_s(Buffer, "%.2f", Record->CycleCount / 1000000.0f);
                GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
                RecordX += MCyclesColWidth - Width;
                PushText(Group, V2(RecordX, RecordY), Font_Menlo_Regular_ID, Buffer, White, Points);
                RecordX += Width + HMargin;

                sprintf_s(Buffer, "%s:%d", Record->FileName, Record->LineNumber);
                PushText(Group, V2(RecordX, RecordY), Font_Menlo_Regular_ID, Buffer, White, Points);
                RecordX = Group->Width - TotalWidth + HMargin;

                RecordY += RecordHeight;

                Record->HitCount = 0;
                Record->CycleCount = 0;
            }
        }
    }
    else {
        for (int i = 0; i < ArrayCount(TimeRecordArray); i++) {
            time_record* DebugRecord = TimeRecordArray + i;
            *DebugRecord = {};
        }
    }
}