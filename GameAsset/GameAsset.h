#ifndef GAME_ASSET
#define GAME_ASSET

#include <initializer_list>
#include "stb_image.h"

#include "GamePlatform.h"
#include "GameMath.h"
#include "Tokenizer.h"

typedef uint32 Asset_ID;

#include "GameColor.h"
#include "animation.h"
#include "GameFont.h"
#include "GameTexture.h"
#include "GameSound.h"
#include "GameMesh.h"
// #include "GameVideo.h"


struct Asset_Header {
    uint32 magic_number;
    uint32 mesh_count;
    uint32 material_count;
    uint32 texture_count;
    uint32 skeleton_count;
    uint32 animation_count;
    uint32 sound_count;
};

struct Asset {
    String path;
    array<Asset_ID> sounds;
    array<Asset_ID> textures;
    array<Asset_ID> meshes;
    array<Asset_ID> materials;
    array<Asset_ID> skeletons;
    array<Asset_ID> animations;
};

struct Catalog {
    Asset test;
};

constexpr int MAX_ASSETS = 128;

struct Asset_Manager {
    Asset_ID next_id;
    Arena arena;
    Catalog catalog;
    hash_table<Asset_ID, Texture*> textures;
    // hash_table<Asset_ID, Mesh*> meshes;
    // hash_table<Asset_ID, Material*> materials;
};

Asset_ID CreateID(Asset_Manager* manager) {
    return manager->next_id++;
}

Texture* AddTexture(Asset_Manager* manager) {
    Texture* texture = PushStruct(&manager->arena, Texture);
    texture->id = CreateID(manager);
    manager->textures.Insert(texture->id, texture);
}

void AddAsset(
    Asset_Manager* manager,
    Asset* asset,
    const char* path,
    std::initializer_list<const char*> files,
    bool force_process = false
) {
    asset->path = path;
    bool process = force_process;
    if (Platform.FileExists(path)) {
        Timestamp asset_file_time = Platform.GetFileInfo(path).timestamp;
        for (const char* file : files) {
            if (!Platform.FileExists(path)) {
                Log(Log_Level::Error, "Failed to add file to asset.");
                continue;
            }

            Timestamp import_file_time = Platform.GetFileInfo(path).timestamp;
            if (import_file_time > asset_file_time) {
                process = true;
            }
        }
    }
    else {
        process = true;
    }

    if (process) {
        // uint32 sound_count = 0;
        // uint32 mesh_count = 0;
        // uint32 material_count = 0;
        uint32 texture_count = 0;
        // uint32 skeleton_count = 0;
        // uint32 animation_count = 0;

        // Count asset types
        for (const char* file : files) {
            String extension = GetFileExtension(file);
            if (extension == "glb") {
                // TODO
            }
            else if (extension == "gltf") {
                // TODO
            }
            else if (extension == "jpeg" || extension == "jpg" || extension == "png") {
                texture_count += 1;
            }
            else if (extension == "wav") {
                // TODO
            }
            else {
                Log(Log_Level::Error, "Invalid asset file extension.");
            }
        }

        asset->textures = array<Asset_ID>(&manager->arena, texture_count);

        for (const char* file : files) {
            String extension = GetFileExtension(file);
            if (extension == "glb") {
                // TODO
            }
            else if (extension == "gltf") {
                // TODO
            }
            else if (extension == "jpeg" || extension == "jpg" || extension == "png") {
                Texture* texture = PushStruct(&manager->arena, Texture);
                texture->id = CreateID(manager);
                texture->pixels = stbi_load(file, &texture->width, &texture->height, &texture->channels, 0);
                texture->size = GetPixelsSize(texture->width, texture->height, texture->channels);
                manager->textures.Insert(texture->id, texture);
            }
            else if (extension == "wav") {
                // TODO
            }
            else {
                Log(Log_Level::Error, "Invalid asset file extension.");
            }
        }
    }
}

// void ProcessImportFiles(game_asset* Asset, Arena* Transient) {
//     Assert(Asset->ImportFiles.IsFull() && Asset->Header.nImportFiles == Asset->ImportFiles.Count);

