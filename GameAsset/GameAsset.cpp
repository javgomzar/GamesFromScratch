#include "GameAsset.h"

void WriteAssetsFile(const char* Path) {
    game_assets Assets = {};

// Assets
    // Fonts
    PushAsset(&Assets, "GameAsset\\Files\\Font\\DejaVuSansMono.ttf", Font_DejaVu_Sans_Mono_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Font\\DejaVuSans.ttf", Font_DejaVu_Sans_ID);

    // Text
    PushAsset(&Assets, "GameAsset\\Files\\Text\\Test.txt", Text_Test_ID);

    // Textures
    PushAsset(&Assets, "GameAsset\\Files\\Texture\\Background.bmp", Texture_Background_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Texture\\Button.bmp", Texture_Button_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Texture\\Empty.bmp", Texture_Empty_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Texture\\Enemy.bmp", Texture_Enemy_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Texture\\Player.bmp", Texture_Player_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Texture\\spain.bmp", Texture_Spain_ID);

    // Heightmaps
    PushAsset(&Assets, "GameAsset\\Files\\Texture\\spain.bmp", Heightmap_Spain_ID);

    // Sound
    PushAsset(&Assets, "GameAsset\\Files\\Sound\\16agosto.wav", Sound_Test_ID);

    // Meshes
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Tetrahedron.mdl", Mesh_Tetrahedron_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Cube.mdl", Mesh_Cube_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Octahedron.mdl", Mesh_Octahedron_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Icosahedron.mdl", Mesh_Icosahedron_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Dodecahedron.mdl", Mesh_Dodecahedron_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Horns.mdl", Mesh_Enemy_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Sphere.mdl", Mesh_Sphere_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Body.mdl", Mesh_Body_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Sword.mdl", Mesh_Sword_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Shield.mdl", Mesh_Shield_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Mesh\\Selector.mdl", Mesh_Selector_ID);

    // Animation
    PushAsset(&Assets, "GameAsset\\Files\\Animation\\Idle.anim", Animation_Idle_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Animation\\Walking.anim", Animation_Walk_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Animation\\Jumping.anim", Animation_Jump_ID);
    PushAsset(&Assets, "GameAsset\\Files\\Animation\\Attack.anim", Animation_Attack_ID);

    // Video
    //PushAsset(&Assets, "GameAsset\\Videos\\The Witness Wrong MOOV.mp4", Video_Test_ID);

// Output file
    void* FileMemory = Platform.AllocateMemory(sizeof(game_assets) + Assets.TotalSize);
    Assets.Memory = (uint8*)FileMemory + sizeof(game_assets);
    memory_arena AssetArena = MemoryArena(Assets.TotalSize, (uint8*)Assets.Memory);

    // Assets
    for (int i = 0; i < Assets.Asset.Count; i++) {
        LoadAsset(&AssetArena, &Assets, &Assets.Asset.Content[i]);
    }

    game_assets* OutputAssets = (game_assets*)FileMemory;
    if (OutputAssets) *OutputAssets = Assets;
    Platform.WriteEntireFile(Path, sizeof(game_assets) + Assets.TotalSize, FileMemory);

    Log(log_level::Info, "Finished writing assets file.");

    Platform.FreeMemory(FileMemory);
}

void LoadAssetsFromFile(
    memory_arena* FontsArena,
    game_assets* Assets, 
    const char* Path
) {
    void* AssetsFileContent = Platform.ReadEntireFile(Path);

    *Assets = *(game_assets*)AssetsFileContent;
    Assets->Memory = (uint8*)AssetsFileContent + sizeof(game_assets);

    for (int i = 0; i < ASSET_COUNT; i++) {
        game_asset Asset = Assets->Asset.Content[i];

        switch (Asset.Type) {
            case Asset_Type_Text: {
                game_text* Text = GetAsset(Assets, Asset.ID.Text);
                Text->Content = (char*)(Assets->Memory + Asset.Offset);
            } break;

            case Asset_Type_Sound: {
                game_sound* Sound = GetAsset(Assets, Asset.ID.Sound);
                Sound->SampleOut = (int16*)(Assets->Memory + Asset.Offset);
            } break;

            case Asset_Type_Texture: {
                game_texture* Texture = GetAsset(Assets, Asset.ID.Texture);
                Texture->Content = (uint32*)(Assets->Memory + Asset.Offset);
            } break;

            case Asset_Type_Heightmap: {
                game_heightmap* Heightmap = GetAsset(Assets, Asset.ID.Heightmap);
                Heightmap->Texture.Content = (uint32*)(Assets->Memory + Asset.Offset);
            } break;

            case Asset_Type_Font: {
                game_font* Font = GetAsset(Assets, Asset.ID.Font);
                uint8* Data = Assets->Memory + Asset.Offset;
                for (int j = 0; j < FONT_CHARACTERS_COUNT; j++) {
                    game_font_character* Character = &Font->Characters[j];
                    if (Character->nContours == 0) Raise("Font character has no contours.");
                    else if (Character->nContours > 0) {
                        Character->Contours = (glyph_contour*)Data;
                        Character->Data = (Data += Character->nContours * sizeof(glyph_contour));
                        for (int k = 0; k < Character->nContours; k++) {
                            Character->Contours[k].Points = (glyph_contour_point*)Data;
                            Data += Character->Contours[k].nPoints * sizeof(glyph_contour_point);
                        }
                    }
                    else if (Character->nContours < 0) {
                        Character->Data = Data;
                        Data += Character->nChildren * sizeof(composite_glyph_record);
                    }
                }

                WriteFontVertices(FontsArena, Font);
                WriteFontCurveTriangles(FontsArena, Font);
                WriteFontSolidTriangles(FontsArena, Font);
            } break;

            case Asset_Type_Mesh: {
                game_mesh* Mesh = GetAsset(Assets, Asset.ID.Mesh);
                Mesh->Vertices = (void*)(Assets->Memory + Asset.Offset);
                uint32 Stride = VertexLayouts[Mesh->VertexLayoutID].Stride;
                Mesh->Edges = (uint32*)((uint8*)Mesh->Vertices + Stride * Mesh->nVertices);
                Mesh->Faces = Mesh->Edges + Mesh->nEdges;
            } break;

            case Asset_Type_Animation: {
                game_animation* Animation = GetAsset(Assets, Asset.ID.Animation);
                Animation->Content = (float*)(Assets->Memory + Asset.Offset);
            } break;

            // case Asset_Type_Video: {
            //     game_video* Video = &Assets->Videos[Asset.ID.Video];
            //     Video->VideoContext.Buffer.Start = Assets->Memory + Asset.Offset;
            //     Video->VideoContext.Buffer.FullSize = Asset.File.ContentSize;
            //     Video->VideoContext.Buffer.ReadSize = 0;
            //     InitializeVideo(&Video->VideoContext);
            //     Video->Width = Video->VideoContext.Frame->width;
            //     Video->Height = Video->VideoContext.Frame->height;
            //     Video->Texture = {};
            //     Video->Texture.ID = game_bitmap_id_count;
            //     MakeBitmapHeader(&Video->Texture.Header, Video->Width, Video->Height, 32);
            //     Video->Texture.BytesPerPixel = 4;
            //     Video->Texture.Pitch = 4 * Video->Texture.Header.Width;
            //     Video->Texture.Content = (uint32*)Video->VideoContext.VideoOut;
            // } break;

            default: {
                Raise("Asset type not implemented.");
            }
        }
    }

    Log(log_level::Info, "Assets loaded.");
}
