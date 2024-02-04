#pragma once

const int MAX_ENTRIES = 500;

enum render_group_entry_type {
	group_type_render_entry_clear,
    group_type_render_entry_line,
	group_type_render_entry_rect,
    group_type_render_entry_textured_rect,
    group_type_render_entry_rect_outline,
	group_type_render_entry_bmp,
    group_type_render_entry_text,
    group_type_render_entry_button,
    group_type_render_entry_debug_lattice,
    group_type_render_entry_debug_shine_tile
};

enum WrapMode {
    Clamp,
    Repeat
};

struct sort_key {
    double Z;
    double Y;
};

bool LessThan(sort_key Key1, sort_key Key2) {
    return Key1.Z < Key2.Z || ((Key1.Z == Key2.Z) && (Key1.Y < Key2.Y));
}

struct render_group_header {
	render_group_entry_type Type;
	sort_key Key;
	uint32 PushBufferOffset;
};

struct render_entry_clear {
	render_group_header Header;
	color Color;
};

struct render_entry_line {
    render_group_header Header;
    color Color;
    game_screen_position Start;
    game_screen_position Finish;
    bool isUI;
};

struct render_entry_rect {
	render_group_header Header;
	game_rect Rect;
	color Color;
    bool isUI;
};

struct render_entry_textured_rect {
    render_group_header Header;
    render_basis Basis;
    v3 Position;
    loaded_bmp* Texture;
    WrapMode Mode;
};

struct render_entry_rect_outline {
    render_group_header Header;
    game_rect Rect;
    color Color;
    bool isUI;
};

struct render_entry_bmp {
	render_group_header Header;
	loaded_bmp* Bitmap;
	v3 Position;
    WrapMode Mode;
};

struct render_entry_text {
    render_group_header Header;
    render_basis Basis;
    Character* Characters;
    game_screen_position Position;
    text Text;
    bool isUI;
};

struct render_entry_button {
    render_group_header Header;
    Character* Characters;
    button* Button;
};

struct render_entry_debug_lattice {
    render_group_header Header;
    color Color;
};

struct render_entry_debug_shine_tile {
    render_group_header Header;
    tile_position Position;
    color Color;
};

struct render_group {
	float MetersToPixels;
	render_basis* DefaultBasis;
    camera* Camera;
    Character* Characters;
	uint32 MaxPushBufferSize;
	uint32 PushBufferSize;
	uint32 PushBufferElementCount;
	uint8* PushBufferBase;
};

struct sort_entry {
    sort_key Key;
    uint32 PushBufferOffset;
};


render_group* AllocateRenderGroup(memory_arena* Arena, memory_index MaxPushBufferSize) {
    render_group* Result = PushStruct(Arena, render_group);
    Result->PushBufferBase = (uint8*)PushSize(Arena, MaxPushBufferSize);
    Result->MaxPushBufferSize = MaxPushBufferSize;
    Result->PushBufferSize = 0;

    Result->DefaultBasis = PushStruct(Arena, render_basis);
    Result->DefaultBasis->X = V3(1, 0, 0);
    Result->DefaultBasis->Y = V3(0, 1, 0);
    Result->DefaultBasis->Z = V3(0, 0, 1);
    Result->MetersToPixels = 1;

    return(Result);
}


#define PushRenderElement(Group, type) (type*)PushRenderElement_(Group, sizeof(type), group_type_##type)
render_group_header* PushRenderElement_(render_group* Group, uint32 Size, render_group_entry_type Type) {
    render_group_header* Result = 0;

    if ((Group->PushBufferSize + Size) < Group->MaxPushBufferSize) {
        Result = (render_group_header*)(Group->PushBufferBase + Group->PushBufferSize);
        Result->Key = { 0 }; // Must be set when pushed
        Result->Type = Type;
        Result->PushBufferOffset = Group->PushBufferSize;

        Group->PushBufferSize += Size;
        Group->PushBufferElementCount++;
    }
    else {
        // Invalid code path
    }

    return Result;
}

