// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"
#include "render_group.h"
#include "time.h"
#include <gl/GL.h>


/*
    TODO:
        - Make it so when you change TileSize everything zooms in (possibly control this with mouse wheel).
        - Improve door creation algorithm. Not all rooms need to be connected but maybe delete redundant doors sometimes and
          connect a little bit more rooms.
        - Flip character's sprite when moving in different directions.
        - Implement tile type Rock.
            * Rocks can be mined with a pickaxe.
            * Rocks can be moved when pushed.
            * Rocks can fill holes.
        - Implement tile type Hole.
            * Holes can't be crossed.
            * Holes can be filled with a rock.
        - Implement tile type filled hole.
        - Optimize rendering filtering which tiles need to be rendered?.
        - Implement inventory.
        - Main menu:
            * Save & load games.
            * Options.
            * Reset map?
*/


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
bool Collision(game_rect Rect, game_screen_position Cursor) {
    return Cursor.X > Rect.Left &&
        Cursor.X < Rect.Left + Rect.Width &&
        Cursor.Y > Rect.Top &&
        Cursor.Y < Rect.Top + Rect.Height;
}

color Lighten(color RGB) {
    color Result;

    Result.R = min(RGB.R + Attenuation, 255);
    Result.G = min(RGB.G + Attenuation, 255);
    Result.B = min(RGB.B + Attenuation, 255);

    return Result;
}

void Plot(game_offscreen_buffer* Buffer, game_screen_position Position, color Color) {
    game_rect ScreenRect;
    ScreenRect.Left = 0;
    ScreenRect.Top = 0;
    ScreenRect.Width = Buffer->Width;
    ScreenRect.Height = Buffer->Height;

    if (Collision(ScreenRect, Position)) {
        uint8* PixelMemory = (uint8*)Buffer->Memory + Position.X * Buffer->BytesPerPixel + Position.Y * Buffer->Pitch;
        uint32* Pixel = (uint32*)PixelMemory;
        *Pixel = GetColorBytes(Color);
    }
}

// Asset loading
 // BMP
loaded_bmp LoadBMP(platform_read_entire_file* PlatformReadEntireFile, const char* FileName) {
    loaded_bmp Result = { 0 };
    Result.Handle = 0;
    read_file_result ReadResult = PlatformReadEntireFile(FileName);
    if (ReadResult.ContentSize != 0) {
        bitmap_header* Header = (bitmap_header*)ReadResult.Content;
        Result.Header = *Header;
        uint32 BytesPerPixel = Header->BitsPerPixel >> 3;
        Result.BytesPerPixel = BytesPerPixel;
        Result.Pitch = Header->Width * BytesPerPixel;
        Result.Content = (uint32*)((uint8*)ReadResult.Content + Header->BitmapOffset);
        
        bool HasAlpha = false;
        if (Result.Header.BitsPerPixel == 32 && Result.Header.Compression == 3) {
            uint32 AlphaMask = ~(Result.Header.RedMask | Result.Header.GreenMask | Result.Header.BlueMask);
            // If not all Alphas are zero, we need to use them
            uint32* Contents = Result.Content;
            for (int32 i = 0; i < Result.Header.Height * Result.Header.Width; i++) {
                if ((*Contents++ & AlphaMask) > 0) {
                    HasAlpha = true;
                    break;
                }
            }

            // If all alphas are zero, turn them to one
            Contents = Result.Content;
            if (!HasAlpha) {
                for (int32 j = 0; j < Result.Header.Height * Result.Header.Width; j++) {
                    *Contents = AlphaMask | (*Contents++ & ~AlphaMask);
                }
            }
            return Result;
        }
    }
    return Result;
}


// UI
void InitializeUI(memory_arena* Arena, game_assets* Assets, UI* UserInterface, platform_read_entire_file* Read) {
    // Initialize your UI elements here
}


// Sound
int16 Amplitude = 4000;
float WriteSineWave(game_sound_buffer* pSoundBuffer, float Hz, float InitialPhase) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    float SineWavePeriod = (float)(pSoundBuffer->SamplesPerSecond) / Hz;
    float PhaseIncrement = (2.0f * Pi) / (float)SineWavePeriod;

    float Phase = InitialPhase;
    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        float SineValue = sinf(Phase);
        Phase += PhaseIncrement;
        while (Phase > Tau) {
            Phase -= Tau;
        }
        uint16 SampleValue = (uint16)(SineValue * Amplitude);
        *SampleOut++ = SampleValue; // LEFT
        *SampleOut++ = SampleValue; // RIGHT
    }
    return Phase;
}

