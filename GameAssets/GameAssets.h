#include "GameMath.h"
#include "Tokenizer.h"

#include "Font/GameFont.h"
#include "Bitmap/GameBitmap.h"
#include "Sound/GameSound.h"
#include "Video/GameVideo.h"
#include "Mesh/GameMesh.h"
#include "Shader/GameShader.h"

#ifndef GAME_ASSETS
#define GAME_ASSETS

/*
    TODO:
        - Fix video. Possibly extract all frames and load them to the asset file.
        - Asset hot reloading
*/

enum game_asset_type {
    Asset_Type_Text,
    Asset_Type_Bitmap,
    Asset_Type_Heightmap,
    Asset_Type_Font,
    Asset_Type_Sound,
    Asset_Type_Video,
    Asset_Type_Mesh,
    Asset_Type_Animation,

    game_asset_type_count
};

enum game_text_id {
    Text_Test_ID,

    game_text_id_count
};

enum game_heightmap_id {
    Heightmap_Spain_ID,

    game_heightmap_id_count
};

enum game_animation_id {
    Animation_Idle_ID,
    Animation_Walk_ID,
    Animation_Jump_ID,
    Animation_Attack_ID,

    game_animation_id_count
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
// | Text                                                                                                                                                             |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_text {
    game_text_id ID;
    uint32 Size;
    char* Content;
};

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Heightmaps                                                                                                                                                       |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct game_heightmap {
    game_bitmap Bitmap;
    uint32 nVertices;
    float* Vertices;
};

const int HEIGHTMAP_RESOLUTION = 20;

uint64 ComputeNeededMemoryForHeightmap(read_file_result File) {
    uint64 BitmapSize = PreprocessBitmap((bitmap_header*)File.Content);
    uint64 VerticesSize = HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION * 4 * 5 * sizeof(float);
    return BitmapSize + VerticesSize;
}

game_heightmap LoadHeightmap(memory_arena* Arena, game_asset* Asset) {
    game_heightmap Result = {};

    Result.Bitmap = LoadBitmapFile(Arena, Asset->File);
    float Width = 10.0f;
    float Height = 10.0f;

    Result.nVertices = HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION * 4;
    Result.Vertices = (float*)PushSize(Arena, Result.nVertices * 5 * sizeof(float));
    float* Pointer = Result.Vertices;
    for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
        for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
            *Pointer++ = Width * (float)i / (float)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0f; // v.y
            *Pointer++ = Height * (float)j / (float)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (float)i / (float)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (float)j / (float)HEIGHTMAP_RESOLUTION; // vt.y

            *Pointer++ = Width * (float)(i + 1) / (float)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0f; // v.y
            *Pointer++ = Height * (float)j / (float)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (float)(i + 1) / (float)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (float)j / (float)HEIGHTMAP_RESOLUTION; // vt.y

            *Pointer++ = Width * (float)i / (float)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0f; // v.y
            *Pointer++ = Height * (float)(j + 1) / (float)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (float)i / (float)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (float)(j + 1) / (float)HEIGHTMAP_RESOLUTION; // vt.y

            *Pointer++ = Width * (float)(i + 1) / (float)HEIGHTMAP_RESOLUTION; // v.x
            *Pointer++ = 0.0f; // v.y
            *Pointer++ = Height * (float)(j + 1) / (float)HEIGHTMAP_RESOLUTION; // v.z
            *Pointer++ = (float)(i + 1) / (float)HEIGHTMAP_RESOLUTION; // vt.x
            *Pointer++ = (float)(j + 1) / (float)HEIGHTMAP_RESOLUTION; // vt.y
        }
    }

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Meshes                                                                                                                                                           |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+


preprocessed_mesh PreprocessMesh(read_file_result File) {
    preprocessed_mesh Result = {};

    if (File.ContentSize > 0) {
        tokenizer Tokenizer = InitTokenizer(File.Content);

        token Token = RequireToken(Tokenizer, Token_Identifier);
        while (Token.Type == Token_Identifier) {
            if (Token == "nV")   Result.nVertices = Parseuint32(Tokenizer);
            else if (Token == "nF") Result.nFaces = Parseuint32(Tokenizer);
            else if (Token == "nB") Result.nBones = Parseuint32(Tokenizer);
            Token = GetToken(Tokenizer);
        }
    }
    return Result;
}