uint32 GetSizeOf(render_group_entry_type Type) {
    switch (Type) {
        case group_type_render_entry_clear:
        {
            return sizeof(render_entry_clear);
        } break;

        case group_type_render_entry_line:
        {
            return sizeof(render_entry_line);
        } break;

        case group_type_render_entry_rect:
        {
            return sizeof(render_entry_rect);
        } break;

        case group_type_render_entry_rect_outline:
        {
            return sizeof(render_entry_rect_outline);
        } break;

        case group_type_render_entry_bmp:
        {
            return sizeof(render_entry_bmp);
        } break;

        case group_type_render_entry_text:
        {
            return sizeof(render_entry_text);
        } break;

        case group_type_render_entry_button:
        {
            return sizeof(render_entry_button);
        } break;

        case group_type_render_entry_debug_lattice:
        {
            return sizeof(render_entry_debug_lattice);
        } break;

        case group_type_render_entry_textured_rect:
        {
            return sizeof(render_entry_textured_rect);
        } break;

        case group_type_render_entry_debug_shine_tile:
        {
            return sizeof(render_entry_debug_shine_tile);
        } break;

        default:
        {
            Assert(false);
        } break;
    }
}

void PushClear(render_group* Group, color Color) {
    render_entry_clear* Entry = PushRenderElement(Group, render_entry_clear);
    Entry->Header.Key.Z = 0;
    Entry->Header.Key.Y = 0;
    Entry->Color = Color;
}

void PushLine(render_group* Group, color Color, game_screen_position Start, game_screen_position Finish) {
    render_entry_line* Entry = PushRenderElement(Group, render_entry_line);
    Entry->Header.Key.Z = max(Start.Z, Finish.Z);
    Entry->Header.Key.Y = 0;
    Entry->Color = Color;
    Entry->Start = Start;
    Entry->Finish = Finish;
}

void PushRect(render_group* Group, game_rect Rect, color Color, double Z, bool isUI) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Z = Z;
    Entry->Header.Key.Y = 0;
    Entry->Rect = Rect;
    Entry->Color = Color;
    Entry->isUI = isUI;
}

void PushTexturedRect(render_group* Group, loaded_bmp* Texture, v3 Position, render_basis Basis, WrapMode Mode) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Position.Z;
    Entry->Header.Key.Y = Position.Y;
    Entry->Texture = Texture;
    Entry->Basis = Basis;
    Entry->Position = Position;
    Entry->Mode = Mode;
}

void PushRectOutline(render_group* Group, game_rect Rect, color Color, bool isUI) {
    render_entry_rect_outline *Entry = PushRenderElement(Group, render_entry_rect_outline);
    Entry->Header.Key.Z = 300;
    Entry->Header.Key.Y = 0;
    Entry->Rect = Rect;
    Entry->Color = Color;
    Entry->isUI = isUI;
}

void PushBMP(render_group* Group, loaded_bmp* Bitmap, v3 Position) {
    render_entry_bmp* Entry = PushRenderElement(Group, render_entry_bmp);
    Entry->Header.Key.Z = Position.Z;
    Entry->Header.Key.Y = Position.Y;
    Entry->Bitmap = Bitmap;
    Entry->Position = Position;
}

void PushText(render_group* Group, game_screen_position Position, text Text, bool isUI) {
    render_entry_text* Entry = PushRenderElement(Group, render_entry_text);
    Entry->Header.Key.Z = Position.Z;
    Entry->Header.Key.Y = Position.Y;
    Entry->Characters = Group->Characters;
    Entry->Position = Position;
    Entry->Text = Text;
    Entry->isUI = isUI;
    float a = (double)Text.Points / 20.0;
    Entry->Basis.X = V3(a, 0, 0);
    Entry->Basis.Y = V3(0, a, 0);
    Entry->Basis.Z = V3(0, 0, a);
}

void PushButton(render_group* Group, Character* Characters, button* Button) {
    render_entry_button* Entry = PushRenderElement(Group, render_entry_button);
    Entry->Header.Key.Z = 0;
    Entry->Header.Key.Y = 0;
    Entry->Button = Button;
    Entry->Characters = Characters;
}

void PushDebugLattice(render_group* Group, color Color) {
    render_entry_debug_lattice* Entry = PushRenderElement(Group, render_entry_debug_lattice);
    Entry->Header.Key.Z = 999;
    Entry->Header.Key.Y = 0;
    Entry->Color = Color;
}

void PushDebugShineTile(render_group* Group, tile_position Position, color Color) {
    render_entry_debug_shine_tile* Entry = PushRenderElement(Group, render_entry_debug_shine_tile);
    Entry->Header.Key.Z = 998;
    Entry->Header.Key.Y = 0;
    Entry->Position = Position;
    Entry->Color = Color;
}

