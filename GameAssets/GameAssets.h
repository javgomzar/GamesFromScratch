// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H

// FFMPEG
#include "..\Linking\include\FFMpeg.h"

#include "..\GameLibrary\GameMath.h"
#include "../Win32PlatformLayer/Win32PlatformLayer.h"

#include <set>
#include <vector>
#include <map>

#ifndef GAME_ASSETS
#define GAME_ASSETS


enum game_asset_type {
    Text,
    Bitmap,
    Font,
    Sound,
    Video,
    Mesh,

    game_asset_type_count
};

enum game_asset_id {
    Text_Test_ID,

    Bitmap_Background_ID,
    Bitmap_Button_ID,
    Bitmap_Empty_ID,
    Bitmap_Enemy_ID,
    Bitmap_Player_ID,

    Font_Cascadia_Mono_ID,

    Sound_Test_ID,

    Mesh_Enemy_ID,
    //Mesh_Shield_ID,
    //Mesh_Sphere_ID,
    //Mesh_Sword_ID,

    //Video_Test_ID,

    game_asset_id_count
};

const int COUNT_TEXT_ASSETS = 1;
const int COUNT_BITMAP_ASSETS = 5;
const int COUNT_FONT_ASSETS = 1;
const int COUNT_SOUND_ASSETS = 1;
const int COUNT_MESH_ASSETS = 4;
const int COUNT_VIDEO_ASSETS = 1;

struct game_asset {
    game_asset_id ID;
    int Index;
    game_asset_type Type;
    read_file_result File;
    const char* Path;
    uint64 MemoryNeeded;
    uint64 Offset;
};

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

    Asset->Offset = Arena->Used;
    game_sound Result = { 0 };
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
#pragma pack(pop)

struct game_bitmap {
    bitmap_header Header;
    uint32 Handle;
    uint32 BytesPerPixel;
    uint32 Pitch;
    uint32 AlphaMask;
    uint32* Content;
};

game_bitmap AssetLoadBitmap(memory_arena* Arena, game_asset* Asset) {
    game_bitmap Result = { 0 };
    Result.Handle = 0;

    bitmap_header* Header = (bitmap_header*)Asset->File.Content;
    Result.Header = *Header;
    uint32 BytesPerPixel = Header->BitsPerPixel >> 3;
    Result.BytesPerPixel = BytesPerPixel;
    Result.Pitch = Header->Width * BytesPerPixel;
    Result.Content = (uint32*)((uint8*)Asset->File.Content + Header->BitmapOffset);

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

    uint64 Size = BytesPerPixel * Header->Width * Header->Height;
    Asset->Offset = Arena->Used;
    void* Destination = PushSize(Arena, Size);

    memcpy(Destination, Result.Content, Size);
    Result.Content = 0;

    return Result;
}

void ClearBitmap(game_bitmap* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = Bitmap->Header.Width * Bitmap->Header.Height * 8;
        ZeroSize(TotalBitmapSize, Bitmap->Content);
    }
}