void FadeIn(game_sound_buffer* pSoundBuffer, uint8 TransitionCount) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    uint16 HalfLife = (uint16)(SampleCount / 2);
    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        float Exp = (1 - expf(-(float)((SampleIndex + TransitionCount * SampleCount) / (float)HalfLife)));
        float LeftValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(LeftValue); // LEFT
        SampleOut++;
        float RightValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(RightValue); // RIGHT
        SampleOut++;
    }
}

void FadeOut(game_sound_buffer* pSoundBuffer, uint8 TransitionCount) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    uint16 HalfLife = (uint16)(SampleCount / 2);
    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        float Exp = expf(-(float)((SampleIndex + TransitionCount * SampleCount) / (float)HalfLife));
        float LeftValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(LeftValue); // LEFT
        SampleOut++;
        float RightValue = (float)*SampleOut * Exp;
        *SampleOut = (uint16)(RightValue); // RIGHT
        SampleOut++;
    }
}

void Silence(game_sound_buffer* pSoundBuffer) {
    uint32 SampleCount = pSoundBuffer->BufferSize;

    int16* SampleOut = pSoundBuffer->SampleOut;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        *SampleOut++ = 0; // LEFT
        *SampleOut++ = 0; // RIGHT
    }
}

void DebugPlotSoundBuffer(game_offscreen_buffer* pScreenBuffer, game_sound_buffer* pSoundBuffer, game_screen_position PlotOrigin) {
    uint32 SampleCount = pSoundBuffer->BufferSize;
    uint16 PlotHeight = 200;
    uint16 PlotWidth = 400;
    uint16 Amplitude = 4000;

    int16* SampleOut = pSoundBuffer->SampleOut;
    for (int i = 0; i < SampleCount; i++) {
        game_screen_position Position = { PlotOrigin.X + (i * PlotWidth / SampleCount), (uint32)((*SampleOut * PlotHeight / Amplitude) + PlotOrigin.Y), 0 };
        Plot(pScreenBuffer, Position, White);
        SampleOut++;
        SampleOut++;
    }
}

void GameOutputSound(game_offscreen_buffer* ScreenBuffer, game_sound_buffer* pSoundBuffer, game_state* pGameState) {
    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
}

//void ResetRoom(room* Room) {
//    Room->Width = rand() % 20 + 5;
//    Room->Height = rand() % 20 + 5;
//    Room->Doors[0] = rand() % 2;
//    Room->Doors[1] = rand() % 2;
//    Room->Doors[2] = rand() % 2;
//    Room->Doors[3] = rand() % 2;
//}

bool IsValid(int Row, int Col) {
    return Row >= 0 && Row < MAP_HEIGHT && Col >= 0 && Col < MAP_HEIGHT;
}

void Advance(tile_pointer* Pointer) {
    Pointer->Row += Pointer->Direction.Row;
    Pointer->Col += Pointer->Direction.Col;
}

enum turn {
    Clockwise,
    CounterClockwise
};

tile_direction Rotate(tile_direction Direction, turn Turn) {
    tile_direction Result = { 0 };
    if (Turn == Clockwise) {
        Result.Row = Direction.Col;
        Result.Col = -Direction.Row;
    }
    else {
        Result.Row = -Direction.Col;
        Result.Col = Direction.Row;
    }
    return Result;
}

void AddPointer(tile_pointer Pointers[], tile_pointer Pointer, int* nPointers) {
    Pointers[*nPointers] = Pointer;
    (*nPointers)++;
}

void RemovePointer(tile_pointer Pointers[], int Index, int* nPointers) {
    Pointers[Index] = Pointers[*nPointers - 1];
    Pointers[*nPointers - 1] = { 0 };
    (*nPointers)--;
}