//     Asset->Animations = array<game_animation>(Transient, Asset->Header.nAnimations);
//     Asset->Fonts = array<game_font>(Transient, Asset->Header.nFonts);
//     Asset->Meshes = array<Mesh>(Transient, Asset->Header.nMeshes);
//     Asset->Sounds = array<game_sound>(Transient, Asset->Header.nSounds);
//     Asset->Texts = array<game_text>(Transient, Asset->Header.nTexts);
//     Asset->Textures = array<game_texture>(Transient, Asset->Header.nTextures);

//     Asset->PreprocessedFonts = array<preprocessed_font>(Transient, Asset->Header.nFonts);
//     Asset->PreprocessedMeshes = array<preprocessed_mesh>(Transient, Asset->Header.nMeshes);  

//     for (int i = 0; i < Asset->ImportFiles.Count; i++) {
//         game_asset_import_file* ImportFile = &Asset->ImportFiles[i];
//         ImportFile->Content = Platform.ReadEntireFile(ImportFile->File.Path, &ImportFile->File);
//         switch(ImportFile->Type) {
//             case game_asset_import_file_type::ASSET: {
//                 // TODO: Add GLTF needed memory computation
//                 Raise("GLTF file processing is not yet done");
//             } break;

//             case game_asset_import_file_type::ANIMATION: {
//                 game_animation Animation;
//                 GetAnimationSizes(ImportFile->Content, &Animation.nFrames, &Animation.nBones);
//                 ImportFile->PersistSize = Animation.nFrames * Animation.nBones * 10 * sizeof(float);
//                 Asset->Animations.Insert(Animation);
//             } break;

//             case game_asset_import_file_type::FONT: {
//                 preprocessed_font Font = PreprocessFont(ImportFile->Content);
//                 Asset->PreprocessedFonts.Insert(Font);
//                 ImportFile->PersistSize = Font.Size;
//             } break;

//             case game_asset_import_file_type::MESH: {
//                 preprocessed_mesh Mesh = PreprocessMesh(ImportFile->Content);
//                 Asset->PreprocessedMeshes.Insert(Mesh);
//                 ImportFile->PersistSize = 0;
//                 if (Mesh.nBones > 0) ImportFile->PersistSize += Mesh.nVertices * (10 * sizeof(float) + 2 * sizeof(int32));
//                 else                 ImportFile->PersistSize += Mesh.nVertices * 8 * sizeof(float);
//                 ImportFile->PersistSize += Mesh.nFaces * 3 * sizeof(uint32);
//                 ImportFile->PersistSize += Mesh.nEdges * 2 * sizeof(uint32);
//             } break;

//             case game_asset_import_file_type::SOUND: {
//                 game_sound Sound;
//                 bool Valid = LoadSound(ImportFile->Content, &Sound);
//                 if (!Valid) {
//                     Raise("Invalid WAV file.");
//                 }
//                 else {
//                     Asset->Sounds.Insert(Sound);
//                 }
//                 ImportFile->PersistSize = Sound.SampleCount * 2;
//             } break;

//             case game_asset_import_file_type::TEXT: {
//                 game_text Text;
//                 Text.Language = language::ENGLISH;
//                 Text.Text.Content = (char*)ImportFile->Content;
//                 Text.Text.Length = ImportFile->File.Size;
//                 Asset->Texts.Insert(Text);
//                 ImportFile->PersistSize = ImportFile->File.Size;
//             } break;

//             case game_asset_import_file_type::TEXTURE: {
//                 game_texture Texture = ReadTexture(ImportFile->Format, ImportFile->Content);
//                 ImportFile->PersistSize = Texture.PixelsSize;
//                 Asset->Textures.Insert(Texture);
//             } break;
//         }

//         Asset->Header.Size += ImportFile->PersistSize;
//     }
// }

