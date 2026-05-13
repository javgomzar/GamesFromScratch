#ifndef GAME_ASSETS
#define GAME_ASSETS

#include "GamePlatform.h"
#include "GameMath.h"
#include "Tokenizer.h"

/*
    TODO:
        - Fix video. Possibly extract all frames and load them to the asset file.
        - Asset hot reloading
*/

ENUM(game_asset_type,
    Asset_Type_Text,
    Asset_Type_Texture,
    Asset_Type_Heightmap,
    Asset_Type_Font,
    Asset_Type_Sound,
    Asset_Type_Mesh,
    Asset_Type_Animation
//  Asset_Type_Video,
);

ENUM(game_text_id,
    Text_Test_ID
);

ENUM(game_texture_id,
    Texture_Empty_ID,
    Texture_Background_ID,
    Texture_Button_ID,
    Texture_Enemy_ID,
    Texture_Player_ID,
    Texture_Spain_ID
);

ENUM(game_heightmap_id,
    Heightmap_Spain_ID
);

ENUM(game_font_id,
    Font_DejaVu_Sans_Mono_ID,
    Font_DejaVu_Sans_ID
);

ENUM(game_animation_id,
    Animation_Idle_ID,
    Animation_Walk_ID,
    Animation_Jump_ID,
    Animation_Attack_ID
);

ENUM(game_sound_id,
    Sound_Test_ID
);

ENUM(game_mesh_id,
    Mesh_Sphere_ID,
    Mesh_Tetrahedron_ID,
    Mesh_Cube_ID,
    Mesh_Octahedron_ID,
    Mesh_Icosahedron_ID,
    Mesh_Dodecahedron_ID,
    Mesh_Enemy_ID,
    Mesh_Body_ID,
    Mesh_Shield_ID,
    Mesh_Sword_ID,
    Mesh_Selector_ID
);


union game_asset_id {
    game_text_id Text;
    game_sound_id Sound;
    game_texture_id Texture;
    game_heightmap_id Heightmap;
    game_font_id Font;
    game_mesh_id Mesh;
    game_animation_id Animation;
//  game_video_id Video;
};

struct game_asset {
    game_asset_id ID;
    game_asset_type Type;
    file_info FileInfo;
    void* FileContent;
    uint64 MemoryNeeded;
    uint64 Offset;
};

#include "GameFont.h"
#include "GameTexture.h"
#include "GameSound.h"
#include "GameMesh.h"
// #include "GameVideo.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Color                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct color {
    float R;
    float G;
    float B;
    float Alpha;
};

color GetColor(float R, float G, float B, float Alpha = 1.0f) {
    return { R, G, B, Alpha };
}

