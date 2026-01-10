#ifndef GAME_ENTITY
#define GAME_ENTITY

#include "GameMath.h"
#include "GamePlatform.h"
#include "GameAssets.h"
#include "GameInput.h"
#include "GameRender.h"
#include "Particles.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Camera                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+s

struct camera {
    uint32 ID;
    v3 Position;
    float Distance;
    float Pitch;
    float Angle;
    basis Basis;
    matrix4 View;
    bool OnAir;
};

basis GetCameraBasis(float Angle, float Pitch) {
    float cosA = cosf(Angle * Degrees);
    float sinA = sinf(Angle * Degrees);
    float cosP = cosf(Pitch * Degrees);
    float sinP = sinf(Pitch * Degrees);

    v3 X = V3(        cosA,  0.0,         sinA);
    v3 Y = V3(-sinA * sinP, cosP,  cosA * sinP);
    v3 Z = V3( sinA * cosP, sinP, -cosA * cosP);

    basis Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    return Result;
}

matrix4 GetViewMatrix(camera* Camera) {
	matrix3 Basis = Camera->Basis;
	Basis.Z = -Basis.Z;
	Basis = transpose(Basis);

	v3 Translation = V3(0,0,Camera->Distance) - Camera->Position * Basis;
	matrix4 Result;
	Result.X = V4(Basis.X, 0);
	Result.Y = V4(Basis.Y, 0);
	Result.Z = V4(Basis.Z, 0);
	Result.W = V4(Translation, 1);

	return Result;
}

v2 GetScreenPosition(float ScreenWidth, float ScreenHeight, camera* Camera, v3 WorldPosition) {
    v4 Point = V4(WorldPosition, 1);
    matrix4 Projection = GetWorldProjectionMatrix(ScreenWidth, ScreenHeight);
    v4 ViewPoint = Point * Camera->View * Projection;
    float Factor = ViewPoint.W == 0 ? 0 : 1.0f/ViewPoint.W;
    v2 DevicePoint = Factor * V2(ViewPoint.X, ViewPoint.Y);
    v2 ScreenPoint = V2(
        0.5f * (1.0f + DevicePoint.X) * ScreenWidth,
        0.5f * (1.0f - DevicePoint.Y) * ScreenHeight
    );
    return ScreenPoint;
}

const int MAX_CAMERAS = 16;
DefineFreeList(MAX_CAMERAS, camera);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Board                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+
const int CELL_SIZE = 20; // in pixels
const int INITIAL_BOARD_WIDTH = 50;
const int INITIAL_BOARD_HEIGHT = 50;

struct board {
    int32 Width;
    int32 Height;
    uint8* Cells;
    uint8* NextCells;
    bool Simulating;
    bool Clearing;
};

void Initialize(board* Board, int32 Width, int32 Height) {
    Board->Width = Width;
    Board->Height = Height;
    Board->Cells = (uint8*)calloc(Width*Height, sizeof(uint8));
    Board->NextCells = (uint8*)calloc(Width*Height, sizeof(uint8));
}

void Clear(board* Board) {
    ZeroSize(Board->Width*Board->Height, Board->Cells);
    ZeroSize(Board->Width*Board->Height, Board->NextCells);
}

bool IsValidCell(board* Board, int32 Row, int32 Col) {
    return Row >= 0 && Row < Board->Height && Col >= 0 && Col < Board->Width;
}

void Resize(board* Board, int32 Width, int32 Height) {
    Board->Cells = (uint8*)realloc(Board->Cells, Width*Height);
    Board->NextCells = (uint8*)realloc(Board->NextCells, Width*Height);
}

bool GetCell(board* Board, int32 Row, int32 Col) {
    Assert(IsValidCell(Board, Row, Col));
    return Board->Cells[Col + Row*Board->Width];
}

void SetCell(board* Board, int32 Row, int32 Col, bool Value) {
    Assert(IsValidCell(Board, Row, Col));
    Board->Cells[Col + Row*Board->Width] = Value ? 255 : 0;
    Board->NextCells[Col + Row*Board->Width] = Value ? 255 : 0;
}