// void PersistAsset(game_asset* Asset, Arena* Transient) {
//     memory_size NeededMemory = sizeof(game_asset_header) + 
//         Asset->Header.nImportFiles * sizeof(game_asset_import_file) + 
//         Asset->Header.nAnimations * sizeof(game_animation) + 
//         Asset->Header.nFonts * sizeof(game_font) + 
//         Asset->Header.nMeshes * sizeof(Mesh) + 
//         Asset->Header.nSounds * sizeof(game_sound) + 
//         Asset->Header.nTexts * sizeof(game_text) + 
//         Asset->Header.nTextures * sizeof(game_texture) +
//         Asset->Header.Size;
//     Arena FileArena = SuballocateMemoryArena(Transient, NeededMemory);

//     game_asset_header* Header = PushStruct(&FileArena, game_asset_header);
//     *Header = Asset->Header;

//     int nthPreprocessedFont = 0;
//     int nthPreprocessedMesh = 0;

//     for (int i = 0; i < Asset->Header.nImportFiles; i++) {
//         game_asset_import_file* ImportFile = PushStruct(&FileArena, game_asset_import_file);
//         ImportFile[i] = Asset->ImportFiles[i];
//         ImportFile->Content = nullptr;
//     }

//     game_animation* Animations = nullptr;
//     if (Asset->Header.nAnimations > 0) {
//         for (int i = 0; i < Asset->Header.nAnimations; i++) {
//             game_animation* Animation = PushStruct(&FileArena, game_animation);
//             if (i == 0) Animations = Animation;
//             *Animation = Asset->Animations[i];
//         }
//     }

//     game_font* Fonts = nullptr;
//     if (Asset->Header.nAnimations > 0) {
//         for (int i = 0; i < Asset->Header.nFonts; i++) {
//             game_font* Font = PushStruct(&FileArena, game_font);
//             if (i == 0) Fonts = Font;
//         }
//     }

//     Mesh* Meshes = nullptr;
//     if (Asset->Header.nMeshes > 0) {
//         for (int i = 0; i < Asset->Header.nMeshes; i++) {
//             Mesh* Mesh = PushStruct(&FileArena, Mesh);
//             if (i == 0) Meshes = Mesh;
//         }
//     }

//     game_sound* Sounds = nullptr;
//     if (Asset->Header.nSounds > 0) {
//         for (int i = 0; i < Asset->Header.nSounds; i++) {
//             game_sound* Sound = PushStruct(&FileArena, game_sound);
//             if (i == 0) Sounds = Sound;
//             *Sound = Asset->Sounds[i];
//         }
//     }

//     game_text* Texts = nullptr;
//     if (Asset->Header.nTexts > 0) {
//         for (int i = 0; i < Asset->Header.nTexts; i++) {
//             game_text* Text = PushStruct(&FileArena, game_text);
//             if (i == 0) Texts = Text;
//             *Text = Asset->Texts[i];
//         }
//     }

//     game_texture* Textures = nullptr;
//     if (Asset->Header.nTextures > 0) {
//         for (int i = 0; i < Asset->Header.nTextures; i++) {
//             game_texture* Texture = PushStruct(&FileArena, game_texture);
//             if (i == 0) Textures = Texture;
//             *Texture = Asset->Textures[i];
//         }
//     }

//     int nthAnimation = 0;
//     int nthFont = 0;
//     int nthMesh = 0;
//     int nthSound = 0;
//     int nthText = 0;
//     int nthTexture = 0;

//     for (int i = 0; i < Asset->Header.nImportFiles; i++) {
//         game_asset_import_file* ImportFile = &Asset->ImportFiles[i];

//         memory_size ArenaStart = FileArena.Used;

//         switch (ImportFile->Type) {
//             case game_asset_import_file_type::ANIMATION: {
//                 game_animation* Animation = Animations + nthAnimation++;
//                 *Animation = LoadAnimation(&FileArena, ImportFile->Content);
//                 Animation->Name = GetFileNameWithoutExtension(ImportFile->File.Path);
//                 Animation->Content = nullptr;
//             } break;

//             case game_asset_import_file_type::FONT: {
//                 int FontIndex = nthFont++;
//                 preprocessed_font* PreprocessedFont = &Asset->PreprocessedFonts[FontIndex];
//                 game_font* Font = Fonts + FontIndex;
//                 *Font = LoadFont(&FileArena, Transient, PreprocessedFont, ImportFile->Content);
//                 Font->Name = GetFileNameWithoutExtension(ImportFile->File.Path);
//                 Font->Vertices = nullptr;
//                 Font->Elements = nullptr;
//             } break;

