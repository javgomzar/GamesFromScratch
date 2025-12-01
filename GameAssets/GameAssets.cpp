#include "GameAssets.h"

void WriteAssetsFile(const char* Path) {
    game_assets Assets = {};

// Vertex layouts
    Assets.VertexLayout[vertex_layout_v2_id]       = VertexLayout(1, vertex_type_v2);
    Assets.VertexLayout[vertex_layout_v2_v2_id]    = VertexLayout(2, vertex_type_v2, vertex_type_v2);
    Assets.VertexLayout[vertex_layout_v3_id]       = VertexLayout(1, vertex_type_v3);
    Assets.VertexLayout[vertex_layout_v3_v2_id]    = VertexLayout(2, vertex_type_v3, vertex_type_v2);
    Assets.VertexLayout[vertex_layout_v3_v2_v3_id] = VertexLayout(3, vertex_type_v3, vertex_type_v2, vertex_type_v3);
    Assets.VertexLayout[vertex_layout_v3_v4_id]    = VertexLayout(2, vertex_type_v3, vertex_type_v4);
    Assets.VertexLayout[vertex_layout_v4_id]       = VertexLayout(1, vertex_type_v4);
    Assets.VertexLayout[vertex_layout_bones_id]    = VertexLayout(5, vertex_type_v3, vertex_type_v2, vertex_type_v3, vertex_type_iv2, vertex_type_v2);
    for (int i = 0; i < vertex_layout_id_count; i++) Assets.VertexLayout[i].ID = (vertex_layout_id)i;

// Assets
    // Fonts
    PushAsset(&Assets, "GameAssets\\Files\\Font\\Menlo-Regular.ttf", Font_Menlo_Regular_ID);

    // Text
    PushAsset(&Assets, "GameAssets\\Files\\Text\\Test.txt", Text_Test_ID);

    // Bitmaps
    PushAsset(&Assets, "GameAssets\\Files\\Bitmap\\Background.bmp", Bitmap_Background_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Bitmap\\Button.bmp", Bitmap_Button_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Bitmap\\Empty.bmp", Bitmap_Empty_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Bitmap\\Enemy.bmp", Bitmap_Enemy_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Bitmap\\Player.bmp", Bitmap_Player_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Bitmap\\spain.bmp", Bitmap_Spain_ID);

    // Heightmaps
    PushAsset(&Assets, "GameAssets\\Files\\Bitmap\\spain.bmp", Heightmap_Spain_ID);

    // Sound
    PushAsset(&Assets, "GameAssets\\Files\\Sound\\16agosto.wav", Sound_Test_ID);

    // Meshes
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Tetrahedron.mdl", Mesh_Tetrahedron_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Cube.mdl", Mesh_Cube_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Octahedron.mdl", Mesh_Octahedron_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Icosahedron.mdl", Mesh_Icosahedron_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Dodecahedron.mdl", Mesh_Dodecahedron_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Horns.mdl", Mesh_Enemy_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Sphere.mdl", Mesh_Sphere_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Body.mdl", Mesh_Body_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Sword.mdl", Mesh_Sword_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Shield.mdl", Mesh_Shield_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Mesh\\Selector.mdl", Mesh_Selector_ID);

    // Animation
    PushAsset(&Assets, "GameAssets\\Files\\Animation\\Idle.anim", Animation_Idle_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Animation\\Walking.anim", Animation_Walk_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Animation\\Jumping.anim", Animation_Jump_ID);
    PushAsset(&Assets, "GameAssets\\Files\\Animation\\Attack.anim", Animation_Attack_ID);

    // Video
    //PushAsset(&Assets, "GameAssets\\Videos\\The Witness Wrong MOOV.mp4", Video_Test_ID);

// Output file
    void* FileMemory = VirtualAlloc(0, sizeof(game_assets) + Assets.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Assets.Memory = (uint8*)FileMemory + sizeof(game_assets);
    memory_arena AssetArena = MemoryArena(Assets.TotalSize, (uint8*)Assets.Memory);

    // Assets
    for (int i = 0; i < Assets.Asset.Count; i++) {
        LoadAsset(&AssetArena, &Assets, &Assets.Asset.Content[i]);
    }

    game_assets* OutputAssets = (game_assets*)FileMemory;
    if (OutputAssets) *OutputAssets = Assets;
    Platform.WriteEntireFile(Path, sizeof(game_assets) + Assets.TotalSize, FileMemory);

    Log(Info, "Finished writing assets file.");

    VirtualFree(FileMemory, 0, MEM_RELEASE);
}

void LoadAssetsFromFile(
    memory_arena* FontsArena,
    game_assets* Assets, 
    const char* Path
) {
    read_file_result AssetsFile = Platform.ReadEntireFile(Path);

    *Assets = *(game_assets*)AssetsFile.Content;
    Assets->Memory = (uint8*)AssetsFile.Content + sizeof(game_assets);

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

            case Asset_Type_Bitmap: {
                game_bitmap* Bitmap = GetAsset(Assets, Asset.ID.Bitmap);
                Bitmap->Content = (uint32*)(Assets->Memory + Asset.Offset);
            } break;

            case Asset_Type_Heightmap: {
                game_heightmap* Heightmap = GetAsset(Assets, Asset.ID.Heightmap);
                Heightmap->Bitmap.Content = (uint32*)(Assets->Memory + Asset.Offset);
                uint64 BitmapSize = PreprocessBitmap(&Heightmap->Bitmap.Header);
                Heightmap->Vertices = (float*)(Assets->Memory + Asset.Offset + BitmapSize);
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
                uint32 Stride = Assets->VertexLayout[Mesh->VertexLayoutID].Stride;
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

    Log(Info, "Assets loaded.");
}
