
#include <iostream>
#include "GameAssets.h"


/*
    TODO:
        - Make fonts load bitmaps into a single bitmap that can be cropped to select a certain character (Atlas).
        - Fix video. Possibly extract all frames and load them to the asset file.
*/

void PushAsset(game_assets* Assets, const char* Path, game_asset_id ID) {
    game_asset Asset = {};
    Asset.ID = ID;

    // Read file
    Asset.File = PlatformReadEntireFile(Path);
    Asset.Path = Path;

    if (Asset.File.ContentSize == 0) {
        Assert(false);
    }

    // Get asset type from file extension
    WIN32_FIND_DATAA Data;
    HANDLE hFind = FindFirstFileA(Path, &Data);

    if (hFind == INVALID_HANDLE_VALUE) {
        Assert(false);
    }

    char* Extension = 0;
    char* Buffer = new char[strlen(Data.cFileName) + 1];
    strcpy_s(Buffer, strlen(Data.cFileName) + 1, Data.cFileName);
    char* _ = strtok_s(Buffer, ".", &Extension);

    if (Extension != 0) {
        if (
            strcmp(Extension, "txt") == 0 ||
            strcmp(Extension, "h.glsl") == 0 ||
            strcmp(Extension, "vert.glsl") == 0 ||
            strcmp(Extension, "frag.glsl") == 0
        ) Asset.Type = Text;
        else if (strcmp(Extension, "bmp") == 0) Asset.Type = Bitmap;
        else if (strcmp(Extension, "wav") == 0) Asset.Type = Sound;
        else if (strcmp(Extension, "mp4") == 0) Asset.Type = Video;
        else if (strcmp(Extension, "mdl") == 0) Asset.Type = Mesh;
        else if (strcmp(Extension, "ttf") == 0) Asset.Type = Font;
        else Assert(false);

        switch (Asset.Type) {
            case Text: {
                Asset.MemoryNeeded = Asset.File.ContentSize + 1;
            } break;

            case Bitmap: {
                bitmap_header Header = *(bitmap_header*)Asset.File.Content;
                Asset.MemoryNeeded = Asset.File.ContentSize - Header.BitmapOffset;
            } break;

            case Font: {
                Asset.MemoryNeeded = ComputeNeededMemoryForFont(Asset.Path);
            } break;

            case Mesh: {
                Asset.MemoryNeeded = ComputeNeededMemoryForMesh(Asset.File);
            } break;

            case Sound: {
                Asset.MemoryNeeded = ComputeNeededMemoryForSound(Asset.File);
            } break;

            default: {
                Assert(false);
            }
        }

        Assets->TotalSize += Asset.MemoryNeeded;
        Assets->Asset[Asset.ID] = Asset;
    }
    else {
        std::cout << "No extension found for file " << Path;
        Assert(false);
    }
}

//void PushDir(game_assets* Assets, const char* Path) {
//    WIN32_FIND_DATAA Data;
//    HANDLE hFind = FindFirstFileA(Path, &Data);
//
//    if (hFind == INVALID_HANDLE_VALUE) {
//        char Buffer[256] = "Invalid asset directory: ";
//        strcat_s(Buffer, Path);
//        OutputDebugStringA(Buffer);
//        Assert(false);
//    }
//
//    do {
//        if (strcmp(Data.cFileName, ".") != 0 && strcmp(Data.cFileName, "..") != 0) {
//            uint64 DestSize = strlen(Path);
//            char* Dir = new char[DestSize];
//            strncpy_s(Dir, DestSize, Path, strlen(Path) - 1);
//            PushAsset(Assets, Dir, Data.cFileName);
//        }
//    } while (FindNextFileA(hFind, &Data));
//}

void PushShader(
    game_assets* Assets, 
    game_shader_id ShaderID, 
    game_asset_id Header, 
    game_asset_id Vertex,
    game_asset_id Fragment
) {
    game_shader* Shader = &Assets->Shaders[ShaderID];
    Shader->HeaderShaderID = Header;
    Shader->VertexShaderID = Vertex;
    Shader->FragmentShaderID = Fragment;
}

void LoadAsset(memory_arena* Arena, game_assets* Assets, game_asset_id ID) {
    game_asset* Asset = &Assets->Asset[ID];
    Asset->Index = Assets->n[Asset->Type]++;
    Asset->Offset = Arena->Used;
    switch (Asset->Type) {
        case Text: {
            char* TextContent = (char*)PushSize(Arena, Asset->MemoryNeeded);
            Assets->Texts[Asset->Index] = TextContent;
            memcpy(TextContent, Asset->File.Content, Asset->MemoryNeeded);
            std::cout << "Loaded text " << Asset->Path << "\n";
        } break;

        //case Video: {
        //    Assets->Videos[Asset->Index] = AssetLoadVideo(Arena, Asset);
        //    std::cout << "Loaded video " << Asset->Path << "\n";
        //} break;

        case Bitmap: {
            Assets->Bitmaps[Asset->Index] = AssetLoadBitmap(Arena, Asset);
            std::cout << "Loaded bitmap " << Asset->Path << "\n";
        } break;

        case Font: {
            Assets->Fonts[Asset->Index] = AssetLoadFont(Arena, Asset);
            std::cout << "Loaded font " << Asset->Path << "\n";
        } break;

        case Sound: {
            Assets->Sounds[Asset->Index] = AssetLoadSound(Arena, Asset);
            std::cout << "Loaded sound " << Asset->Path << "\n";
        } break;

        case Mesh: {
            Assets->Meshes[Asset->Index] = AssetLoadMesh(Arena, Asset);
            std::cout << "Loaded mesh " << Asset->Path << "\n";
        } break;

        default: {
            std::cout << "Asset " << Asset->Path << " was ignored because of unknown type.\n";
        }
    }
    Assert(Asset->MemoryNeeded == Arena->Used - Asset->Offset);
    Asset->File = { 0 };
}

