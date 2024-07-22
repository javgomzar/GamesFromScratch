// GameLibrary.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "GameLibrary.h"
#include "render_group.h"
#include "time.h"

#include "Tests.h"

/*
    TODO:
        - Base points for bmps (the goal is to flip bmps from this point)
        - Improve door creation algorithm. Not all rooms need to be connected but maybe delete redundant doors sometimes and
          connect a little bit more rooms.
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
        - Make follower collide with walls.
        - Dijkstras algorithm for shortest path.
        - Pre-render text so lengths can be calculated.
        - Load characters assets in one single texture.
        - Sound mixer.
        - Main menu:
            * Save & load games.
            * Options.
            * Reset map?
        - Initialize enemies in different rooms. 
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
        uint8* PixelMemory = (uint8*)Buffer->Memory + (int)Position.X * Buffer->BytesPerPixel + (int)Position.Y * Buffer->Pitch;
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

void PlaySound(game_sound* Sound, game_sound_buffer* pSoundBuffer) {
    if (Sound->Played + pSoundBuffer->BufferSize > Sound->SampleCount) {
        Sound->Played = 0;
    }

    uint32 SampleCount = pSoundBuffer->BufferSize;
    int16* SampleOut = pSoundBuffer->SampleOut;
    int16* SampleIn = Sound->SampleOut + Sound->Played;
    for (uint32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++) {
        *SampleOut++ = *SampleIn++; // LEFT
        *SampleOut++ = *SampleIn++; // RIGHT
    }

    Sound->Played += 2*pSoundBuffer->BufferSize;
}

void GameOutputSound(game_assets* Assets, game_sound_buffer* pSoundBuffer, game_state* pGameState, game_input* Input) {
    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
    // WriteSineWave(pSoundBuffer, 480, 0);
    switch (pGameState->Scene) {
        case Intro:
        {
            PlaySound(&Assets->TitleMusic, pSoundBuffer);
        } break;
        case Main:
        {

        } break;
    }
}

game_sound LoadWAV(platform_read_entire_file* PlatformReadEntireFile, const char* FileName) {
    read_file_result File = PlatformReadEntireFile(FileName);
    DWORD* Pointer = (DWORD*)File.Content;
    
    DWORD ChunkType = *Pointer++;
    if (ChunkType != 'FFIR') {
        Assert(false);
    }

    DWORD RIFFChunkSize = *Pointer++;
    DWORD FileType = *Pointer++;
    if (FileType != 'EVAW') {
        Assert(false);
    }

    ChunkType = *Pointer++;
    if (ChunkType != ' tmf') {
        Assert(false);
    }
    DWORD ChunkSize = *Pointer++;
    waveformat WaveFMT = *(waveformat*)Pointer;

    Pointer += 4;
    ChunkType = *Pointer++;
    if (ChunkType != 'atad') {
        Assert(false);
    }
    ChunkSize = *Pointer++;

    game_sound Result = { 0 };
    Result.SampleOut = (int16*)Pointer;
    Result.SampleCount = ChunkSize / 2;
    return Result;
}

// Video
game_video LoadVideo(memory_arena* Arena, const char* Filename) {
    game_video Result = { 0 };
    Result.VideoContext = PushStruct(Arena, video_context);

    InitializeVideo(Filename, Result.VideoContext);
    int Width = Result.VideoContext->Frame->width;
    int Height = Result.VideoContext->Frame->height;
    Result.VideoContext->VideoOut = PushSize(Arena, Width * Height * 4);

    return Result;
}

void PushVideo(render_group* Group, game_video* Video, game_rect Rect, int Z, double SecondsElapsed) {
    
    if (!Video->VideoContext->Ended) {
        Video->TimeElapsed += SecondsElapsed;
        // DEBUG
        //char Text[256];
        //sprintf_s(Text, "%.02f Time elapsed | %.02f Time played\n", Video->TimeElapsed, Video->VideoContext->PTS * Video->VideoContext->TimeBase);
        //OutputDebugStringA(Text);

        if (Video->TimeElapsed > Video->VideoContext->PTS * Video->VideoContext->TimeBase) {
            LoadFrame(Video->VideoContext);
            Video->VideoContext->Width = Rect.Width;
            Video->VideoContext->Height = Rect.Height;
            WriteFrame(Video->VideoContext);
        }
    }
    else {
    }
    _PushVideo(Group, Video, Rect, Z);
}

void PushVideoLoop(render_group* Group, game_video* Video, game_rect Rect, int Z, double SecondsElapsed, int64_t StartOffset, int64_t EndOffset) {

    PushVideo(Group, Video, Rect, Z, SecondsElapsed);
    auto& VideoContext = Video->VideoContext;
    auto& FormatContext = VideoContext->FormatContext;
    auto& CodecContext = VideoContext->CodecContext;
    auto& StreamIndex = VideoContext->VideoStreamIndex;
    auto& PTS = VideoContext->PTS; // Presentation time-stamp (in time-base units)

    if (PTS >= EndOffset) {
        av_seek_frame(FormatContext, StreamIndex, StartOffset, AVSEEK_FLAG_ANY);
        LoadFrame(VideoContext);
        Video->TimeElapsed = Video->VideoContext->PTS * Video->VideoContext->TimeBase;
    }
}


// Returns true if row and col are non-negative and inside the map
bool IsValid(int Row, int Col) {
    return Row >= 0 && Row < MAP_HEIGHT && Col >= 0 && Col < MAP_HEIGHT;
}

bool IsValid(tile_position TilePosition) {
    return IsValid(TilePosition.Row, TilePosition.Col);
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

tile_position GetRoomCenter(room Room) {
    int Row = Room.Top + (Room.Height / 2);
    int Col = Room.Left + (Room.Width / 2);
    return { Row, Col };
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
        double PChest = 0.05;
        if (Room.Width > 1 && Room.Height > 1 && randf() < PChest) {
            tile_position Center = GetRoomCenter(Room);
            Map[Center.Row][Center.Col] = { Chest };
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

/*
This functions returns a bool that is true if given rooms are contiguous.
In this case, this function writes the number of contiguous tiles in nContiguous,
and writes the tile positions that are between these two rooms in the array ContiguousTiles.

Example of use:

int nContiguousTiles = 0;
tile_position ContiguousTiles[100] = { 0 };
bool RoomsAreContiguous = AreContiguous(Rooms[i], Rooms[j], &nContiguousTiles, ContiguousTiles);
if (RoomsAreContiguous) {
    if (randf() < PDoor) {
        for (int i=0; i<nContiguousTiles; i++) {
            ContiguousTiles[i] // do something with a contiguous tile
        }
    }
}
*/
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
    float PDoor = 0.6f;
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
}