void ProcessRooms(tile Map[MAP_HEIGHT][MAP_WIDTH], int* NumberOfRooms, room Rooms[MAX_ROOMS]) {
    int nRooms = 0;

    tile_position TopLeftCorners[MAX_ROOMS] = { 0 };

    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            switch (Map[i][j].Type) {
                case Floor:
                {
                    if (i == 0) {
                        if (j == 0) {
                            TopLeftCorners[nRooms] = { i, j, 0 };
                            nRooms++;
                        }
                        else if (Map[i][j - 1].Type != Floor) {
                            TopLeftCorners[nRooms] = { i, j, 0 };
                            nRooms++;
                        }
                    }
                    else if (j == 0) {
                        if (Map[i - 1][j].Type != Floor) {
                            TopLeftCorners[nRooms] = { i, j, 0 };
                            nRooms++;
                        }
                    }
                    else if (Map[i - 1][j].Type != Floor && Map[i][j - 1].Type != Floor) {
                        TopLeftCorners[nRooms] = { i, j, 0 };
                        nRooms++;
                    }
                } break;
            }
        }
    }

    for (int i = 0; i < nRooms; i++) {
        tile_position Position = TopLeftCorners[i];
        room Room = { 0 };

        Room.ID = i;
        Room.Explored = false;
        Room.Left = Position.Col;
        Room.Top = Position.Row;
        Room.Width = 0;

        for (int j = Room.Left; j < MAP_WIDTH && Map[Room.Top][j].Type == Floor; j++) {
            Room.Width++;
        }

        Room.Height = 0;
        for (int k = Room.Top; k < MAP_HEIGHT && Map[k][Room.Left].Type == Floor; k++) {
            Room.Height++;
        }

        Rooms[i] = Room;

        // Chests
        float PChest = 0.02f;
        if (Room.Width > 1 && Room.Height > 1 && randf() < PChest) {
            int Row = Room.Top + (Room.Height / 2);
            int Col = Room.Left + (Room.Width / 2);
            Map[Row][Col] = { Chest };
        }
    }

    *NumberOfRooms = nRooms;
}

bool Contains(room Room, tile_position Position) {
    return Room.Top <= Position.Row && Position.Row < Room.Top + Room.Height &&
        Room.Left <= Position.Col && Position.Col < Room.Left + Room.Width;
}

// Returns ID of room containing given position
int GetRoom(int nRooms, room Rooms[], tile_position Position) {
    for (int i = 0; i < nRooms; i++) {
        if (Contains(Rooms[i], Position)) {
            return i;
        }
    }
    return -1;
}

bool AreContiguous(room Room1, room Room2, int* nContiguous, tile_position ContiguousTiles[100]) {
    *nContiguous = 0;
    bool Result = false;

    for (int i = Room1.Top; i < Room1.Top + Room1.Height; i++) {
        tile_position Position = { i, Room1.Left - 2, 0 };
        if (Contains(Room2, Position)) {
            Result = true;
            ContiguousTiles[*nContiguous] = {Position.Row, Position.Col + 1, 0};
            (*nContiguous)++;
        }

        Position = { i, Room1.Left + Room1.Width + 1, 0 };
        if (Contains(Room2, Position)) {
            Result = true;
            ContiguousTiles[*nContiguous] = { Position.Row, Position.Col - 1, 0 };
            (*nContiguous)++;
        }
    }

    for (int j = Room1.Left; j < Room1.Left + Room1.Width; j++) {
        tile_position Position = { Room1.Top - 2, j, 0 };
        if (Contains(Room2, Position)) {
            Result = true;
            ContiguousTiles[*nContiguous] = { Position.Row + 1, Position.Col, 0 };
            (*nContiguous)++;
        }

        Position = { Room1.Top + Room1.Height + 1, j, 0 };
        if (Contains(Room2, Position)) {
            Result = true;
            ContiguousTiles[*nContiguous] = { Position.Row - 1, Position.Col, 0 };
            (*nContiguous)++;
        }
    }

    return Result;
}