//             case game_asset_import_file_type::MESH: {
//                 preprocessed_mesh* PreprocessedMesh = &Asset->PreprocessedMeshes[nthPreprocessedMesh++];
//                 Mesh* Mesh = Meshes + nthMesh++;
//                 *Mesh = LoadMesh(&FileArena, PreprocessedMesh, ImportFile->Content);
//                 Mesh->Name = GetFileNameWithoutExtension(ImportFile->File.Path);
//                 Mesh->Vertices = nullptr;
//                 Mesh->Edges = nullptr;
//                 Mesh->Faces = nullptr;
//             } break;

//             case game_asset_import_file_type::SOUND: {
//                 game_sound* Sound = Sounds + nthSound++;
//                 void* Destination = PushSize(&FileArena, ImportFile->PersistSize);
//                 memcpy(Destination, Sound->SampleOut, ImportFile->PersistSize);
//                 Sound->Name = GetFileNameWithoutExtension(ImportFile->File.Path);
//                 Sound->SampleOut = nullptr;
//             } break;

//             case game_asset_import_file_type::TEXT: {
//                 game_text* Text = Texts + nthText;
//                 *Text = Asset->Texts[nthText++];
//                 CopyString(&FileArena, Text->Text);
//                 Text->Name = GetFileNameWithoutExtension(ImportFile->File.Path);
//                 Text->Text.Content = nullptr;
//             } break;

//             case game_asset_import_file_type::TEXTURE: {
//                 game_texture* Texture = Textures + nthTexture++;
//                 WriteTexture(&FileArena, Texture);
//                 Texture->Name = GetFileNameWithoutExtension(ImportFile->File.Path);
//                 Texture->Content = nullptr;
//             } break;
//         }

//         Assert(ImportFile->PersistSize == FileArena.Used - ArenaStart);

//         Platform.FreeMemory(ImportFile->Content);
//         ImportFile->Content = nullptr;
//     }

//     Assert(FileArena.Used == FileArena.Size);

//     bool Success = Platform.WriteEntireFile(Asset->File.Path, FileArena.Size, FileArena.Base);
//     Assert(Success);

//     PopSize(Transient, NeededMemory);
// }

// game_asset* AddAsset(
//     game_asset_manager* AssetManager,
//     string ManifestPath,
//     game_asset_tag Tags = game_asset_tag::none
// ) {
//     void* Manifest = Platform.ReadEntireFile(ManifestPath);

//     game_asset Result;
//     Result.Header.ID = AssetManager->NewAssetID++;
//     Result.Header.Tags = Tags;
//     Result.Header.nImportFiles = 0;
//     Result.Header.nAnimations = 0;
//     Result.Header.nFonts = 0;
//     Result.Header.nMeshes = 0;
//     Result.Header.nSounds = 0;
//     Result.Header.nTexts = 0;
//     Result.Header.nTextures = 0;
//     Result.Header.Size = 0;
//     Result.Loaded = false;
//     Result.Persist = false;
    
//     tokenizer Tokenizer = InitTokenizer(Manifest);
//     token Token = GetToken(Tokenizer);
//     while (Token.Type == Token_Identifier) {
//         if (Token == "Name") {
//             RequireToken(Tokenizer, Token_Colon);
//             Token = RequireToken(Tokenizer, Token_Identifier);
//             Result.Name = string(Token.Text, Token.Length);
//         }
//         else if (Token == "Path") {
//             RequireToken(Tokenizer, Token_Colon);
//             AdvanceWhitespaceAndComments(Tokenizer);
//             int PathLength = ParsePath(Tokenizer.At);
//             string AssetFilePath = string(Tokenizer.At, PathLength);
//             if (Platform.FileExists(AssetFilePath)) {
//                 Result.File = Platform.GetFileInfo(AssetFilePath);
//                 LoadAssetsFromFile(AssetFilePath);
//             }
//             else {
//                 Result.File.Path = AssetFilePath;
//                 Result.Persist = true;
//             }
//             Advance(Tokenizer, PathLength);
//         }
//         else if (Token == "Version") {
//             RequireToken(Tokenizer, Token_Colon);
//             Result.Header.Version = ParseInt(Tokenizer);
//         }
//         else if (Token == "Files") {
//             RequireToken(Tokenizer, Token_Colon);
//             tokenizer TokenizerCopy = Tokenizer;
//             int nImportFiles = 0;
//             do {
//                 AdvanceWhitespaceAndComments(TokenizerCopy);
//                 int PathLength = ParsePath(TokenizerCopy.At);
//                 nImportFiles += 1;
//                 Advance(TokenizerCopy, PathLength);
//                 Token = GetToken(TokenizerCopy);
//             } while(Token.Type == Token_Semicolon);