void PushDoor(render_group* Group, game_assets* Assets, tile Map[MAP_HEIGHT][MAP_WIDTH], int Row, int Col) {
    v3 DoorPosition = ToWorldCoord({ Row - 1, Col, 1 });
    DoorPosition.Z = 3;

    if (Row > 0 && Map[Row - 1][Col].Type == Floor) {
        PushBMP(Group, &Assets->FloorBMP, ToWorldCoord({ Row, Col, 0 }));
    }

    PushBMP(Group, &Assets->DoorBMP, DoorPosition);
}

void PushMap(render_group* Group, tile Map[MAP_HEIGHT][MAP_WIDTH], int nRooms, room Rooms[], game_assets* Assets) {
    
    for (int i = 0; i < nRooms; i++) {
        room Room = Rooms[i];
        if (Room.Explored) {
            v3 Position = ToWorldCoord({ Room.Top, Room.Left, 0 });
            render_basis Basis = {
                { Room.Width, 0, 0 },
                { 0, Room.Height, 0},
                { 0, 0, 1 }
            };
            PushTexturedRect(Group, &Assets->FloorBMP,{ Position.X, Position.Y, 0 }, Basis, Repeat);

            for (int i = Room.Top; i < Room.Top + Room.Height; i++) {
                for (int j = Room.Left; j < Room.Left + Room.Width; j++) {
                    if (Map[i][j].Type == Chest) {
                        PushBMP(Group, &Assets->ChestBMP, ToWorldCoord({ i,j,1 }));
                    }
                }
            }

            if (Room.Left > 0) {
                int j = Room.Left - 1;
                for (int i = Room.Top; i < Room.Top + Room.Height; i++) {
                    if (Map[i][j].Type == Door) {
                        PushDoor(Group, Assets, Map, i, j);
                    }
                }
            }

            if (Room.Left + Room.Width < MAP_WIDTH) {
                int j = Room.Left + Room.Width;
                for (int i = Room.Top; i < Room.Top + Room.Height; i++) {
                    if (Map[i][j].Type == Door) {
                        PushDoor(Group, Assets, Map, i, j);
                    }
                }
            }

            if (Room.Top > 0) {
                int i = Room.Top - 1;
                for (int j = Room.Left; j < Room.Left + Room.Width; j++) {
                    if (Map[i][j].Type == Door) {
                        PushDoor(Group, Assets, Map, i, j);
                    }
                }
            }

            if (Room.Top + Room.Height < MAP_HEIGHT) {
                int i = Room.Top + Room.Height;
                for (int j = Room.Left; j < Room.Left + Room.Width; j++) {
                    if (Map[i][j].Type == Door) {
                        PushDoor(Group, Assets, Map, i, j);
                    }
                }
            }
        }
    }
}

void PushEntity(render_group* Group, entity Entity, camera Camera) {
    PushTexturedRect(Group, Entity.BMP, Entity.Position + Entity.BMPOffset, Entity.Basis, Clamp);
}

void PushHealthBar(render_group* Group, int HP, int MaxHP, char* HPTextContent, char* HealthTextContent) {
    
    double X = Group->Camera->Width - 120;
    double p = (double)HP / (double)MaxHP;
    PushRect(Group, { X,20,100,25 }, Gray, 9999, true);
    PushRect(Group, { X,20,p*100.0,25 }, Red, 10000, true);

    text HPText = { 0 };
    HPText.Color = White;
    HPText.Length = 2;
    HPText.Points = 10;
    HPText.Content = HPTextContent;

    PushText(Group, {X - 27, 52, 10001}, HPText, true);

    text HealthText = { 0 };
    HealthText.Color = White;
    if (HP == 100) {
        HealthText.Length = 7;
    }
    else {
        HealthText.Length = 6;
    }
    HealthText.Points = 10;
    HealthText.Content = HealthTextContent;

    PushText(Group, { X , 52, 10001 }, HealthText, true);
}

void ClearEntries(render_group* Group) {
    Group->PushBufferElementCount = 0;
    Group->PushBufferSize = 0;
}

void SwapEntries(sort_entry* Entry1, sort_entry* Entry2) {
    sort_key Key1 = Entry1->Key;
    uint32 Offset1 = Entry1->PushBufferOffset;

    *Entry1 = *Entry2;
    *Entry2 = { Key1, Offset1 };
}