int main() {
    game_assets Assets = {};

// Assets
    // Fonts
    PushAsset(&Assets, "..\\..\\Assets\\Fonts\\CascadiaMono.ttf", Font_Cascadia_Mono_ID);

    // Text
    PushAsset(&Assets, "..\\..\\Assets\\Texts\\Test.txt", Text_Test_ID);

    // Bitmaps
    PushAsset(&Assets, "..\\..\\Assets\\Bitmaps\\Background.bmp", Bitmap_Background_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Bitmaps\\Button.bmp", Bitmap_Button_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Bitmaps\\Empty.bmp", Bitmap_Empty_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Bitmaps\\Enemy.bmp", Bitmap_Enemy_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Bitmaps\\Player.bmp", Bitmap_Player_ID);
    
    // Sound
    PushAsset(&Assets, "..\\..\\Assets\\Sounds\\16agosto.wav", Sound_Test_ID);

    // Meshes
    PushAsset(&Assets, "..\\..\\Assets\\Models\\Enemy.mdl", Mesh_Enemy_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Models\\Sphere.mdl", Mesh_Sphere_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Models\\Body.mdl", Mesh_Body_ID);

    // Video
    //PushAsset(&Assets, "..\\..\\Assets\\Videos\\The Witness.mp4", Video_Test_ID);

// Shaders
    // Header
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\HeaderShader.h.glsl", Header_Shader_ID);

    // Vertex
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\VertexShader.vert.glsl", Vertex_Shader_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\FramebufferVertexShader.vert.glsl", Vertex_Shader_Framebuffer_ID);

    // Fragment
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\FramebufferFragmentShader.frag.glsl", Fragment_Shader_Framebuffer_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\AntialiasingFragmentShader.frag.glsl", Fragment_Shader_Antialiasing_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\OutlineInitFragmentShader.frag.glsl", Fragment_Shader_Outline_Init_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\JumpFloodFragmentShader.frag.glsl", Fragment_Shader_JFA_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\OutlineFragmentShader.frag.glsl", Fragment_Shader_Outline_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\SingleColorFragmentShader.frag.glsl", Fragment_Shader_Single_Color_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\KernelFragmentShader.frag.glsl", Fragment_Shader_Kernel_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\SphereFragmentShader.frag.glsl", Fragment_Shader_Sphere_ID);
    PushAsset(&Assets, "..\\..\\Assets\\Shaders\\TextureFragmentShader.frag.glsl", Fragment_Shader_Texture_ID);

    PushShader(&Assets, Shader_Framebuffer_ID, Header_Shader_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Framebuffer_ID);
    PushShader(&Assets, Shader_Texture_ID, Header_Shader_ID, Vertex_Shader_ID, Fragment_Shader_Texture_ID);
    PushShader(&Assets, Shader_Sphere_ID, Header_Shader_ID, Vertex_Shader_ID, Fragment_Shader_Sphere_ID);
    PushShader(&Assets, Shader_Single_Color_ID, Header_Shader_ID, Vertex_Shader_ID, Fragment_Shader_Single_Color_ID);
    PushShader(&Assets, Shader_Outline_Init_ID, Header_Shader_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Outline_Init_ID);
    PushShader(&Assets, Shader_JFA_ID, Header_Shader_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_JFA_ID);
    PushShader(&Assets, Shader_Outline_ID, Header_Shader_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Outline_ID);
    PushShader(&Assets, Shader_Kernel_ID, Header_Shader_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Kernel_ID);
    PushShader(&Assets, Shader_Antialiasing_ID, Header_Shader_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Antialiasing_ID);

// Output file
    void* FileMemory = VirtualAlloc(0, sizeof(game_assets) + Assets.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Assets.Memory = (uint8*)FileMemory + sizeof(game_assets);
    memory_arena AssetArena;
    InitializeArena(&AssetArena, Assets.TotalSize, (uint8*)Assets.Memory);

    for (int i = 0; i < game_asset_id_count; i++) {
        LoadAsset(&AssetArena, &Assets, (game_asset_id)i);
    }

    game_assets* OutputAssets = (game_assets*)FileMemory;
    if (OutputAssets) *OutputAssets = Assets;
    PlatformWriteEntireFile("..\\..\\game_assets", sizeof(game_assets) + Assets.TotalSize, FileMemory);

    std::cout << "\nFINISHED\n\n";
}
