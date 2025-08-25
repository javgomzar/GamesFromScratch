#include "GamePlatform.h"

#ifndef GAME_BITMAP
#define GAME_BITMAP

INTROSPECT
enum game_bitmap_id {
    Bitmap_Background_ID,
    Bitmap_Button_ID,
    Bitmap_Empty_ID,
    Bitmap_Enemy_ID,
    Bitmap_Player_ID,

    game_bitmap_id_count
};

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
    uint32 Handle;
    uint32 BytesPerPixel;
    uint32 Pitch;
    uint32 AlphaMask;
    uint32* Content;
};

uint64 PreprocessBitmap(bitmap_header* Header);
game_bitmap LoadBitmapFile(memory_arena* Arena, read_file_result File);
void ClearBitmap(game_bitmap* Bitmap);
game_bitmap MakeEmptyBitmap(memory_arena* Arena, int32 Width, int32 Height, bool ClearToZero);
void SaveBMP(platform_api* Platform, const char* Path, game_bitmap* BMP);
void MakeBitmapHeader(bitmap_header* Header, int Width, int Height, int BitsPerPixel = 32);

#endif