void SortEntries(render_group* RenderGroup, sort_entry Entries[MAX_ENTRIES]) {
    uint32 Count = RenderGroup->PushBufferElementCount;

    uint32 BaseAddress = 0;
    for (int i = 0; i < Count; i++) {
        render_group_header* Header = (render_group_header*)(RenderGroup->PushBufferBase + BaseAddress);
        Entries[i] = { Header->Key, Header->PushBufferOffset };

        BaseAddress += GetSizeOf(Header->Type);
    }

    for (int i = 0; i < Count - 1; i++) {
        if (LessThan(Entries[i+1].Key, Entries[i].Key))
        {
            int j = i;
            do {
                SwapEntries(&Entries[j], &Entries[j + 1]);
                j--;
            } while (j > 0 && LessThan(Entries[j + 1].Key, Entries[j].Key));
        }
    }
}


// +------------------------------------------------------------------------------------------------------------------+
// |  Renders                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------+


void Clear(loaded_bmp* OutputTarget, color Color) {
    uint32 ColorBytes = GetColorBytes(Color);
    uint8* Row = (uint8*)OutputTarget->Content;
    for (int Y = 0; Y < OutputTarget->Header.Height; Y++) {
        uint32* Pixel = (uint32*)Row;
        for (int X = 0; X < OutputTarget->Header.Width; X++) {
            *Pixel++ = ColorBytes;
        }
        Row += OutputTarget->Pitch;
    }
}

//uint32* GetPixel(loaded_bmp* Bitmap, game_screen_position Position) {
//    if (Position.X > Bitmap->Header.Width || Position.Y > Bitmap->Header.Height) {
//        return (uint32*)Bitmap->Content;
//    }
//    return (uint32*)Bitmap->Content + Position.X + Position.Y * Bitmap->Header.Width;
//}

bool IsInside(game_screen_position Position, game_rect Rect) {
    bool A = Position.X >= Rect.Left && Position.X <= Rect.Left + Rect.Width;
    bool B = Position.Y >= Rect.Top && Position.Y <= Rect.Top + Rect.Height;
    return A && B;
}

int CheckLineSide(v2 P0, v2 P1, v2 Position) {
    // 1 means to the right, -1 to the left, 0, is on the line
    if (P0.X == P1.X) {
        if (Position.X == P0.X) {
            return 0;
        }
        else {
            return (Position.X > P0.X) ? 1 : -1;
        }
    }

    // if the line is horizontal, 1 is up, -1 is down
    if (P0.Y == P1.Y) {
        if (Position.Y == P0.Y) {
            return 0;
        }
        else {
            return (Position.Y > P0.Y) ? -1 : 1;
        }
    }

    double X = (Position.X - P0.X) / (P1.X - P0.X);
    double Y = (Position.Y - P0.Y) / (P1.Y - P0.Y);

    if (X == Y) {
        return 0;
    }
    else {
        return (X > Y) ? 1 : -1;
    }
}

void RenderLine(loaded_bmp* OutputTarget, color Color, game_screen_position Start, game_screen_position Finish) {
    // Deciding if we need to render at all
    game_rect Rect = {0};
    Rect.Width = OutputTarget->Header.Width;
    Rect.Height = OutputTarget->Header.Height;

    if (!IsInside(Start, Rect)                 && 
        !IsInside(Finish, Rect)                && 
        !IsInside({ Start.X, Finish.Y }, Rect) && 
        !IsInside({ Finish.X, Start.Y }, Rect)) {
        return;
    }

    // int32 Thickness = 1; // TODO: Add thickness

    int32 DX = Sign((double)(Finish.X - Start.X));
    int32 DY = Sign((double)(Finish.Y - Start.Y));

    v2 P0 = V2(Start.X, Start.Y);
    v2 P1 = V2(Finish.X, Finish.Y);

    int32 X0 = Start.X;
    int32 Y0 = Start.Y;

    uint32 ColorBytes = GetColorBytes(Color);
    uint32* Row = OutputTarget->Content + X0 + Y0 * OutputTarget->Header.Width;
    while (true) {
        uint32* Pixel = Row;

        if (X0 > 0 && X0 < OutputTarget->Header.Width &&
            Y0 > 0 && Y0 < OutputTarget->Header.Height) {
            *Pixel = ColorBytes;
        }

        if (X0 == Finish.X && Y0 == Finish.Y) {
            break;
        }

        //int A11 = CheckLineSide(P0, P1, V2(X0, Y0));
        int A12 = CheckLineSide(P0, P1, V2(X0 + DX, Y0));
        int A21 = CheckLineSide(P0, P1, V2(X0, Y0 + DY));
        int A22 = CheckLineSide(P0, P1, V2(X0 + DX, Y0 + DY));

        if (A22 != A12 || Start.Y == Finish.Y) {
            X0 += DX;
            Row += DX;
        }

        if (A22 != A21 || Start.X == Finish.X) {
            Y0 += DY;
            Row += DY * OutputTarget->Header.Width;
        }
    }
}