tile_position RandomTile(tile Map[MAP_HEIGHT][MAP_WIDTH], tile_type Type) {
    int Row = rand() % MAP_HEIGHT;
    int Col = rand() % MAP_WIDTH;
    while (Map[Row][Col].Type != Type) {
        Row = rand() % MAP_HEIGHT;
        Col = rand() % MAP_WIDTH;
    }
    return { Row, Col };
}

// Returns the door between Room1 and Room2 if it exists. If it doesn't, returns {-1,-1}.
tile_position GetDoor(tile Map[MAP_HEIGHT][MAP_WIDTH], room Room1, room Room2) {
    int nContiguous;
    tile_position ContiguousTiles[100] = { 0 };
    if (AreContiguous(Room1, Room2, &nContiguous, ContiguousTiles)) {
        for (int i = 0; i < nContiguous; i++) {
            tile_position Tile = ContiguousTiles[i];
            if (Map[Tile.Row][Tile.Col].Type == Door) {
                return Tile;
            }
        }
    }
    return { -1, -1 };
}


// Entities
void InitializeEntity(entity* Entity, tile_position TilePosition, loaded_bmp* BMP) {
    *Entity = { 0 };

    Entity->BMP = BMP;
    Entity->BMPOffset = { -(double)(BMP->Header.Width) / 2, -(double)(BMP->Header.Height), 0 };
    Entity->Basis = { V3(1,0,0), V3(0,1,0), V3(0,0,1) };

    Entity->TilePosition = TilePosition;
    Entity->Position = ToWorldCoord(TilePosition);
    Entity->Position.Z = 1.0;
    Entity->Velocity = { 0,0,0 };
}

