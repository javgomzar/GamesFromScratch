#include "GameAsset.h"

game_asset_manager InitializeAssetManager(memory_arena* Permanent, memory_arena* Transient) {
    game_asset_manager Result = {};
    Result.Transient = Transient;

// Assets
    // Fonts
    PushAsset(&Result, "GameAsset\\Files\\Font\\DejaVuSansMono.ttf", Font_DejaVu_Sans_Mono_ID);
    PushAsset(&Result, "GameAsset\\Files\\Font\\DejaVuSans.ttf", Font_DejaVu_Sans_ID);

    // Text
    PushAsset(&Result, "GameAsset\\Files\\Text\\Test.txt", Text_Test_ID);

    // Textures
    PushAsset(&Result, "GameAsset\\Files\\Texture\\Background.bmp", Texture_Background_ID);
    PushAsset(&Result, "GameAsset\\Files\\Texture\\Button.bmp", Texture_Button_ID);
    PushAsset(&Result, "GameAsset\\Files\\Texture\\Empty.bmp", Texture_Empty_ID);
    PushAsset(&Result, "GameAsset\\Files\\Texture\\Enemy.bmp", Texture_Enemy_ID);
    PushAsset(&Result, "GameAsset\\Files\\Texture\\Player.bmp", Texture_Player_ID);
    PushAsset(&Result, "GameAsset\\Files\\Texture\\spain.bmp", Texture_Spain_ID);

    // Heightmaps
    PushAsset(&Result, "GameAsset\\Files\\Texture\\spain.bmp", Heightmap_Spain_ID);

    // Sound
    PushAsset(&Result, "GameAsset\\Files\\Sound\\16agosto.wav", Sound_Test_ID);

    // Meshes
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Tetrahedron.mdl", Mesh_Tetrahedron_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Cube.mdl", Mesh_Cube_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Octahedron.mdl", Mesh_Octahedron_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Icosahedron.mdl", Mesh_Icosahedron_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Dodecahedron.mdl", Mesh_Dodecahedron_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Horns.mdl", Mesh_Enemy_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Sphere.mdl", Mesh_Sphere_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Body.mdl", Mesh_Body_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Sword.mdl", Mesh_Sword_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Shield.mdl", Mesh_Shield_ID);
    PushAsset(&Result, "GameAsset\\Files\\Mesh\\Selector.mdl", Mesh_Selector_ID);

    // Animation
    PushAsset(&Result, "GameAsset\\Files\\Animation\\Idle.anim", Animation_Idle_ID);
    PushAsset(&Result, "GameAsset\\Files\\Animation\\Walking.anim", Animation_Walk_ID);
    PushAsset(&Result, "GameAsset\\Files\\Animation\\Jumping.anim", Animation_Jump_ID);
    PushAsset(&Result, "GameAsset\\Files\\Animation\\Attack.anim", Animation_Attack_ID);

    // Video
    //PushAsset(&Result, "GameAsset\\Videos\\The Witness Wrong MOOV.mp4", Video_Test_ID);

    Result.Arena = SuballocateMemoryArena(Permanent, Result.TotalSize);
    Result.FontsArena = SuballocateMemoryArena(Permanent, Megabytes(1));

    return Result;
}

void WriteAssetsFile(game_asset_manager* Manager, const char* Path) {
// Output file
    Platform.WriteEntireFile(Path, Manager->TotalSize, Manager->Arena.Base);

    Log(log_level::Info, "Finished writing assets file.");
}

void LoadAssetsFromFile(
    game_asset_manager* Manager, 
    const char* Path
) {
    uint8* FileContent = (uint8*)Platform.ReadEntireFile(Path);

    for (int i = 0; i < ASSET_COUNT; i++) {
        game_asset Asset = Manager->Asset.Content[i];

        switch (Asset.Type) {
            case Asset_Type_Text: {
                game_text* Text = GetAsset(Manager, Asset.ID.Text);
                Text->Content = (char*)(FileContent + Asset.Offset);
            } break;

            case Asset_Type_Sound: {
                game_sound* Sound = GetAsset(Manager, Asset.ID.Sound);
                Sound->SampleOut = (int16*)(FileContent + Asset.Offset);
            } break;

            case Asset_Type_Texture: {
                game_texture* Texture = GetAsset(Manager, Asset.ID.Texture);
                Texture->Content = (uint32*)(FileContent + Asset.Offset);
            } break;

            case Asset_Type_Heightmap: {
                game_heightmap* Heightmap = GetAsset(Manager, Asset.ID.Heightmap);
                Heightmap->Texture.Content = (uint32*)(FileContent + Asset.Offset);
            } break;

            case Asset_Type_Font: {
                game_font* Font = GetAsset(Manager, Asset.ID.Font);
                uint8* Data = FileContent + Asset.Offset;
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

                TriangulateFont(&Manager->FontsArena, Manager->Transient, Font);
            } break;

            case Asset_Type_Mesh: {
                game_mesh* Mesh = GetAsset(Manager, Asset.ID.Mesh);
                Mesh->Vertices = (void*)(FileContent + Asset.Offset);
                uint32 Stride = VertexLayouts[Mesh->VertexLayoutID].Stride;
                Mesh->Edges = (uint32*)((uint8*)Mesh->Vertices + Stride * Mesh->nVertices);
                Mesh->Faces = Mesh->Edges + Mesh->nEdges;
            } break;

            case Asset_Type_Animation: {
                game_animation* Animation = GetAsset(Manager, Asset.ID.Animation);
                Animation->Content = (float*)(FileContent + Asset.Offset);
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