void RenderRectangle(loaded_bmp* OutputTarget, game_rect Rect, color Color) {
    int32 TargetWidth = (int32)OutputTarget->Header.Width;
    int32 TargetHeight = (int32)OutputTarget->Header.Height;
    int32 BytesPerPixel = OutputTarget->Header.BitsPerPixel >> 3;

    // Crop extra pixels
    int MinX = Rect.Left;
    if (Rect.Left < 0) {
        MinX = 0;
    }
    else if (MinX > TargetWidth) {
        return;
    }

    int MaxX = MinX + Rect.Width;
    if (MaxX > TargetWidth) {
        MaxX = TargetWidth;
    }

    int MinY = Rect.Top;
    if (MinY < 0) {
        MinY = 0;
    }
    else if (MinY > TargetHeight) {
        return;
    }

    int MaxY = MinY + Rect.Height;
    if (MaxY > TargetHeight) {
        MaxY = TargetHeight;
    }

    uint32 ColorBytes = GetColorBytes(Color);
    uint8* Row = (uint8*)OutputTarget->Content + MinX * BytesPerPixel + MinY * OutputTarget->Pitch;
    for (int Y = MinY; Y < MaxY; Y++) {
        uint32* Pixel = (uint32*)Row;
        for (int X = MinX; X < MaxX; X++) {
            *Pixel++ = ColorBytes;
        }
        Row += OutputTarget->Pitch;
    }
}


void RenderBMP(loaded_bmp* OutputTarget, loaded_bmp* BMP, game_screen_position Position) {
    int32 BMPWidth = BMP->Header.Width;
    int32 BMPHeight = BMP->Header.Height;

    int32 TargetWidth = (int32)OutputTarget->Header.Width;
    int32 TargetHeight = (int32)OutputTarget->Header.Height;

    if (Position.X + BMPWidth > 0 && Position.X < TargetWidth &&
        Position.Y + BMPHeight > 0 && Position.Y < TargetHeight) {
        int32 BlitWidth;
        int32 BlitHeight;
        game_screen_position SourcePosition;
        game_screen_position DestinationPosition;

        // Cropping
        if (Position.X < 0) {
            BlitWidth = BMPWidth + Position.X;
            SourcePosition.X = -Position.X;
            DestinationPosition.X = 0;
        }
        else {
            SourcePosition.X = 0;
            DestinationPosition.X = Position.X;
            if (Position.X + BMPWidth > TargetWidth) {
                BlitWidth = TargetWidth - Position.X;
            }
            else {
                BlitWidth = BMPWidth;
            }
        }

        if (Position.Y < 0) {
            BlitHeight = BMPHeight + Position.Y;
            SourcePosition.Y = -Position.Y;
            DestinationPosition.Y = 0;
        }
        else {
            SourcePosition.Y = 0;
            DestinationPosition.Y = Position.Y;
            if (Position.Y + BMPHeight > TargetHeight) {
                BlitHeight = TargetHeight - Position.Y;
            }
            else {
                BlitHeight = BMPHeight;
            }
        }

        // BMP starts on the last row
        uint32* SourceRow = BMP->Content + BMP->Header.Width * (BMP->Header.Height - (int)SourcePosition.Y - 1) + (int)SourcePosition.X;
        uint8* DestinationRow = (uint8*)(OutputTarget->Content + (int)DestinationPosition.X) + (int)DestinationPosition.Y * OutputTarget->Pitch;
        for (int32 Y = 0; Y < BlitHeight; Y++) {
            uint32* Destination = (uint32*)DestinationRow;
            uint32* Source = SourceRow;
            for (int32 X = 0; X < BlitWidth; X++) {
                color BMPColor = GetColor(*Source++, BMP->Header.RedMask, BMP->Header.GreenMask, BMP->Header.BlueMask);
                color BackgroundColor = GetColor(*Destination, 0x00ff0000, 0x0000ff00, 0x000000ff);

                if (BMPColor.R != BackgroundColor.R || BMPColor.G != BackgroundColor.G || BMPColor.B != BackgroundColor.B) {
                    *Destination++ = GetColorBytes(Blend(BMPColor, BackgroundColor));
                }
                else {
                    Destination++;
                }
            }
            SourceRow -= BMP->Header.Width;
            DestinationRow += OutputTarget->Pitch;
        }
    }
}