void Update(tile Map[MAP_HEIGHT][MAP_WIDTH], player* Player, projectile* Projectile, game_input* Input) {    
    double MinVelocity = 0.1;
    double Drag = 0.8;
    double MaxAcceleration = 0.3;

    v3 Acceleration = { 0,0,0 };
    double MaxVelocity;
    switch (Input->Mode) {
        case Keyboard:
        {
            // Movement
            if (Input->Keyboard.D.IsDown) {
                Acceleration.X += 1;
            }
            if (Input->Keyboard.A.IsDown) {
                Acceleration.X += -1;
            }
            if (Input->Keyboard.S.IsDown) {
                Acceleration.Y += 1;
            }
            if (Input->Keyboard.W.IsDown) {
                Acceleration.Y += -1;
            }

            Acceleration = 0.3 * normalize(Acceleration);

            if ((!Input->Keyboard.D.IsDown && !Input->Keyboard.A.IsDown) ||
                (Input->Keyboard.D.IsDown && Input->Keyboard.A.IsDown)) {
                Acceleration.X = -Drag * Player->Entity.Velocity.X;
            }

            if ((!Input->Keyboard.S.IsDown && !Input->Keyboard.W.IsDown) ||
                (Input->Keyboard.S.IsDown && Input->Keyboard.W.IsDown)) {
                Acceleration.Y = -Drag * Player->Entity.Velocity.Y;
            }

            MaxVelocity = 5.0;

        } break;
        case Controller:
        {
            Acceleration.X = Input->Controller.LeftJoystick.X;
            Acceleration.Y = -Input->Controller.LeftJoystick.Y;

            double mod = module(Acceleration);
            if (mod > 1.0) {
                MaxVelocity = 5.0;
            }
            else {
                MaxVelocity = 5.0 * module(Acceleration);
            }

            if (module(Acceleration) < 0.1) {
                Acceleration = -Drag * Player->Entity.Velocity;
            }
            else if (module(Acceleration) > MaxAcceleration) {
                    Acceleration = MaxAcceleration * normalize(Acceleration);
            }

        } break;
    }

    v3 Velocity = Player->Entity.Velocity + Acceleration;

    if (abs(Velocity.X) < MinVelocity) {
        Velocity.X = 0;
    }

    if (abs(Velocity.Y) < MinVelocity) {
        Velocity.Y = 0;
    }

    // Max velocity capping
    bool VelocityCapped = false;
    v3 OldVelocity = Velocity;
    if (module(Velocity) > MaxVelocity) {
        Velocity = MaxVelocity * normalize(Velocity);
        VelocityCapped = true;
    }

    v3 Position = Player->Entity.Position + Velocity;
    tile_position TilePosition = ToTilePosition(Position);
    tile_type NewTile = IsValid(TilePosition) ? Map[TilePosition.Row][TilePosition.Col].Type : Wall;
    
    // Wall collision
    if (NewTile != Floor && NewTile != Door) {
        if (VelocityCapped) {
            Velocity.X = abs(OldVelocity.X) <= MaxVelocity ? OldVelocity.X : Sign(OldVelocity.X) * MaxVelocity;
            Velocity.Y = abs(OldVelocity.Y) <= MaxVelocity ? OldVelocity.Y : Sign(OldVelocity.Y) * MaxVelocity;
        }

        v3 HorizontalSlideVelocity = V3(Velocity.X, 0, 0);
        v3 VerticalSlideVelocity = V3(0, Velocity.Y, 0);

        v3 HorizontalSlidePosition = Player->Entity.Position + HorizontalSlideVelocity;
        v3 VerticalSlidePosition = Player->Entity.Position + VerticalSlideVelocity;

        tile_position HorizontalSlideTile = ToTilePosition(HorizontalSlidePosition);
        tile_position VerticalSlideTile = ToTilePosition(VerticalSlidePosition);

        tile_type HorizontalSlideTileType = IsValid(HorizontalSlideTile) ? Map[HorizontalSlideTile.Row][HorizontalSlideTile.Col].Type : Wall;
        tile_type VerticalSlideTileType = IsValid(VerticalSlideTile) ? Map[VerticalSlideTile.Row][VerticalSlideTile.Col].Type : Wall;

        bool Horizontal = HorizontalSlideTileType == Floor || HorizontalSlideTileType == Door;
        bool Vertical = VerticalSlideTileType == Floor || VerticalSlideTileType == Door;

        if (Horizontal && Vertical) {
            if (abs(Velocity.X) > abs(Velocity.Y)) {
                Horizontal = true;
                Vertical = false;
            }
            else {
                Horizontal = false;
                Vertical = true;
            }
        }

        if (Horizontal) {
            Position = HorizontalSlidePosition;
            TilePosition = HorizontalSlideTile;
            Velocity.Y = 0;
        }
        else if (Vertical) {
            Position = VerticalSlidePosition;
            TilePosition = VerticalSlideTile;
            Velocity.X = 0;
        }
        else {
            Position = Player->Entity.Position;
            Velocity = { 0 };
            TilePosition = Player->Entity.TilePosition;
        }
    }
    
    // Debug position and velocity
    if (false) {
    //if (Input->Controller.RB.IsDown) {
        char DebugTextPosition[100];
        char DebugTextVelocity[100];
        sprintf_s(DebugTextPosition, "Position: (%f,%f,%f)\n", Player->Entity.Position.X, Player->Entity.Position.Y, Player->Entity.Position.Z);
        sprintf_s(DebugTextVelocity, "Velocity: (%f,%f,%f)\n", Player->Entity.Velocity.X, Player->Entity.Velocity.Y, Player->Entity.Velocity.Z);
        //OutputDebugStringA(DebugTextPosition);
        OutputDebugStringA(DebugTextVelocity);
    }

    // Commiting position and velocity
    Player->Entity.Position = Position;
    Player->Entity.Velocity = Velocity;
    Player->Entity.TilePosition = TilePosition;

    if (Player->Entity.Velocity.X != 0 || Player->Entity.Velocity.Y != 0 || Player->Entity.Velocity.Z != 0) {
        Player->Direction = normalize(Player->Entity.Velocity);
    }

    // Projectile
    if (!Projectile->Active &&
    ((Input->Mode == Keyboard) && Input->Keyboard.Space.IsDown) ||
    ((Input->Mode == Controller) && Input->Controller.AButton.IsDown)) 
    {
        Projectile->Active = true;
        Projectile->Entity.Position = Player->Entity.Position;
        Projectile->Entity.Velocity = Projectile->Celerity * Player->Direction;
    }

    // Changing BMP with direction
    if (module(Player->Entity.Velocity) > MinVelocity) {
        double Quad = pow(Player->Entity.Velocity.Y, 2) - pow(Player->Entity.Velocity.X, 2);
        if (Quad >= 0) {
            if (Player->Entity.Velocity.Y < 0) {
                Player->Entity.BMP = Player->BackBMP;
            }
            else {
                Player->Entity.BMP = Player->FrontBMP;
            }
        }
        else {
            Player->Entity.BMP = Player->SideBMP;
        }

        if (Player->Entity.Velocity.X < 0) {
            Player->Entity.Basis.X = V3(-1, 0, 0);
            Player->Entity.BMPOffset.X = (double)(Player->FrontBMP->Header.Width) / 2;
        }
        else if (Player->Entity.Velocity.X > 0) {
            Player->Entity.Basis.X = V3(1, 0, 0);
            Player->Entity.BMPOffset.X = -(double)(Player->FrontBMP->Header.Width) / 2;
        }
    }
}

