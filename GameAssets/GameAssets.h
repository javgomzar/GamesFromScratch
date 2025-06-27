// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H

// FFMPEG
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavformat/avio.h"
}

#include "GameMath.h"
#include "Tokenizer.h"
#include "Win32PlatformLayer.h"

#ifndef GAME_ASSETS
#define GAME_ASSETS

/*
    TODO:
        - Fix video. Possibly extract all frames and load them to the asset file.
        - Asset hot reloading
*/

enum game_asset_type {
    Text,
    Bitmap,
    Heightmap,
    Font,
    Sound,
    Video,
    Mesh,
    Animation,

    game_asset_type_count
};

enum game_text_id {
    Text_Test_ID,

    game_text_id_count
};

enum game_sound_id {
    Sound_Test_ID,

    game_sound_id_count
};

enum game_bitmap_id {
    Bitmap_Background_ID,
    Bitmap_Button_ID,
    Bitmap_Empty_ID,
    Bitmap_Enemy_ID,
    Bitmap_Player_ID,

    game_bitmap_id_count
};

enum game_heightmap_id {
    Heightmap_Spain_ID,

    game_heightmap_id_count
};

enum game_font_id {
    Font_Cascadia_Mono_ID,
    Font_Menlo_Regular_ID,

    game_font_id_count
};

enum game_mesh_id {
    Mesh_Enemy_ID,
    Mesh_Sphere_ID,
    Mesh_Body_ID,
    Mesh_Shield_ID,
    Mesh_Sword_ID,
    Mesh_Selector_ID,
    
    game_mesh_id_count
};

enum game_animation_id {
    Animation_Idle_ID,
    Animation_Walk_ID,
    Animation_Jump_ID,
    Animation_Attack_ID,

    game_animation_id_count
};

enum game_video_id {
    //Video_Test_ID,

    game_video_id_count
};

union game_asset_id {
    game_text_id Text;
    game_sound_id Sound;
    game_bitmap_id Bitmap;
    game_heightmap_id Heightmap;
    game_font_id Font;
    game_mesh_id Mesh;
    game_animation_id Animation;
    game_video_id Video;
};

