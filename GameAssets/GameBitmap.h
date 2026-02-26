#include "GamePlatform.h"

#ifndef GAME_BITMAP
#define GAME_BITMAP

ENUM(game_bitmap_id,
    Bitmap_Empty_ID,
    Bitmap_Background_ID,
    Bitmap_Button_ID,
    Bitmap_Enemy_ID,
    Bitmap_Player_ID,
    Bitmap_Spain_ID
);

#pragma pack(push, 1)
struct bitmap_header {
    uint16 FileType;
    uint32 FileSize;
    uint16 Reserved1;
    uint16 Reserved2;
    uint32 BitmapOffset;
    uint32 Size;
    int32 Width;
    int32 Height;
    uint16 Planes;
    uint16 BitsPerPixel;
    uint32 Compression;
    uint32 SizeOfBitmap;
    int32 HorzResolution;
    int32 VertResolution;
    uint32 ColorUser;
    uint32 ColorsImportant;
    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
};

struct bitmap_header_v5 {
    uint16 FileType;
    uint32 FileSize;
    uint16 Reserved1;
    uint16 Reserved2;
    uint32 BitmapOffset;
    uint32 Size;
    int32 Width;
    int32 Height;
    uint16 Planes;
    uint16 BitsPerPixel;
    uint32 Compression;
    uint32 SizeOfBitmap;
    int32 HorzResolution;
    int32 VertResolution;
    uint32 ColorUser;
    uint32 ColorsImportant;
    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
    uint32 AlphaMask;
    uint32 CSType;
    long RedX;          /* X coordinate of red endpoint */
    long RedY;          /* Y coordinate of red endpoint */
    long RedZ;          /* Z coordinate of red endpoint */
    long GreenX;        /* X coordinate of green endpoint */
    long GreenY;        /* Y coordinate of green endpoint */
    long GreenZ;        /* Z coordinate of green endpoint */
    long BlueX;         /* X coordinate of blue endpoint */
    long BlueY;         /* Y coordinate of blue endpoint */
    long BlueZ;         /* Z coordinate of blue endpoint */
    uint32 GammaRed;
    uint32 GammaGreen;
    uint32 GammaBlue;
    uint32 Intent;
    uint32 ProfileData;
    uint32 ProfileSize;
    uint32 Reserved;
};
#pragma pack(pop)

struct game_bitmap {
    game_bitmap_id ID;
    bitmap_header Header;
    uint32 Pitch;
    uint32 AlphaMask;
    uint32* Content;
};

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

    return 4 * Header->Width * Header->Height;
}

game_bitmap LoadBitmapFile(memory_arena* Arena, void* FileContent) {
    game_bitmap Result = {};
    bitmap_header Header = *(bitmap_header*)FileContent;
    Result.Header = Header;
    uint32 BytesPerPixel = Header.BitsPerPixel >> 3;
    Result.Pitch = Header.Width * BytesPerPixel;
    Result.Content = (uint32*)((uint8*)FileContent + Header.BitmapOffset);

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
        // 4-byte alignment
        RowSize = (RowSize / 4 + 1) * 4;
    }

    uint64 PixelsSize = 4 * Header.Width * Header.Height;
    uint32* Destination = (uint32*)PushSize(Arena, PixelsSize);

    uint8* Source = (uint8*)Result.Content;
    for (int Row = 0; Row < Header.Height; Row++) {
        uint32 BytesRead = 0;
        for (int Col = 0; Col < Header.Width; Col++) {
            uint8 R = *Source++;
            uint8 G = *Source++;
            uint8 B = *Source++;

            uint8 A = 255;
            if (BytesPerPixel == 4) {
                A = *Source++;
            }
            uint32 Pixel = (A << 24) | (R << 16) | (G << 8) | B;
            *Destination++ = Pixel;

            BytesRead += BytesPerPixel;
        }

        if (BytesRead < RowSize) {
            Source += RowSize - BytesRead;
        }
    }

    return Result;
}

void ClearBitmap(game_bitmap* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = 4 * Bitmap->Header.Width * Bitmap->Header.Height;
        ZeroSize(TotalBitmapSize, Bitmap->Content);
    }
}

void MakeBitmapHeader(bitmap_header* Header, int Width, int Height) {
    Header->FileType = 19778;
    Header->Width = Width;
    Header->Height = Height;
    Header->BitmapOffset = 138;
    Header->Size = 124;
    Header->Planes = 1;
    Header->BitsPerPixel = 32;
    Header->FileSize = Width * Height * Header->BitsPerPixel;
    Header->Compression = 3;
    Header->SizeOfBitmap = Width * Height * 4 + Header->BitmapOffset;
    Header->HorzResolution = 3777;
    Header->VertResolution = 3777;
    Header->RedMask = 0x00ff0000;
    Header->GreenMask = 0x0000ff00;
    Header->BlueMask = 0x000000ff;
}

game_bitmap MakeEmptyBitmap(
    memory_arena* Arena,
    int32 Width, int32 Height,
    int BytesPerPixel,
    bool ClearToZero = true
) {
    game_bitmap Result = {};

    MakeBitmapHeader(&Result.Header, Width, Height);

    Result.Pitch = BytesPerPixel * Width;
    Result.AlphaMask = 0xff000000;

    Result.Content = (uint32*)PushSize(Arena, 4 * Width * Height);
    if (ClearToZero) {
        ClearBitmap(&Result);
    }
    return Result;
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

#endif