void InitMap(tile Map[MAP_HEIGHT][MAP_WIDTH], room Rooms[], int* nRooms) {

    // Setting all the map to floor
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            Map[i][j] = { Floor };
        }
    }

    // Declarations
    // Algorithm parameters
    float PBiffurcation = 0.1f;
    int MinIdleSteps = 10;
    tile_pointer Pointers[100] = { 0 };
    tile_pointer* StartPointer = &Pointers[0];

    // Starting pointer
    switch (rand() % 4) {
        case 0:
        {
            StartPointer->Row = rand() % MAP_HEIGHT;
            StartPointer->Col = 0;
            StartPointer->Direction.Row = 0;
            StartPointer->Direction.Col = 1;

        } break;
        case 1:
        {
            StartPointer->Row = 0;
            StartPointer->Col = rand() % MAP_WIDTH;
            StartPointer->Direction.Row = 1;
            StartPointer->Direction.Col = 0;
        } break;
        case 2:
        {
            StartPointer->Row = rand() % MAP_HEIGHT;
            StartPointer->Col = MAP_WIDTH - 1;
            StartPointer->Direction.Row = 0;
            StartPointer->Direction.Col = -1;
        } break;
        case 3:
        {
            StartPointer->Row = MAP_HEIGHT - 1;
            StartPointer->Col = rand() % MAP_WIDTH;
            StartPointer->Direction.Row = -1;
            StartPointer->Direction.Col = 0;
        } break;
    }

    Map[StartPointer->Row][StartPointer->Col] = { Wall };

    // Main loop
    int nPointers = 1;
    while (nPointers > 0) {
        for (int i = 0; i < nPointers; i++) {
            tile_pointer* Pointer = &Pointers[i];
            Advance(Pointer);

            if (!IsValid(Pointer->Row, Pointer->Col) ||
                Map[Pointer->Row][Pointer->Col].Type == Wall)
            {
                RemovePointer(Pointers, i, &nPointers);
                i--;
            }
            else {
                Map[Pointer->Row][Pointer->Col] = { Wall };

                // Bifurcations
                if (Pointer->IdleSteps > MinIdleSteps && randf() < PBiffurcation) {
                    switch (rand() % 4) {
                        case 0:
                        {
                            AddPointer(Pointers, { Pointer->Row, Pointer->Col, Rotate(Pointer->Direction, Clockwise), 0 }, &nPointers);
                        } break;
                        case 1:
                        {
                            AddPointer(Pointers, { Pointer->Row, Pointer->Col, Rotate(Pointer->Direction, CounterClockwise), 0 }, &nPointers);
                        } break;
                        case 2:
                        {
                            AddPointer(Pointers, { Pointer->Row, Pointer->Col, Rotate(Pointer->Direction, Clockwise), 0 }, &nPointers);
                            Pointer->Direction = Rotate(Pointer->Direction, CounterClockwise);
                        } break;
                        case 3:
                        {
                            AddPointer(Pointers, { Pointer->Row, Pointer->Col, Rotate(Pointer->Direction, Clockwise), 0 }, &nPointers);
                            AddPointer(Pointers, { Pointer->Row, Pointer->Col, Rotate(Pointer->Direction, CounterClockwise), 0 }, &nPointers);
                        } break;
                    }
                }
                else {
                    Pointer->IdleSteps++;
                }
            }
        }
    }

    ProcessRooms(Map, nRooms, Rooms);

    // Doors
    float PDoor = 0.5f;
    for (int i = 0; i < *nRooms; i++) {
        for (int j = 0; j < i; j++) {

            int nContiguousTiles = 0;
            tile_position ContiguousTiles[100] = {0};
            bool RoomsAreContiguous = AreContiguous(Rooms[i], Rooms[j], &nContiguousTiles, ContiguousTiles);
            if (RoomsAreContiguous) {
                if (randf() < PDoor) {
                    int DoorResult = rand() % nContiguousTiles;
                    tile_position Position = ContiguousTiles[DoorResult];
                    Map[Position.Row][Position.Col] = { Door };
                }
            }
        }
    }


    //float PDoor = 0.05f;
    //for (int i = 1; i < MAP_HEIGHT - 1; i++) {
    //    for (int j = 1; j < MAP_WIDTH - 1; j++) {
    //        if (Map[i][j].Type == Wall) {
    //            int AdyacentDoors = 0;

    //            if (Map[i - 1][j].Type == Door) AdyacentDoors++;
    //            if (Map[i + 1][j].Type == Door) AdyacentDoors++;
    //            if (Map[i][j - 1].Type == Door) AdyacentDoors++;
    //            if (Map[i][j + 1].Type == Door) AdyacentDoors++;

    //            if (AdyacentDoors == 0) {
    //                int VerticalWalls = 0;
    //                int HorizontalWalls = 0;

    //                // Vertical
    //                if (Map[i - 1][j].Type == Wall) VerticalWalls++;
    //                if (Map[i + 1][j].Type == Wall) VerticalWalls++;

    //                // Horizontal
    //                if (Map[i][j - 1].Type == Wall) HorizontalWalls++;
    //                if (Map[i][j + 1].Type == Wall) HorizontalWalls++;

    //                if (HorizontalWalls + VerticalWalls == 2 && VerticalWalls*HorizontalWalls == 0) {
    //                    if (randf() < PDoor) {
    //                        Map[i][j] = { Door };
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}
}