//void RenderWhiteNoise(game_offscreen_buffer* Buffer) {
//    uint8* Row = (uint8*)Buffer->Memory;
//    for (int Y = 0; Y < Buffer->Height; ++Y) {
//        uint32* Pixel = (uint32*)Row;
//        for (int X = 0; X < Buffer->Width; ++X) {
//            uint8 Gray = rand() % 255;
//            uint8 Red = Gray;
//            uint8 Green = Gray;
//            uint8 Blue = Gray;
//
//            uint32 RGB_color = (Red << 16) | (Green << 8) | Blue;
//
//            *Pixel++ = RGB_color;
//        }
//        Row += Buffer->Pitch;
//    }
//}

//void RenderWeirdGradient(game_offscreen_buffer* Buffer, game_state* pGameState) {
//    //int XOffset = pGameState->XOffset;
//    //int YOffset = pGameState->YOffset;
//
//    uint8* Row = (uint8*)Buffer->Memory;
//    for (int Y = 0; Y < Buffer->Height; ++Y) {
//        uint32* Pixel = (uint32*)Row;
//        for (int X = 0; X < Buffer->Width; ++X) {
//            uint8 Red = 0xff;
//            //uint8 Green = X + XOffset;
//            uint8 Blue = Y + YOffset;
//
//            uint32 RGB_color = (Red << 16) | (Green << 8) | Blue;
//
//            *Pixel++ = RGB_color;
//        }
//        Row += Buffer->Pitch;
//    }
//}


// Bitmaps
void ClearBitmap(loaded_bmp* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = Bitmap->Header.Width * Bitmap->Header.Height * 32;
        ZeroSize(TotalBitmapSize, Bitmap->Content);
    }
}

loaded_bmp MakeEmptyBitmap(memory_arena* Arena, int32 Width, int32 Height, bool ClearToZero = true) {
    loaded_bmp Result;
    Result.Header = { 0 };
    Result.Header.Width = Width;
    Result.Header.Height = Height;
    Result.Header.BitsPerPixel = 32;
    Result.BytesPerPixel = 4;
    Result.Pitch = 4 * Width;
    int32 TotalBitmapSize = Width * Height * 32;
    Result.Header.FileSize = TotalBitmapSize;

    Result.Header.RedMask = 0x00ff0000;
    Result.Header.GreenMask = 0x0000ff00;
    Result.Header.BlueMask = 0x000000ff;
    Result.AlphaMask = 0xff000000;

    Result.Content = (uint32*)PushSize(Arena, TotalBitmapSize / 8);
    if (ClearToZero) {
        ClearBitmap(&Result);
    }
    return Result;
}

// Text
void LoadFTBMP(FT_Bitmap* SourceBMP, loaded_bmp* DestBMP) {
    DestBMP->Header.Width = SourceBMP->width;
    DestBMP->Header.Height = SourceBMP->rows;
    uint32* DestRow = DestBMP->Content + DestBMP->Header.Width * (DestBMP->Header.Height - 1);
    uint8* Source = SourceBMP->buffer;
    for (int Y = 0; Y < SourceBMP->rows; Y++) {
        uint32* Pixel = DestRow;
        for (int X = 0; X < SourceBMP->width; X++) {
            // FreeType BMPs come with only one byte representing alpha. We load it as a white BMP so changing
            // the color is easier with OpenGL.
            *Pixel++ = (*Source++ << 24) | 0x00ffffff;
        }
        DestRow -= SourceBMP->pitch;
    }
}

