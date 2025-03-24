// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H

// FFMPEG
//#include "..\Linking\include\FFMpeg.h"

#include "..\GameLibrary\GameMath.h"
#include "..\Win32PlatformLayer\Win32PlatformLayer.h"

#ifndef GAME_ASSETS
#define GAME_ASSETS


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

    game_font_id_count
};

enum game_mesh_id {
    Mesh_Enemy_ID,
    Mesh_Sphere_ID,
    Mesh_Body_ID,
    //Mesh_Shield_ID,
    //Mesh_Sword_ID,
    
    game_mesh_id_count
};

enum game_animation_id {
    Animation_Walking_ID,
    Animation_Jumping_ID,

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
    double Alpha;
    double R;
    double G;
    double B;
};

color Color(double R, double G, double B, double Alpha = 1.0) {
    return { Alpha, R, G, B };
}

color Color(color Color, double Alpha) {
    return { Alpha, Color.R, Color.G, Color.B };
}

color operator*(double Luminosity, color Color) {
    return {
        Color.Alpha,
        min(Luminosity * Color.R, 1.0),
        min(Luminosity * Color.G, 1.0),
        min(Luminosity * Color.B, 1.0),
    };
}

static int Attenuation = 100;
static color Black = { 1.0, 0.0, 0.0, 0.0 };
static color White = { 1.0, 1.0, 1.0, 1.0 };
static color Gray = { 1.0, 0.5, 0.5, 0.5 };
static color DarkGray = { 1.0, 0.1, 0.1, 0.1 };
static color Red = { 1.0, 1.0, 0.0, 0.0 };
static color Green = { 1.0, 0.0, 1.0, 0.0 };
static color Blue = { 1.0, 0.0, 0.0, 1.0 };
static color Magenta = { 1.0, 1.0, 0.0, 1.0 };
static color Yellow = { 1.0, 1.0, 1.0, 0.0 };
static color Cyan = { 1.0, 0.0, 1.0, 1.0 };
static color Orange = { 1.0, 1.0, 0.63, 0.0 };
static color BackgroundBlue = { 1.0, 0.4, 0.4, 0.8 };