// Main
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    render_group* Group = Memory->Group;

    if (!Memory->IsInitialized) {
        // Memory arenas
        InitializeArena(&pGameState->RenderArena, Megabytes(5), (uint8*)Memory->PermanentStorage + sizeof(game_state) + pGameState->TextArena.Size);

        // Assets ----------------------------------------------------------------------------------------------------------------------------------------
        // Load your assets here

        Memory->Assets.PlayerBMP = LoadBMP(Platform->ReadEntireFile, "..\\GameLibrary\\RogueMedia\\Player.bmp");
        Memory->Assets.FloorBMP = LoadBMP(Platform->ReadEntireFile, "..\\GameLibrary\\RogueMedia\\Floor.bmp");
        Memory->Assets.DoorBMP = LoadBMP(Platform->ReadEntireFile, "..\\GameLibrary\\RogueMedia\\Door.bmp");
        Memory->Assets.ChestBMP = LoadBMP(Platform->ReadEntireFile, "..\\GameLibrary\\RogueMedia\\Treasure.bmp");

        // User Interface
        // InitializeUI();

        // Initialize map
        srand(time(0));
        *pGameState->Rooms = { 0 };
        InitMap(pGameState->Map, pGameState->Rooms, &pGameState->nRooms);

        // Initialize game state
        pGameState->TileSize = 30;

        pGameState->PlayerPosition = { MAP_HEIGHT / 2, MAP_WIDTH / 2, 2 };
        while (pGameState->Map[pGameState->PlayerPosition.Row][pGameState->PlayerPosition.Col].Type != Floor) {
            pGameState->PlayerPosition.Row--;
            pGameState->PlayerPosition.Col--;
        }

        game_screen_position Position = ToScreenCoord(pGameState->PlayerPosition, pGameState->TileSize);
        pGameState->Camera = { V3(Position.X - ScreenBuffer->Width / 2, Position.Y - ScreenBuffer->Height / 2, 0) , {0,0,0}};

        // Renderer --------------------------------------------------------------------------------------------------------------------------------------
        Memory->Group = AllocateRenderGroup(&pGameState->RenderArena, Megabytes(4));
        Memory->Group->Camera = &pGameState->Camera;
        Group = Memory->Group;

        Memory->IsInitialized = true;
    }

    PushClear(Group, Black);

    // Controls
    // Character movement
    tile_direction PlayerMovement = { 0,0 };
    if (Input->Keyboard.D.IsDown && !Input->Keyboard.D.WasDown) {
        PlayerMovement.Col = 1;
    }
    if (Input->Keyboard.A.IsDown && !Input->Keyboard.A.WasDown) {
        PlayerMovement.Col = -1;
    }
    if (Input->Keyboard.S.IsDown && !Input->Keyboard.S.WasDown) {
        PlayerMovement.Row = 1;
    }
    if (Input->Keyboard.W.IsDown && !Input->Keyboard.W.WasDown) {
        PlayerMovement.Row = -1;
    }
    
    tile_position NewPosition = pGameState->PlayerPosition + PlayerMovement;

    if (IsValid(NewPosition.Row, NewPosition.Col) && 
        pGameState->Map[NewPosition.Row][NewPosition.Col].Type != Wall && 
        pGameState->Map[NewPosition.Row][NewPosition.Col].Type != Chest) {
        pGameState->PlayerPosition = NewPosition;
    }

    // Room exploration
    int CurrentRoom = GetRoom(pGameState->nRooms, pGameState->Rooms, pGameState->PlayerPosition);
    if (CurrentRoom != -1) {
        pGameState->Rooms[CurrentRoom].Explored = true;
    }

    // Bombs?
    if (Input->Keyboard.Enter.IsDown && !Input->Keyboard.Enter.WasDown) {
        tile_position Position = pGameState->PlayerPosition;
        if (Position.Row - 1 >= 0 && pGameState->Map[Position.Row - 1][Position.Col].Type == Wall) {
            pGameState->Map[Position.Row - 1][Position.Col].Type = Door;
        }
        if (Position.Col - 1 >= 0 && pGameState->Map[Position.Row][Position.Col-1].Type == Wall) {
            pGameState->Map[Position.Row][Position.Col-1].Type = Door;
        }
    }
    
    // Camera movement
    v3 Direction = { 0 };

    if (Input->Keyboard.Right.IsDown) {
        Direction.X += 1;
    }
    if (Input->Keyboard.Left.IsDown) {
        Direction.X -= 1;
    }
    if (Input->Keyboard.Up.IsDown) {
        Direction.Y -= 1;
    }
    if (Input->Keyboard.Down.IsDown) {
        Direction.Y += 1;
    }
    Direction = normalize(Direction);

    pGameState->Camera.Velocity = 10 * Direction;

    UpdateCamera(&pGameState->Camera);

    // TODO: Move camera pressing the wheel button

    // Reset room
    if (Input->Keyboard.Space.IsDown && !Input->Keyboard.Space.WasDown) {
        InitMap(pGameState->Map, pGameState->Rooms, &pGameState->nRooms);
    }

    // Debug camera position and velocity
    /*
    char TextBuffer[256];
    sprintf_s(TextBuffer, " %.02f,%.02f,%.02f Position\n %.02f,%.02f,%.02f Velocity\n", 
        pGameState->Camera.Position.X, pGameState->Camera.Position.Y, pGameState->Camera.Position.Z,
        pGameState->Camera.Velocity.X, pGameState->Camera.Velocity.Y, pGameState->Camera.Velocity.Z);
    OutputDebugStringA(TextBuffer);
    */

    GameOutputSound(ScreenBuffer, SoundBuffer, pGameState);

    // Render
    loaded_bmp Target = { 0 };
    Target.Header.Width = ScreenBuffer->Width;
    Target.Header.Height = ScreenBuffer->Height;
    Target.Pitch = ScreenBuffer->Pitch;
    Target.Content = (uint32*)ScreenBuffer->Memory;

    PushMap(Group, pGameState->Map, pGameState->nRooms, pGameState->Rooms, &Memory->Assets, pGameState->PlayerPosition, pGameState->TileSize);
    
    PushBMP(Group, &Memory->Assets.PlayerBMP, ToScreenCoord({pGameState->PlayerPosition.Row - 1, pGameState->PlayerPosition.Col, pGameState->PlayerPosition.Z}, pGameState->TileSize));

    static bool ShowDebugInfo = false;
    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        ShowDebugInfo = !ShowDebugInfo;
    }

    if (ShowDebugInfo) {
        game_rect DebugInfoRect = { pGameState->Camera.Position.X, pGameState->Camera.Position.Y, 450, 120 };
        PushDebugLattice(Group, pGameState->TileSize, {0.2f, 1.0f, 1.0f, 0.0f });
        PushRect(Group, DebugInfoRect, {0.5f, 0.0f, 0.0f, 0.0f},1000);
        PushRectOutline(Group, DebugInfoRect, Gray);
        text Text = { 0 };
        Text.Color = White;
        Text.Length = 48;
        Text.Points = 20;
        Text.Content = Memory->DebugInfo;
        PushText(Group, Assets->Characters, { (int)pGameState->Camera.Position.X, (int)pGameState->Camera.Position.Y + 30, 1001 }, Text);

        // Mouse
        PushDebugShineTile(Group, ToTilePosition(Input->Mouse.Cursor, pGameState->TileSize, pGameState->Camera), pGameState->TileSize);
    }


    // Software renderer as a fallback (toggle with Space)
    //static bool SoftwareRenderer = false;
    //if (Input->Keyboard.Space.IsDown && !Input->Keyboard.Space.WasDown) {
    //    SoftwareRenderer = !SoftwareRenderer;
    //}
    //if (SoftwareRenderer) {
    //    RenderGroupToOutput(Group, &Target);
    //}
    //else {
    //    Platform.OpenGLRender(Group, &Target);
    //}
    
    Platform->OpenGLRender(Group, Target.Header.Width, Target.Header.Height);

    // Clear render group
    ClearEntries(Group);
}