struct game_asset {
    game_asset_id ID;
    game_asset_type Type;
    read_file_result File;
    uint64 MemoryNeeded;
    uint64 Offset;
};

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Text                                                                                                                                                             |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_text {
    game_text_id ID;
    uint32 Size;
    char* Content;
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Color                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct color {
    float R;
    float G;
    float B;
    float Alpha;
};

color Color(float R, float G, float B, float Alpha = 1.0f) {
    return { R, G, B, Alpha };
}

color Color(color Color, float Alpha) {
    return { Color.R, Color.G, Color.B, Alpha };
}

color operator*(float Luminosity, color Color) {
    return {
        Clamp(Luminosity * Color.R, 0.0f, 1.0f),
        Clamp(Luminosity * Color.G, 0.0f, 1.0f),
        Clamp(Luminosity * Color.B, 0.0f, 1.0f),
        Color.Alpha
    };
}

static color Black = { 0.0f, 0.0f, 0.0f, 1.0f };
static color White = { 1.0f, 1.0f, 1.0f, 1.0f };
static color Gray = { 0.5f, 0.5f, 0.5f, 1.0f };
static color DarkGray = { 0.1f, 0.1f, 0.1f, 1.0f };
static color Red = { 1.0f, 0.0f, 0.0f, 1.0f };
static color Green = { 0.0f, 1.0f, 0.0f, 1.0f };
static color Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
static color Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
static color Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
static color Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
static color Orange = { 1.0f, 0.63f, 0.0f, 1.0f };
static color BackgroundBlue = { 0.4f, 0.4f, 0.8f, 1.0f };

uint32 GetColorBytes(color Color) {
    uint8 Alpha = Color.Alpha * 255.0f;
    uint8 R = Color.R * 255.0f;
    uint8 G = Color.G * 255.0f;
    uint8 B = Color.B * 255.0f;
    return (Alpha << 24) | (R << 16) | (G << 8) | B;
}

//color GetColor(uint32 Bytes, uint32 RedMask, uint32 GreenMask, uint32 BlueMask) {
//    uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
//    
//    uint32 RedShift;
//    uint32 GreenShift;
//    uint32 BlueShift;
//    uint32 AlphaShift;
//    _BitScanForward((DWORD*)&RedShift, RedMask);
//    _BitScanForward((DWORD*)&GreenShift, GreenMask);
//    _BitScanForward((DWORD*)&BlueShift, BlueMask);
//    _BitScanForward((DWORD*)&AlphaShift, AlphaMask);
//
//    color Color;
//    Color.R = (double)((RedMask & Bytes) >> RedShift) / 255.0;
//    Color.G = (double)((GreenMask & Bytes) >> GreenShift) / 255.0;
//    Color.B = (double)((BlueMask & Bytes) >> BlueShift) / 255.0;
//    Color.Alpha = (double)((AlphaMask & Bytes) >> AlphaShift) / 255.0;
//    return Color;
//}

color operator+(color Color1, color Color2) {
    return Color(
        Color1.R + Color2.R,
        Color1.G + Color2.G,
        Color1.B + Color2.B
    );
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Sound                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct waveformat {
    unsigned short    wFormatTag;        /* format type */
    unsigned short    nChannels;         /* number of channels (i.e. mono, stereo...) */
    unsigned long     nSamplesPerSec;    /* sample rate */
    unsigned long     nAvgBytesPerSec;   /* for buffer estimation */
    unsigned short    nBlockAlign;       /* block size of data */
    unsigned short    wBitsPerSample;    /* Number of bits per sample of mono data */
    unsigned short    cbSize;            /* The count in bytes of the size of extra information (after cbSize) */
};

struct game_sound {
    game_sound_id ID;
    waveformat WaveFormat;
    uint32 SampleCount;
    uint32 Played;
    int16* SampleOut;
};

uint64 ComputeNeededMemoryForSound(read_file_result File) {
    unsigned long* Pointer = (unsigned long*)File.Content;

    unsigned long ChunkType = *Pointer++;
    if (ChunkType != 'FFIR') {
        Assert(false);
    }

    unsigned long RIFFChunkSize = *Pointer++;
    unsigned long FileType = *Pointer++;
    if (FileType != 'EVAW') {
        Assert(false);
    }

    ChunkType = *Pointer++;
    if (ChunkType != ' tmf') {
        Assert(false);
    }
    unsigned long ChunkSize = *Pointer++;
    waveformat WaveFMT = *(waveformat*)Pointer;

    Pointer += 4;
    ChunkType = *Pointer++;
    if (ChunkType != 'atad') {
        Assert(false);
    }
    ChunkSize = *Pointer++;

    return ChunkSize;
}

game_sound LoadSound(memory_arena* Arena, game_asset* Asset) {
    unsigned long* Pointer = (unsigned long*)Asset->File.Content;

    unsigned long ChunkType = *Pointer++;
    if (ChunkType != 'FFIR') {
        Assert(false);
    }

    unsigned long RIFFChunkSize = *Pointer++;
    unsigned long FileType = *Pointer++;
    if (FileType != 'EVAW') {
        Assert(false);
    }

    ChunkType = *Pointer++;
    if (ChunkType != ' tmf') {
        Assert(false);
    }
    unsigned long ChunkSize = *Pointer++;
    waveformat WaveFMT = *(waveformat*)Pointer;

    Pointer += 4;
    ChunkType = *Pointer++;
    if (ChunkType != 'atad') {
        Assert(false);
    }
    ChunkSize = *Pointer++;

    game_sound Result = {};
    Result.ID = Asset->ID.Sound;
    Result.SampleOut = (int16*)PushSize(Arena, Asset->MemoryNeeded);
    Result.SampleCount = ChunkSize / 2;
    Result.WaveFormat = WaveFMT;

    memcpy(Result.SampleOut, Pointer, Asset->MemoryNeeded);

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Bitmaps                                                                                                                                                          |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

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
    CIEXYZTRIPLE Endpoints;
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

/* Returns number of bytes of pixels. Deals with 4-byte alignment for rows when pixels are 3 bytes wide. */
uint64 ComputeNeededMemoryForBitmap(bitmap_header* Header) {
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

game_bitmap LoadBitmap(memory_arena* Arena, game_asset* Asset) {
    game_bitmap Result = {};
    Result.ID = Asset->ID.Bitmap;
    Result.Handle = 0;

    Result = LoadBitmapFile(Arena, Asset->File);

    return Result;
}

void ClearBitmap(game_bitmap* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = Bitmap->Header.Width * Bitmap->Header.Height * Bitmap->BytesPerPixel;
        ZeroSize(TotalBitmapSize, Bitmap->Content);
    }
}

void MakeBitmapHeader(bitmap_header* Header, int Width, int Height, int BitsPerPixel = 32) {
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

void SaveBMP(const char* Path, game_bitmap* BMP) {
    PlatformWriteEntireFile(Path, sizeof(BMP->Header), &BMP->Header);
    uint32 Offset = BMP->Header.BitmapOffset - sizeof(BMP->Header);
    char Zero = 0;
    for (uint32 i = 0; i < Offset; i++) {
        PlatformAppendToFile(Path, 1, &Zero);
    }
    PlatformAppendToFile(Path, BMP->Header.Width * BMP->Header.Height * BMP->BytesPerPixel, BMP->Content);
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

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Heightmaps                                                                                                                                                       |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_heightmap {
    game_bitmap Bitmap;
    uint32 nVertices;
    double* Vertices;
};

const int HEIGHTMAP_RESOLUTION = 20;

uint64 ComputeNeededMemoryForHeightmap(read_file_result File) {
    uint64 BitmapSize = ComputeNeededMemoryForBitmap((bitmap_header*)File.Content);
    uint64 VerticesSize = HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION * 4 * 5 * sizeof(double);
    return BitmapSize + VerticesSize;
}

game_heightmap LoadHeightmap(memory_arena* Arena, game_asset* Asset) {
    game_heightmap Result = {};

    Result.Bitmap = LoadBitmapFile(Arena, Asset->File);
    double Width = 10.0;
    double Height = 10.0;

    Result.nVertices = HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION * 4;
    Result.Vertices = (double*)PushSize(Arena, Result.nVertices * 5 * sizeof(double));
    double* Pointer = Result.Vertices;
    for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
        for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
            *Pointer++ = Width * (double)i / (double)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0; // v.y
            *Pointer++ = Height * (double)j / (double)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (double)i / (double)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (double)j / (double)HEIGHTMAP_RESOLUTION; // vt.y

            *Pointer++ = Width * (double)(i + 1) / (double)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0; // v.y
            *Pointer++ = Height * (double)j / (double)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (double)(i + 1) / (double)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (double)j / (double)HEIGHTMAP_RESOLUTION; // vt.y

            *Pointer++ = Width * (double)i / (double)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0; // v.y
            *Pointer++ = Height * (double)(j + 1) / (double)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (double)i / (double)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (double)(j + 1) / (double)HEIGHTMAP_RESOLUTION; // vt.y

            *Pointer++ = Width * (double)(i + 1) / (double)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0; // v.y
            *Pointer++ = Height * (double)(j + 1) / (double)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (double)(i + 1) / (double)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (double)(j + 1) / (double)HEIGHTMAP_RESOLUTION; // vt.y
        }
    }

    return Result;
}


// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Fonts                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const int FONT_CHARACTERS_COUNT = 94;
const int LOAD_POINTS = 20;

struct game_font_character {
    unsigned char Letter;
    signed long Advance;
    signed long Width;
    signed long Height;
    int Left;
    int Top;
    int AtlasX;
    int AtlasY;
};

struct game_font {
    game_font_id ID;
    signed long SpaceAdvance;
    signed long LineJump;
    game_font_character Characters[FONT_CHARACTERS_COUNT];
    game_bitmap Bitmap;
};

void LoadFTBMP(FT_Bitmap* SourceBMP, game_bitmap* DestBMP) {
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

void GetFontBMPWidthAndHeight(FT_Face Font, uint32* Width, uint32* Height) {
    uint32 ResultWidth = 0, ResultHeight = 0, RowWidth = 0, MaxHeight = 0;
    FT_Error FTError = FT_Set_Char_Size(Font, 0, LOAD_POINTS*64, 128, 128);
    if (FTError) Assert(false);

    char Starts[3] = {'!', 'A', 'a'};
    char Ends[3] = {'@', '`', '~'};

    for (int i = 0; i < 3; i++) {
        for (unsigned char c = Starts[i]; c <= Ends[i]; c++) {
            FTError = FT_Load_Char(Font, c, FT_LOAD_RENDER);
            if (FTError) Assert(false);

            FT_GlyphSlot Slot = Font->glyph;
            FT_Bitmap FTBMP = Slot->bitmap;

            RowWidth += FTBMP.width;
            if (MaxHeight < FTBMP.rows) MaxHeight = FTBMP.rows;
        }

        if (RowWidth > ResultWidth) ResultWidth = RowWidth;
        RowWidth = 0;

        ResultHeight += MaxHeight;
        MaxHeight = 0;
    }

    *Width = ResultWidth;
    *Height = ResultHeight;
}

uint64 ComputeNeededMemoryForFont(const char* Path) {
    FT_Library FTLibrary;
    FT_Face Font;
    FT_Error error = FT_Init_FreeType(&FTLibrary);
    if (error) Assert(false);

    error = FT_New_Face(FTLibrary, Path, 0, &Font);
    if (error == FT_Err_Unknown_File_Format) Raise("Freetype error: Unknown file format.");
    else if (error) Assert(false);
    
    uint32 Width = 0, Height = 0;
    GetFontBMPWidthAndHeight(Font, &Width, &Height);

    FT_Done_Face(Font);
    FT_Done_FreeType(FTLibrary);

    return 4 * Width * Height;
}

game_font LoadFont(memory_arena* Arena, game_asset* Asset) {
    game_font Result = {};
    Result.ID = Asset->ID.Font;

    FT_Library FTLibrary;
    FT_Face Font;
    FT_Error error = FT_Init_FreeType(&FTLibrary);
    if (error) Assert(false);

    error = FT_New_Face(FTLibrary, Asset->File.Path, 0, &Font);
    if (error == FT_Err_Unknown_File_Format) Raise("Freetype error: Unknown file format.");
    else if (error) Assert(false);
        
    // Initializing char bitmaps
    error = FT_Set_Char_Size(Font, 0, LOAD_POINTS * 64, 128, 128);
    if (error) Assert(false);

    error = FT_Load_Char(Font, ' ', FT_LOAD_RENDER);
    if (error) Assert(false);
    Result.SpaceAdvance = Font->glyph->advance.x >> 6;

    uint32 Width = 0, Height = 0;
    GetFontBMPWidthAndHeight(Font, &Width, &Height);

    Result.Bitmap = MakeEmptyBitmap(Arena, Width, Height, true);

    uint32* Buffer = new uint32[3686400];
    memory_arena BufferArena = MemoryArena(3686400, (uint8*)Buffer);

    unsigned char c = '!';
    int X = 0;
    int Y = 0;
    int MaxHeight = 0;
    for (int i = 0; i < FONT_CHARACTERS_COUNT; i++) {
        game_font_character* pCharacter = &Result.Characters[i];
        error = FT_Load_Char(Font, c, FT_LOAD_RENDER);
        if (error) Assert(false);
        
        FT_GlyphSlot Slot = Font->glyph;
        FT_Bitmap FTBMP = Slot->bitmap;
        game_bitmap Test = MakeEmptyBitmap(&BufferArena, FTBMP.width, FTBMP.rows, true);
        LoadFTBMP(&FTBMP, &Test);

        pCharacter->Letter = c;
        pCharacter->Advance = Slot->advance.x >> 6;
        pCharacter->Left = Slot->bitmap_left;
        pCharacter->Top = Slot->bitmap_top;
        pCharacter->Height = FTBMP.rows;
        pCharacter->Width = FTBMP.width;

        for (int Row = 0; Row <= FTBMP.rows; Row++) {
            for (int Col = 0; Col <= FTBMP.width; Col++) {
                uint32* PixelAddress = GetPixelAddress(&Result.Bitmap, X + Col, Y + Row);
                *PixelAddress = GetPixel(&Test, Col, Row);
            }
        }

        pCharacter->AtlasX = X;
        pCharacter->AtlasY = Y;

        PopSize(&BufferArena, FTBMP.width * FTBMP.rows * 4);

        if (FTBMP.rows > MaxHeight) MaxHeight = FTBMP.rows;
        X += FTBMP.width;
        if (c == '@' || c == '`') {
            X = 0;
            Y += MaxHeight;
            MaxHeight = 0;
        }

        c++;
    }

    Result.LineJump = Result.Characters[0].Height * 3 / 2;

    FT_Done_Face(Font);
    FT_Done_FreeType(FTLibrary);

    delete [] Buffer;

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Video                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct video_buffer {
    uint8* Start;
    int ReadSize;
    int FullSize;
};

struct video_context {
    int TargetWidth;
    int TargetHeight;
    int VideoStreamIndex;
    AVFormatContext* FormatContext;
    AVIOContext* IOContext;
    AVCodecContext* CodecContext;
    AVPacket* Packet;
    AVFrame* Frame;
    SwsContext* ScalerContext;
    video_buffer Buffer;
    void* VideoOut;
    bool Ended;
    double TimeBase;
    int64_t PTS;
};

struct game_video {
    game_video_id ID;
    int Width;
    int Height;
    game_bitmap Texture;
    video_context VideoContext;
    int Handle;
    bool Loop;
    double TimeElapsed;
};

game_video LoadVideo(memory_arena* Arena, game_asset* Asset) {
    game_video Result = {};
    Result.ID = Asset->ID.Video;
    void* Dest = PushSize(Arena, Asset->MemoryNeeded);
    memcpy(Dest, Asset->File.Content, Asset->File.ContentSize);
    return Result;
}

void LoadFrame(video_context* VideoContext) {
    auto& FormatContext = VideoContext->FormatContext;
    auto& CodecContext = VideoContext->CodecContext;
    auto& StreamIndex = VideoContext->VideoStreamIndex;
    auto& Frame = VideoContext->Frame;
    auto& Packet = VideoContext->Packet;

    while (av_read_frame(FormatContext, Packet) >= 0) {
        if (Packet->stream_index != StreamIndex) {
            av_packet_unref(Packet);
            continue;
        }

        int result = avcodec_send_packet(CodecContext, Packet);
        if (result < 0) throw("Packet sending failed.");

        result = avcodec_receive_frame(CodecContext, Frame);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            av_packet_unref(Packet);
            av_frame_unref(Frame);
            continue;
        }
        else if (result < 0) {
            throw("Failed to decode packet.");
        }

        av_packet_unref(Packet);
        break;
    }

    if (!Frame) {
        throw("No frame was found.");
    }

    VideoContext->PTS = Frame->pts;

    return;
}

void CloseVideo(video_context* VideoContext) {
    avformat_close_input(&VideoContext->FormatContext);
    avformat_free_context(VideoContext->FormatContext);
    avcodec_free_context(&VideoContext->CodecContext);
    sws_freeContext(VideoContext->ScalerContext);
    av_frame_free(&VideoContext->Frame);
    av_packet_free(&VideoContext->Packet);

    return;
}

void WriteFrame(video_context* VideoContext, int TargetWidth, int TargetHeight) {
    auto& Frame = VideoContext->Frame;
    auto& CodecContext = VideoContext->CodecContext;
    auto& ScalerContext = VideoContext->ScalerContext;

    if (TargetWidth != VideoContext->TargetWidth || TargetHeight != VideoContext->TargetHeight) {
        VideoContext->TargetWidth = TargetWidth;
        VideoContext->TargetHeight = TargetHeight;
        if (VideoContext->TargetWidth || VideoContext->TargetHeight) {
            sws_freeContext(VideoContext->ScalerContext);
        }
        ScalerContext = sws_getContext(
            Frame->width, Frame->height, 
            CodecContext->pix_fmt, 
            TargetWidth, TargetHeight, 
            AV_PIX_FMT_RGB0,
            SWS_BILINEAR,
            NULL, 
            NULL, 
            NULL
        );
        if (!ScalerContext) {
            CloseVideo(VideoContext);
            VideoContext->Ended = true;
            return;
        }
    }

    uint8_t* Dest[4] = {(uint8_t*)VideoContext->VideoOut, 0, 0, 0};
    int DestLinesize[4] = { VideoContext->TargetWidth * 4, 0, 0, 0};
    int ScaleResult = sws_scale(ScalerContext, Frame->data, Frame->linesize, 0, Frame->height, Dest, DestLinesize);

    av_frame_unref(Frame);
}

static int ReadPacket(void* Opaque, uint8* Buffer, int BufferSize) {
    video_buffer* VideoBuffer = (video_buffer*)Opaque;
    int Remaining = VideoBuffer->FullSize - VideoBuffer->ReadSize;
    int ToRead = min(Remaining, BufferSize);
    if (ToRead == 0) return AVERROR_EOF;
    memcpy(Buffer, VideoBuffer->Start + VideoBuffer->ReadSize, ToRead);
    VideoBuffer->ReadSize += ToRead;
    return ToRead;
}

int64 SeekPacket(void* Opaque, int64 Offset, int Whence) {
    video_buffer* VideoBuffer = (video_buffer*)Opaque;
    int NewReadSize = 0;
    switch (Whence) {
        case AVSEEK_SIZE: return VideoBuffer->FullSize; break;
        case SEEK_SET: NewReadSize = Offset; break;
        case SEEK_CUR: NewReadSize = VideoBuffer->ReadSize + Offset; break;
        case SEEK_END: NewReadSize = VideoBuffer->FullSize + Offset; break;
        default: return EOF;
    }
    if (NewReadSize > VideoBuffer->FullSize) return EOF;
    VideoBuffer->ReadSize = NewReadSize;
    return NewReadSize;
}

void InitializeVideo(video_context* VideoContext) {
    auto& FormatContext = VideoContext->FormatContext;

    FormatContext = avformat_alloc_context();
    if (!FormatContext) {
        throw("Format context not allocated.");
    }

    int BufferSize = 0x1000;
    uint8* pBuffer = (uint8*)av_malloc(BufferSize);
    VideoContext->IOContext = avio_alloc_context(
        pBuffer,
        BufferSize,
        0,
        &VideoContext->Buffer,
        &ReadPacket,
        NULL,
        &SeekPacket
    );

    FormatContext->pb = VideoContext->IOContext;
    FormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
    FormatContext->iformat = av_find_input_format("mp4");

    // If loading video from file, change second parameter to URI
    int AVError = avformat_open_input(&FormatContext, NULL, NULL, NULL);
    if (AVError) {
        char StrError[256];
        av_strerror(AVError, StrError, 256);
        Log(Error, StrError);
        Assert(false);
    }

    // Finding video stream (not audio stream or others)
    AVStream* VideoStream = 0;
    for (unsigned int i = 0; i < FormatContext->nb_streams; i++) {
        AVStream* Stream = FormatContext->streams[i];
        if (Stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            VideoStream = Stream;
            break;
        }
    }

    if (VideoStream == 0) throw("VideoStream not found.");

    VideoContext->VideoStreamIndex = VideoStream->index;
    VideoContext->TimeBase = (double)VideoStream->time_base.num / (double)VideoStream->time_base.den;

// Allocating resources
    // Codec context & params
    AVCodecParameters* CodecParams = VideoStream->codecpar;
    const AVCodec* Codec = avcodec_find_decoder(CodecParams->codec_id);
    if (!Codec) throw("Decoder not found.");

    VideoContext->CodecContext = avcodec_alloc_context3(Codec);
    if (!VideoContext->CodecContext) throw("Codec context could not be allocated.");

    if (avcodec_parameters_to_context(VideoContext->CodecContext, CodecParams) < 0) throw("Codec params could not be loaded to context.");
    if (avcodec_open2(VideoContext->CodecContext, Codec, NULL) < 0) throw("Codec could not be opened.");

    // Packet & Frame
    VideoContext->Packet = av_packet_alloc();
    if (!VideoContext->Packet) throw("Packet could not be allocated.");

    VideoContext->Frame = av_frame_alloc();
    if (!VideoContext->Frame) throw("Frame could not be allocated.");

    LoadFrame(VideoContext);
    int Width = VideoContext->Frame->width;
    int Height = VideoContext->Frame->height;
    VideoContext->VideoOut = VirtualAlloc(0, Width * Height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Vertex layouts                                                                                                                                                   |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

enum shader_type {
    shader_type_float,
    shader_type_vec2,
    shader_type_vec3,
    shader_type_vec4,
    shader_type_int,
    shader_type_ivec2,
    shader_type_ivec3,
    shader_type_ivec4,
    shader_type_mat2,
    shader_type_mat3,
    shader_type_mat4,

    shader_type_count
};

const char* ShaderTypeTokens[shader_type_count] = {
    "float",
    "vec2",
    "vec3",
    "vec4",
    "int",
    "ivec2",
    "ivec3",
    "ivec4",
    "mat2",
    "mat3",
    "mat4"
};

shader_type GetShaderType(token Token) {
    for (int i = 0; i < shader_type_count; i++) {
        if (Token == ShaderTypeTokens[i]) {
            return (shader_type)i;
        }
    }
    char ErrorBuffer[256];
    sprintf_s(ErrorBuffer, "Invalid shader attribute type token `%s`.", Token.Text);
    Raise(ErrorBuffer);
    return shader_type_count;
}

uint32 GetShaderTypeSizeInBytes(shader_type Type) {
    switch (Type) {
        case shader_type_float: { return 4; } break;
        case shader_type_vec2:  { return 8; } break;
        case shader_type_vec3:  { return 12; } break;
        case shader_type_vec4:  { return 16; } break;
        case shader_type_int:   { return 4; } break;
        case shader_type_ivec2: { return 8; } break;
        case shader_type_ivec3: { return 12; } break;
        case shader_type_ivec4: { return 16; } break;
        case shader_type_mat2:  { return 16; } break;
        case shader_type_mat3:  { return 36; } break;
        case shader_type_mat4:  { return 64; } break;
        default: Raise("Invalid vertex attribute type `shader_type_count`.");
    }
    return 0;
}

// Returns number of elements for a given shader type. For example, output is 3 for type `vec3`.
int GetShaderTypeSize(shader_type Type) {
    switch (Type) {
        case shader_type_float: { return 1; } break;
        case shader_type_vec2:  { return 2; } break;
        case shader_type_vec3:  { return 3; } break;
        case shader_type_vec4:  { return 4; } break;
        case shader_type_int:   { return 1; } break;
        case shader_type_ivec2: { return 2; } break;
        case shader_type_ivec3: { return 3; } break;
        case shader_type_ivec4: { return 4; } break;
        case shader_type_mat2:  { return 4; } break;
        case shader_type_mat3:  { return 9; } break;
        case shader_type_mat4:  { return 16; } break;
        default: Raise("Invalid vertex attribute type `shader_type_count`.");
    }
    return 0;
}

struct vertex_attribute {
    shader_type Type;
    uint32 Location;
    uint32 Size;
    uint32 Offset;
};

bool operator!=(vertex_attribute Attribute1, vertex_attribute Attribute2) {
    return 
        Attribute1.Type != Attribute2.Type ||
        Attribute1.Location != Attribute2.Location ||
        Attribute1.Size != Attribute2.Size ||
        Attribute1.Offset != Attribute2.Offset;
}

enum vertex_layout_id {
    vertex_layout_vec3_id,
    vertex_layout_vec3_vec2_id,
    vertex_layout_vec3_vec2_vec3_id,
    vertex_layout_bones_id,

    vertex_layout_id_count
};

const uint8 MAX_VERTEX_ATTRIBUTES = 16;
struct vertex_layout {
    vertex_layout_id ID;
    vertex_attribute Attributes[MAX_VERTEX_ATTRIBUTES];
    uint32 Stride;
    uint8 nAttributes;
};

void AddAttribute(vertex_layout* VertexLayout, shader_type Type) {
    vertex_attribute* Attribute = &VertexLayout->Attributes[VertexLayout->nAttributes];
    Attribute->Location = VertexLayout->nAttributes++;
    Attribute->Type = Type;
    Attribute->Size = GetShaderTypeSizeInBytes(Type);
    Attribute->Offset = VertexLayout->Stride;
    VertexLayout->Stride += Attribute->Size;
}

vertex_layout VertexLayout(uint8 nAttributes, ...) {
    vertex_layout Result = {};
    Result.Stride = 0;
    Result.nAttributes = 0;

    va_list Types;
    va_start(Types, nAttributes);

    for (int i = 0; i < nAttributes; i++) {
        shader_type Type = va_arg(Types, shader_type);
        AddAttribute(&Result, Type);
    }
    return Result;
}

bool operator==(vertex_layout Layout1, vertex_layout Layout2) {
    if (Layout1.Stride == Layout2.Stride && Layout1.nAttributes == Layout2.nAttributes) {
        for (int i = 0; i < Layout1.nAttributes; i++) {
            if (Layout1.Attributes[i] != Layout2.Attributes[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Meshes                                                                                                                                                           |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const int BONE_NAME_LENGTH = 32;

struct bone {
    int ID;
    char Name[BONE_NAME_LENGTH];
    v3 Head;
    v3 Tail;
    transform Transform;
};

const int MAX_ARMATURE_BONES = 32;
struct armature {
    uint32 nBones;
    bone Bones[MAX_ARMATURE_BONES];
};

struct game_mesh {
    armature Armature;
    game_mesh_id ID;
    vertex_layout_id LayoutID;
    bool HasArmature;
    uint32 nVertices;
    uint32 nFaces;
    void* Vertices;
    uint32* Faces;
};

void GetMeshSizes(void* Content, uint32* nVertices, uint32* nFaces, uint32* nBones) {
    tokenizer Tokenizer = InitTokenizer(Content);
    token Token = RequireToken(Tokenizer, Token_Identifier);
    while (Token.Type == Token_Identifier) {
        if (Token == "nV")   *nVertices = Parseuint32(Tokenizer);
        else if (Token == "nF") *nFaces = Parseuint32(Tokenizer);
        else if (Token == "nB") *nBones = Parseuint32(Tokenizer);
        Token = GetToken(Tokenizer);
    }
}

uint64 GetMeshVerticesSize(uint32 nVertices, bool HasArmature) {
    if (HasArmature) return nVertices * (10 * sizeof(float) + 2 * sizeof(int32));
    else return nVertices * 8 * sizeof(float);
}

uint64 GetMeshFacesSize(uint32 nFaces) {
    return nFaces * 3 * sizeof(uint32);
}

/*
    8 `float` for each triplet of vertex, texture, normal `(vec3, vec2, vec3)` and
    3 unsigned integers for each triangle face. Also, 2 `int` for bone ids and 2 `float` 
    for bone weights if armature is present.
*/
uint64 GetTotalMeshSize(uint32 nVertices, uint32 nFaces, bool HasArmature) {
    return GetMeshVerticesSize(nVertices, HasArmature) + GetMeshFacesSize(nFaces);
}

uint64 GetTotalMeshSize(game_mesh* Mesh) {
    return GetTotalMeshSize(Mesh->nVertices, Mesh->nVertices, Mesh->HasArmature);
}

uint64 ComputeNeededMemoryForMesh(read_file_result File) {
    uint64 Result = 0;

    if (File.ContentSize > 0) {
        uint32 nVertices = 0, nFaces = 0, nBones = 0;
        GetMeshSizes(File.Content, &nVertices, &nFaces, &nBones);
        Result = GetTotalMeshSize(nVertices, nFaces, nBones > 0);
    }
    return Result;
}

game_mesh LoadMesh(memory_arena* Arena, game_asset* Asset) {
    game_mesh Result = {};
    Result.ID = Asset->ID.Mesh;
    
    read_file_result File = Asset->File;

    if (File.ContentSize > 0) {
        GetMeshSizes(File.Content, &Result.nVertices, &Result.nFaces, &Result.Armature.nBones);
        tokenizer Tokenizer = InitTokenizer(File.Content);

        AdvanceUntilLine(Tokenizer, 2);

        Result.HasArmature = Result.Armature.nBones > 0;
        Result.LayoutID = Result.HasArmature ? vertex_layout_bones_id : vertex_layout_vec3_vec2_vec3_id;
        uint32 VertexSize = Result.HasArmature ? 10 * sizeof(float) + 2 * sizeof(int32) : 8 * sizeof(float);
        Result.Vertices = PushSize(Arena, Result.nVertices * VertexSize);
        Result.Faces = PushArray(Arena, 3 * Result.nFaces, uint32);

        float* pOutV = (float*)Result.Vertices;
        for (int i = 0; i < Result.nVertices; i++) {
            v3 Position = ParseV3(Tokenizer);
            v3 Normal = ParseV3(Tokenizer);
            v2 Texture = ParseV2(Tokenizer);

            *pOutV++ = Position.X; *pOutV++ = Position.Y; *pOutV++ = Position.Z;
            *pOutV++ = Texture.X;  *pOutV++ = Texture.Y;
            *pOutV++ = Normal.X;   *pOutV++ = Normal.Y;   *pOutV++ = Normal.Z;

            if (Result.HasArmature) {
                iv2 BoneIDs = ParseIV2(Tokenizer);
                v2 Weights = ParseV2(Tokenizer);
                int32* pOutB = (int32*)pOutV;
                *pOutB++ = BoneIDs.X;
                *pOutB++ = BoneIDs.Y;

                pOutV = (float*)pOutB;
                *pOutV++ = Weights.X;
                *pOutV++ = Weights.Y;
            }
        }

        uint32* pOutF = Result.Faces;
        for (int i = 0; i < Result.nFaces; i++) {
            uv3 Face = ParseUV3(Tokenizer);
            *pOutF++ = Face.X;
            *pOutF++ = Face.Y;
            *pOutF++ = Face.Z;
        }

        token Token;
        for (int i = 0; i < Result.Armature.nBones; i++) {
            bone Bone = {};
            Bone.ID = Parseuint32(Tokenizer);
            Token = RequireToken(Tokenizer, Token_Identifier);
            int Length = min(Token.Length, BONE_NAME_LENGTH);
            for (int j = 0; j < Length; j++) {
                Bone.Name[j] = Token.Text[j];
            }
            Bone.Head = ParseV3(Tokenizer);
            Bone.Tail = ParseV3(Tokenizer);
            Result.Armature.Bones[Bone.ID] = Bone;
        }
    }

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Animation                                                                                                                                                        |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_animation {
    game_animation_id ID;
    uint32 nFrames;
    uint32 nBones;
    float* Content;
};

struct game_animator {
    game_animation* Animation;
    armature* Armature;
    uint32 CurrentFrame;
    bool Active;
    bool Loop;
};

void Update(game_animator* Animator) {
    game_animation* Animation = Animator->Animation;
    armature* Armature = Animator->Armature;

    float* Pointer = Animation->Content + Animator->CurrentFrame * Armature->nBones * 10;
    if(Animator->Active) {
        for (int i = 0; i < Armature->nBones; i++) {
            bone* Bone = &Armature->Bones[i];
            Bone->Transform.Translation.X = *Pointer++;
            Bone->Transform.Translation.Y = *Pointer++;
            Bone->Transform.Translation.Z = *Pointer++;
            Bone->Transform.Rotation.c = *Pointer++;
            Bone->Transform.Rotation.i = *Pointer++;
            Bone->Transform.Rotation.j = *Pointer++;
            Bone->Transform.Rotation.k = *Pointer++;
            Bone->Transform.Scale.X = *Pointer++;
            Bone->Transform.Scale.Y = *Pointer++;
            Bone->Transform.Scale.Z = *Pointer++;
        }

        Animator->CurrentFrame++;
        if (Animator->CurrentFrame >= Animation->nFrames - 1) {
            Animator->CurrentFrame = 0;
            if (!Animator->Loop) Animator->Active = false;
        }
    }
    else {
        for (int i = 0; i < Armature->nBones; i++) {
            bone* Bone = &Armature->Bones[i];
            Bone->Transform.Translation = V3(0,0,0);
            Bone->Transform.Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
            Bone->Transform.Scale = Scale();
        }
    }
}

void GetAnimationSizes(void* Content, uint32* nFrames, uint32* nBones) {
    tokenizer Tokenizer = InitTokenizer(Content);
    token Token = RequireToken(Tokenizer, Token_Identifier);
    while (Token.Type == Token_Identifier) {
        if (Token == "nF")     *nFrames = Parseuint32(Tokenizer);
        else if (Token == "nB") *nBones = Parseuint32(Tokenizer);
        Token = GetToken(Tokenizer);
    }
}

uint64 ComputeNeededMemoryForAnimation(read_file_result File) {
    uint64 Result = 0;

    if (File.ContentSize > 0) {
        uint32 nFrames = 0, nBones = 0;
        GetAnimationSizes(File.Content, &nFrames, &nBones);

        Result = nFrames * nBones * 10 * sizeof(float);
    }
    return Result;
}

game_animation LoadAnimation(memory_arena* Arena, game_asset* Asset) {
    game_animation Result = {};
    Result.ID = Asset->ID.Animation;

    GetAnimationSizes(Asset->File.Content, &Result.nFrames, &Result.nBones);

    Result.Content = PushArray(Arena, Result.nFrames * Result.nBones * 10, float);

    tokenizer Tokenizer = InitTokenizer(Asset->File.Content);
    AdvanceUntilLine(Tokenizer, 2);
    
    float* pOut = Result.Content;
    for (uint32 i = 0; i < Result.nFrames; i++) {
        for (uint32 j = 0; j < Result.nBones; j++) {
            uint32 Frame = Parseuint32(Tokenizer);
            uint32 BoneID = Parseuint32(Tokenizer);
            Assert(Frame == i && BoneID == j);

            v3 Translation = ParseV3(Tokenizer);
            quaternion Rotation = ParseQuaternion(Tokenizer);
            v3 Scale = ParseV3(Tokenizer);

            *pOut++ = Translation.X;
            *pOut++ = Translation.Y;
            *pOut++ = Translation.Z;
            *pOut++ = Rotation.c;
            *pOut++ = Rotation.i;
            *pOut++ = Rotation.j;
            *pOut++ = Rotation.k;
            *pOut++ = Scale.X;
            *pOut++ = Scale.Y;
            *pOut++ = Scale.Z;
        }
    }

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders                                                                                                                                                          |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

enum game_shader_type {
    Vertex_Shader,
    Tessellation_Control_Shader,
    Tessellation_Evaluation_Shader,
    Geometry_Shader,
    Fragment_Shader,

    game_shader_type_count
};

enum game_shader_id {
    // Vertex shaders
    Vertex_Shader_Passthrough_ID,
    Vertex_Shader_Screen_ID,
    Vertex_Shader_Perspective_ID,
    Vertex_Shader_Bones_ID,
#if GAME_RENDER_API_VULKAN
    Vertex_Shader_Vulkan_Test_ID,
#endif

    // Tessellation control shaders
    Tessellation_Control_Shader_ID,

    // Tessellation evaluation shaders
    Tessellation_Evaluation_Shader_Standard_ID,
    Tessellation_Evaluation_Shader_Trochoidal_ID,

    // Geometry shaders
    Geometry_Shader_Test_ID,
    Geometry_Shader_Debug_Normals_ID,

    // Fragment shaders
    Fragment_Shader_Antialiasing_ID,
    Fragment_Shader_Single_Color_ID,
    Fragment_Shader_Texture_ID,
    Fragment_Shader_Framebuffer_Attachment_ID,
    Fragment_Shader_Outline_ID,
    Fragment_Shader_Kernel_ID,
    Fragment_Shader_Mesh_ID,
    Fragment_Shader_Sphere_ID,
    Fragment_Shader_Jump_Flood_ID,
    Fragment_Shader_Heightmap_ID,
    Fragment_Shader_Sea_ID,
#if GAME_RENDER_API_VULKAN
    Fragment_Shader_Vulkan_Test_ID,
#endif

    game_shader_id_count
};

const int MAX_SHADER_UNIFORM_BLOCK_MEMBERS = 16;

struct shader_uniform_block {
    uint32 Set;
    uint32 Binding;
    uint32 nMembers;
    shader_type Member[MAX_SHADER_UNIFORM_BLOCK_MEMBERS];
};

bool operator==(shader_uniform_block UBO1, shader_uniform_block UBO2) {
    if (UBO1.Set == UBO2.Set && UBO1.Binding == UBO2.Binding && UBO1.nMembers == UBO2.nMembers) {
        for (int i = 0; i < UBO1.nMembers; i++) {
            if (UBO1.Member[i] != UBO2.Member[i]) return false;
        }
        return true;
    }
    return false;
}

bool operator!=(shader_uniform_block UBO1, shader_uniform_block UBO2) {
    if (UBO1.Set != UBO2.Set || UBO1.Binding != UBO2.Binding || UBO1.nMembers != UBO2.nMembers) {
        return true;
    }
    else {
        for (int i = 0; i < UBO1.nMembers; i++) {
            if (UBO1.Member[i] != UBO2.Member[i]) return true;
        }
    }
    return false;
}

struct shader_uniform_sampler {
    uint32 Set;
    uint32 Binding;
};

const int SHADER_UNIFORM_BLOCKS = 8;

struct alignas(16) global_uniforms {
    matrix4 projection;
    matrix4 view;
    v2 resolution;
    float time;
};

struct alignas(16) light_uniforms {
    alignas(16) v3 direction;
    alignas(16) v3 color;
    float ambient;
    float diffuse;
};

struct alignas(16) color_uniforms {
    v4 color;
};

struct alignas(16) model_uniforms {
    matrix4 model;
    matrix4 normal;
};

struct alignas(16) bone_uniforms {
    matrix4 bone_transforms[32];
    matrix4 bone_normal_transforms[32];
    alignas(16) int n_bones;
};

struct alignas(16) outline_uniforms {
    float width;
    int level;
};

struct alignas(16) kernel_uniforms {
    float XX, XY, XZ, _Pad0;
    float YX, YY, YZ, _Pad1;
    float ZX, ZY, ZZ, _Pad2;
};

struct alignas(16) antialiasing_uniforms {
    int samples;
};

const int SHADER_SETS = 3;
const int MAX_SHADER_SET_BINDINGS = 8;
const int MAX_SHADER_UBOS = 8;
const int MAX_SHADER_SAMPLERS = 4;

struct game_shader {
    game_shader_id ID;
    game_shader_type Type;
    read_file_result File;
    char* Code;
    uint64 BinarySize;
    uint32* Binary;
    vertex_layout VertexLayout;
    uint32 nUBOs;
    shader_uniform_block UBO[MAX_SHADER_UBOS];
    uint32 nSamplers;
    shader_uniform_sampler Sampler[MAX_SHADER_SAMPLERS];
};

enum game_shader_pipeline_id {
    Shader_Pipeline_Antialiasing_ID,
    Shader_Pipeline_Framebuffer_ID,
    Shader_Pipeline_Screen_Single_Color_ID,
    Shader_Pipeline_World_Single_Color_ID,
    Shader_Pipeline_Bones_Single_Color_ID,
    Shader_Pipeline_Texture_ID,
    Shader_Pipeline_Mesh_ID,
    Shader_Pipeline_Mesh_Bones_ID,
    Shader_Pipeline_Sphere_ID,
    Shader_Pipeline_Jump_Flood_ID,
    Shader_Pipeline_Outline_ID,
    Shader_Pipeline_Heightmap_ID,
    Shader_Pipeline_Trochoidal_ID,
    Shader_Pipeline_Debug_Normals_ID,
#if GAME_RENDER_API_VULKAN
    Shader_Pipeline_Vulkan_Test_ID,
#endif

    game_shader_pipeline_id_count
};

struct game_shader_pipeline {
    vertex_layout_id VertexLayoutID;
    game_shader_pipeline_id ID;
    game_shader_id Pipeline[game_shader_type_count];
    bool IsProvided[game_shader_type_count];
    bool Bindings[SHADER_SETS][MAX_SHADER_SET_BINDINGS];
};

enum game_compute_shader_id {
    Compute_Shader_Outline_Init_ID,
    Compute_Shader_Jump_Flood_ID,
    Compute_Shader_Kernel_ID,
    Compute_Shader_Test_ID,

    game_compute_shader_id_count
};

struct game_compute_shader {
    game_compute_shader_id ID;
    uint32 Size;
    char* Code;
};


// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Game assets                                                                                                                                                      |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const int ASSET_COUNT =
game_text_id_count +
game_sound_id_count +
game_bitmap_id_count +
game_heightmap_id_count +
game_font_id_count +
game_mesh_id_count +
game_animation_id_count + 
game_video_id_count;

ArrayDefinition(ASSET_COUNT, game_asset)

struct game_assets {
    game_asset_array Asset;
    game_text Text[game_text_id_count];
    game_bitmap Bitmap[game_bitmap_id_count];
    game_heightmap Heightmap[game_heightmap_id_count];
    game_font Font[game_font_id_count];
    game_sound Sound[game_sound_id_count];
    game_mesh Mesh[game_mesh_id_count];
    game_animation Animation[game_animation_id_count];
    uint64 AssetsSize;
    game_video Videos[1];
    vertex_layout VertexLayouts[vertex_layout_id_count];
    uint32 nBindings[SHADER_SETS];
    shader_uniform_block UBOs[SHADER_SETS][MAX_SHADER_SET_BINDINGS];
    uint32 nSamplers;
    game_shader Shader[game_shader_id_count];
    game_shader_pipeline ShaderPipeline[game_shader_pipeline_id_count];
    game_compute_shader ComputeShader[game_compute_shader_id_count];
    uint64 ShadersSize;
    uint64 ComputeShadersSize;
    uint64 TotalSize;
    uint8* Memory;
};


game_text*      GetAsset(game_assets* Assets, game_text_id ID)      { return &Assets->Text[ID]; }
game_sound*     GetAsset(game_assets* Assets, game_sound_id ID)     { return &Assets->Sound[ID]; }
game_bitmap*    GetAsset(game_assets* Assets, game_bitmap_id ID)    { return &Assets->Bitmap[ID]; }
game_heightmap* GetAsset(game_assets* Assets, game_heightmap_id ID) { return &Assets->Heightmap[ID]; }
game_font*      GetAsset(game_assets* Assets, game_font_id ID)      { return &Assets->Font[ID]; }
game_mesh*      GetAsset(game_assets* Assets, game_mesh_id ID)      { return &Assets->Mesh[ID]; }
game_animation* GetAsset(game_assets* Assets, game_animation_id ID) { return &Assets->Animation[ID]; }
game_video*     GetAsset(game_assets* Assets, game_video_id ID)     { return &Assets->Videos[ID]; }

void PushAsset(game_assets* Assets, const char* Path, game_text_id ID) {
    game_asset Asset = {};
    Asset.Type = Text;
    Asset.ID.Text = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = Asset.File.ContentSize + 1;
    
    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_sound_id ID) {
    game_asset Asset = {};
    Asset.Type = Sound;
    Asset.ID.Sound = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForSound(Asset.File);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_bitmap_id ID) {
    game_asset Asset = {};
    Asset.Type = Bitmap;
    Asset.ID.Bitmap = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForBitmap((bitmap_header*)Asset.File.Content);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_heightmap_id ID) {
    game_asset Asset = {};
    Asset.Type = Heightmap;
    Asset.ID.Heightmap = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForHeightmap(Asset.File);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_font_id ID) {
    game_asset Asset = {};
    Asset.Type = Font;
    Asset.ID.Font = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForFont(Asset.File.Path);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_mesh_id ID) {
    game_asset Asset = {};
    Asset.Type = Mesh;
    Asset.ID.Mesh = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForMesh(Asset.File);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_animation_id ID) {
    game_asset Asset = {};
    Asset.Type = Animation;
    Asset.ID.Animation = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForAnimation(Asset.File);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_video_id ID) {
    game_asset Asset = {};
    Asset.Type = Video;
    Asset.ID.Video = ID;
    Asset.File = PlatformReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = Asset.File.ContentSize;

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void LoadAsset(memory_arena* Arena, game_assets* Assets, game_asset* Asset) {
    Asset->Offset = Arena->Used;
    game_asset_id ID = Asset->ID;
    char LogBuffer[512];
    switch (Asset->Type) {
        case Text: {
            char* TextContent = (char*)PushSize(Arena, Asset->MemoryNeeded);
            Assets->Text[ID.Text].ID = ID.Text;
            Assets->Text[ID.Text].Size = Asset->MemoryNeeded;
            Assets->Text[ID.Text].Content = TextContent;
            memcpy(TextContent, Asset->File.Content, Asset->MemoryNeeded);
            sprintf_s(LogBuffer, "Loaded text %s.", Asset->File.Path);
        } break;

        case Video: {
           Assets->Videos[ID.Video] = LoadVideo(Arena, Asset);
           sprintf_s(LogBuffer, "Loaded video %s.", Asset->File.Path);
        } break;

        case Bitmap: {
            Assets->Bitmap[ID.Bitmap] = LoadBitmap(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded bitmap %s.", Asset->File.Path);
        } break;

        case Heightmap: {
            Assets->Heightmap[ID.Heightmap] = LoadHeightmap(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded heightmap %s.", Asset->File.Path);
        } break;

        case Font: {
            Assets->Font[ID.Font] = LoadFont(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded font %s.", Asset->File.Path);
        } break;

        case Sound: {
            Assets->Sound[ID.Sound] = LoadSound(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded sound %s.", Asset->File.Path);
        } break;

        case Mesh: {
            Assets->Mesh[ID.Mesh] = LoadMesh(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded mesh %s.", Asset->File.Path);
        } break;

        case Animation: {
            Assets->Animation[ID.Animation] = LoadAnimation(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded animation %s.", Asset->File.Path);
        } break;

        default: {
            sprintf_s(LogBuffer, "Asset ignored %s.", Asset->File.Path);
        }
    }

    Log(Info, LogBuffer);
    Assert(Asset->MemoryNeeded == Arena->Used - Asset->Offset);
    PlatformFreeFileMemory(Asset->File.Content);
    Asset->File.Content = 0;
}

game_shader*          GetShader        (game_assets* Assets, game_shader_id ID)          { return &Assets->Shader[ID]; }
game_shader_pipeline* GetShaderPipeline(game_assets* Assets, game_shader_pipeline_id ID) { return &Assets->ShaderPipeline[ID]; }
game_compute_shader*  GetShader        (game_assets* Assets, game_compute_shader_id ID)  { return &Assets->ComputeShader[ID]; }

void PushShader(game_assets* Assets, const char* Path, game_shader_id ID) {
    game_shader* Shader = GetShader(Assets, ID);
    Shader->ID = ID;

    WIN32_FIND_DATAA Data;
    HANDLE hFind = FindFirstFileA(Path, &Data);

    Assert(hFind != INVALID_HANDLE_VALUE);

    char* Extension = 0;
    char* Buffer = new char[strlen(Data.cFileName) + 1];
    strcpy_s(Buffer, strlen(Data.cFileName) + 1, Data.cFileName);
    char* _ = strtok_s(Buffer, ".", &Extension);

    if (Extension != 0) {
        if      (strcmp(Extension, "frag") == 0)  { Shader->Type = Fragment_Shader; }
        else if (strcmp(Extension, "vert") == 0)  { Shader->Type = Vertex_Shader; }
        else if (strcmp(Extension, "geom") == 0)  { Shader->Type = Geometry_Shader; }
        else if (strcmp(Extension, "tesc")  == 0) { Shader->Type = Tessellation_Control_Shader; }
        else if (strcmp(Extension, "tese")  == 0) { Shader->Type = Tessellation_Evaluation_Shader; }
        else Assert(false);
    }
    else Assert(false);

    Shader->File = PlatformReadEntireFile(Path);
    Shader->Code = (char*)Shader->File.Content;

    // Extra char with value 0 to separate shaders
    Assets->TotalSize += Shader->File.ContentSize + 1;
    Assets->ShadersSize += Shader->File.ContentSize + 1;

    delete [] Buffer;
}

void PushShaderPipeline(game_assets* Assets, game_shader_pipeline_id ID, int nShaders, ...) {
    Assert(nShaders <= game_shader_type_count);

    game_shader_pipeline* ShaderPipeline = GetShaderPipeline(Assets, ID);
    ShaderPipeline->ID = ID;
    
    va_list Shaders;
    va_start(Shaders, nShaders);

    for (int i = 0; i < nShaders; i++) {
        game_shader_id ShaderID = va_arg(Shaders, game_shader_id);

        game_shader* Shader = &Assets->Shader[ShaderID];

        if (ShaderPipeline->IsProvided[Shader->Type]) Raise("Shader of this type has alredy been attached to pipeline.");
        else {
            ShaderPipeline->IsProvided[Shader->Type] = true;
            ShaderPipeline->Pipeline[Shader->Type] = Shader->ID;
        }
    }
}

void PushShader(game_assets* Assets, const char* Path, game_compute_shader_id ID) {
    game_compute_shader* Shader = GetShader(Assets, ID);
    Shader->ID = ID;

    WIN32_FIND_DATAA Data;
    HANDLE hFind = FindFirstFileA(Path, &Data);

    Assert(hFind != INVALID_HANDLE_VALUE);

    char* Extension = 0;
    char* Buffer = new char[strlen(Data.cFileName) + 1];
    strcpy_s(Buffer, strlen(Data.cFileName) + 1, Data.cFileName);
    char* _ = strtok_s(Buffer, ".", &Extension);

    if (strcmp(Extension, "comp") != 0) {
        Raise("Extension of compute shader file should be '.comp'.");
    }
    else {
        read_file_result ShaderFile = PlatformReadEntireFile(Path);
        Shader->Size = ShaderFile.ContentSize;
        Shader->Code = (char*)ShaderFile.Content;

        // Extra char with value 0 to separate shaders
        Assets->TotalSize += Shader->Size + 1;
        Assets->ComputeShadersSize += Shader->Size + 1;
    }

    delete [] Buffer;
}

void LoadShader(memory_arena* Arena, game_shader* Shader) {
    char* Destination = (char*)PushSize(Arena, Shader->File.ContentSize + 1);
    memcpy(Destination, Shader->Code, Shader->File.ContentSize);

    // Get attributes and uniforms
    tokenizer Tokenizer = InitTokenizer(Shader->Code);
    token Token = GetToken(Tokenizer);
    uint32 nAttributes = 0;
    vertex_attribute Attributes[MAX_VERTEX_ATTRIBUTES] = {};
    while (Token.Type != Token_End) {
        // Attributes
        if (Shader->Type == Vertex_Shader && Token == "layout") {
            vertex_attribute Attribute = {};

            Token = RequireToken(Tokenizer, Token_OpenParen);
            Token = RequireToken(Tokenizer, "location");
            Token = RequireToken(Tokenizer, Token_Equal);
            Attribute.Location = Parseuint32(Tokenizer);

            Token = RequireToken(Tokenizer, Token_CloseParen);
            Token = RequireToken(Tokenizer, Token_Identifier);
            // We only need input vertex attributes
            if (Token == "in") {
                Token = GetToken(Tokenizer);
                Attribute.Type = GetShaderType(Token);
                Attribute.Size = GetShaderTypeSize(Attribute.Type);
                Attributes[nAttributes++] = Attribute;
            }
        }

        // Uniforms
        if (Token == "VULKAN") {
            Token = RequireToken(Tokenizer, "layout");

            shader_uniform_block UBO = {};
            shader_uniform_sampler Sampler = {};

            Token = RequireToken(Tokenizer, Token_OpenParen);
            Token = RequireToken(Tokenizer, Token_Identifier);
            while (Token.Type != Token_CloseParen && Token.Type != Token_End) {
                if (Token == "set") {
                    Token = RequireToken(Tokenizer, Token_Equal);
                    UBO.Set = Parseuint32(Tokenizer);
                    Sampler.Set = UBO.Set;
                }
                else if (Token == "binding") {
                    Token = RequireToken(Tokenizer, Token_Equal);
                    UBO.Binding = Parseuint32(Tokenizer);
                    Sampler.Binding = UBO.Binding;
                }
                Token = GetToken(Tokenizer);
            }

            if (Token.Type == Token_End) {
                break;
            }

            Token = RequireToken(Tokenizer, Token_Identifier);
            if (Token == "uniform") {
                Token = GetToken(Tokenizer);
                if (Token == "sampler2D" || Token == "sampler2DMS") {
                    Assert(Sampler.Set == 2);
                    Shader->Sampler[Shader->nSamplers++] = Sampler;
                }
                else {
                    AdvanceUntil(Tokenizer, '{');
                    Token = RequireToken(Tokenizer, Token_OpenBrace);
                    Token = GetToken(Tokenizer);
                    while (Token.Type != Token_CloseBrace && Token.Type != Token_End) {
                        shader_type Type = GetShaderType(Token);
                        AdvanceUntil(Tokenizer, ';');
                        Token = GetToken(Tokenizer);
                        Token = GetToken(Tokenizer);
                        UBO.Member[UBO.nMembers++] = Type;
                    }
                    Shader->UBO[Shader->nUBOs++] = UBO;
                }
            }
        }
        Token = GetToken(Tokenizer);
    }
    
    Shader->VertexLayout = {};
    for (int i = 0; i < nAttributes; i++) {
        for (int j = 0; j < nAttributes; j++) {
            vertex_attribute Attribute = Attributes[j];
            if (Attribute.Location == i) {
                AddAttribute(&Shader->VertexLayout, Attribute.Type);
                break;
            }
        }
    }

    PlatformFreeFileMemory(Shader->Code);

    char LogBuffer[256];
    sprintf_s(LogBuffer, "Loaded shader %s.", Shader->File.Path);
    Log(Info, LogBuffer);
}

void LoadComputeShader(memory_arena* Arena, game_compute_shader* Shader) {
    char* Destination = (char*)PushSize(Arena, Shader->Size + 1);
    memcpy(Destination, Shader->Code, Shader->Size);
    PlatformFreeFileMemory(Shader->Code);
}

void LoadShaderPipelines(game_assets* Assets) {
    Assets->nSamplers = 0;
    bool UBOLoaded[SHADER_SETS][MAX_SHADER_SET_BINDINGS] = {};
    for (int i = 0; i < game_shader_pipeline_id_count; i++) {
        game_shader_pipeline* Pipeline = &Assets->ShaderPipeline[i];
        
        // Vertex layout
        game_shader* VertexShader = GetShader(Assets, Pipeline->Pipeline[Vertex_Shader]);
        bool VertexLayoutFound = false;
        for (int j = 0; j < vertex_layout_id_count; j++) {
            if (VertexShader->VertexLayout == Assets->VertexLayouts[j]) {
                VertexLayoutFound = true;
                Pipeline->VertexLayoutID = (vertex_layout_id)j;
                break;
            }
        }
        if (!VertexLayoutFound) {
            Raise("Vertex layout was not found.");
        }

        // Uniform layout
        for (int j = 0; j < game_shader_type_count; j++) {
            if (Pipeline->IsProvided[j]) {
                game_shader* Shader = &Assets->Shader[Pipeline->Pipeline[j]];
                if (Assets->nSamplers < Shader->nSamplers) Assets->nSamplers = Shader->nSamplers;
                for (int k = 0; k < Shader->nUBOs; k++) {
                    shader_uniform_block UBO = Shader->UBO[k];
                    Pipeline->Bindings[UBO.Set][UBO.Binding] = true;
                    shader_uniform_block* LoadUBO = &Assets->UBOs[UBO.Set][UBO.Binding];
                    if (UBOLoaded[UBO.Set][UBO.Binding]) {
                        if (UBO != *LoadUBO) {
                            Raise("Inconsistent UBO definition.");
                        }
                    }
                    else {
                        *LoadUBO = UBO;
                        UBOLoaded[UBO.Set][UBO.Binding] = true;
                        Assets->nBindings[UBO.Set]++;
                    }
                }

                if (Shader->nSamplers > Assets->nSamplers) Assets->nSamplers = Shader->nSamplers;
            }
        }
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Assets file                                                                                                                                                      |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void LoadAssetsFromFile(platform_read_entire_file Read, game_assets* Assets, const char* Path) {
    read_file_result AssetsFile = Read(Path);

    *Assets = *(game_assets*)AssetsFile.Content;
    Assets->Memory = (uint8*)AssetsFile.Content + sizeof(game_assets);

    for (int i = 0; i < ASSET_COUNT; i++) {
        game_asset Asset = Assets->Asset.Content[i];

        switch (Asset.Type) {
            case Text: {
                game_text* Text = GetAsset(Assets, Asset.ID.Text);
                Text->Content = (char*)(Assets->Memory + Asset.Offset);
            } break;

            case Sound: {
                game_sound* Sound = GetAsset(Assets, Asset.ID.Sound);
                Sound->SampleOut = (int16*)(Assets->Memory + Asset.Offset);
            } break;

            case Bitmap: {
                game_bitmap* Bitmap = GetAsset(Assets, Asset.ID.Bitmap);
                Bitmap->Content = (uint32*)(Assets->Memory + Asset.Offset);
            } break;

            case Heightmap: {
                game_heightmap* Heightmap = GetAsset(Assets, Asset.ID.Heightmap);
                Heightmap->Bitmap.Content = (uint32*)(Assets->Memory + Asset.Offset);
                uint64 BitmapSize = ComputeNeededMemoryForBitmap(&Heightmap->Bitmap.Header);
                Heightmap->Vertices = (double*)(Assets->Memory + Asset.Offset + BitmapSize);
            } break;

            case Font: {
                game_font* Font = GetAsset(Assets, Asset.ID.Font);
                Font->Bitmap.Content = (uint32*)(Assets->Memory + Asset.Offset);
            } break;

            case Mesh: {
                game_mesh* Mesh = GetAsset(Assets, Asset.ID.Mesh);
                Mesh->Vertices = (void*)(Assets->Memory + Asset.Offset);
                vertex_layout Layout = Assets->VertexLayouts[Mesh->LayoutID];
                Mesh->Faces = (uint32*)((uint8*)Mesh->Vertices + Layout.Stride * Mesh->nVertices);
            } break;

            case Animation: {
                game_animation* Animation = GetAsset(Assets, Asset.ID.Animation);
                Animation->Content = (float*)(Assets->Memory + Asset.Offset);
            } break;

            case Video: {
                game_video* Video = &Assets->Videos[Asset.ID.Video];
                Video->VideoContext.Buffer.Start = Assets->Memory + Asset.Offset;
                Video->VideoContext.Buffer.FullSize = Asset.File.ContentSize;
                Video->VideoContext.Buffer.ReadSize = 0;
                InitializeVideo(&Video->VideoContext);
                Video->Width = Video->VideoContext.Frame->width;
                Video->Height = Video->VideoContext.Frame->height;
                Video->Texture = {};
                Video->Texture.ID = game_bitmap_id_count;
                MakeBitmapHeader(&Video->Texture.Header, Video->Width, Video->Height, 32);
                Video->Texture.BytesPerPixel = 4;
                Video->Texture.Pitch = 4 * Video->Texture.Header.Width;
                Video->Texture.Content = (uint32*)Video->VideoContext.VideoOut;
            } break;

            default: {
                Raise("Asset type not implemented.");
            }
        }
    }

    Log(Info, "Assets loaded.");

    char* Pointer = (char*)(Assets->Memory + Assets->AssetsSize);
    for (int i = 0; i < game_shader_id_count; i++) {
        game_shader* Shader = &Assets->Shader[i];

        Shader->Code = Pointer;
        Pointer += Shader->File.ContentSize + 1;
    }

    for (int i = 0; i < game_compute_shader_id_count; i++) {
        game_compute_shader* Shader = &Assets->ComputeShader[i];

        Shader->Code = Pointer;
        Pointer += Shader->Size + 1;
    }

    Log(Info, "Shaders loaded.");
}

void WriteAssetsFile(const char* Path) {
    game_assets Assets = {};

// Assets
    // Fonts
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Fonts\\CascadiaMono.ttf", Font_Cascadia_Mono_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Fonts\\Menlo-Regular.ttf", Font_Menlo_Regular_ID);

    // Text
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Texts\\Test.txt", Text_Test_ID);

    // Bitmaps
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Bitmaps\\Background.bmp", Bitmap_Background_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Bitmaps\\Button.bmp", Bitmap_Button_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Bitmaps\\Empty.bmp", Bitmap_Empty_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Bitmaps\\Enemy.bmp", Bitmap_Enemy_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Bitmaps\\Player.bmp", Bitmap_Player_ID);

    // Heightmaps
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Bitmaps\\spain.bmp", Heightmap_Spain_ID);

    // Sound
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Sounds\\16agosto.wav", Sound_Test_ID);

    // Meshes
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Horns.mdl", Mesh_Enemy_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Sphere.mdl", Mesh_Sphere_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Body.mdl", Mesh_Body_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Sword.mdl", Mesh_Sword_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Shield.mdl", Mesh_Shield_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Selector.mdl", Mesh_Selector_ID);

    // Animation
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Animations\\Idle.anim", Animation_Idle_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Animations\\Walking.anim", Animation_Walk_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Animations\\Jumping.anim", Animation_Jump_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Animations\\Attack.anim", Animation_Attack_ID);

    // Video
    //PushAsset(&Assets, "..\\GameAssets\\Assets\\Videos\\The Witness Wrong MOOV.mp4", Video_Test_ID);

// Shaders
    // Vertex layouts
    Assets.VertexLayouts[vertex_layout_vec3_id] = VertexLayout(1, shader_type_vec3);
    Assets.VertexLayouts[vertex_layout_vec3_vec2_id] = VertexLayout(2, shader_type_vec3, shader_type_vec2);
    Assets.VertexLayouts[vertex_layout_vec3_vec2_vec3_id] = VertexLayout(3, shader_type_vec3, shader_type_vec2, shader_type_vec3);
    Assets.VertexLayouts[vertex_layout_bones_id] = VertexLayout(5, shader_type_vec3, shader_type_vec2, shader_type_vec3, shader_type_ivec2, shader_type_vec2);
    for (int i = 0; i < vertex_layout_id_count; i++) Assets.VertexLayouts[i].ID = (vertex_layout_id)i;

    // Vertex
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\Passthrough.vert", Vertex_Shader_Passthrough_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\Screen.vert", Vertex_Shader_Screen_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\Perspective.vert", Vertex_Shader_Perspective_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\Bones.vert", Vertex_Shader_Bones_ID);
#if GAME_RENDER_API_VULKAN
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\VulkanTest.vert", Vertex_Shader_Vulkan_Test_ID);
#endif

    // Fragment
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Antialiasing.frag", Fragment_Shader_Antialiasing_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\FramebufferAttachment.frag", Fragment_Shader_Framebuffer_Attachment_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Texture.frag", Fragment_Shader_Texture_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Outline.frag", Fragment_Shader_Outline_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\SingleColor.frag", Fragment_Shader_Single_Color_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Kernel.frag", Fragment_Shader_Kernel_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Sphere.frag", Fragment_Shader_Sphere_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Mesh.frag", Fragment_Shader_Mesh_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\JumpFlood.frag", Fragment_Shader_Jump_Flood_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Heightmap.frag", Fragment_Shader_Heightmap_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Sea.frag", Fragment_Shader_Sea_ID);
#if GAME_RENDER_API_VULKAN
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\VulkanTest.frag", Fragment_Shader_Vulkan_Test_ID);
#endif

    // Geometry
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Geometry\\Test.geom", Geometry_Shader_Test_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Geometry\\DebugNormals.geom", Geometry_Shader_Debug_Normals_ID);

    // Tessellation
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Tessellation\\Tessellation.tesc", Tessellation_Control_Shader_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Tessellation\\Tessellation.tese", Tessellation_Evaluation_Shader_Standard_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Tessellation\\Trochoidal.tese", Tessellation_Evaluation_Shader_Trochoidal_ID);

    // Shader pipelines
    PushShaderPipeline(&Assets, Shader_Pipeline_Antialiasing_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Antialiasing_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Framebuffer_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Framebuffer_Attachment_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Texture_ID, 2, Vertex_Shader_Screen_ID, Fragment_Shader_Texture_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Mesh_ID, 2, Vertex_Shader_Perspective_ID, Fragment_Shader_Mesh_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Mesh_Bones_ID, 2, Vertex_Shader_Bones_ID, Fragment_Shader_Mesh_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Sphere_ID, 2, Vertex_Shader_Perspective_ID, Fragment_Shader_Sphere_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_World_Single_Color_ID, 2, Vertex_Shader_Perspective_ID, Fragment_Shader_Single_Color_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Screen_Single_Color_ID, 2, Vertex_Shader_Screen_ID, Fragment_Shader_Single_Color_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Bones_Single_Color_ID, 2, Vertex_Shader_Bones_ID, Fragment_Shader_Single_Color_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Outline_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Outline_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Jump_Flood_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Jump_Flood_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Debug_Normals_ID, 3,
        Vertex_Shader_Bones_ID,
        Geometry_Shader_Debug_Normals_ID,
        Fragment_Shader_Single_Color_ID
    );
    //PushShaderPipeline(&Assets, Shader_Pipeline_Kernel_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Kernel_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Heightmap_ID, 4, 
        Vertex_Shader_Passthrough_ID, 
        Tessellation_Control_Shader_ID, 
        Tessellation_Evaluation_Shader_Standard_ID, 
        Fragment_Shader_Heightmap_ID
    );
    PushShaderPipeline(&Assets, Shader_Pipeline_Trochoidal_ID, 4,
        Vertex_Shader_Passthrough_ID,
        Tessellation_Control_Shader_ID,
        Tessellation_Evaluation_Shader_Trochoidal_ID,
        Fragment_Shader_Sea_ID
    );
#if GAME_RENDER_API_VULKAN
    PushShaderPipeline(&Assets, Shader_Pipeline_Vulkan_Test_ID, 2, Vertex_Shader_Vulkan_Test_ID, Fragment_Shader_Vulkan_Test_ID);
#endif
    
    // Compute
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Compute\\OutlineInit.comp", Compute_Shader_Outline_Init_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Compute\\JumpFlood.comp", Compute_Shader_Jump_Flood_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Compute\\Test.comp", Compute_Shader_Test_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Compute\\Kernel.comp", Compute_Shader_Kernel_ID);

// Output file
    void* FileMemory = VirtualAlloc(0, sizeof(game_assets) + Assets.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Assets.Memory = (uint8*)FileMemory + sizeof(game_assets);
    memory_arena AssetArena = MemoryArena(Assets.TotalSize, (uint8*)Assets.Memory);

    // Assets
    for (int i = 0; i < Assets.Asset.Count; i++) {
        LoadAsset(&AssetArena, &Assets, &Assets.Asset.Content[i]);
    }

    // Shaders
    for (int i = 0; i < game_shader_id_count; i++) {
        LoadShader(&AssetArena, &Assets.Shader[i]);
    }

    // Compute shaders
    for (int i = 0; i < game_compute_shader_id_count; i++) {
        LoadComputeShader(&AssetArena, &Assets.ComputeShader[i]);
    }

    // Shader pipelines vertex and uniform layouts
    LoadShaderPipelines(&Assets);

    game_assets* OutputAssets = (game_assets*)FileMemory;
    if (OutputAssets) *OutputAssets = Assets;
    PlatformWriteEntireFile(Path, sizeof(game_assets) + Assets.TotalSize, FileMemory);

    Log(Info, "Finished writing assets file.");

    VirtualFree(FileMemory, 0, MEM_RELEASE);
}

#endif