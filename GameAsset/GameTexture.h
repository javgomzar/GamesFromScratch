#ifndef GAME_TEXTURE
#define GAME_TEXTURE

#include "GameAsset.h"

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

INTROSPECT
struct Texture {
    Asset_ID id;
    int32 width;
    int32 height;
    int32 channels;
    int32 size;
    uint8* pixels;
};

uint64 GetPixelsSize(int32 width, int32 height, int32 channels) {
    return (uint64)width * (uint64)height * (uint64)channels;
}

// game_texture ReadTexture(file_format Format, void* FileContent) {
//     game_texture Result = {};

//     if (Format == file_format::BMP) {
//         bitmap_header* Header = (bitmap_header*)FileContent;
//         Result.BytesPerPixel = Header->BitsPerPixel >> 3;
//         Result.Width = Header->Width;
//         Result.Height = Header->Height;
//         Result.RedMask = Header->RedMask;
//         Result.GreenMask = Header->GreenMask;
//         Result.BlueMask = Header->BlueMask;
//         Result.PixelsSize = GetPixelsSize(Header);
//         Result.Content = (uint32*)((uint8*)FileContent + Header->BitmapOffset);

//         Result.RowSize = Header->Width * Result.BytesPerPixel;
//         if (Header->Size == 40 && Result.BytesPerPixel == 3) {
//             // 4-byte alignment
//             Result.RowSize = (Result.RowSize / 4 + 1) * 4;
//         }

//         bool HasAlpha = false;
//         if (Result.BytesPerPixel == 4 && Header->Compression == 3) {
//             uint32 AlphaMask = ~(Header->RedMask | Header->GreenMask | Header->BlueMask);
//             // If not all Alphas are zero, we need to use them
//             uint32* Contents = (uint32*)((uint8*)FileContent + Header->BitmapOffset);
//             for (int32 i = 0; i < Header->Height * Header->Width; i++) {
//                 if ((*Contents++ & AlphaMask) > 0) {
//                     HasAlpha = true;
//                     break;
//                 }
//             }

//             // If all alphas are zero, turn them to one
//             Contents = Result.Content;
//             if (!HasAlpha) {
//                 for (int32 j = 0; j < Header->Height * Header->Width; j++) {
//                     *Contents = AlphaMask | (*Contents++ & ~AlphaMask);
//                 }
//             }
//         }
//     }
//     else if (Format == file_format::JPEG) {
//         Raise("JPEG file loading not implemented.");
//     }
//     else if (Format == file_format::PNG) {
//         Raise("PNG file loading not implemented.");
//     }
//     else {
//         Raise("Invalid texture format.");
//     }

//     return Result;
// }

// void WriteTexture(Arena* Arena, game_texture* Texture) {    
//     uint32* Destination = (uint32*)PushSize(Arena, Texture->PixelsSize);

//     uint8* Source = (uint8*)Texture->Content;
//     for (int Row = 0; Row < Texture->Height; Row++) {
//         uint32 BytesRead = 0;
//         for (int Col = 0; Col < Texture->Width; Col++) {
//             uint8 R = *Source++;
//             uint8 G = *Source++;
//             uint8 B = *Source++;

//             uint8 A = 255;
//             if (Texture->BytesPerPixel == 4) {
//                 A = *Source++;
//             }
//             uint32 Pixel = (A << 24) | (R << 16) | (G << 8) | B;
//             *Destination++ = Pixel;

//             BytesRead += Texture->BytesPerPixel;
//         }

//         if (BytesRead < Texture->RowSize) {
//             Source += Texture->RowSize - BytesRead;
//         }
//     }
// }

// void ClearBitmap(game_texture* Bitmap) {
//     if (Bitmap->Content) {
//         int32 TotalBitmapSize = 4 * Bitmap->Width * Bitmap->Height;
//         ZeroSize(TotalBitmapSize, Bitmap->Content);
//     }
// }

// bitmap_header MakeBitmapHeader(int Width, int Height) {
//     bitmap_header Header = {};
//     Header.FileType = 19778;
//     Header.Width = Width;
//     Header.Height = Height;
//     Header.BitmapOffset = 138;
//     Header.Size = 124;
//     Header.Planes = 1;
//     Header.BitsPerPixel = 32;
//     Header.FileSize = Width * Height * Header.BitsPerPixel;
//     Header.Compression = 3;
//     Header.SizeOfBitmap = Width * Height * 4 + Header.BitmapOffset;
//     Header.HorzResolution = 3777;
//     Header.VertResolution = 3777;
//     Header.RedMask = 0x00ff0000;
//     Header.GreenMask = 0x0000ff00;
//     Header.BlueMask = 0x000000ff;
//     return Header;
// }

// game_texture MakeEmptyBitmap(
//     Arena* Arena,
//     int32 Width, int32 Height,
//     int BytesPerPixel,
//     bool ClearToZero = true
// ) {
//     game_texture Result = {};
//     Result.Width = Width;
//     Result.Height = Height;
//     Result.BytesPerPixel = 4;
//     Result.RedMask = 0x00ff0000;
//     Result.GreenMask = 0x0000ff00;
//     Result.BlueMask = 0x000000ff;
//     Result.AlphaMask = 0xff000000;

//     Result.Content = (uint32*)PushSize(Arena, 4 * Width * Height);
//     if (ClearToZero) {
//         ClearBitmap(&Result);
//     }
//     return Result;
// }

// uint32* GetPixelAddress(game_texture* BMP, int X, int Y) {
//     Assert(X >= 0 && X <= BMP->Width);
//     Assert(Y >= 0 && Y <= BMP->Height);
//     return BMP->Content + BMP->Width * (BMP->Height - Y) + X;
// }

// uint32 GetPixel(game_texture* BMP, int X, int Y) {
//     Assert(X >= 0 && X <= BMP->Width);
//     Assert(Y >= 0 && Y <= BMP->Height);
//     return *GetPixelAddress(BMP, X, Y);
// }

// void SetPixel(game_texture* BMP, int X, int Y, uint32 Value) {
//     Assert(X >= 0 && X <= BMP->Width);
//     Assert(Y >= 0 && Y <= BMP->Height);
//     uint32* PixelAddress = GetPixelAddress(BMP, X, Y);
//     *PixelAddress = Value;
// }

// // Heightmaps

// typedef game_texture game_heightmap;

const int HEIGHTMAP_RESOLUTION = 16;

// uint64 ComputeNeededMemoryForHeightmap(void* FileContent) {
//     uint64 BitmapSize = GetPixelsSize((bitmap_header*)FileContent);
//     return BitmapSize;
// }

// game_heightmap ReadHeightmap(file_format Format, void* FileContent) {
//     game_heightmap Result = ReadTexture(Format, FileContent);
//     return Result;
// }

#endif