void UpdateCell(board* Board, int32 Row, int32 Col, bool Value) {
    Board->NextCells[Col + Row*Board->Width] = Value ? 255 : 0;
}

void ToggleCell(board* Board, int32 Row, int32 Col) {
    bool Value = Board->Cells[Col + Row*Board->Width];
    Board->Cells[Col + Row*Board->Width] = Value ? 0 : 255;
}

rectangle CellRect(float Left, float Top, float Row, float Col) {
    return  { Left + Col*CELL_SIZE, Top + Row*CELL_SIZE, CELL_SIZE, CELL_SIZE };
}

void PushBoard(render_group* Group, board* Board, float Left, float Top) {
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        White,
        vertex_layout_v2_v2_id,
        4,
        6,
        SORT_ORDER_DEBUG_OVERLAY,
        {
            .BoardData = Board->Cells,
            .Flags = BOARD_FLAG,
        }
    );

    float Width = Board->Width*CELL_SIZE;
    float Height = Board->Height*CELL_SIZE;

    float* Vertices = (float*)Result->Vertices;
    uint32* Elements = Result->ElementEntry.Pointer;
    uint32 Offset = (uint32)Result->VertexEntry.Offset;

    *Vertices++ = Left;         *Vertices++ = Top;          *Vertices++ = 0.0f; *Vertices++ = 1.0f;
    *Vertices++ = Left + Width; *Vertices++ = Top;          *Vertices++ = 1.0f; *Vertices++ = 1.0f;
    *Vertices++ = Left;         *Vertices++ = Top + Height; *Vertices++ = 0.0f; *Vertices++ = 0.0f;
    *Vertices++ = Left + Width; *Vertices++ = Top + Height; *Vertices++ = 1.0f; *Vertices++ = 0.0f;

    Elements[0] = Offset + 0;
    Elements[1] = Offset + 1;
    Elements[2] = Offset + 2;
    Elements[3] = Offset + 3;
    Elements[4] = Offset + 2;
    Elements[5] = Offset + 1;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Game state                                                                                                                                   |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct game_state {
    particle_emitter* Emitter;
    board Board;
    camera ActiveCamera;
    double dt;
    float Time;
    bool Exit;
};

void UpdateGameState(render_group* Group, game_state* State, game_input* Input) {
    board* Board = &State->Board;

    float Left = 0.5f*Group->Width - 0.5f*Board->Width*CELL_SIZE;
    float Top = 0.5f*Group->Height - 0.5f*Board->Height*CELL_SIZE;

    int32 MouseRow = (Input->Mouse.Cursor.Y - Top) / CELL_SIZE;
    int32 MouseCol = (Input->Mouse.Cursor.X - Left) / CELL_SIZE;

    if (Board->Clearing) {
        Clear(Board);
        Board->Clearing = false;
    }
    else {
        for (int32 Row = 0; Row < Board->Height; Row++) 
        for (int32 Col = 0; Col < Board->Width; Col++) {
            bool Alive = GetCell(Board, Row, Col);

            if (Board->Simulating) {
                int32 AliveNeighbors = 0;
                for (int i = -1; i <= 1; i++)
                for (int j = -1; j <= 1; j++) {
                    int32 NeighRow = Row + i;
                    int32 NeighCol = Col + j;
                    if (IsValidCell(Board, NeighRow, NeighCol)) {
                        if (GetCell(Board, NeighRow, NeighCol)) {
                            AliveNeighbors += 1;
                        }
                    }
                }

                // New cell born
                if (!Alive) {
                    if (AliveNeighbors == 3) {
                        UpdateCell(Board, Row, Col, true);
                        Alive = true;
                    }
                }
                // Over or underpopulation
                else if (AliveNeighbors <= 2 || AliveNeighbors > 4) {
                    UpdateCell(Board, Row, Col, false);
                    Alive = false;
                }
            }
        }

        memcpy(Board->Cells, Board->NextCells, Board->Width*Board->Height);
    }

    PushBoard(Group, Board, Left, Top);

    if (IsValidCell(Board, MouseRow, MouseCol)) {
        PushRectOutline(Group, CellRect(Left, Top, MouseRow, MouseCol), White);
    }
}

#endif