void Update(tile Map[MAP_HEIGHT][MAP_WIDTH], follower* Follower, v3 PlayerPosition, game_input* Input) {
    // Movement
    if (module(PlayerPosition - Follower->Entity.Position) > 60) {
        double MaxVelocity = 5.0;
        double MinVelocity = 0.1;
        v3 Direction = normalize(PlayerPosition - Follower->Entity.Position);
        double Distance = module(PlayerPosition - Follower->Entity.Position);
        v3 Velocity = 0.1 * (Distance - 60) * Direction;

        bool VelocityCapped = false;
        v3 OldVelocity = Velocity;
        if (module(Velocity) > MaxVelocity) {
            Velocity = MaxVelocity * Direction;
            VelocityCapped = true;
        }

        if (abs(Velocity.X) < MinVelocity) {
            Velocity.X = 0;
        }

        if (abs(Velocity.Y) < MinVelocity) {
            Velocity.Y = 0;
        }

        v3 Position = Follower->Entity.Position + Velocity;
        tile_position TilePosition = ToTilePosition(Position);
        tile_type NewTile = IsValid(TilePosition) ? Map[TilePosition.Row][TilePosition.Col].Type : Wall;

        // Wall collision
        if (NewTile != Floor && NewTile != Door) {
            if (VelocityCapped) {
                Velocity.X = abs(OldVelocity.X) <= MaxVelocity ? OldVelocity.X : Sign(OldVelocity.X) * MaxVelocity;
                Velocity.Y = abs(OldVelocity.Y) <= MaxVelocity ? OldVelocity.Y : Sign(OldVelocity.Y) * MaxVelocity;
            }

            v3 HorizontalSlideVelocity = V3(Velocity.X, 0, 0);
            v3 VerticalSlideVelocity = V3(0, Velocity.Y, 0);

            v3 HorizontalSlidePosition = Follower->Entity.Position + HorizontalSlideVelocity;
            v3 VerticalSlidePosition = Follower->Entity.Position + VerticalSlideVelocity;

            tile_position HorizontalSlideTile = ToTilePosition(HorizontalSlidePosition);
            tile_position VerticalSlideTile = ToTilePosition(VerticalSlidePosition);

            tile_type HorizontalSlideTileType = IsValid(HorizontalSlideTile) ? Map[HorizontalSlideTile.Row][HorizontalSlideTile.Col].Type : Wall;
            tile_type VerticalSlideTileType = IsValid(VerticalSlideTile) ? Map[VerticalSlideTile.Row][VerticalSlideTile.Col].Type : Wall;

            bool Horizontal = HorizontalSlideTileType == Floor || HorizontalSlideTileType == Door;
            bool Vertical = VerticalSlideTileType == Floor || VerticalSlideTileType == Door;

            if (Horizontal && Vertical) {
                if (abs(Velocity.X) > abs(Velocity.Y)) {
                    Horizontal = true;
                    Vertical = false;
                }
                else {
                    Horizontal = false;
                    Vertical = true;
                }
            }

            if (Horizontal) {
                Position = HorizontalSlidePosition;
                TilePosition = HorizontalSlideTile;
                Velocity.Y = 0;
            }
            else if (Vertical) {
                Position = VerticalSlidePosition;
                TilePosition = VerticalSlideTile;
                Velocity.X = 0;
            }
            else {
                Position = Follower->Entity.Position;
                Velocity = { 0 };
                TilePosition = Follower->Entity.TilePosition;
            }

            if (abs(Velocity.X) > 0.0) {
                bool Debug = true;
            }
        }

        Follower->Entity.Velocity = Velocity;
        Follower->Entity.Position = Position;
        Follower->Entity.TilePosition = TilePosition;
        
        // Debug position and velocity
        //if (false) {
            //if (Input->Controller.RB.IsDown) {
        if (Input->Keyboard.B.IsDown) {
            char DebugTextPosition[100];
            char DebugTextVelocity[100];
            sprintf_s(DebugTextPosition, "Position: (%f,%f,%f)\n", Follower->Entity.Position.X, Follower->Entity.Position.Y, Follower->Entity.Position.Z);
            sprintf_s(DebugTextVelocity, "Velocity: (%f,%f,%f)\n", Follower->Entity.Velocity.X, Follower->Entity.Velocity.Y, Follower->Entity.Velocity.Z);
            //OutputDebugStringA(DebugTextPosition);
            OutputDebugStringA(DebugTextVelocity);
        }

        // BMP change
        double Quad = pow(Follower->Entity.Velocity.Y, 2) - pow(Follower->Entity.Velocity.X, 2);
        if (Quad >= 0) {
            if (Follower->Entity.Velocity.Y < 0) {
                Follower->Entity.BMP = Follower->BackBMP;
            }
            else {
                Follower->Entity.BMP = Follower->FrontBMP;
            }
        }
        else {
            Follower->Entity.BMP = Follower->SideBMP;
        }

        if (Follower->Entity.Velocity.X < 0) {
            Follower->Entity.Basis.X = V3(-1, 0, 0);
            Follower->Entity.BMPOffset.X = (double)(Follower->FrontBMP->Header.Width) / 2;
        }
        else if (Follower->Entity.Velocity.X > 0) {
            Follower->Entity.Basis.X = V3(1, 0, 0);
            Follower->Entity.BMPOffset.X = -(double)(Follower->FrontBMP->Header.Width) / 2;
        }
    }
    else {
        Follower->Entity.Velocity = { 0,0,0 };
    }
}