uint32 GetColorBytes(color Color) {
    uint8 Alpha = Color.Alpha * 255.0;
    uint8 R = Color.R * 255.0;
    uint8 G = Color.G * 255.0;
    uint8 B = Color.B * 255.0;
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

game_sound AssetLoadSound(memory_arena* Arena, game_asset* Asset) {
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

game_bitmap AssetLoadBitmap(memory_arena* Arena, game_asset* Asset) {
    game_bitmap Result = {};
    Result.ID = Asset->ID.Bitmap;
    Result.Handle = 0;

    Result = LoadBitmapFile(Arena, Asset->File);

    return Result;
}

void ClearBitmap(game_bitmap* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = Bitmap->Header.Width * Bitmap->Header.Height * 8;
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

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Heightmaps                                                                                                                                                       |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_heightmap {
    game_bitmap Bitmap;
    uint32 VAO;
    uint32 VBO;
    uint32 nVertices;
    double* Vertices;
};

const int HEIGHTMAP_RESOLUTION = 20;

uint64 ComputeNeededMemoryForHeightmap(read_file_result File) {
    uint64 BitmapSize = ComputeNeededMemoryForBitmap((bitmap_header*)File.Content);
    uint64 VerticesSize = HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION * 4 * 5 * sizeof(double);
    return BitmapSize + VerticesSize;
}

game_heightmap AssetLoadHeightmap(memory_arena* Arena, game_asset* Asset) {
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

    Result.VBO = 0;
    Result.VAO = 0;

    return Result;
}


// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Fonts                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const int FONT_CHARACTERS_COUNT = 94;

struct game_font_character {
    unsigned char Letter;
    signed long Advance;
    signed long Width;
    signed long Height;
    int Left;
    int Top;
    game_bitmap Bitmap;
};

struct game_font {
    game_font_id ID;
    signed long SpaceAdvance;
    game_font_character Characters[FONT_CHARACTERS_COUNT];
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

uint64 ComputeNeededMemoryForFont(const char* Path) {
    uint64 Result = 0;

    FT_Library FTLibrary;
    FT_Face Font;
    FT_Error error = FT_Init_FreeType(&FTLibrary);
    if (error) {
        Assert(false);
    }
    else {
        error = FT_New_Face(FTLibrary, Path, 0, &Font);
        if (error == FT_Err_Unknown_File_Format) {
            Assert(false);
        }
        else if (error) {
            Assert(false);
        }
        else {
            // Initializing char bitmaps
            int Points = 20;
            error = FT_Set_Char_Size(Font, 0, Points * 64, 128, 128);
            if (error) {
                Assert(false);
            }
            
            for (unsigned char c = '!'; c <= '~'; c++) {
                error = FT_Load_Char(Font, c, FT_LOAD_RENDER);
                if (error) {
                    Assert(false);
                }
                else {
                    FT_GlyphSlot Slot = Font->glyph;
                    FT_Bitmap FTBMP = Slot->bitmap;

                    Result += FTBMP.width * FTBMP.rows * 4;
                }
            }
        }
        FT_Done_Face(Font);
        FT_Done_FreeType(FTLibrary);
    }

    return Result;
}

game_font AssetLoadFont(memory_arena* Arena, game_asset* Asset) {
    game_font Result = {};
    Result.ID = Asset->ID.Font;

    FT_Library FTLibrary;
    FT_Face Font;
    FT_Error error = FT_Init_FreeType(&FTLibrary);
    if (error) {
        Assert(false);
    }
    else {
        error = FT_New_Face(FTLibrary, Asset->File.Path, 0, &Font);
        if (error == FT_Err_Unknown_File_Format) {
            Assert(false);
        }
        else if (error) {
            Assert(false);
        }
        else {
            // Initializing char bitmaps
            int Points = 20;
            error = FT_Set_Char_Size(Font, 0, Points * 64, 128, 128);
            if (error) {
                Assert(false);
            }

            error = FT_Load_Char(Font, ' ', FT_LOAD_RENDER);
            if (error) Assert(false);
            Result.SpaceAdvance = Font->glyph->advance.x >> 6;

            unsigned char c = '!';
            for (int i = 0; i < FONT_CHARACTERS_COUNT; i++) {
                game_font_character* pCharacter = &Result.Characters[i];
                game_bitmap* Bitmap = &pCharacter->Bitmap;
                error = FT_Load_Char(Font, c, FT_LOAD_RENDER);
                if (error) Assert(false);
                else {
                    FT_GlyphSlot Slot = Font->glyph;
                    FT_Bitmap FTBMP = Slot->bitmap;
                    *Bitmap = MakeEmptyBitmap(Arena, FTBMP.width, FTBMP.rows, true);
                    LoadFTBMP(&FTBMP, Bitmap);

                    pCharacter->Letter = c;
                    pCharacter->Advance = Slot->advance.x >> 6;
                    pCharacter->Left = Slot->bitmap_left;
                    pCharacter->Top = Slot->bitmap_top;
                    pCharacter->Height = Slot->bitmap.rows;
                    pCharacter->Width = Slot->bitmap.width;
                    pCharacter->Bitmap = *Bitmap++;
                    pCharacter->Bitmap.Handle = 0;

                    c++;
                }
            }
            FT_Done_Face(Font);
            FT_Done_FreeType(FTLibrary);
        }
    }
    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Video                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
//
//struct game_video {
//    game_video_id ID;
//    video_context VideoContext;
//    int Handle;
//    bool Loop;
//    double TimeElapsed;
//};
//
//game_video AssetLoadVideo(memory_arena* Arena, game_asset* Asset) {
//    game_video Result = { Asset->ID.Video, 0 };
//    void* Dest = PushSize(Arena, Asset->MemoryNeeded);
//    memcpy(Dest, Asset->File.Content, Asset->File.ContentSize);
//    return Result;
//}
//
//static int ReadPacket(void* Opaque, unsigned char* Buffer, int BufferSize) {
//    video_buffer* VideoBuffer = (video_buffer*)Opaque;
//    int Size = VideoBuffer->Size < BufferSize ? VideoBuffer->Size : BufferSize;
//    memcpy(Buffer, VideoBuffer->Pointer, Size);
//    VideoBuffer->Pointer += Size;
//    VideoBuffer->Size -= Size;
//    if (VideoBuffer->Size <= 0) return AVERROR_EOF;
//    else return Size;
//}
//
//int64 SeekPacket(void* Opaque, int64 Where, int Whence) {
//    video_buffer* VideoBuffer = (video_buffer*)Opaque;
//    switch (Whence) {
//        case AVSEEK_SIZE: {
//            return VideoBuffer->FullSize;
//        } break;
//
//        case SEEK_SET: {
//            if (VideoBuffer->FullSize > Where) {
//                VideoBuffer->Pointer = VideoBuffer->Start + Where;
//                VideoBuffer->Size = VideoBuffer->FullSize - Where;
//            }
//            else return EOF;
//        } break;
//
//        case SEEK_CUR: {
//            if (VideoBuffer->Size > Where) {
//                VideoBuffer->Pointer += Where;
//                VideoBuffer->Size -= Where;
//            }
//            else return EOF;
//        } break;
//
//        case SEEK_END: {
//            if (VideoBuffer->FullSize > Where) {
//                VideoBuffer->Pointer = (VideoBuffer->Start + VideoBuffer->FullSize) - Where;
//                int curPos = VideoBuffer->Pointer - VideoBuffer->Start;
//                VideoBuffer->Size = VideoBuffer->FullSize - curPos;
//            }
//            else return EOF;
//        } break;
//    }
//    return VideoBuffer->Pointer - VideoBuffer->Start;
//}
//
//void InitializeVideoBuffer(video_buffer* Buffer, void* Content, int64 FileSize) {
//    Buffer->Start = (unsigned char*)Content;
//    Buffer->Pointer = Buffer->Start;
//    Buffer->FullSize = FileSize;
//    Buffer->Size = Buffer->FullSize;
//}
//
//void InitializeVideo(video_context* VideoContext) {
//    auto& FormatContext = VideoContext->FormatContext;
//
//    FormatContext = avformat_alloc_context();
//    if (!FormatContext) {
//        throw("Format context not allocated.");
//    }
//    else {
//
//        int BufferSize = 0x8000;
//        unsigned char* pBuffer = (unsigned char*)av_malloc(BufferSize);
//        AVIOContext* IOContext = avio_alloc_context(
//            (unsigned char*)pBuffer,
//            BufferSize,
//            0,
//            &VideoContext->Buffer,
//            &ReadPacket,
//            NULL,
//            &SeekPacket
//        );
//
//        //avformat_open_input(&FormatContext, "..\\..\\GameAssets\\Assets\\Videos\\Video.mp4", NULL, NULL);
//        
//        FormatContext->pb = VideoContext->IOContext;
//        FormatContext->flags = AVFMT_FLAG_CUSTOM_IO;
//        FormatContext->iformat = av_find_input_format("mp4");
//
//        int AVError = avformat_open_input(&FormatContext, "", NULL, NULL);
//        if (AVError) {
//            char StrError[256];
//            av_strerror(AVError, StrError, 256);
//            throw("File could not be opened.");
//        }
//
//        // Finding video stream (not audio stream or others)
//        AVStream* VideoStream = 0;
//        for (unsigned int i = 0; i < FormatContext->nb_streams; i++) {
//            AVStream* Stream = FormatContext->streams[i];
//            if (Stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//                VideoStream = Stream;
//                break;
//            }
//        }
//
//        if (VideoStream == 0) throw("VideoStream not found.");
//
//        VideoContext->VideoStreamIndex = VideoStream->index;
//        VideoContext->TimeBase = (double)VideoStream->time_base.num / (double)VideoStream->time_base.den;
//
//        // Allocating resources
//            // Codec context & params
//        AVCodecParameters* CodecParams = VideoStream->codecpar;
//        const AVCodec* Codec = avcodec_find_decoder(CodecParams->codec_id);
//        if (!Codec) throw("Decoder not found.");
//
//        VideoContext->CodecContext = avcodec_alloc_context3(Codec);
//        if (!VideoContext->CodecContext) throw("Codec context could not be allocated.");
//
//        if (avcodec_parameters_to_context(VideoContext->CodecContext, CodecParams) < 0) throw("Codec params could not be loaded to context.");
//        if (avcodec_open2(VideoContext->CodecContext, Codec, NULL) < 0) throw("Codec could not be opened.");
//
//        // Packet & Frame
//        VideoContext->Packet = av_packet_alloc();
//        if (!VideoContext->Packet) throw("Packet could not be allocated.");
//
//        VideoContext->Frame = av_frame_alloc();
//        if (!VideoContext->Frame) throw("Frame could not be allocated.");
//
//        int Width = VideoContext->Frame->width;
//        int Height = VideoContext->Frame->height;
//        VideoContext->VideoOut = VirtualAlloc(0, Width * Height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//
//        return;
//    }
//}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Meshes                                                                                                                                                           |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct bone {
    int ID;
    char Name[32];
    v3 Head;
    v3 Tail;
    transform Transform;
};

const int MAX_ARMATURE_BONES = 32;
struct armature {
    int nBones;
    bone Bones[MAX_ARMATURE_BONES];
};

struct game_mesh {
    game_mesh_id ID;
    armature Armature;
    int nVertices;
    void* Vertices;
    int nFaces;
    uint32* Faces;
    uint32 VBO;
    uint32 VAO;
    uint32 EBO;
};

uint32 Parseuint32(char*& Pointer) {
    char* End = 0;
    uint32 Result = strtol(Pointer, &End, 10);
    Pointer = End;
    return Result;
}

int ParseInt(char*& Pointer) {
    char* End = 0;
    int Result = strtol(Pointer, &End, 10);
    Pointer = End;
    return Result;
}

void ParseString(char*& Pointer, char* Buffer) {
    char* Out = Buffer;
    while (*Pointer != ' ') {
        *Out++ = *Pointer++;
    }
}

v2 ParseV2(char*& Pointer) {
    char* VXEnd = 0;
    double VX = strtod(Pointer, &VXEnd);
    char* VYEnd = 0;
    double VY = strtod(VXEnd + 1, &VYEnd);
    Pointer = VYEnd;
    return V2(VX, VY);
}

v3 ParseV3(char*& Pointer) {
    char* VXEnd = 0;
    double VX = strtod(Pointer, &VXEnd);
    char* VYEnd = 0;
    double VY = strtod(VXEnd + 1, &VYEnd);
    char* VZEnd = 0;
    double VZ = strtod(VYEnd + 1, &VZEnd);
    Pointer = VZEnd;
    return V3(VX, VY, VZ);
}

v4 ParseV4(char*& Pointer) {
    char* VWEnd = 0;
    double VW = strtod(Pointer, &VWEnd);
    char* VXEnd = 0;
    double VX = strtod(VWEnd + 1, &VXEnd);
    char* VYEnd = 0;
    double VY = strtod(VXEnd + 1, &VYEnd);
    char* VZEnd = 0;
    double VZ = strtod(VYEnd + 1, &VZEnd);
    Pointer = VZEnd;
    return V4(VX, VY, VZ, VW);
}

iv2 ParseIV2(char*& Pointer) {
    char* VXEnd = 0;
    int VX = strtol(Pointer, &VXEnd, 10);
    char* VYEnd = 0;
    int VY = strtol(VXEnd + 1, &VYEnd, 10);
    Pointer = VYEnd;
    return IV2(VX, VY);
}

iv3 ParseIV3(char*& Pointer) {
    char* VXEnd = 0;
    int VX = strtol(Pointer, &VXEnd, 10);
    char* VYEnd = 0;
    int VY = strtol(VXEnd + 1, &VYEnd, 10);
    char* VZEnd = 0;
    int VZ = strtol(VYEnd + 1, &VZEnd, 10);
    Pointer = VZEnd;
    return IV3(VX, VY, VZ);
}

void GetMeshSizes(char* Text, int* nVertices, int* nFaces, int* nBones) {
    Assert(Text[0] == 'n' && Text[1] == 'V' && Text[2] == ' ');
    Text += 3;

    char *nVEnd, *nFEnd, *nBEnd = 0;
    *nVertices = strtol(Text, &nVEnd, 10);

    while (Text++ != nVEnd);

    Assert(Text[0] == 'n' && Text[1] == 'F' && Text[2] == ' ');
    Text += 3;

    *nFaces = strtol(Text, &nFEnd, 10);

    while (++Text != nFEnd);

    if (
        Text[0] == '\n' || 
        (Text[0] == '\r' && Text[1] == '\n')
    ) *nBones = 0;
    else {
        Text++;
        Assert(Text[0] == 'n' && Text[1] == 'B' && Text[2] == ' ');
        Text += 3;
        *nBones = strtol(Text, &nBEnd, 10);
    }
}

uint64 ComputeNeededMemoryForMesh(read_file_result File) {
    uint64 Result = 0;

    if (File.ContentSize > 0) {
        char* Pointer = (char*)File.Content;

        int nVertices = 0, nFaces = 0, nBones = 0;
        GetMeshSizes(Pointer, &nVertices, &nFaces, &nBones);

        // We need eight doubles for each combination of vertex, texture, normal (v3, v2, v3) and
        // 3 unsigned integers for each triangle. Also, two ints for bone ids and two doubles for bone weights if armature is present
        if (nBones > 0) Result = nVertices * (10 * sizeof(double) + 2 * sizeof(uint32));
        else Result = nVertices * 8 * sizeof(double);
        Result += nFaces * 3 * sizeof(uint32);
    }
    return Result;
}

game_mesh AssetLoadMesh(memory_arena* Arena, game_asset* Asset) {
    game_mesh Result = {};
    Result.ID = Asset->ID.Mesh;
    
    read_file_result File = Asset->File;

    if (File.ContentSize > 0) {
        char* Pointer = (char*)File.Content;

        int nVertices, nFaces, nBones = 0;
        GetMeshSizes(Pointer, &nVertices, &nFaces, &nBones);

        Result.nVertices = nVertices;
        Result.nFaces = nFaces;
        Result.Armature.nBones = nBones;

        int VerticesSize = nBones > 0 ? 10 * sizeof(double) + 2 * sizeof(int) : 8 * sizeof(double);
        Result.Vertices = PushSize(Arena, nVertices * VerticesSize);
        Result.Faces = PushArray(Arena, 3 * Result.nFaces, uint32);

        // Skip until vertices
        while (*Pointer++ != '\n');

        double* pOutV = (double*)Result.Vertices;
        for (int i = 0; i < Result.nVertices; i++) {
            v3 Position = ParseV3(Pointer);
            Pointer++;
            v3 Normal = ParseV3(Pointer);
            Pointer++;
            v2 Texture = ParseV2(Pointer);
            Pointer++;

            *pOutV++ = Position.X;
            *pOutV++ = Position.Y;
            *pOutV++ = Position.Z;

            *pOutV++ = Texture.X;
            *pOutV++ = Texture.Y;
            
            *pOutV++ = Normal.X;
            *pOutV++ = Normal.Y;
            *pOutV++ = Normal.Z;
            
            if (nBones > 0) {
                iv2 BoneIDs = ParseIV2(Pointer);
                Pointer++;
                v2 Weights = ParseV2(Pointer);
                Pointer++;
                int* pOutB = (int*)pOutV;
                *pOutB++ = BoneIDs.X;
                *pOutB++ = BoneIDs.Y;

                pOutV = (double*)pOutB;
                *pOutV++ = Weights.X;
                *pOutV++ = Weights.Y;
            }
        }

        uint32* pOutF = Result.Faces;
        for (int i = 0; i < Result.nFaces; i++) {
            iv3 Face = ParseIV3(Pointer);
            *pOutF++ = Face.X;
            *pOutF++ = Face.Y;
            *pOutF++ = Face.Z;
            Pointer++;
        }

        for (int i = 0; i < Result.Armature.nBones; i++) {
            bone Bone = {};
            Bone.ID = Parseuint32(Pointer);
            Pointer++;
            ParseString(Pointer, Bone.Name);
            Pointer++;
            Bone.Head = ParseV3(Pointer);
            Pointer++;
            Bone.Tail = ParseV3(Pointer);
            Pointer++;
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
    int nFrames;
    int nBones;
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

void GetAnimationSizes(char* Text, int* nFrames, int* nBones) {
    Assert(Text[0] == 'n' && Text[1] == 'F' && Text[2] == ' ');
    Text += 3;

    char *nFEnd, *nBEnd = 0;
    *nFrames = strtol(Text, &nFEnd, 10);

    while (Text++ != nFEnd);

    Assert(Text[0] == 'n' && Text[1] == 'B' && Text[2] == ' ');
    Text += 3;

    *nBones = strtol(Text, &nBEnd, 10);
}

uint64 ComputeNeededMemoryForAnimation(read_file_result File) {
    uint64 Result = 0;

    if (File.ContentSize > 0) {
        char* Pointer = (char*)File.Content;

        int nFrames = 0, nBones = 0;
        GetAnimationSizes(Pointer, &nFrames, &nBones);

        Result = nFrames * nBones * 10 * sizeof(float);
    }
    return Result;
}

game_animation AssetLoadAnimation(memory_arena* Arena, game_asset* Asset) {
    game_animation Result = {};
    Result.ID = Asset->ID.Animation;

    GetAnimationSizes((char*)Asset->File.Content, &Result.nFrames, &Result.nBones);

    Result.Content = PushArray(Arena, Result.nFrames * Result.nBones * 10, float);

    float* pOut = Result.Content;
    char* Pointer = (char*)Asset->File.Content;
    while (*Pointer++ != '\n');

    for (uint32 i = 0; i < Result.nFrames; i++) {
        for (uint32 j = 0; j < Result.nBones; j++) {
            uint32 Frame = Parseuint32(Pointer);
            Pointer++;
            uint32 BoneID = Parseuint32(Pointer);
            Pointer++;
            Assert(Frame == i && BoneID == j);

            v3 Translation = ParseV3(Pointer);
            Pointer++;
            v4 Rotation = ParseV4(Pointer);
            Pointer++;
            v3 Scale = ParseV3(Pointer);
            Pointer++;

            *pOut++ = Translation.X;
            *pOut++ = Translation.Y;
            *pOut++ = Translation.Z;
            *pOut++ = Rotation.W;
            *pOut++ = Rotation.X;
            *pOut++ = Rotation.Y;
            *pOut++ = Rotation.Z;
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
    Vertex_Shader_Perspective_ID,
    Vertex_Shader_Tessellation_ID,

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

    game_shader_id_count
};

struct game_shader {
    game_shader_id ID;
    game_shader_type Type;
    uint32 ShaderID;
    uint32 Size;
    char* Code;
};

enum game_shader_pipeline_id {
    Shader_Pipeline_Antialiasing_ID,
    Shader_Pipeline_Framebuffer_ID,
    Shader_Pipeline_Single_Color_ID,
    Shader_Pipeline_Texture_ID,
    Shader_Pipeline_Mesh_ID,
    Shader_Pipeline_Sphere_ID,
    Shader_Pipeline_Jump_Flood_ID,
    Shader_Pipeline_Outline_ID,
    Shader_Pipeline_Heightmap_ID,
    Shader_Pipeline_Trochoidal_ID,
    Shader_Pipeline_Debug_Normals_ID,

    game_shader_pipeline_id_count
};

struct game_shader_pipeline {
    game_shader_pipeline_id ID;
    uint32 ProgramID;
    game_shader_id Pipeline[game_shader_type_count];
    bool IsProvided[game_shader_type_count];
};

enum game_compute_shader_id {
    Outline_Init_Compute_Shader_ID,
    //Jump_Flood_Compute_Shader_ID,
    Test_Compute_Shader_ID,

    game_compute_shader_id_count
};

struct game_compute_shader {
    game_compute_shader_id ID;
    uint32 ShaderID;
    uint32 ProgramID;
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

struct game_assets {
    game_asset Asset[ASSET_COUNT];
    int nAssets;
    game_text Text[game_text_id_count];
    game_bitmap Bitmap[game_bitmap_id_count];
    game_heightmap Heightmap[game_heightmap_id_count];
    game_font Font[game_font_id_count];
    game_sound Sound[game_sound_id_count];
    game_mesh Mesh[game_mesh_id_count];
    game_animation Animation[game_animation_id_count];
    uint64 AssetsSize;
    //game_video Videos[1];
    int nShaders;
    game_shader Shader[game_shader_id_count];
    int nShaderPipelines;
    game_shader_pipeline ShaderPipeline[game_shader_pipeline_id_count];
    int nComputeShaders;
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
//game_video* GetAsset(game_assets* Assets, game_video_id ID) { return &Assets->Videos[ID]; }

void PushAsset(game_assets* Assets, const char* Path, game_text_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Text;
    Asset->ID.Text = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    Asset->MemoryNeeded = Asset->File.ContentSize + 1;
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_sound_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Sound;
    Asset->ID.Sound = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    Asset->MemoryNeeded = ComputeNeededMemoryForSound(Asset->File);
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_bitmap_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Bitmap;
    Asset->ID.Bitmap = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    Asset->MemoryNeeded = ComputeNeededMemoryForBitmap((bitmap_header*)Asset->File.Content);
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_heightmap_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Heightmap;
    Asset->ID.Heightmap = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    Asset->MemoryNeeded = ComputeNeededMemoryForHeightmap(Asset->File);
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_font_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Font;
    Asset->ID.Font = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    Asset->MemoryNeeded = ComputeNeededMemoryForFont(Asset->File.Path);
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_mesh_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Mesh;
    Asset->ID.Mesh = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    Asset->MemoryNeeded = ComputeNeededMemoryForMesh(Asset->File);
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_animation_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Animation;
    Asset->ID.Animation = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    Asset->MemoryNeeded = ComputeNeededMemoryForAnimation(Asset->File);
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_video_id ID) {
    game_asset* Asset = &Assets->Asset[Assets->nAssets++];
    Asset->Type = Video;
    Asset->ID.Video = ID;
    Asset->File = PlatformReadEntireFile(Path);
    Assert(Asset->File.ContentSize > 0);
    //Asset.MemoryNeeded = ComputeNeededMemoryForVideo(Asset.File);
    Asset->MemoryNeeded = 0;
    Assets->TotalSize += Asset->MemoryNeeded;
    Assets->AssetsSize += Asset->MemoryNeeded;
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
            sprintf_s(LogBuffer, "Loaded text %s\n", Asset->File.Path);
        } break;

        //case Video: {
        //    Assets->Videos[Asset->Index] = AssetLoadVideo(Arena, Asset);
        //    std::cout << "Loaded video " << Asset->Path << "\n";
        //} break;

        case Bitmap: {
            Assets->Bitmap[ID.Bitmap] = AssetLoadBitmap(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded bitmap %s\n", Asset->File.Path);
        } break;

        case Heightmap: {
            Assets->Heightmap[ID.Heightmap] = AssetLoadHeightmap(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded heightmap %s\n", Asset->File.Path);
        } break;

        case Font: {
            Assets->Font[ID.Font] = AssetLoadFont(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded font %s\n", Asset->File.Path);
        } break;

        case Sound: {
            Assets->Sound[ID.Sound] = AssetLoadSound(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded sound %s\n", Asset->File.Path);
        } break;

        case Mesh: {
            Assets->Mesh[ID.Mesh] = AssetLoadMesh(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded mesh %s\n", Asset->File.Path);
        } break;

        case Animation: {
            Assets->Animation[ID.Animation] = AssetLoadAnimation(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded animation %s\n", Asset->File.Path);
        }

        default: {
            sprintf_s(LogBuffer, "Asset ignored %s\n", Asset->File.Path);
        }
    }

    Log(Info, LogBuffer);
    Assert(Asset->MemoryNeeded == Arena->Used - Asset->Offset);
    PlatformFreeFileMemory(Asset->File.Content);
    Asset->File = { 0 };
}

game_shader*          GetShader        (game_assets* Assets, game_shader_id ID)          { return &Assets->Shader[ID]; }
game_shader_pipeline* GetShaderPipeline(game_assets* Assets, game_shader_pipeline_id ID) { return &Assets->ShaderPipeline[ID]; }
game_compute_shader*  GetComputeShader (game_assets* Assets, game_compute_shader_id ID)  { return &Assets->ComputeShader[ID]; }

void PushShader(game_assets* Assets, const char* Path, game_shader_id ID) {
    game_shader* Shader = GetShader(Assets, ID);
    Shader->ID = ID;

    WIN32_FIND_DATAA Data;
    HANDLE hFind = FindFirstFileA(Path, &Data);

    if (hFind == INVALID_HANDLE_VALUE) Assert(false);

    char* Extension = 0;
    char* Buffer = new char[strlen(Data.cFileName) + 1];
    strcpy_s(Buffer, strlen(Data.cFileName) + 1, Data.cFileName);
    char* _ = strtok_s(Buffer, ".", &Extension);

    if (Extension != 0) {
        if      (strcmp(Extension, "frag.glsl") == 0) { Shader->Type = Fragment_Shader; }
        else if (strcmp(Extension, "vert.glsl") == 0) { Shader->Type = Vertex_Shader; }
        else if (strcmp(Extension, "geom.glsl") == 0) { Shader->Type = Geometry_Shader; }
        else if (strcmp(Extension, "tcs.glsl")  == 0) { Shader->Type = Tessellation_Control_Shader; }
        else if (strcmp(Extension, "tes.glsl")  == 0) { Shader->Type = Tessellation_Evaluation_Shader; }
        else Assert(false);
    }
    else Assert(false);

    read_file_result ShaderFile = PlatformReadEntireFile(Path);
    Shader->Size = ShaderFile.ContentSize;
    Shader->Code = (char*)ShaderFile.Content;

    // Extra char with value 0 to separate shaders
    Assets->TotalSize += Shader->Size + 1;
    Assets->ShadersSize += Shader->Size + 1;
    Assets->nShaders++;
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

        if (ShaderPipeline->IsProvided[Shader->Type]) throw("Shader of this type has alredy been attached to pipeline.");
        else {
            ShaderPipeline->IsProvided[Shader->Type] = true;
            ShaderPipeline->Pipeline[Shader->Type] = Shader->ID;
        }
    }

    Assets->nShaderPipelines++;
}

void PushShader(game_assets* Assets, const char* Path, game_compute_shader_id ID) {
    game_compute_shader* Shader = GetComputeShader(Assets, ID);
    Shader->ID = ID;

    WIN32_FIND_DATAA Data;
    HANDLE hFind = FindFirstFileA(Path, &Data);

    if (hFind == INVALID_HANDLE_VALUE) Assert(false);

    char* Extension = 0;
    char* Buffer = new char[strlen(Data.cFileName) + 1];
    strcpy_s(Buffer, strlen(Data.cFileName) + 1, Data.cFileName);
    char* _ = strtok_s(Buffer, ".", &Extension);

    if (strcmp(Extension, "comp.glsl") != 0) {
        throw("Extension of compute shader file should be 'comp.glsl'.");
    }
    else {
        read_file_result ShaderFile = PlatformReadEntireFile(Path);
        Shader->Size = ShaderFile.ContentSize;
        Shader->Code = (char*)ShaderFile.Content;

        // Extra char with value 0 to separate shaders
        Assets->TotalSize += Shader->Size + 1;
        Assets->ComputeShadersSize += Shader->Size + 1;
        Assets->nComputeShaders++;
    }
}

void LoadShader(memory_arena* Arena, game_shader* Shader) {
    char* Destination = (char*)PushSize(Arena, Shader->Size + 1);
    memcpy(Destination, Shader->Code, Shader->Size);
    PlatformFreeFileMemory(Shader->Code);
}

void LoadComputeShader(memory_arena* Arena, game_compute_shader* Shader) {
    char* Destination = (char*)PushSize(Arena, Shader->Size + 1);
    memcpy(Destination, Shader->Code, Shader->Size);
    PlatformFreeFileMemory(Shader->Code);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Assets file                                                                                                                                                      |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void LoadAssetsFromFile(platform_read_entire_file Read, game_assets* Assets, const char* Path) {
    read_file_result AssetsFile = Read(Path);

    *Assets = *(game_assets*)AssetsFile.Content;
    Assets->Memory = (uint8*)AssetsFile.Content + sizeof(game_assets);

    for (int i = 0; i < ASSET_COUNT; i++) {
        game_asset Asset = Assets->Asset[i];

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
                uint8* Pointer = (uint8*)(Assets->Memory + Asset.Offset);
                for (int i = 0; i < FONT_CHARACTERS_COUNT; i++) {
                    game_bitmap* Character = &Font->Characters[i].Bitmap;
                    Character->Content = (uint32*)Pointer;
                    Pointer += Character->Header.Width * Character->Header.Height * Character->BytesPerPixel;
                }
            } break;

            case Mesh: {
                game_mesh* Mesh = GetAsset(Assets, Asset.ID.Mesh);
                Mesh->Vertices = (double*)(Assets->Memory + Asset.Offset);
                int VertexSize = Mesh->Armature.nBones > 0 ? 10 * sizeof(double) + 2 * sizeof(uint32) : 8 * sizeof(double);
                Mesh->Faces = (uint32*)((uint8*)Mesh->Vertices + VertexSize * Mesh->nVertices);
            } break;

            case Animation: {
                game_animation* Animation = GetAsset(Assets, Asset.ID.Animation);
                Animation->Content = (float*)(Assets->Memory + Asset.Offset);
            } break;

            //case Video: {
            //    game_video* Video = &Assets->Videos[Asset.Index];
            //    InitializeVideoBuffer(&Video->VideoContext.Buffer, (void*)(Assets->Memory + Asset.Offset), Asset.MemoryNeeded - 1);
            //    InitializeVideo(&Video->VideoContext);
            //} break;

            default: {
                Log(Error, "ERROR: Asset type not implemented.\n");
                Assert(false);
            }
        }
    }

    Log(Info, "Assets loaded.\n");

    char* Pointer = (char*)(Assets->Memory + Assets->AssetsSize);
    for (int i = 0; i < Assets->nShaders; i++) {
        game_shader* Shader = &Assets->Shader[i];

        Shader->Code = Pointer;
        Pointer += Shader->Size + 1;
    }

    for (int i = 0; i < Assets->nComputeShaders; i++) {
        game_compute_shader* Shader = &Assets->ComputeShader[i];

        Shader->Code = Pointer;
        Pointer += Shader->Size + 1;
    }

    Log(Info, "Shaders loaded.\n");
}

void WriteAssetFile() {
    game_assets Assets = {};

// Assets
    // Fonts
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Fonts\\CascadiaMono.ttf", Font_Cascadia_Mono_ID);

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
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Enemy.mdl", Mesh_Enemy_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Sphere.mdl", Mesh_Sphere_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Models\\Body.mdl", Mesh_Body_ID);

    // Animation
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Animations\\Walking.anim", Animation_Walking_ID);
    PushAsset(&Assets, "..\\GameAssets\\Assets\\Animations\\Jumping.anim", Animation_Jumping_ID);

    // Video
    //PushAsset(&Assets, "..\\Assets\\Videos\\The Witness.mp4", Video_Test_ID);
    
    Assert(Assets.nAssets == ASSET_COUNT);

// Shaders
    // Vertex
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\Passthrough.vert.glsl", Vertex_Shader_Passthrough_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\Perspective.vert.glsl", Vertex_Shader_Perspective_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Vertex\\Tessellation.vert.glsl", Vertex_Shader_Tessellation_ID);

    // Fragment
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Antialiasing.frag.glsl", Fragment_Shader_Antialiasing_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\FramebufferAttachment.frag.glsl", Fragment_Shader_Framebuffer_Attachment_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Texture.frag.glsl", Fragment_Shader_Texture_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Outline.frag.glsl", Fragment_Shader_Outline_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\SingleColor.frag.glsl", Fragment_Shader_Single_Color_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Kernel.frag.glsl", Fragment_Shader_Kernel_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Sphere.frag.glsl", Fragment_Shader_Sphere_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Mesh.frag.glsl", Fragment_Shader_Mesh_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\JumpFlood.frag.glsl", Fragment_Shader_Jump_Flood_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Heightmap.frag.glsl", Fragment_Shader_Heightmap_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Fragment\\Sea.frag.glsl", Fragment_Shader_Sea_ID);

    // Geometry
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Geometry\\Test.geom.glsl", Geometry_Shader_Test_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Geometry\\DebugNormals.geom.glsl", Geometry_Shader_Debug_Normals_ID);

    // Tessellation
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Tessellation\\Tessellation.tcs.glsl", Tessellation_Control_Shader_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Tessellation\\Tessellation.tes.glsl", Tessellation_Evaluation_Shader_Standard_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Tessellation\\Trochoidal.tes.glsl", Tessellation_Evaluation_Shader_Trochoidal_ID);

    Assert(Assets.nShaders == game_shader_id_count);

    // Shader pipelines
    PushShaderPipeline(&Assets, Shader_Pipeline_Antialiasing_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Antialiasing_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Framebuffer_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Framebuffer_Attachment_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Texture_ID, 2, Vertex_Shader_Perspective_ID, Fragment_Shader_Texture_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Mesh_ID, 2, Vertex_Shader_Perspective_ID, Fragment_Shader_Mesh_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Sphere_ID, 2, Vertex_Shader_Perspective_ID, Fragment_Shader_Sphere_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Single_Color_ID, 2, Vertex_Shader_Perspective_ID, Fragment_Shader_Single_Color_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Outline_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Outline_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Jump_Flood_ID, 2, Vertex_Shader_Passthrough_ID, Fragment_Shader_Jump_Flood_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Debug_Normals_ID, 3, 
        Vertex_Shader_Perspective_ID, 
        Geometry_Shader_Debug_Normals_ID, 
        Fragment_Shader_Single_Color_ID
    );
    //PushShaderPipeline(&Assets, Shader_Pipeline_Kernel_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Kernel_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Heightmap_ID, 4, 
        Vertex_Shader_Tessellation_ID, 
        Tessellation_Control_Shader_ID, 
        Tessellation_Evaluation_Shader_Standard_ID, 
        Fragment_Shader_Heightmap_ID
    );
    PushShaderPipeline(&Assets, Shader_Pipeline_Trochoidal_ID, 4,
        Vertex_Shader_Tessellation_ID,
        Tessellation_Control_Shader_ID,
        Tessellation_Evaluation_Shader_Trochoidal_ID,
        Fragment_Shader_Sea_ID
    );
    
    // Compute
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Compute\\OutlineInit.comp.glsl", Outline_Init_Compute_Shader_ID);
    //PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Compute\\JumpFlood.comp.glsl", Jump_Flood_Compute_Shader_ID);
    PushShader(&Assets, "..\\GameAssets\\Assets\\Shaders\\Compute\\Test.comp.glsl", Test_Compute_Shader_ID);

    Assert(Assets.nShaderPipelines == game_shader_pipeline_id_count);

// Output file
    void* FileMemory = VirtualAlloc(0, sizeof(game_assets) + Assets.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Assets.Memory = (uint8*)FileMemory + sizeof(game_assets);
    memory_arena AssetArena;
    InitializeArena(&AssetArena, Assets.TotalSize, (uint8*)Assets.Memory);

    // Assets
    for (int i = 0; i < Assets.nAssets; i++) {
        LoadAsset(&AssetArena, &Assets, &Assets.Asset[i]);
    }

    // Shaders
    for (int i = 0; i < game_shader_id_count; i++) {
        LoadShader(&AssetArena, &Assets.Shader[i]);
    }

    // Compute shaders
    for (int i = 0; i < game_compute_shader_id_count; i++) {
        LoadComputeShader(&AssetArena, &Assets.ComputeShader[i]);
    }

    game_assets* OutputAssets = (game_assets*)FileMemory;
    if (OutputAssets) *OutputAssets = Assets;
    PlatformWriteEntireFile("..\\GameAssets\\game_assets", sizeof(game_assets) + Assets.TotalSize, FileMemory);

    Log(Info, "Finished writing assets file.\n");
}

#endif