//             Result.ImportFiles = array<game_asset_import_file>(&AssetManager->AssetArena, nImportFiles);

//             do {
//                 AdvanceWhitespaceAndComments(Tokenizer);
//                 int PathLength = ParsePath(Tokenizer.At);
//                 string Path = string(Tokenizer.At, PathLength);
//                 AddFile(&Result, Path);
//                 Advance(Tokenizer, PathLength);
//                 Token = GetToken(Tokenizer);
//             } while(Token.Type == Token_Semicolon);

//             if (Result.Persist) {
//                 ProcessImportFiles(&Result, AssetManager->Transient);
//             }
//         }
//         Token = GetToken(Tokenizer);
//     }

//     if (Result.Persist) {
//         PersistAsset(&Result, AssetManager->Transient);
//     }

//     return AssetManager->Assets.Insert(Result);
// }

// void LoadAsset(game_asset_manager* Manager, game_asset* Asset) {
//     Arena* Arena = &Manager->Arena;
//     Asset->Offset = Arena->Used;
//     game_asset_id ID = Asset->ID;

//     switch (Asset->Type) {
//         case Asset_Type_Text: {
//             char* TextContent = (char*)PushSize(Arena, Asset->MemoryNeeded);
//             Manager->Text[ID.Text].ID = ID.Text;
//             Manager->Text[ID.Text].Size = Asset->MemoryNeeded;
//             Manager->Text[ID.Text].Content = TextContent;
//             memcpy(TextContent, Asset->FileContent, Asset->MemoryNeeded);
//             string LogText = Format(Manager->Transient, "Loaded text {s}.", 1, Asset->FileInfo.Path);
//             Log(log_level::Info, LogText.Content);
//         } break;

//         // case Asset_Type_Video: {
//         //    Assets->Videos[ID.Video] = LoadVideo(Arena, Asset);
//         //    sprintf(LogBuffer, "Loaded video %s.", Asset->File.Path);
//         // } break;

//         case Asset_Type_Texture: {
//             string Extension = GetFileExtension(Asset->FileInfo.Path);
//             Manager->Texture[ID.Texture] = LoadTexture(Arena, Extension, Asset->FileContent);
//             Manager->Texture[ID.Texture].ID = ID.Texture;
//             string LogText = Format(Manager->Transient, "Loaded bitmap {s}.", 1, Asset->FileInfo.Path);
//             Log(log_level::Info, LogText.Content);
//         } break;

//         case Asset_Type_Heightmap: {
//             Manager->Heightmap[ID.Heightmap] = LoadHeightmap(Arena, Asset);
//             string LogText = Format(Manager->Transient, "Loaded heightmap {s}.", 1, Asset->FileInfo.Path);
//             Log(log_level::Info, LogText.Content);
//         } break;

//         case Asset_Type_Font: {
//             Manager->Font[ID.Font] = LoadFont(Arena, Manager->Transient, &PreprocessedAssets.Font[ID.Font]);
//             Manager->Font[ID.Font].ID = ID.Font;
//             TriangulateFont(&Manager->FontsArena, Manager->Transient, &Manager->Font[ID.Font]);
//             string LogText = Format(Manager->Transient, "Loaded font {s}.", 1, Asset->FileInfo.Path);
//             Log(log_level::Info, LogText.Content);
//         } break;