void Update(enemy* Enemy, projectile* Projectile, v3 PlayerPosition, double Time) {
    if (Projectile->Active &&
        (Projectile->Entity.TilePosition.Row == Enemy->Entity.TilePosition.Row) &&
        (Projectile->Entity.TilePosition.Col == Enemy->Entity.TilePosition.Col))
    {
        Enemy->HP -= 1;
        Projectile->Active = false;
    }

    v3 Direction = normalize(PlayerPosition - Enemy->Entity.Position);

    v3 Velocity = (abs(sin(3 * Time)) + 0.2) * Direction;
    v3 Position = Enemy->Entity.Position + Velocity;
    tile_position TilePosition = ToTilePosition(Position);
    tile_position PlayerTilePosition = ToTilePosition(PlayerPosition);

    if ((TilePosition.Row == PlayerTilePosition.Row) && 
        (TilePosition.Col == PlayerTilePosition.Col)) 
    {
        Position = Enemy->Entity.Position;
        Velocity = { 0 };
        TilePosition = Enemy->Entity.TilePosition;
    }

    Enemy->Entity.Velocity = Velocity;
    Enemy->Entity.Position = Position;
    Enemy->Entity.TilePosition = TilePosition;

    if (Projectile->Active &&
        (Projectile->Entity.TilePosition.Row == TilePosition.Row) && 
        (Projectile->Entity.TilePosition.Col == TilePosition.Col)) 
    {
        Enemy->HP -= 1;
        Projectile->Active = false;
    }

}

void Update(tile Map[MAP_HEIGHT][MAP_WIDTH], projectile* Projectile) {
    if (Projectile->Active) {
        Projectile->Entity.Position = Projectile->Entity.Position + Projectile->Entity.Velocity;
        Projectile->Entity.TilePosition = ToTilePosition(Projectile->Entity.Position);
    }

    tile_position TilePosition = ToTilePosition(Projectile->Entity.Position);
    if (!IsValid(TilePosition) || Map[TilePosition.Row][TilePosition.Col].Type == Wall) {
        Projectile->Active = false;
    }
}

void Update(color_selector* ColorSelector, game_input* Input) {
    v3 Position = ColorSelector->Position;
    color Color;
    if (Input->Mouse.LeftClick.IsDown) {
        // Hue selector
        if ((Position.X + 130 <= Input->Mouse.Cursor.X) && (Input->Mouse.Cursor.X <= Position.X + 155) && 
            (Position.Y <= Input->Mouse.Cursor.Y) && (Input->Mouse.Cursor.Y <= Position.Y + 120)) {
            ColorSelector->Hue = 1.0 - (Input->Mouse.Cursor.Y - ColorSelector->Position.Y) / 120.0;
        }

        // Saturation & Luminosity
        if ((Position.X <= Input->Mouse.Cursor.X) && (Input->Mouse.Cursor.X <= Position.X + 120) &&
            (Position.Y <= Input->Mouse.Cursor.Y) && (Input->Mouse.Cursor.Y <= Position.Y + 120)) {
            ColorSelector->Saturation = (Input->Mouse.Cursor.X - Position.X) / 120.0;
            ColorSelector->Luminosity = 1.0 - (Input->Mouse.Cursor.Y - Position.Y) / 120.0;
        }
    }

    ColorSelector->Color = GetColor(ColorSelector->Hue, ColorSelector->Saturation, ColorSelector->Luminosity);
}


