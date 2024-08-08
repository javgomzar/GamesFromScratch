#pragma once
#include "FFMPEG.h"

const int MAX_ENTRIES = 500;



enum render_group_entry_type {
    group_type_render_entry_clear,
    group_type_render_entry_line,
    group_type_render_entry_triangle,
    group_type_render_entry_rect,
    group_type_render_entry_textured_rect,
    group_type_render_entry_rect_outline,
    group_type_render_entry_text,
    group_type_render_entry_button,
    group_type_render_entry_video
};

enum wrap_mode {
    Clamp,
    Repeat,
    Crop
};

struct sort_key {
    double Z;
};

bool LessThan(sort_key Key1, sort_key Key2) {
    return Key1.Z < Key2.Z;
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
    v3 Start;
    v3 Finish;
};

struct render_entry_triangle {
    render_group_header Header;
    game_triangle Triangle;
    color Color;
};

struct render_entry_rect {
    render_group_header Header;
    game_rect Rect;
    color Color;
};

struct render_entry_textured_rect {
    render_group_header Header;
    basis Basis;
    game_rect Rect;
    loaded_bmp* Texture;
    color Color;
    wrap_mode Mode;
    double MinTexX;
    double MinTexY;
    double MaxTexX;
    double MaxTexY;
};

struct render_entry_rect_outline {
    render_group_header Header;
    game_rect Rect;
    color Color;
};

struct render_entry_text {
    render_group_header Header;
    character* Characters;
    game_screen_position Position;
    color Color;
    int Points;
    string String;
    bool Wrapped;
};

struct render_entry_button {
    render_group_header Header;
    character* Characters;
    button* Button;
};

struct render_entry_video {
    render_group_header Header;
    game_video* Video;
    game_rect Rect;
};

struct render_group {
    int32 Width;
    int32 Height;
    float MetersToPixels;
    basis DefaultBasis;
    uint32 MaxPushBufferSize;
    uint32 PushBufferSize;
    uint32 PushBufferElementCount;
    uint8* PushBufferBase;
    bool OpenGLActive;
    bool VSyncActive;
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

    Result->DefaultBasis.X = V3(1, 0, 0);
    Result->DefaultBasis.Y = V3(0, 1, 0);
    Result->DefaultBasis.Z = V3(0, 0, 1);
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
        Assert(false);
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

        case group_type_render_entry_triangle:
        {
            return sizeof(render_entry_triangle);
        } break;

        case group_type_render_entry_rect:
        {
            return sizeof(render_entry_rect);
        } break;

        case group_type_render_entry_rect_outline:
        {
            return sizeof(render_entry_rect_outline);
        } break;

        case group_type_render_entry_text:
        {
            return sizeof(render_entry_text);
        } break;

        case group_type_render_entry_button:
        {
            return sizeof(render_entry_button);
        } break;

        case group_type_render_entry_textured_rect:
        {
            return sizeof(render_entry_textured_rect);
        } break;

        case group_type_render_entry_video:
        {
            return sizeof(render_entry_video);
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
    Entry->Color = Color;
}

void PushLine(render_group* Group, color Color, v3 Start, v3 Finish) {
    render_entry_line* Entry = PushRenderElement(Group, render_entry_line);
    Entry->Header.Key.Z = max(Start.Z, Finish.Z);
    Entry->Color = Color;
    Entry->Start = Start;
    Entry->Finish = Finish;
}

void PushTriangle(render_group* Group, game_triangle Triangle, color Color) {
    render_entry_triangle* Entry = PushRenderElement(Group, render_entry_triangle);
    Entry->Header.Key.Z = Triangle.Point0.Z;
    Entry->Color = Color;
    Entry->Triangle = Triangle;
}

void PushCircle(render_group* Group, v3 Center, double Radius, color Color) {
    int N = 0;
    if (Radius > 10.0) {
        N = 100;
    }
    else {
        N = 10;
    }
    double dTheta = Tau / (double)N;
    double Theta = 0;
    for (int i = 0; i < N; i++) {
        game_triangle Triangle = { 0 };
        Triangle.Points[0] = Center;
        Triangle.Points[1] = Center + Radius * V3(cos(Theta), sin(Theta), Center.Z);
        Theta += dTheta;
        Triangle.Points[2] = Center + Radius * V3(cos(Theta), sin(Theta), Center.Z);
        PushTriangle(Group, Triangle, Color);
    }
}

void PushRect(render_group* Group, game_rect Rect, color Color, double Z) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Z = Z;
    Entry->Rect = Rect;
    Entry->Color = Color;
}

void PushTexturedRectClamp(
    render_group* Group,
    loaded_bmp* Texture,
    game_rect Rect,
    basis Basis = Identity(1.0),
    color Color = White,
    double Z = 0.0,
    bool FlipX = false, bool FlipY = false
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Basis = normalize(Basis);
    Entry->Mode = Clamp;

    Entry->MinTexX = FlipX ? 1.0 : 0.0;
    Entry->MaxTexX = FlipX ? 0.0 : 1.0;
    Entry->MinTexY = FlipY ? 1.0 : 0.0;
    Entry->MaxTexY = FlipY ? 0.0 : 1.0;
}