game_bitmap MakeEmptyBitmap(memory_arena* Arena, int32 Width, int32 Height, bool ClearToZero = true) {
    game_bitmap Result = { 0 };
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

    FT_Library FTLibrary;
    FT_Face Font;
    FT_Error error = FT_Init_FreeType(&FTLibrary);
    if (error) {
        Assert(false);
    }
    else {
        error = FT_New_Face(FTLibrary, Asset->Path, 0, &Font);
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

            Asset->Offset = Arena->Used;
            unsigned char c = '!';
            for (int i = 0; i < FONT_CHARACTERS_COUNT; i++) {
                game_font_character* pCharacter = &Result.Characters[i];
                game_bitmap* CharacterBMP = &pCharacter->Bitmap;
                error = FT_Load_Char(Font, c, FT_LOAD_RENDER);
                if (error) {
                    Assert(false);
                }
                else {
                    FT_GlyphSlot Slot = Font->glyph;
                    FT_Bitmap FTBMP = Slot->bitmap;
                    *CharacterBMP = MakeEmptyBitmap(Arena, FTBMP.width, FTBMP.rows, true);
                    LoadFTBMP(&FTBMP, CharacterBMP);

                    pCharacter->Letter = c;
                    pCharacter->Advance = Slot->advance.x >> 6;
                    pCharacter->Left = Slot->bitmap_left;
                    pCharacter->Top = Slot->bitmap_top;
                    pCharacter->Height = Slot->metrics.height;
                    pCharacter->Width = Slot->metrics.width;
                    pCharacter->Bitmap = *CharacterBMP++;
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

struct game_video {
    video_context* VideoContext;
    int Handle;
    bool Loop;
    double TimeElapsed;
};

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Meshes                                                                                                                                                           |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct vertex {
    v3 Vertex;
    v3 Normal;
    v2 Texture;
};

struct game_mesh {
    int nVertices;
    double* Vertices;
    int nFaces;
    uint32* Faces;
    uint32 VBO;
    uint32 VAO;
    uint32 EBO;
    game_bitmap* Texture;
};

v2 ParseV2(char* Pointer) {
    char* VXEnd = 0;
    double VX = strtod(Pointer, &VXEnd);
    char* VYEnd = 0;
    double VY = strtod(VXEnd + 1, &VYEnd);
    return V2(VX, VY);
}

v3 ParseV3(char* Pointer) {
    char* VXEnd = 0;
    double VX = strtod(Pointer, &VXEnd);
    char* VYEnd = 0;
    double VY = strtod(VXEnd + 1, &VYEnd);
    char* VZEnd = 0;
    double VZ = strtod(VYEnd + 1, &VZEnd);
    return V3(VX, VY, VZ);
}

iv3 ParseIV3(char* Pointer) {
    char* VXEnd = 0;
    int VX = strtol(Pointer, &VXEnd, 10);
    char* VYEnd = 0;
    int VY = strtol(VXEnd + 1, &VYEnd, 10);
    char* VZEnd = 0;
    int VZ = strtol(VYEnd + 1, &VZEnd, 10);
    return IV3(VX, VY, VZ);
}

uint64 ComputeNeededMemoryForMesh(read_file_result File) {
    uint64 Result = 0;

    if (File.ContentSize > 0) {
        char* Pointer = (char*)File.Content;
        uint32 ReadSize = 0;

        // Skip until faces
        char ReadChar = *Pointer;
        while (ReadChar != 'f') {
            while (ReadChar != '\n') {
                ReadChar = *Pointer++; ReadSize++;
            }
            ReadChar = *Pointer++; ReadSize++;
        }
        ReadChar = *Pointer++; ReadSize++;

        // Parse faces
        std::set<iv3> Faces = {};
        int nFaces = 0;

        while (ReadSize < File.ContentSize) {
            int nFaceVertices = 0;
            while (*Pointer != '\n') {
                iv3 V = ParseIV3(Pointer);
                Faces.insert(V);
                nFaceVertices++;
                while (*Pointer++ != ' ' && *Pointer != '\n') { ReadSize++; };
                ReadSize++;
            }
            // Number of triangles in a polygon
            if (nFaceVertices > 2) nFaces += nFaceVertices - 2;
            Pointer += 3; ReadSize += 3;
        }

        // We need eight doubles for each combination of vertex, normal, texture (v3, v3, v2) and
        // 3 integers for each triangle.
        Result = Faces.size() * 8 * sizeof(double) + nFaces * 3 * sizeof(int);
    }
    return Result;
}

game_mesh AssetLoadMesh(memory_arena* Arena, game_asset* Asset) {
    game_mesh Result = { 0 };
    
    read_file_result File = Asset->File;

    if (File.ContentSize > 0) {
        char* Pointer = (char*)File.Content;
        uint32 ReadSize = 0;

        // Skip until vertices
        char ReadChar = *Pointer;
        while (ReadChar != 'v') {
            while (ReadChar != '\n') {
                ReadChar = *Pointer++; ReadSize++;
            }
            ReadChar = *Pointer;
        }

        // Parse vertices
        std::vector<v3> Vertices = {};
        char NextChar = *(Pointer + 1);
        while (ReadChar == 'v' && NextChar == ' ') {
            Pointer += 2; ReadSize += 2;
            v3 V = ParseV3(Pointer);
            Vertices.push_back(V);
            while (ReadChar != '\n') {
                ReadChar = *Pointer++; 
                ReadSize++;
            }
            ReadChar = *Pointer;
            NextChar = *(Pointer + 1);
        }

        // Parse texture vertices
        std::vector<v2> Textures = {};
        while (ReadChar == 'v' && NextChar == 't') {
            Pointer += 3; ReadSize += 3;
            v2 V = ParseV2(Pointer);
            Textures.push_back(V);
            while (ReadChar != '\n') {
                ReadChar = *Pointer++;
                ReadSize++;
            }
            ReadChar = *Pointer;
            NextChar = *(Pointer + 1);
        }

        // Parse normals
        std::vector<v3> Normals = {};
        while (ReadChar == 'v' && NextChar == 'n') {
            Pointer += 3; ReadSize += 3;
            v3 V = ParseV3(Pointer);
            Normals.push_back(V);
            while (ReadChar != '\n') {
                ReadChar = *Pointer++;
                ReadSize++; 
            }
            ReadChar = *Pointer;
            NextChar = *(Pointer + 1);
        }

        // Skip until faces
        while (ReadChar != 'f') {
            while (ReadChar != '\n') {
                ReadChar = *Pointer++; ReadSize++;
            }
            ReadChar = *Pointer++; ReadSize++;
        }
        Pointer++; ReadSize++;
        ReadChar = *Pointer;

        // Parse faces
        std::set<iv3> FaceVertices = {};
        std::vector<iv3> Triangles = {};
        while (ReadSize < File.ContentSize) {
            int nFaceVertices = 0;
            iv3 Triangle[3] = {};
            while (ReadChar != '\n' && ReadSize < File.ContentSize) {
                iv3 V = ParseIV3(Pointer);
                if (V == IV3(26, 85, 30)) {
                    OutputDebugStringA("ERROR");
                }
                FaceVertices.insert(V);
                if (nFaceVertices <= 2) {
                    Triangle[nFaceVertices] = V;
                }
                nFaceVertices++;
                if (nFaceVertices > 3) {
                    Triangle[1] = Triangle[2];
                    Triangle[2] = V;
                }
                if (nFaceVertices >= 3) {
                    Triangles.push_back(Triangle[0]);
                    Triangles.push_back(Triangle[1]);
                    Triangles.push_back(Triangle[2]);
                }
                while (ReadChar != ' ' && ReadChar != '\n' && ReadSize < File.ContentSize) { ReadChar = *++Pointer; ReadSize++; };
                if (ReadChar == ' ') {
                    ReadChar = *++Pointer; ReadSize++;
                }
            }
            Pointer += 3; ReadSize += 3;
            if (ReadSize < File.ContentSize) {
                ReadChar = *Pointer;
            }
        }

        // Push mesh
        Result.nVertices = FaceVertices.size();
        Asset->Offset = Arena->Used;
        Result.Vertices = PushArray(Arena, 5 * Result.nVertices, double);
        std::set<iv3>::iterator itr;
        std::map<iv3, int> dict = {};
        double* pOutputVertex = Result.Vertices;
        int n = 0;
        for (itr = FaceVertices.begin(); itr != FaceVertices.end(); itr++) {
            iv3 FaceVertex = *itr;
            dict[FaceVertex] = n++;
            v3 Vertex = Vertices.at(FaceVertex.X-1);
            v2 Texture = Textures.at(FaceVertex.Y-1);
            v3 Normal = Normals.at(FaceVertex.Z-1);
            *pOutputVertex++ = Vertex.X;
            *pOutputVertex++ = Vertex.Y;
            *pOutputVertex++ = Vertex.Z;
            *pOutputVertex++ = Normal.X;
            *pOutputVertex++ = Normal.Y;
            *pOutputVertex++ = Normal.Z;
            *pOutputVertex++ = Texture.X;
            *pOutputVertex++ = Texture.Y;
        }

        for (const iv3& v : Triangles) {
            *pOutputVertex++ = dict[v];
        }

        Result.nFaces = Triangles.size();
        Result.Faces = PushArray(Arena, 3 * Result.nFaces, uint32);
    }

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders                                                                                                                                                          |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_shader {
    uint32 ProgramID;
    read_file_result HeaderShaderCode;
    read_file_result VertexShaderCode;
    read_file_result FragmentShaderCode;
};

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Game assets                                                                                                                                                      |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_assets {
    game_asset Asset[game_asset_id_count];
    int n[game_asset_type_count];
    char* Texts[COUNT_TEXT_ASSETS];
    game_bitmap Bitmaps[COUNT_BITMAP_ASSETS];
    game_font Fonts[COUNT_FONT_ASSETS];
    game_sound Sounds[COUNT_SOUND_ASSETS];
    game_mesh Meshes[COUNT_MESH_ASSETS];
    game_video Videos[COUNT_VIDEO_ASSETS];
    uint64 TotalSize;
    uint8* Memory;
};

//character* Characters;
//loaded_bmp TestImage;
//loaded_bmp TestImage2;
//loaded_bmp EmptyTexture;
//game_sound TestSound;
//mesh TestMesh;
//mesh TestMesh2;
//shader TextureShader;
//shader SphereShader;
//shader FramebufferShader;
//shader SingleColorShader;
//shader OutlineInitShader;
//shader JumpFloodShader;
//shader OutlineShader;
//shader KernelShader;
//shader AntialiasingShader;
//game_video TestVideo;

#define GetAsset(Assets, id, type) (type*)_GetAsset(Assets, id)
void* _GetAsset(game_assets* Assets, game_asset_id ID) {
    game_asset* Asset = &Assets->Asset[ID];

    switch (Asset->Type) {
        case Bitmap: return (void*)&Assets->Bitmaps[Asset->Index]; break;
        case Sound: return (void*)&Assets->Sounds[Asset->Index]; break;
        case Mesh: return (void*)&Assets->Meshes[Asset->Index]; break;
        case Video: return (void*)&Assets->Videos[Asset->Index]; break;
        default: Assert(false); return 0;
    }
}

void LoadAssetsFromFile(game_assets* Assets, const char* Path) {
    read_file_result AssetsFile = PlatformReadEntireFile(Path);

    *Assets = *(game_assets*)AssetsFile.Content;
    Assets->Memory = (uint8*)AssetsFile.Content + sizeof(game_assets);

    for (int i = 0; i < game_asset_id_count; i++) {
        game_asset Asset = Assets->Asset[i];

        switch (Asset.Type) {
            case Text: {
                Assets->Texts[Asset.Index] = (char*)(Assets->Memory + Asset.Offset);
            } break;

            case Bitmap: {
                game_bitmap* Bitmap = &Assets->Bitmaps[Asset.Index];
                Bitmap->Content = (uint32*)(Assets->Memory + Asset.Offset);
            } break;

            case Mesh: {
                game_mesh* Mesh = &Assets->Meshes[Asset.Index];
                Mesh->Vertices = (double*)(Assets->Memory + Asset.Offset);
                Mesh->Faces = (uint32*)(Mesh->Vertices + 5 * Mesh->nVertices);
            } break;

            case Sound: {
                game_sound* Sound = &Assets->Sounds[Asset.Index];
                Sound->SampleOut = (int16*)(Assets->Memory + Asset.Offset);
            } break;
            
            //case Video: {

            //} break;

            case Font: {
                game_font* Font = &Assets->Fonts[Asset.Index];
                uint8* Pointer = (uint8*)(Assets->Memory + Asset.Offset);
                for (int i = 0; i < FONT_CHARACTERS_COUNT; i++) {
                    game_bitmap* Character = &Font->Characters[i].Bitmap;
                    Character->Content = (uint32*)Pointer;
                    Pointer += Character->Header.Width * Character->Header.Height * Character->BytesPerPixel;
                }
            } break;

            default: {
                Log(Error, "ERROR: Asset type not implemented.\n");
                Assert(false);
            }
        }
    }

    Log(Info, "Assets loaded.");
}

#endif