color ChangeAlpha(color Color, float Alpha) {
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

color Black          = { 0.0f, 0.0f, 0.0f, 1.0f };
color White          = { 1.0f, 1.0f, 1.0f, 1.0f };
color Gray           = { 0.5f, 0.5f, 0.5f, 1.0f };
color DarkGray       = { 0.1f, 0.1f, 0.1f, 1.0f };
color Red            = { 1.0f, 0.0f, 0.0f, 1.0f };
color Green          = { 0.0f, 1.0f, 0.0f, 1.0f };
color Blue           = { 0.0f, 0.0f, 1.0f, 1.0f };
color Magenta        = { 1.0f, 0.0f, 1.0f, 1.0f };
color Yellow         = { 1.0f, 1.0f, 0.0f, 1.0f };
color Cyan           = { 0.0f, 1.0f, 1.0f, 1.0f };
color Orange         = { 1.0f, 0.63f, 0.0f, 1.0f };
color Purple         = { 0.5f, 0.0f, 0.6f, 1.0f};
color BackgroundBlue = { 0.4f, 0.4f, 0.8f, 1.0f };

uint32 GetColorBytes(color Color) {
    uint8 Alpha = Color.Alpha * 255.0f;
    uint8 R = Color.R * 255.0f;
    uint8 G = Color.G * 255.0f;
    uint8 B = Color.B * 255.0f;
    return (Alpha << 24) | (R << 16) | (G << 8) | B;
}

color HSV2RGB(float H, float S, float V, float Alpha = 1.0f) {
    float R = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 1.0f, 1.0f)      * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float G = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 2.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float B = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 1.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    return V * GetColor(R, G, B, Alpha);
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
    return GetColor(
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
    game_heightmap_id ID;
    game_texture Texture;
};

const int HEIGHTMAP_RESOLUTION = 16;

uint64 ComputeNeededMemoryForHeightmap(void* FileContent) {
    uint64 BitmapSize = PreprocessBitmap((bitmap_header*)FileContent);
    return BitmapSize;
}

game_heightmap LoadHeightmap(memory_arena* Arena, game_asset* Asset) {
    game_heightmap Result = {};
    Result.ID = Asset->ID.Heightmap;

    string Extension = GetFileExtension(Asset->FileInfo.Path);

    Result.Texture = LoadTexture(Arena, Extension, Asset->FileContent);
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
            Bone->Transform.Scale = GetScale();
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

uint64 ComputeNeededMemoryForAnimation(void* FileContent) {
    uint64 Result = 0;

    uint32 nFrames = 0, nBones = 0;
    GetAnimationSizes(FileContent, &nFrames, &nBones);

    Result = nFrames * nBones * 10 * sizeof(float);
    return Result;
}

game_animation LoadAnimation(memory_arena* Arena, game_asset* Asset) {
    game_animation Result = {};
    Result.ID = Asset->ID.Animation;

    GetAnimationSizes(Asset->FileContent, &Result.nFrames, &Result.nBones);

    Result.Content = PushArray(Arena, Result.nFrames * Result.nBones * 10, float);

    tokenizer Tokenizer = InitTokenizer(Asset->FileContent);
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
    (uint32)game_text_id_count +
    (uint32)game_sound_id_count +
    (uint32)game_texture_id_count +
    (uint32)game_heightmap_id_count +
    (uint32)game_font_id_count +
    (uint32)game_mesh_id_count +
    (uint32)game_animation_id_count; // + 
//    game_video_id_count;

ArrayDefinition(ASSET_COUNT, game_asset)

struct game_assets {
    game_asset_array Asset;
    game_text Text[game_text_id_count];
    game_texture Texture[game_texture_id_count];
    game_heightmap Heightmap[game_heightmap_id_count];
    game_font Font[game_font_id_count];
    game_sound Sound[game_sound_id_count];
    game_mesh Mesh[game_mesh_id_count];
    game_animation Animation[game_animation_id_count];
    uint64 AssetsSize;
    //game_video Videos[1];
    uint32 nSamplers;
    uint64 ShadersSize;
    uint64 ComputeShadersSize;
    uint64 TotalSize;
    uint8* Memory;
};

struct preprocessed_assets {
    preprocessed_font Font[game_font_id_count];
    preprocessed_sound Sound[game_sound_id_count];
    preprocessed_mesh Mesh[game_mesh_id_count];
};

static preprocessed_assets PreprocessedAssets;

game_text*      GetAsset(game_assets* Assets, game_text_id ID)      { return &Assets->Text[ID]; }
game_sound*     GetAsset(game_assets* Assets, game_sound_id ID)     { return &Assets->Sound[ID]; }
game_texture*   GetAsset(game_assets* Assets, game_texture_id ID)   { return &Assets->Texture[ID]; }
game_heightmap* GetAsset(game_assets* Assets, game_heightmap_id ID) { return &Assets->Heightmap[ID]; }
game_font*      GetAsset(game_assets* Assets, game_font_id ID)      { return &Assets->Font[ID]; }
game_mesh*      GetAsset(game_assets* Assets, game_mesh_id ID)      { return &Assets->Mesh[ID]; }
game_animation* GetAsset(game_assets* Assets, game_animation_id ID) { return &Assets->Animation[ID]; }
//game_video*     GetAsset(game_assets* Assets, game_video_id ID)     { return &Assets->Videos[ID]; }

void PushAsset(game_assets* Assets, const char* Path, game_text_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Text;
    Asset.ID.Text = ID;
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);
    Asset.MemoryNeeded = Asset.FileInfo.Size + 1;
    
    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_sound_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Sound;
    Asset.ID.Sound = ID;
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);

    preprocessed_sound Preprocessed = PreprocessSound(Asset.FileContent);
    PreprocessedAssets.Sound[ID] = Preprocessed;
    Asset.MemoryNeeded = Preprocessed.Size;

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_texture_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Texture;
    Asset.ID.Texture = ID;
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);
    Asset.MemoryNeeded = PreprocessBitmap((bitmap_header*)Asset.FileContent);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_heightmap_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Heightmap;
    Asset.ID.Heightmap = ID;
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForHeightmap(Asset.FileContent);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_font_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Font;
    Asset.ID.Font = ID;
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);
    preprocessed_font Preprocessed = PreprocessFont(Asset.FileInfo, Asset.FileContent);
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
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);

    preprocessed_mesh Preprocessed = PreprocessMesh(Asset.FileInfo, Asset.FileContent);
    Asset.MemoryNeeded = 0;
    if (Preprocessed.nBones > 0) Asset.MemoryNeeded += Preprocessed.nVertices * (10 * sizeof(float) + 2 * sizeof(int32));
    else                         Asset.MemoryNeeded += Preprocessed.nVertices * 8 * sizeof(float);
    Asset.MemoryNeeded += Preprocessed.nFaces * 3 * sizeof(uint32);
    Asset.MemoryNeeded += Preprocessed.nEdges * 2 * sizeof(uint32);

    PreprocessedAssets.Mesh[ID] = Preprocessed;

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