void PushTexturedRectRepeat(
    render_group* Group,
    loaded_bmp* Texture,
    game_rect Rect,
    basis Basis = Identity(1.0),
    color Color = White,
    double Z = 0.0,
    bool FlipX = false, bool FlipY = false
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Basis = normalize(Basis);
    Entry->Mode = Repeat;

    double ScaleX = module(Basis.X);
    double ScaleY = module(Basis.Y);

    double MinX = 0.0;
    double MinY = -Rect.Height / (ScaleY * (double)Texture->Header.Height);
    double MaxX = Rect.Width / (ScaleX * (double)Texture->Header.Width);
    double MaxY = 1.0;
    Entry->MinTexX = FlipX ? MaxX : MinX;
    Entry->MaxTexX = FlipX ? MinX : MaxX;
    Entry->MinTexY = FlipY ? MaxY : MinY;
    Entry->MaxTexY = FlipY ? MinY : MaxY;
}

void PushTexturedRectCrop(
    render_group* Group,
    loaded_bmp* Texture,
    game_rect Rect,
    v3 Offset = V3(0,0,0),
    basis Basis = Identity(1.0),
    color Color = White,
    double Z = 0.0,
    bool FlipX = false, bool FlipY = false
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Basis = normalize(Basis);
    Entry->Mode = Crop;

    double ScaleX = module(Basis.X);
    double ScaleY = module(Basis.Y);

    double MinX = Offset.X / ScaleX / (double)Texture->Header.Width;
    double MinY = 1.0 - (Rect.Height + Offset.Y) / ScaleY / (double)Texture->Header.Height;
    double MaxX = (Rect.Width + Offset.X) / ScaleX / (double)Texture->Header.Width;
    double MaxY = 1.0 - Offset.Y / ScaleY / (double)Texture->Header.Height;
    Entry->MinTexX = FlipX ? MaxX : MinX;
    Entry->MaxTexX = FlipX ? MinX : MaxX;
    Entry->MinTexY = FlipY ? MaxY : MinY;
    Entry->MaxTexY = FlipY ? MinY : MaxY;
}

void PushRectOutline(render_group* Group, game_rect Rect, color Color) {
    render_entry_rect_outline* Entry = PushRenderElement(Group, render_entry_rect_outline);
    Entry->Header.Key.Z = 300;
    Entry->Rect = Rect;
    Entry->Color = Color;
}

void PushText(render_group* Group, game_screen_position Position, character* Characters, color Color, int Points, string String, bool Wrapped) {
    render_entry_text* Entry = PushRenderElement(Group, render_entry_text);
    Entry->Header.Key.Z = Position.Z;
    Entry->Position = Position;
    Entry->Characters = Characters;
    Entry->Color = Color;
    Entry->Points = Points;
    Entry->String = String;
    Entry->Wrapped = Wrapped;
}

void PushBone(render_group* Group, bone Bone, v3 Position, bool Debug = false) {
    if (Bone.BMP) {
        game_rect Rect = {
            Position.X + Bone.Start.X + Bone.BMPOffset.X,
            Position.Y + Bone.Start.Y + Bone.BMPOffset.Y,
            (double)Bone.BMP->Header.Width * 4.0,
            (double)Bone.BMP->Header.Height * 4.0
        };
        PushTexturedRect(Group, Bone.BMP, Rect, Clamp, White, Bone.Basis, Bone.Start.Z, Bone.FlipX, Bone.FlipY, Bone.BMPOffset);
    }
    if (Debug) {
        double Z = Bone.BMPOffset.Z + 100.0;
        PushLine(Group, White, Position + Bone.Start + V3(0,0,Z), Position + Bone.Finish + V3(0,0,10));
        PushCircle(Group, Position + Bone.Start + V3(0, 0, Z), 2.0, White);
        PushCircle(Group, Position + Bone.Finish + V3(0, 0, Z), 2.0, White);
    }
}

void PushSkeleton(render_group* Group, int N, player_bone* Bones, v3 Position, bool Debug = false) {
    for (int i = 0; i < N; i++) {
        PushBone(Group, Bones[i].Bone, Position, Debug);
    }
}

void PushButton(render_group* Group, character* Characters, button* Button) {
    render_entry_button* Entry = PushRenderElement(Group, render_entry_button);
    Entry->Header.Key.Z = 0;
    Entry->Button = Button;
    Entry->Characters = Characters;
}

void _PushVideo(render_group* Group, game_video* Video, game_rect Rect, int Z) {
    render_entry_video* Entry = PushRenderElement(Group, render_entry_video);
    
    Entry->Header.Key.Z = Z;
    Entry->Video = Video;
    Entry->Rect = Rect;
}