void RenderText(loaded_bmp* OutputTarget, memory_arena* Arena, FT_Face* Font, game_screen_position Position, text Text) {
    FT_Error error;

    error = FT_Set_Char_Size(*Font, 0, Text.Points * 64, 128, 128);
    if (error) {
        Assert(false);
    }
    else {
        FT_GlyphSlot Slot = (*Font)->glyph;
        int PenX = Position.X;
        int PenY = Position.Y;

        error = FT_Load_Char(*Font, '\n', FT_LOAD_RENDER);
        if (error) {
            Assert(false);
        }

        int LineJump = (int)(0.023f * (float)Slot->metrics.height); // 0.023 because height is in 64ths of pixel

        for (int i = 0; i < Text.Length; i++) {
            error = FT_Load_Char(*Font, Text.Content[i], FT_LOAD_RENDER);
            if (error) {
                Assert(false);
            }

            // Carriage returns
            if (Text.Content[i] == '\n') {
                PenY += LineJump;
                PenX = Position.X;
            }
            else {
                if (Text.Wrapped && PenX + (Slot->metrics.width >> 6) > OutputTarget->Header.Width) {
                    PenX = Position.X;
                    PenY += LineJump;
                }
                FT_Bitmap FTBMP = Slot->bitmap;
                loaded_bmp BMP = MakeEmptyBitmap(Arena, FTBMP.width, FTBMP.rows, true);
                LoadFTBMP(&FTBMP, &BMP);
                RenderBMP(OutputTarget, &BMP, { (double)(PenX + Slot->bitmap_left), (double)(PenY - Slot->bitmap_top), 0 });
                PopSize(Arena, BMP.Header.FileSize / 8);
                PenX += Slot->advance.x >> 6;
            }
        }
    }
}

/*
void RenderGroupToOutput(render_group* Group, loaded_bmp* OutputTarget) {
    v2 ScreenCenter = { 0.5f * (float)OutputTarget->Header.Width, 0.5f * (float)OutputTarget->Header.Height };

    for (uint32 BaseAddress = 0; BaseAddress < Group->PushBufferSize; ) {
        render_group_header* Header = (render_group_header*)(Group->PushBufferBase + BaseAddress);
        switch (Header->Type) {
            case group_type_render_entry_clear: {
                render_entry_clear* Entry = (render_entry_clear*)Header;
                Clear(OutputTarget, Entry->Color);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_line: {
                render_entry_line* Entry = (render_entry_line*)Header;
                RenderLine(OutputTarget, Entry->Color, Entry->Start, Entry->Finish);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_rect: {
                render_entry_rect* Entry = (render_entry_rect*)Header;
                RenderRectangle(OutputTarget, Entry->Rect, Entry->Color);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_bmp: {
                render_entry_bmp* Entry = (render_entry_bmp*)Header;
                RenderBMP(OutputTarget, Entry->Bitmap, Entry->Position);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_text: {
                render_entry_text* Entry = (render_entry_text*)Header;
                RenderText(OutputTarget, Entry->Arena, Entry->Font, Entry->Position, Entry->Text);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_button: {
                render_entry_button* Entry = (render_entry_button*)Header;
                button* Button = Entry->Button;
                RenderBMP(OutputTarget, Button->Clicked ? &Button->ClickedImage : &Button->Image, { Button->Collider.Left, Button->Collider.Top, 0 });
                RenderText(OutputTarget, Entry->Arena, Button->Face, { 
                    Button->Collider.Left + Button->Image.Header.Width / 2,
                    Button->Collider.Top + Button->Image.Header.Height / 2,
                    0 }, Button->Text);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_rect_outline: {
                render_entry_rect_outline* Entry = (render_entry_rect_outline*)Header;

                game_screen_position P11 = { Entry->Rect.Left, Entry->Rect.Top, 0};
                game_screen_position P12 = { Entry->Rect.Left + Entry->Rect.Width, Entry->Rect.Top, 0 };
                game_screen_position P21 = { Entry->Rect.Left, Entry->Rect.Top + Entry->Rect.Height, 0 };
                game_screen_position P22 = { Entry->Rect.Left + Entry->Rect.Width, Entry->Rect.Top + Entry->Rect.Height, 0 };

                RenderLine(OutputTarget, Entry->Color, P11, P12);
                RenderLine(OutputTarget, Entry->Color, P12, P22);
                RenderLine(OutputTarget, Entry->Color, P22, P21);
                RenderLine(OutputTarget, Entry->Color, P21, P11);

                BaseAddress += sizeof(*Entry);
            } break;
            default: {
                // Invalid code path
                Assert(false);
            } break;
        }
    }
}
*/