uint32 GetMeshVerticesSize(uint32 nVertices, bool HasArmature) {
    uint32 VertexSize = HasArmature ? 10 * sizeof(float) + 2 * sizeof(int32) : 8 * sizeof(float);
    return VertexSize * nVertices;
}

game_mesh LoadMesh(memory_arena* Arena, game_asset* Asset, preprocessed_mesh* Preprocessed) {
    game_mesh Result = {};
    Result.ID = Asset->ID.Mesh;
    
    read_file_result File = Asset->File;

    if (File.ContentSize > 0) {
        tokenizer Tokenizer = InitTokenizer(File.Content);
        AdvanceUntilLine(Tokenizer, 2);

        Result.nVertices = Preprocessed->nVertices;
        Result.nFaces = Preprocessed->nFaces;
        Result.Armature.nBones = Preprocessed->nBones;
        bool HasArmature = Result.Armature.nBones > 0;
        Result.LayoutID = HasArmature ? vertex_layout_bones_id : vertex_layout_vec3_vec2_vec3_id;
        uint32 VerticesSize = GetMeshVerticesSize(Preprocessed->nVertices, HasArmature);
        Result.Vertices = PushSize(Arena, VerticesSize);
        Result.Faces = PushArray(Arena, 3 * Preprocessed->nFaces, uint32);

        float* pOutV = (float*)Result.Vertices;
        for (int i = 0; i < Result.nVertices; i++) {
            v3 Position = ParseV3(Tokenizer);
            v3 Normal = ParseV3(Tokenizer);
            v2 Texture = ParseV2(Tokenizer);

            *pOutV++ = Position.X; *pOutV++ = Position.Y; *pOutV++ = Position.Z;
            *pOutV++ = Texture.X;  *pOutV++ = Texture.Y;
            *pOutV++ = Normal.X;   *pOutV++ = Normal.Y;   *pOutV++ = Normal.Z;

            if (HasArmature) {
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
// | Game assets                                                                                                                                                      |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const uint32 ASSET_COUNT =
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
    platform_api* Platform;
    game_asset_array Asset;
    game_text Text[game_text_id_count];
    game_bitmap Bitmap[game_bitmap_id_count];
    game_heightmap Heightmap[game_heightmap_id_count];
    game_font Font[game_font_id_count];
    game_sound Sound[game_sound_id_count];
    game_mesh Mesh[game_mesh_id_count];
    game_animation Animation[game_animation_id_count];
    uint64 AssetsSize;
    //game_video Videos[1];
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

struct preprocessed_assets {
    preprocessed_font Font[game_bitmap_id_count];
    preprocessed_sound Sound[game_sound_id_count];
    preprocessed_mesh Mesh[game_mesh_id_count];
};

static preprocessed_assets PreprocessedAssets;

game_text*      GetAsset(game_assets* Assets, game_text_id ID)      { return &Assets->Text[ID]; }
game_sound*     GetAsset(game_assets* Assets, game_sound_id ID)     { return &Assets->Sound[ID]; }
game_bitmap*    GetAsset(game_assets* Assets, game_bitmap_id ID)    { return &Assets->Bitmap[ID]; }
game_heightmap* GetAsset(game_assets* Assets, game_heightmap_id ID) { return &Assets->Heightmap[ID]; }
game_font*      GetAsset(game_assets* Assets, game_font_id ID)      { return &Assets->Font[ID]; }
game_mesh*      GetAsset(game_assets* Assets, game_mesh_id ID)      { return &Assets->Mesh[ID]; }
game_animation* GetAsset(game_assets* Assets, game_animation_id ID) { return &Assets->Animation[ID]; }
//game_video*     GetAsset(game_assets* Assets, game_video_id ID)     { return &Assets->Videos[ID]; }

void PushAsset(game_assets* Assets, const char* Path, game_text_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Text;
    Asset.ID.Text = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = Asset.File.ContentSize + 1;
    
    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_sound_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Sound;
    Asset.ID.Sound = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);

    preprocessed_sound Preprocessed = PreprocessSound(Asset.File);
    PreprocessedAssets.Sound[ID] = Preprocessed;
    Asset.MemoryNeeded = Preprocessed.Size;

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_bitmap_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Bitmap;
    Asset.ID.Bitmap = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = PreprocessBitmap((bitmap_header*)Asset.File.Content);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_heightmap_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Heightmap;
    Asset.ID.Heightmap = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForHeightmap(Asset.File);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_font_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Font;
    Asset.ID.Font = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    preprocessed_font Preprocessed = PreprocessFont(Asset.File);
    PreprocessedAssets.Font[ID] = Preprocessed;
    Asset.MemoryNeeded = Preprocessed.Size;

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_mesh_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Mesh;
    Asset.ID.Mesh = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);

    preprocessed_mesh Preprocessed = PreprocessMesh(Asset.File);
    Asset.MemoryNeeded = 0;
    if (Preprocessed.nBones > 0) Asset.MemoryNeeded += Preprocessed.nVertices * (10 * sizeof(float) + 2 * sizeof(int32));
    else                         Asset.MemoryNeeded += Preprocessed.nVertices * 8 * sizeof(float);
    Asset.MemoryNeeded += Preprocessed.nFaces * 3 * sizeof(uint32);

    PreprocessedAssets.Mesh[ID] = Preprocessed;

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_animation_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Animation;
    Asset.ID.Animation = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
    Assert(Asset.File.ContentSize > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForAnimation(Asset.File);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_video_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Video;
    Asset.ID.Video = ID;
    Asset.File = Assets->Platform->ReadEntireFile(Path);
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
        case Asset_Type_Text: {
            char* TextContent = (char*)PushSize(Arena, Asset->MemoryNeeded);
            Assets->Text[ID.Text].ID = ID.Text;
            Assets->Text[ID.Text].Size = Asset->MemoryNeeded;
            Assets->Text[ID.Text].Content = TextContent;
            memcpy(TextContent, Asset->File.Content, Asset->MemoryNeeded);
            sprintf_s(LogBuffer, "Loaded text %s.", Asset->File.Path);
        } break;

        // case Asset_Type_Video: {
        //    Assets->Videos[ID.Video] = LoadVideo(Arena, Asset);
        //    sprintf_s(LogBuffer, "Loaded video %s.", Asset->File.Path);
        // } break;

        case Asset_Type_Bitmap: {
            Assets->Bitmap[ID.Bitmap] = LoadBitmapFile(Arena, Asset->File);
            Assets->Bitmap[ID.Bitmap].ID = ID.Bitmap;
            sprintf_s(LogBuffer, "Loaded bitmap %s.", Asset->File.Path);
        } break;

        case Asset_Type_Heightmap: {
            Assets->Heightmap[ID.Heightmap] = LoadHeightmap(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded heightmap %s.", Asset->File.Path);
        } break;

        case Asset_Type_Font: {
            Assets->Font[ID.Font] = LoadFont(Arena, &PreprocessedAssets.Font[ID.Font]);
            Assets->Font[ID.Font].ID = ID.Font;
            sprintf_s(LogBuffer, "Loaded font %s.", Asset->File.Path);
        } break;

        case Asset_Type_Sound: {
            Assets->Sound[ID.Sound] = LoadSound(Arena, &PreprocessedAssets.Sound[ID.Sound]);
            Assets->Sound[ID.Sound].ID = ID.Sound;
            sprintf_s(LogBuffer, "Loaded sound %s.", Asset->File.Path);
        } break;

        case Asset_Type_Mesh: {
            Assets->Mesh[ID.Mesh] = LoadMesh(Arena, Asset, &PreprocessedAssets.Mesh[ID.Sound]);
            sprintf_s(LogBuffer, "Loaded mesh %s.", Asset->File.Path);
        } break;

        case Asset_Type_Animation: {
            Assets->Animation[ID.Animation] = LoadAnimation(Arena, Asset);
            sprintf_s(LogBuffer, "Loaded animation %s.", Asset->File.Path);
        } break;

        default: {
            sprintf_s(LogBuffer, "Asset ignored %s.", Asset->File.Path);
        }
    }

    Log(Info, LogBuffer);
    Assert(Asset->MemoryNeeded == Arena->Used - Asset->Offset);
    Assets->Platform->FreeFileMemory(Asset->File.Content);
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

    Shader->File = Assets->Platform->ReadEntireFile(Path);
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
        read_file_result ShaderFile = Assets->Platform->ReadEntireFile(Path);
        Shader->Size = ShaderFile.ContentSize;
        Shader->Code = (char*)ShaderFile.Content;

        // Extra char with value 0 to separate shaders
        Assets->TotalSize += Shader->Size + 1;
        Assets->ComputeShadersSize += Shader->Size + 1;
    }

    delete [] Buffer;
}

void WriteAssetsFile(platform_api* Platform, const char* Path);
void LoadAssetsFromFile(platform_read_entire_file Read, game_assets* Assets, const char* Path);

#endif