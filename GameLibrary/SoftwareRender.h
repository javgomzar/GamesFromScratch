#pragma once

#include "RenderGroup.h"

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

bool IsInside(v3 Position, game_rect Rect) {
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

void RenderLine(loaded_bmp* OutputTarget, color Color, v3 Start, v3 Finish) {
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


void RenderBMP(loaded_bmp* OutputTarget, loaded_bmp* BMP, v3 Position) {
    int32 BMPWidth = BMP->Header.Width;
    int32 BMPHeight = BMP->Header.Height;

    int32 TargetWidth = (int32)OutputTarget->Header.Width;
    int32 TargetHeight = (int32)OutputTarget->Header.Height;

    if (Position.X + BMPWidth > 0 && Position.X < TargetWidth &&
        Position.Y + BMPHeight > 0 && Position.Y < TargetHeight) {
        int32 BlitWidth;
        int32 BlitHeight;
        v3 SourcePosition;
        v3 DestinationPosition;

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
                    //*Destination++ = GetColorBytes(Mix(BMPColor, BackgroundColor));
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