//         case Asset_Type_Sound: {
//             Manager->Sound[ID.Sound] = LoadSound(Arena, &PreprocessedAssets.Sound[ID.Sound]);
//             Manager->Sound[ID.Sound].ID = ID.Sound;
//             string LogText = Format(Manager->Transient, "Loaded sound {s}.", 1, Asset->FileInfo.Path);
//             Log(log_level::Info, LogText.Content);
//         } break;

//         case Asset_Type_Mesh: {
//             Manager->Mesh[ID.Mesh] = LoadMesh(Arena, &PreprocessedAssets.Mesh[ID.Sound]);
//             Manager->Mesh[ID.Mesh].ID = ID.Mesh;
//             string LogText = Format(Manager->Transient, "Loaded mesh {s}.", 1, Asset->FileInfo.Path);
//             Log(log_level::Info, LogText.Content);
//         } break;

//         case Asset_Type_Animation: {
//             Manager->Animation[ID.Animation] = LoadAnimation(Arena, Asset);
//             string LogText = Format(Manager->Transient, "Loaded animation {s}.", 1, Asset->FileInfo.Path);
//             Log(log_level::Info, LogText.Content);
//         } break;

//         default: {
//             string LogText = Format(Manager->Transient, "Asset {s} ignored.", 1, Asset->FileInfo.Path);
//             Log(log_level::Warn, LogText.Content);
//         }
//     }

//     uint64 UsedMemory = Arena->Used - Asset->Offset;
//     Assert(Asset->MemoryNeeded == UsedMemory, "Assets memory needed doesn't match.");
//     Platform.FreeMemory(Asset->FileContent);
//     Asset->FileContent = nullptr;
// }

// void LoadAssetsFromFile(
//     game_asset_manager* Manager, 
//     const char* Path
// ) {
//     uint8* FileContent = (uint8*)Platform.ReadEntireFile(Path);

//     for (int i = 0; i < ASSET_COUNT; i++) {
//         game_asset Asset = Manager->Asset.Content[i];

//         switch (Asset.Type) {
//             case Asset_Type_Text: {
//                 game_text* Text = GetAsset(Manager, Asset.ID.Text);
//                 Text->Content = (char*)(FileContent + Asset.Offset);
//             } break;

//             case Asset_Type_Sound: {
//                 game_sound* Sound = GetAsset(Manager, Asset.ID.Sound);
//                 Sound->SampleOut = (int16*)(FileContent + Asset.Offset);
//             } break;

//             case Asset_Type_Texture: {
//                 game_texture* Texture = GetAsset(Manager, Asset.ID.Texture);
//                 Texture->Content = (uint32*)(FileContent + Asset.Offset);
//             } break;

//             case Asset_Type_Heightmap: {
//                 game_heightmap* Heightmap = GetAsset(Manager, Asset.ID.Heightmap);
//                 Heightmap->Texture.Content = (uint32*)(FileContent + Asset.Offset);
//             } break;

//             case Asset_Type_Font: {
//                 game_font* Font = GetAsset(Manager, Asset.ID.Font);
//                 uint8* Data = FileContent + Asset.Offset;
//                 for (int j = 0; j < FONT_CHARACTERS_COUNT; j++) {
//                     game_font_character* Character = &Font->Characters[j];
//                     if (Character->nContours == 0) Raise("Font character has no contours.");
//                     else if (Character->nContours > 0) {
//                         Character->Contours = (glyph_contour*)Data;
//                         Character->Data = (Data += Character->nContours * sizeof(glyph_contour));
//                         for (int k = 0; k < Character->nContours; k++) {
//                             Character->Contours[k].Points = (glyph_contour_point*)Data;
//                             Data += Character->Contours[k].nPoints * sizeof(glyph_contour_point);
//                         }
//                     }
//                     else if (Character->nContours < 0) {
//                         Character->Data = Data;
//                         Data += Character->nChildren * sizeof(composite_glyph_record);
//                     }
//                 }

//                 TriangulateFont(&Manager->FontsArena, Manager->Transient, Font);
//             } break;

