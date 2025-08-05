#include "GameBitmap.h"

/* Returns number of bytes for pixels. Deals with 4-byte alignment for rows when pixels are 3 bytes wide. */
uint64 PreprocessBitmap(bitmap_header* Header) {
    Assert(
        Header->Size == 40 || // BITMAPINFOHEADER
        Header->Size == 124   // BITMAPV5HEADER
    );

    uint32 ExtraBytes = 0;
    if (Header->Size == 124) {
        bitmap_header_v5 v5Header = *(bitmap_header_v5*)Header;
        ExtraBytes = v5Header.ProfileSize;
    }

    uint32 BytesPerPixel = (Header->BitsPerPixel >> 3);
    uint32 RowSize = Header->Width * BytesPerPixel;

    if (Header->Size == 40 && BytesPerPixel == 3) {
        // 4-byte alignment apparently
        RowSize = (RowSize / 4 + 1) * 4;
    }

    uint64 PixelsSize = RowSize * Header->Height;
    
    Assert(PixelsSize + Header->BitmapOffset + ExtraBytes == Header->FileSize);
    return PixelsSize;
}

game_bitmap LoadBitmapFile(memory_arena* Arena, read_file_result File) {
    game_bitmap Result = {};
    bitmap_header Header = *(bitmap_header*)File.Content;
    Result.Header = Header;
    uint32 BytesPerPixel = Header.BitsPerPixel >> 3;
    Result.BytesPerPixel = BytesPerPixel;
    Result.Pitch = Header.Width * BytesPerPixel;
    Result.Content = (uint32*)((uint8*)File.Content + Header.BitmapOffset);

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
    }

    uint32 RowSize = Header.Width * BytesPerPixel;
    if (Header.Size == 40 && BytesPerPixel == 3) {
        // 4-byte alignment apparently
        RowSize = (RowSize / 4 + 1) * 4;
    }

    uint64 PixelsSize = RowSize * Header.Height;
    void* Destination = PushSize(Arena, PixelsSize);

    memcpy(Destination, Result.Content, PixelsSize);
    return Result;
}

void ClearBitmap(game_bitmap* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = Bitmap->Header.Width * Bitmap->Header.Height * Bitmap->BytesPerPixel;
        ZeroSize(TotalBitmapSize, Bitmap->Content);
    }
}

void MakeBitmapHeader(bitmap_header* Header, int Width, int Height, int BitsPerPixel) {
    Header->FileType = 19778;
    Header->Width = Width;
    Header->Height = Height;
    Header->BitmapOffset = 138;
    Header->Size = 124;
    Header->Planes = 1;
    Header->BitsPerPixel = BitsPerPixel;
    Header->FileSize = Width * Height * Header->BitsPerPixel;
    Header->Compression = 3;
    Header->SizeOfBitmap = Width * Height * 4 + Header->BitmapOffset;
    Header->HorzResolution = 3777;
    Header->VertResolution = 3777;
    Header->RedMask = 0x00ff0000;
    Header->GreenMask = 0x0000ff00;
    Header->BlueMask = 0x000000ff;
}

game_bitmap MakeEmptyBitmap(memory_arena* Arena, int32 Width, int32 Height, bool ClearToZero = true) {
    game_bitmap Result = {};

    MakeBitmapHeader(&Result.Header, Width, Height);

    Result.BytesPerPixel = 4;
    Result.Pitch = 4 * Width;
    Result.AlphaMask = 0xff000000;

    Result.Content = (uint32*)PushSize(Arena, Width * Height * Result.BytesPerPixel);
    if (ClearToZero) {
        ClearBitmap(&Result);
    }
    return Result;
}

void SaveBMP(platform_api* Platform, const char* Path, game_bitmap* BMP) {
    Platform->WriteEntireFile(Path, sizeof(BMP->Header), &BMP->Header);
    uint32 Offset = BMP->Header.BitmapOffset - sizeof(BMP->Header);
    char Zero = 0;
    for (uint32 i = 0; i < Offset; i++) {
        Platform->AppendToFile(Path, 1, &Zero);
    }
    Platform->AppendToFile(Path, BMP->Header.Width * BMP->Header.Height * BMP->BytesPerPixel, BMP->Content);
}

uint32* GetPixelAddress(game_bitmap* BMP, int X, int Y) {
    Assert(X >= 0 && X <= BMP->Header.Width);
    Assert(Y >= 0 && Y <= BMP->Header.Height);
    return BMP->Content + BMP->Header.Width * (BMP->Header.Height - Y) + X;
}

uint32 GetPixel(game_bitmap* BMP, int X, int Y) {
    Assert(X >= 0 && X <= BMP->Header.Width);
    Assert(Y >= 0 && Y <= BMP->Header.Height);
    return *GetPixelAddress(BMP, X, Y);
}

void SetPixel(game_bitmap* BMP, int X, int Y, uint32 Value) {
    Assert(X >= 0 && X <= BMP->Header.Width);
    Assert(Y >= 0 && Y <= BMP->Header.Height);
    uint32* PixelAddress = GetPixelAddress(BMP, X, Y);
    *PixelAddress = Value;
}