void PushCombatMenu(render_group* Group, character* Characters, combat_menu* Menu) {
    if (Menu->Active) {
        // Background
        game_rect MenuRect = { 0, Group->Height - 250, 200, 250 };
        PushRect(Group, MenuRect, DarkGray, 500);

        game_screen_position AttackTextPosition = { 50, Group->Height - 200, 501 };
        PushText(Group, AttackTextPosition, Characters, White, 16, Menu->AttackText, false);

        game_screen_position TechniqueTextPosition = { 25, Group->Height - 150, 501 };
        PushText(Group, TechniqueTextPosition, Characters, White, 16, Menu->TechniqueText, false);

        game_screen_position MagicTextPosition = { 60, Group->Height - 100, 501 };
        PushText(Group, MagicTextPosition, Characters, White, 16, Menu->MagicText, false);

        game_screen_position ItemsTextPosition = { 60, Group->Height - 50, 501 };
        PushText(Group, ItemsTextPosition, Characters, White, 16, Menu->ItemsText, false);

        v3 CursorPosition = V3(0, Group->Height - 198 + 50 * Menu->Cursor, 501);
        game_triangle Triangle = {
            V3(CursorPosition.X, CursorPosition.Y - 10, 502),
            V3(CursorPosition.X + 10, CursorPosition.Y, 502),
            V3(CursorPosition.X, CursorPosition.Y + 10, 502)
        };
        PushTriangle(Group, Triangle, White);
        v3 Start = { 0, Group->Height - 198 + 50 * Menu->Cursor, 502 };
        v3 Finish = { 150, Group->Height - 198 + 50 * Menu->Cursor, 502 };
        PushLine(Group, White, Start, Finish);
    }
}

void PushHealthBar(render_group* Group, game_screen_position Position, int HP, int MaxHP) {
    PushRect(Group, { Position.X, Position.Y, 100, 4 }, DarkGray, Position.Z);
    PushRect(Group, { Position.X, Position.Y, 100.0 * ((double)HP) / ((double)MaxHP), 4 }, Red, Position.Z + 1);
}

void PushTurnQueue(render_group* Group, game_assets* Assets, turn_queue* TurnQueue) {
    for (int i = 0; i < TurnQueue->Length; i++) {
        game_rect Rect = {20, 0.3*Group->Height + i*20, 60, 20};
        PushRectOutline(Group, Rect, Black);
        PushTexturedRect(Group, TurnQueue->BMPs[i], Rect, Crop, White, Identity(4.0), 10, false, false, TurnQueue->BMPOffsets[i]);
    }
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
        if (LessThan(Entries[i + 1].Key, Entries[i].Key))
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
    game_rect Rect = { 0 };
    Rect.Width = OutputTarget->Header.Width;
    Rect.Height = OutputTarget->Header.Height;

    if (!IsInside(Start, Rect) &&
        !IsInside(Finish, Rect) &&
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

//void RenderText(loaded_bmp* OutputTarget, memory_arena* Arena, FT_Face* Font, game_screen_position Position, text Text) {
//    FT_Error error;
//
//    error = FT_Set_Char_Size(*Font, 0, Text.Points * 64, 128, 128);
//    if (error) {
//        Assert(false);
//    }
//    else {
//        FT_GlyphSlot Slot = (*Font)->glyph;
//        int PenX = Position.X;
//        int PenY = Position.Y;
//
//        error = FT_Load_Char(*Font, '\n', FT_LOAD_RENDER);
//        if (error) {
//            Assert(false);
//        }
//
//        int LineJump = (int)(0.023f * (float)Slot->metrics.height); // 0.023 because height is in 64ths of pixel
//
//        for (int i = 0; i < Text.Length; i++) {
//            error = FT_Load_Char(*Font, Text.Content[i], FT_LOAD_RENDER);
//            if (error) {
//                Assert(false);
//            }
//
//            // Carriage returns
//            if (Text.Content[i] == '\n') {
//                PenY += LineJump;
//                PenX = Position.X;
//            }
//            else {
//                if (Text.Wrapped && PenX + (Slot->metrics.width >> 6) > OutputTarget->Header.Width) {
//                    PenX = Position.X;
//                    PenY += LineJump;
//                }
//                FT_Bitmap FTBMP = Slot->bitmap;
//                loaded_bmp BMP = MakeEmptyBitmap(Arena, FTBMP.width, FTBMP.rows, true);
//                LoadFTBMP(&FTBMP, &BMP);
//                RenderBMP(OutputTarget, &BMP, { (double)(PenX + Slot->bitmap_left), (double)(PenY - Slot->bitmap_top), 0 });
//                PopSize(Arena, BMP.Header.FileSize / 8);
//                PenX += Slot->advance.x >> 6;
//            }
//        }
//    }
//}

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