//             case Asset_Type_Mesh: {
//                 Mesh* Mesh = GetAsset(Manager, Asset.ID.Mesh);
//                 Mesh->Vertices = (void*)(FileContent + Asset.Offset);
//                 uint32 Stride = VertexLayouts[Mesh->VertexLayoutID].Stride;
//                 Mesh->Edges = (uint32*)((uint8*)Mesh->Vertices + Stride * Mesh->nVertices);
//                 Mesh->Faces = Mesh->Edges + Mesh->nEdges;
//             } break;

//             case Asset_Type_Animation: {
//                 game_animation* Animation = GetAsset(Manager, Asset.ID.Animation);
//                 Animation->Content = (float*)(FileContent + Asset.Offset);
//             } break;

//             // case Asset_Type_Video: {
//             //     game_video* Video = &Assets->Videos[Asset.ID.Video];
//             //     Video->VideoContext.Buffer.Start = Assets->Memory + Asset.Offset;
//             //     Video->VideoContext.Buffer.FullSize = Asset.File.ContentSize;
//             //     Video->VideoContext.Buffer.ReadSize = 0;
//             //     InitializeVideo(&Video->VideoContext);
//             //     Video->Width = Video->VideoContext.Frame->width;
//             //     Video->Height = Video->VideoContext.Frame->height;
//             //     Video->Texture = {};
//             //     Video->Texture.ID = game_bitmap_id_count;
//             //     MakeBitmapHeader(&Video->Texture.Header, Video->Width, Video->Height, 32);
//             //     Video->Texture.BytesPerPixel = 4;
//             //     Video->Texture.Pitch = 4 * Video->Texture.Header.Width;
//             //     Video->Texture.Content = (uint32*)Video->VideoContext.VideoOut;
//             // } break;

//             default: {
//                 Raise("Asset type not implemented.");
//             }
//         }
//     }

//     Log(log_level::Info, "Assets loaded.");
// }

// game_animation* GetAnimation(game_asset_manager* AssetManager, string Name) {
//     for (int i = 0; i < AssetManager->Assets.Count; i++) {
//         game_asset* Asset = &AssetManager->Assets[i];
//         for (int j = 0; j < Asset->Header.nAnimations; j++) {
//             game_animation* Animation = &Asset->Animations[j];
//             if (Animation->Name == Name) {
//                 return Animation;
//             }
//         }
//     }
//     return nullptr;
// }

// game_font* GetFont(game_asset_manager* AssetManager, string Name) {
//     for (int i = 0; i < AssetManager->Assets.Count; i++) {
//         game_asset* Asset = &AssetManager->Assets[i];
//         for (int j = 0; j < Asset->Header.nFonts; j++) {
//             game_font* Font = &Asset->Fonts[j];
//             if (Font->Name == Name) {
//                 return Font;
//             }
//         }
//     }
//     return nullptr;
// }

// Mesh* GetMesh(game_asset_manager* AssetManager, string Name) {
//     for (int i = 0; i < AssetManager->Assets.Count; i++) {
//         game_asset* Asset = &AssetManager->Assets[i];
//         for (int j = 0; j < Asset->Header.nMeshes; j++) {
//             Mesh* Mesh = &Asset->Meshes[j];
//             if (Mesh->Name == Name) {
//                 return Mesh;
//             }
//         }
//     }
//     return nullptr;
// }

// game_texture* GetTexture(game_asset_manager* AssetManager, string Name) {
//     for (int i = 0; i < AssetManager->Assets.Count; i++) {
//         game_asset* Asset = &AssetManager->Assets[i];
//         for (int j = 0; j < Asset->Header.nTextures; j++) {
//             game_texture* Texture = &Asset->Textures[j];
//             if (Texture->Name == Name) {
//                 return Texture;
//             }
//         }
//     }
//     return nullptr;
// }

Asset_Manager InitializeAssetManager(Arena* permanent) {
    Asset_Manager result;
    result.arena = SuballocateMemoryArena(permanent, Megabytes(1));
    result.next_id = 1;

    // game_asset* Test = AddAsset(&Result, "GameAsset/Files/Manifest/Test.man" , game_asset_tag::TEST);

    return result;
}

#endif