// Scenes


// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_assets* Assets = &Memory->Assets;
    platform_api* Platform = &Memory->Platform;
    //render_group* Group = Memory->Group;
    static video_context VideoContext = { 0 };
    bool firstFrame = false;

    if (!Memory->IsInitialized) {
        Tests();
        firstFrame = true;

        // Assets ----------------------------------------------------------------------------------------------------------------------------------------
        // Text
        Assets->TitleText = PushString(&pGameState->TextArena, 32, "Press any key to continue");
        Assets->DialogText = PushString(&pGameState->TextArena, 160, "Sabe una cosa? Quien es el mesenhero de Dios? Y quien es el mesenhero del mesenhero? Y quien es el mesenhero del mensehero de Dios? Estamo en el apoclipsis.");
        Assets->HPText = PushString(&pGameState->TextArena, 3, "HP");
        Assets->HPNumbersText = PushString(&pGameState->TextArena, 10, "100/100");

        // Images
        Assets->PlayerBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Wilfred.bmp");
        Assets->PlayerBackBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\WilfredBack.bmp");
        Assets->PlayerSideBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\WilfredSide.bmp");
        Assets->FloorBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Floor.bmp");
        Assets->DoorBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Door.bmp");
        Assets->ChestBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Treasure.bmp");
        Assets->EnemyBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Enemy.bmp");
        Assets->EnemyBackBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\EnemyBack.bmp");
        Assets->ProjectileBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Projectile.bmp");
        Assets->BombBMP = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\Bomb.bmp");
        Assets->FadeFrame = LoadBMP(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Bitmaps\\FadeFrame.bmp");

        // Sound
        Assets->TitleMusic = LoadWAV(Platform->ReadEntireFile, "..\\..\\GameLibrary\\Media\\Sound\\wilfred_theme.wav");

        // Video
        Assets->IntroVideo = LoadVideo(&pGameState->VideoArena, "..\\..\\GameLibrary\\Media\\Video\\WILFREDCHILLIN2.mp4");

        // User Interface
        // InitializeUI();

        // Initialize map
        srand(time(0));
        *pGameState->Rooms = { 0 };
        InitMap(pGameState->Map, pGameState->Rooms, &pGameState->nRooms);

        // Initialize game state
        pGameState->Scene = Intro;
         
        // Initialize player
        pGameState->Player.FrontBMP = &Memory->Assets.PlayerBMP;
        pGameState->Player.BackBMP = &Memory->Assets.PlayerBackBMP;
        pGameState->Player.SideBMP = &Memory->Assets.PlayerSideBMP;

        InitializeEntity(&pGameState->Player.Entity, RandomTile(pGameState->Map, Floor), pGameState->Player.FrontBMP);

        pGameState->Player.MaxHP = 100;
        pGameState->Player.HP = pGameState->Player.MaxHP;
        pGameState->Player.Direction = {0, 1.0, 0};

        // Initialize follower
        pGameState->Follower.FrontBMP = &Memory->Assets.PlayerBMP;
        pGameState->Follower.BackBMP = &Memory->Assets.PlayerBackBMP;
        pGameState->Follower.SideBMP = &Memory->Assets.PlayerSideBMP;

        InitializeEntity(&pGameState->Follower.Entity, pGameState->Player.Entity.TilePosition, pGameState->Follower.FrontBMP);

        // Initialize enemy
        pGameState->Enemy.FrontBMP = &Memory->Assets.EnemyBMP;
        pGameState->Enemy.BackBMP = &Memory->Assets.EnemyBackBMP;

        InitializeEntity(&pGameState->Enemy.Entity, pGameState->Player.Entity.TilePosition, pGameState->Enemy.FrontBMP);

        pGameState->Enemy.MaxHP = 5;
        pGameState->Enemy.HP = pGameState->Enemy.MaxHP;        

        // Initialize projectile
        pGameState->Projectile.Active = false;
        pGameState->Projectile.FrontBMP = &Assets->ProjectileBMP;

        // Initialize projectile
        InitializeEntity(&pGameState->Projectile.Entity, pGameState->Player.Entity.TilePosition, pGameState->Projectile.FrontBMP);

        pGameState->Projectile.Celerity = 10.0;

        //game_screen_position Position = ToScreenCoord(pGameState->PlayerPosition, TILESIZE);
        pGameState->Camera = { pGameState->Player.Entity.Position, {0,0,0}, Group->Width, Group->Height, 1.0 };

        // Renderer --------------------------------------------------------------------------------------------------------------------------------------
        Group->Camera = &pGameState->Camera;

        Memory->IsInitialized = true;
    }

    switch (pGameState->Scene) {
    case Intro:
    {
        PushClear(Group, Black);

        static bool Start = false;
        static double StartTime = 0;
        
        if (Input->Keyboard.Enter.IsDown && !Input->Keyboard.Enter.WasDown && !Start) {
            Start = true;
            StartTime = pGameState->Time;
        };

        if (Start) {
            game_rect WilfredRect = { 0 };
            if (Group->Width >= Group->Height) {
                WilfredRect.Top = 0;
                WilfredRect.Left = (double)Group->Width / 2.0 - (double)Group->Height / 2.0;
                WilfredRect.Width = (double)Group->Height;
                WilfredRect.Height = (double)Group->Height;
            }
            else {
                WilfredRect.Left = 0;
                WilfredRect.Top = (double)Group->Height / 2.0 - (double)Group->Width / 2.0;
                WilfredRect.Width = (double)Group->Width;
                WilfredRect.Height = (double)Group->Width;
            }

            GameOutputSound(Assets, SoundBuffer, pGameState, Input);
            PushVideoLoop(Group, &Assets->IntroVideo, WilfredRect, 10, pGameState->LastFrameTime, 185474, 250982);
            PushTexturedRect(Group, WilfredRect, &Assets->FadeFrame, 20, true);
            
            PushText(Group, {(double)Group->Width / 2.0 - 235, (double)Group->Height / 1.2 - 2, 30}, Assets->Characters, Black, 18, Assets->TitleText, false, true);
            PushText(Group, { (double)Group->Width / 2.0 - 237, (double)Group->Height / 1.2, 31 }, Assets->Characters, White, 18, Assets->TitleText, false, true);

            if ((pGameState->Time > StartTime + 0.2) && (Input->Keyboard.Any || Input->Controller.Any)) {
                pGameState->Scene = Main;
                CloseVideo(Assets->IntroVideo.VideoContext);
            }
        }

    } break;

    case Main:
    {
        if (Input->Mouse.Wheel > 0) {
            pGameState->Camera.Zoom *= 1.2;
        }
        else if (Input->Mouse.Wheel < 0) {
            pGameState->Camera.Zoom /= 1.2;
        }

        if (Input->Keyboard.E.IsDown && !Input->Keyboard.E.WasDown) {
            pGameState->Player.HP--;
        }

        pGameState->Camera.Width = (int)Group->Width;
        pGameState->Camera.Height = (int)Group->Height;

        PushClear(Group, Black);

        // Reset room
        if (Input->Keyboard.F2.IsDown && !Input->Keyboard.F2.WasDown) {
            InitMap(pGameState->Map, pGameState->Rooms, &pGameState->nRooms);
        }

        // Update entities
        Update(pGameState->Map, &pGameState->Player, &pGameState->Projectile, Input);

        Update(pGameState->Map, &pGameState->Follower, pGameState->Player.Entity.Position, Input);

        Update(pGameState->Map, &pGameState->Projectile);

        int PlayerRoom = GetRoom(pGameState->nRooms, pGameState->Rooms, pGameState->Player.Entity.TilePosition);
        int EnemyRoom = GetRoom(pGameState->nRooms, pGameState->Rooms, pGameState->Enemy.Entity.TilePosition);
        if (PlayerRoom == EnemyRoom) {
            Update(&pGameState->Enemy, &pGameState->Projectile, pGameState->Player.Entity.Position, pGameState->Time);
        }

        // Room exploration
        int CurrentRoom = GetRoom(pGameState->nRooms, pGameState->Rooms, pGameState->Player.Entity.TilePosition);
        if (CurrentRoom != -1) {
            pGameState->Rooms[CurrentRoom].Explored = true;
        }

        // Bombs?
        //if (Input->Keyboard.Enter.IsDown && !Input->Keyboard.Enter.WasDown) {
        //    tile_position Position = pGameState->PlayerPosition;
        //    if (Position.Row - 1 >= 0 && pGameState->Map[Position.Row - 1][Position.Col].Type == Wall) {
        //        pGameState->Map[Position.Row - 1][Position.Col].Type = Door;
        //    }
        //    if (Position.Col - 1 >= 0 && pGameState->Map[Position.Row][Position.Col-1].Type == Wall) {
        //        pGameState->Map[Position.Row][Position.Col-1].Type = Door;
        //    }
        //}

        // Camera movement
        /*v3 CameraDirection = { 0,0,0 };
        if (Input->Keyboard.Right.IsDown) {
            CameraDirection.X += 1;
        }
        if (Input->Keyboard.Left.IsDown) {
            CameraDirection.X -= 1;
        }
        if (Input->Keyboard.Up.IsDown) {
            CameraDirection.Y -= 1;
        }
        if (Input->Keyboard.Down.IsDown) {
            CameraDirection.Y += 1;
        }
        CameraDirection = normalize(CameraDirection);

        pGameState->Camera.Velocity = 10 * CameraDirection;*/

        // Camera autofollow player
        v3 CameraVelocity = 0.1 * (pGameState->Player.Entity.Position - pGameState->Camera.Position);
        if (module(CameraVelocity) > 0.5) {
            pGameState->Camera.Velocity = CameraVelocity;
        }
        else {
            pGameState->Camera.Velocity = { 0,0,0 };
        }
        pGameState->Camera.Position = pGameState->Camera.Position + pGameState->Camera.Velocity;

        // Debug camera position and velocity
        /*
        char TextBuffer[256];
        sprintf_s(TextBuffer, " %.02f,%.02f,%.02f Position\n %.02f,%.02f,%.02f Velocity\n",
            pGameState->Camera.Position.X, pGameState->Camera.Position.Y, pGameState->Camera.Position.Z,
            pGameState->Camera.Velocity.X, pGameState->Camera.Velocity.Y, pGameState->Camera.Velocity.Z);
        OutputDebugStringA(TextBuffer);
        */

        Silence(SoundBuffer);

        // Render
        PushMap(Group, pGameState->Map, pGameState->nRooms, pGameState->Rooms, &Memory->Assets);

        PushEntity(Group, pGameState->Player.Entity, pGameState->Camera);

        PushEntity(Group, pGameState->Follower.Entity, pGameState->Camera);                               

        PushEntity(Group, pGameState->Enemy.Entity, pGameState->Camera);
        int BarWidth = 40;
        int BarHeight = 7;
        game_rect Rect = {
            pGameState->Enemy.Entity.Position.X - BarWidth / 2,
            pGameState->Enemy.Entity.Position.Y + pGameState->Enemy.Entity.BMPOffset.Y - 10,
            BarWidth,
            BarHeight
        };
        PushHealthBar(Group, Rect, pGameState->Enemy.HP, pGameState->Enemy.MaxHP);

        if (pGameState->Projectile.Active) {
            PushEntity(Group, pGameState->Projectile.Entity, pGameState->Camera);
        }

        game_rect DialogRect = { 0, 0.7 * Group->Height, Group->Width, 0.3 * Group->Height };

        static bool Dialog = false;
        if (Input->Keyboard.Enter.IsDown && !Input->Keyboard.Enter.WasDown) {
            Dialog = !Dialog;
        }

        static int Counter = 0;
        if (Dialog) {                          
            Counter++;
            if (Counter == 2) {
                if (Assets->DialogText.Length <= 160) {
                    Assets->DialogText.Length++;
                }
                Counter = 0;
            }
            PushText(Group, { 0,0.7 * Group->Height + 40,10 }, Assets->Characters, White, 20, Assets->DialogText, true, true);

            PushRect(Group, DialogRect, { 0.5f, 0.0f, 0.0f, 0.0f }, 9, true);
            PushRectOutline(Group, DialogRect, Gray, true);
        }
        else {
            Assets->DialogText.Length = 0;
            Counter = 0;
        }

        // UI
            // Health bar
        PushHealthBar(Group, &pGameState->Player, Assets->HPText, Assets->HPNumbersText);


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
        } break;

        default: 
        {
            Assert(false);
        }
    }

    // Debug info
    if (Input->Keyboard.F1.IsDown && !Input->Keyboard.F1.WasDown) {
        pGameState->ShowDebugInfo = !pGameState->ShowDebugInfo;
    }

    if (pGameState->ShowDebugInfo) {
        game_rect DebugInfoRect = { 0, 0, 470, 150 };
        PushRect(Group, DebugInfoRect, { 0.5, 0.0, 0.0, 0.0 }, 999, true);
        PushRectOutline(Group, DebugInfoRect, Gray, true);

        PushDebugLattice(Group, { 0.2f, 1.0f, 1.0f, 0.0f });
        PushText(Group, { 0, 30, 1001 }, Assets->Characters, White, 20, Memory->DebugInfo, false, true);

        // Mouse
        game_screen_position MousePosition = { Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y, 0 };
        PushDebugShineTile(Group, ToTilePosition(MousePosition, pGameState->Camera), { 0.5f, 1.0f, 1.0f, 1.0f });

        // Player position
        PushDebugShineTile(Group, pGameState->Player.Entity.TilePosition, { 0.5f, 0, 0, 1.0f });

        // Follower position
        PushDebugShineTile(Group, pGameState->Follower.Entity.TilePosition, { 0.5f, 0, 0, 1.0f });

        // Enemy position
        PushDebugShineTile(Group, pGameState->Enemy.Entity.TilePosition, { 0.5f, 1.0f, 0, 0 });

        // Projectile position
        if (pGameState->Projectile.Active) {
            PushDebugShineTile(Group, pGameState->Projectile.Entity.TilePosition, { 0.5f, 1.0f, 1.0f, 0 });
        }
    }
}

