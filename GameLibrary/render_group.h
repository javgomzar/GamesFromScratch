

struct render_basis {
	v3 P;
};

enum render_group_entry_type {
	group_type_render_entry_clear,
	group_type_render_entry_rect,
	group_type_render_entry_bmp,
    group_type_render_entry_text,
    group_type_render_entry_button
};

struct render_group_header {
	render_group_entry_type Type;
	uint32 Key;
	uint32 PushBufferOffset;
};

struct render_entry_clear {
	render_group_header Header;
	color Color;
};

struct render_entry_rect {
	render_group_header Header;
	game_rect Rect;
	color Color;
};

struct render_entry_bmp {
	render_group_header Header;
	loaded_bmp* Bitmap;
	game_screen_position Position;
};

struct render_entry_text {
    render_group_header Header;
    memory_arena* Arena;
    FT_Face* Font;
    game_screen_position Position;
    text Text;
};

struct render_entry_button {
    render_group_header Header;
    memory_arena* Arena;
    button Button;
};

struct render_group {
	float MetersToPixels;
	render_basis* DefaultBasis;

	uint32 MaxPushBufferSize;
	uint32 PushBufferSize;
	uint32 PushBufferElementCount;
	uint8* PushBufferBase;
};


render_group* AllocateRenderGroup(memory_arena* Arena, memory_index MaxPushBufferSize) {
    render_group* Result = PushStruct(Arena, render_group);
    Result->PushBufferBase = (uint8*)PushSize(Arena, MaxPushBufferSize);
    Result->MaxPushBufferSize = MaxPushBufferSize;
    Result->PushBufferSize = 0;

    Result->DefaultBasis = PushStruct(Arena, render_basis);
    Result->DefaultBasis->P = V3(0, 0, 0);
    Result->MetersToPixels = 1;

    return(Result);
}


#define PushRenderElement(Group, type) (type*)PushRenderElement_(Group, sizeof(type), group_type_##type)
render_group_header* PushRenderElement_(render_group* Group, uint32 Size, render_group_entry_type Type) {
    render_group_header* Result = 0;

    if ((Group->PushBufferSize + Size) < Group->MaxPushBufferSize) {
        Result = (render_group_header*)(Group->PushBufferBase + Group->PushBufferSize);
        Result->Key = 0; // Must be set when pushed
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

void PushClear(render_group* Group, color Color) {
    render_entry_clear* Entry = PushRenderElement(Group, render_entry_clear);
    Entry->Header.Key = 0;
    Entry->Color = Color;
}

void PushRect(render_group* Group, game_rect Rect, color Color) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key = 0;
    Entry->Rect = Rect;
    Entry->Color = Color;
}

void PushBMP(render_group* Group, loaded_bmp* Bitmap, game_screen_position Position) {
    render_entry_bmp* Entry = PushRenderElement(Group, render_entry_bmp);
    Entry->Header.Key = Position.Z;
    Entry->Bitmap = Bitmap;
    Entry->Position = Position;
}

void PushText(render_group* Group, memory_arena* Arena, FT_Face* Font, game_screen_position Position, text Text) {
    render_entry_text* Entry = PushRenderElement(Group, render_entry_text);
    Entry->Header.Key = Position.Z;
    Entry->Arena = Arena;
    Entry->Font = Font;
    Entry->Position = Position;
    Entry->Text = Text;
}

void PushButton(render_group* Group, memory_arena* Arena, button Button) {
    render_entry_button* Entry = PushRenderElement(Group, render_entry_button);
    Entry->Arena = Arena;
    Entry->Button = Button;
}

void ClearEntries(render_group* Group) {
    Group->PushBufferElementCount = 0;
    Group->PushBufferSize = 0;
}

void SortEntries(render_group* RenderGroup) {
    uint32 Count = RenderGroup->PushBufferElementCount;
    for (uint32 i = 0; i < Count - 1; i++) {
        // TODO
    }
}

void SwapEntries(render_group* RenderGroup, uint32 Entry1, uint32 Entry2) {

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
        uint32* SourceRow = BMP->Content + BMP->Header.Width * (BMP->Header.Height - SourcePosition.Y - 1) + SourcePosition.X;
        uint8* DestinationRow = (uint8*)(OutputTarget->Content + DestinationPosition.X) + DestinationPosition.Y * OutputTarget->Pitch;
        for (int32 Y = 0; Y < BlitHeight; Y++) {
            uint32* Destination = (uint32*)DestinationRow;
            uint32* Source = SourceRow;
            for (int32 X = 0; X < BlitWidth; X++) {
                color BMPColor = GetColor(*Source++, BMP->Header.RedMask, BMP->Header.GreenMask, BMP->Header.BlueMask);
                color BackgroundColor = GetColor(*Destination, 0x00ff0000, 0x0000ff00, 0x000000ff);

                if (BMPColor.R != BackgroundColor.R || BMPColor.G != BackgroundColor.G || BMPColor.B != BackgroundColor.B) {
                    *Destination++ = GetColorBytes(BMP->HasAlpha ? Blend(BMPColor, BackgroundColor) : BMPColor);
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
    Result.HasAlpha = true;

    Result.Content = (uint32*)PushSize(Arena, TotalBitmapSize / 8);
    if (ClearToZero) {
        ClearBitmap(&Result);
    }
    return Result;
}


void LoadFTBMP(FT_Bitmap* SourceBMP, loaded_bmp* DestBMP, color Color) {
    uint32* DestRow = DestBMP->Content + DestBMP->Header.Width * (DestBMP->Header.Height - 1);
    uint8* Source = SourceBMP->buffer;
    for (int Y = 0; Y < SourceBMP->rows; Y++) {
        uint32* Pixel = DestRow;
        for (int X = 0; X < SourceBMP->width; X++) {
            color OutColor = { *Source++, Color.R, Color.G, Color.B };
            uint32 Out = GetColorBytes(OutColor);
            *Pixel++ = Out;
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
                LoadFTBMP(&FTBMP, &BMP, Text.Color);
                RenderBMP(OutputTarget, &BMP, { PenX + Slot->bitmap_left, PenY - Slot->bitmap_top, 0 });
                PopSize(Arena, BMP.Header.FileSize / 8);
                PenX += Slot->advance.x >> 6;
            }
        }
    }
}


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
                button Button = Entry->Button;
                RenderBMP(OutputTarget, Button.Clicked ? &Button.ClickedImage : &Button.Image, { Button.Collider.Left, Button.Collider.Top, 0 });
                RenderText(OutputTarget, Entry->Arena, Button.Face, { 
                    Button.Collider.Left + Button.Image.Header.Width / 2,
                    Button.Collider.Top + Button.Image.Header.Height / 2,
                    0 }, Button.Text);

                BaseAddress += sizeof(*Entry);
            } break;
            default: {
                // Invalid code path
                Assert(false);
            } break;
        }
    }
}