void PushAsset(game_assets* Assets, const char* Path, game_animation_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Animation;
    Asset.ID.Animation = ID;
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);
    Asset.MemoryNeeded = ComputeNeededMemoryForAnimation(Asset.FileContent);

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};

/*
void PushAsset(game_assets* Assets, const char* Path, game_video_id ID) {
    game_asset Asset = {};
    Asset.Type = Asset_Type_Video;
    Asset.ID.Video = ID;
    Asset.FileContent = Platform.ReadEntireFile(Path, &Asset.FileInfo);
    Assert(Asset.FileInfo.Size > 0);
    Asset.MemoryNeeded = Asset.FileInfo.Size;

    Append(&Assets->Asset, Asset);
    Assets->TotalSize += Asset.MemoryNeeded;
    Assets->AssetsSize += Asset.MemoryNeeded;
};
*/

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
            memcpy(TextContent, Asset->FileContent, Asset->MemoryNeeded);
            sprintf(LogBuffer, "Loaded text %s.", Asset->FileInfo.Path.Content);
        } break;

        // case Asset_Type_Video: {
        //    Assets->Videos[ID.Video] = LoadVideo(Arena, Asset);
        //    sprintf(LogBuffer, "Loaded video %s.", Asset->File.Path);
        // } break;

        case Asset_Type_Texture: {
            string Extension = GetFileExtension(Asset->FileInfo.Path);
            Assets->Texture[ID.Texture] = LoadTexture(Arena, Extension, Asset->FileContent);
            Assets->Texture[ID.Texture].ID = ID.Texture;
            sprintf(LogBuffer, "Loaded bitmap %s.", Asset->FileInfo.Path.Content);
        } break;

        case Asset_Type_Heightmap: {
            Assets->Heightmap[ID.Heightmap] = LoadHeightmap(Arena, Asset);
            sprintf(LogBuffer, "Loaded heightmap %s.", Asset->FileInfo.Path.Content);
        } break;

        case Asset_Type_Font: {
            Assets->Font[ID.Font] = LoadFont(Arena, &PreprocessedAssets.Font[ID.Font]);
            Assets->Font[ID.Font].ID = ID.Font;
            sprintf(LogBuffer, "Loaded font %s.", Asset->FileInfo.Path.Content);
        } break;

        case Asset_Type_Sound: {
            Assets->Sound[ID.Sound] = LoadSound(Arena, &PreprocessedAssets.Sound[ID.Sound]);
            Assets->Sound[ID.Sound].ID = ID.Sound;
            sprintf(LogBuffer, "Loaded sound %s.", Asset->FileInfo.Path.Content);
        } break;

        case Asset_Type_Mesh: {
            Assets->Mesh[ID.Mesh] = LoadMesh(Arena, &PreprocessedAssets.Mesh[ID.Sound]);
            Assets->Mesh[ID.Mesh].ID = ID.Mesh;
            sprintf(LogBuffer, "Loaded mesh %s.", Asset->FileInfo.Path.Content);
        } break;

        case Asset_Type_Animation: {
            Assets->Animation[ID.Animation] = LoadAnimation(Arena, Asset);
            sprintf(LogBuffer, "Loaded animation %s.", Asset->FileInfo.Path.Content);
        } break;

        default: {
            sprintf(LogBuffer, "Asset ignored %s.", Asset->FileInfo.Path.Content);
        }
    }

    Log(log_level::Info, LogBuffer);
    uint64 UsedMemory = Arena->Used - Asset->Offset;
    Assert(Asset->MemoryNeeded == UsedMemory, "Assets memory needed doesn't match.");
    Platform.FreeMemory(Asset->FileContent);
    Asset->FileContent = nullptr;
}


void WriteAssetsFile(platform_api* Platform, const char* Path);
void LoadAssetsFromFile(memory_arena* FontsArena, game_assets* Assets, const char* Path);

#endif