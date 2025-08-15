#include "GameAssets.h"

#include "GameFont.cpp"
#include "GameBitmap.cpp"
#include "GameShader.cpp"
#include "GameMesh.cpp"

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

void WriteAssetsFile(platform_api* Platform, const char* Path) {
    game_assets Assets = {};
    Assets.Platform = Platform;

// Assets
    // Fonts
    PushAsset(&Assets, "..\\GameAssets\\Font\\Files\\CascadiaMono.ttf", Font_Cascadia_Mono_ID);
    PushAsset(&Assets, "..\\GameAssets\\Font\\Files\\Menlo-Regular.ttf", Font_Menlo_Regular_ID);

    // Text
    PushAsset(&Assets, "..\\GameAssets\\Text\\Files\\Test.txt", Text_Test_ID);

    // Bitmaps
    PushAsset(&Assets, "..\\GameAssets\\Bitmap\\Files\\Background.bmp", Bitmap_Background_ID);
    PushAsset(&Assets, "..\\GameAssets\\Bitmap\\Files\\Button.bmp", Bitmap_Button_ID);
    PushAsset(&Assets, "..\\GameAssets\\Bitmap\\Files\\Empty.bmp", Bitmap_Empty_ID);
    PushAsset(&Assets, "..\\GameAssets\\Bitmap\\Files\\Enemy.bmp", Bitmap_Enemy_ID);
    PushAsset(&Assets, "..\\GameAssets\\Bitmap\\Files\\Player.bmp", Bitmap_Player_ID);

    // Heightmaps
    PushAsset(&Assets, "..\\GameAssets\\Bitmap\\Files\\spain.bmp", Heightmap_Spain_ID);

    // Sound
    PushAsset(&Assets, "..\\GameAssets\\Sound\\Files\\16agosto.wav", Sound_Test_ID);

    // Meshes
    PushAsset(&Assets, "..\\GameAssets\\Mesh\\Files\\Horns.mdl", Mesh_Enemy_ID);
    PushAsset(&Assets, "..\\GameAssets\\Mesh\\Files\\Sphere.mdl", Mesh_Sphere_ID);
    PushAsset(&Assets, "..\\GameAssets\\Mesh\\Files\\Body.mdl", Mesh_Body_ID);
    PushAsset(&Assets, "..\\GameAssets\\Mesh\\Files\\Sword.mdl", Mesh_Sword_ID);
    PushAsset(&Assets, "..\\GameAssets\\Mesh\\Files\\Shield.mdl", Mesh_Shield_ID);
    PushAsset(&Assets, "..\\GameAssets\\Mesh\\Files\\Selector.mdl", Mesh_Selector_ID);

    // Animation
    PushAsset(&Assets, "..\\GameAssets\\Animation\\Files\\Idle.anim", Animation_Idle_ID);
    PushAsset(&Assets, "..\\GameAssets\\Animation\\Files\\Walking.anim", Animation_Walk_ID);
    PushAsset(&Assets, "..\\GameAssets\\Animation\\Files\\Jumping.anim", Animation_Jump_ID);
    PushAsset(&Assets, "..\\GameAssets\\Animation\\Files\\Attack.anim", Animation_Attack_ID);

    // Video
    //PushAsset(&Assets, "..\\GameAssets\\Videos\\The Witness Wrong MOOV.mp4", Video_Test_ID);

// Shaders
    // Vertex layouts
    Assets.VertexLayouts[vertex_layout_vec2_id]           = VertexLayout(1, shader_type_vec2);
    Assets.VertexLayouts[vertex_layout_vec2_vec2_id]      = VertexLayout(2, shader_type_vec2, shader_type_vec2);
    Assets.VertexLayouts[vertex_layout_vec3_id]           = VertexLayout(1, shader_type_vec3);
    Assets.VertexLayouts[vertex_layout_vec3_vec2_id]      = VertexLayout(2, shader_type_vec3, shader_type_vec2);
    Assets.VertexLayouts[vertex_layout_vec3_vec2_vec3_id] = VertexLayout(3, shader_type_vec3, shader_type_vec2, shader_type_vec3);
    Assets.VertexLayouts[vertex_layout_vec3_vec4_id]      = VertexLayout(2, shader_type_vec3, shader_type_vec4);
    Assets.VertexLayouts[vertex_layout_vec4_id]           = VertexLayout(1, shader_type_vec4);
    Assets.VertexLayouts[vertex_layout_bones_id]          = VertexLayout(5, shader_type_vec3, shader_type_vec2, shader_type_vec3, shader_type_ivec2, shader_type_vec2);
    for (int i = 0; i < vertex_layout_id_count; i++) Assets.VertexLayouts[i].ID = (vertex_layout_id)i;

    // Vertex
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Vertex\\Passthrough.vert", Vertex_Shader_Passthrough_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Vertex\\Screen.vert", Vertex_Shader_Screen_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Vertex\\ScreenTexture.vert", Vertex_Shader_Screen_Texture_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Vertex\\Perspective.vert", Vertex_Shader_Perspective_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Vertex\\Bones.vert", Vertex_Shader_Bones_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Vertex\\Barycentric.vert", Vertex_Shader_Barycentric_ID);
#if GAME_RENDER_API_VULKAN
    PushShader(&Assets, "..\\GameAssets\\Shaders\\Vertex\\VulkanTest.vert", Vertex_Shader_Vulkan_Test_ID);
#endif

    // Geometry
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Geometry\\Test.geom", Geometry_Shader_Test_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Geometry\\DebugNormals.geom", Geometry_Shader_Debug_Normals_ID);

    // Tessellation
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Tessellation\\Heightmap.tesc", TESC_Heightmap_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Tessellation\\Bezier.tesc", TESC_Bezier_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Tessellation\\Heightmap.tese", TESE_Heightmap_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Tessellation\\Trochoidal.tese", TESE_Trochoidal_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Tessellation\\Bezier.tese", TESE_Bezier_ID);

    // Fragment
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Antialiasing.frag", Fragment_Shader_Antialiasing_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\FramebufferAttachment.frag", Fragment_Shader_Framebuffer_Attachment_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Texture.frag", Fragment_Shader_Texture_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Outline.frag", Fragment_Shader_Outline_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\SingleColor.frag", Fragment_Shader_Single_Color_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Kernel.frag", Fragment_Shader_Kernel_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Sphere.frag", Fragment_Shader_Sphere_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Mesh.frag", Fragment_Shader_Mesh_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\JumpFlood.frag", Fragment_Shader_Jump_Flood_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Heightmap.frag", Fragment_Shader_Heightmap_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\Sea.frag", Fragment_Shader_Sea_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\BezierExterior.frag", Fragment_Shader_Bezier_Exterior_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\BezierInterior.frag", Fragment_Shader_Bezier_Interior_ID);
#if GAME_RENDER_API_VULKAN
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Fragment\\VulkanTest.frag", Fragment_Shader_Vulkan_Test_ID);
#endif

    // Shader pipelines
    PushShaderPipeline(&Assets, Shader_Pipeline_Antialiasing_ID,        2, Vertex_Shader_Passthrough_ID,    Fragment_Shader_Antialiasing_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Framebuffer_ID,         2, Vertex_Shader_Passthrough_ID,    Fragment_Shader_Framebuffer_Attachment_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Texture_ID,             2, Vertex_Shader_Screen_Texture_ID, Fragment_Shader_Texture_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Mesh_ID,                2, Vertex_Shader_Perspective_ID,    Fragment_Shader_Mesh_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Mesh_Bones_ID,          2, Vertex_Shader_Bones_ID,          Fragment_Shader_Mesh_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Sphere_ID,              2, Vertex_Shader_Perspective_ID,    Fragment_Shader_Sphere_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_World_Single_Color_ID,  2, Vertex_Shader_Perspective_ID,    Fragment_Shader_Single_Color_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Screen_Single_Color_ID, 2, Vertex_Shader_Screen_ID,         Fragment_Shader_Single_Color_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Bones_Single_Color_ID,  2, Vertex_Shader_Bones_ID,          Fragment_Shader_Single_Color_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Outline_ID,             2, Vertex_Shader_Passthrough_ID,    Fragment_Shader_Outline_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Bezier_Exterior_ID,     2, Vertex_Shader_Barycentric_ID,    Fragment_Shader_Bezier_Exterior_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Bezier_Interior_ID,     2, Vertex_Shader_Barycentric_ID,    Fragment_Shader_Bezier_Interior_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Jump_Flood_ID,          2, Vertex_Shader_Passthrough_ID,    Fragment_Shader_Jump_Flood_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Debug_Normals_ID,       3,
        Vertex_Shader_Bones_ID,
        Geometry_Shader_Debug_Normals_ID,
        Fragment_Shader_Single_Color_ID
    );
    //PushShaderPipeline(&Assets, Shader_Pipeline_Kernel_ID, Vertex_Shader_Framebuffer_ID, Fragment_Shader_Kernel_ID);
    PushShaderPipeline(&Assets, Shader_Pipeline_Heightmap_ID, 4, 
        Vertex_Shader_Passthrough_ID, 
        TESC_Heightmap_ID, 
        TESE_Heightmap_ID, 
        Fragment_Shader_Heightmap_ID
    );
    PushShaderPipeline(&Assets, Shader_Pipeline_Trochoidal_ID, 4,
        Vertex_Shader_Passthrough_ID,
        TESC_Heightmap_ID,
        TESE_Trochoidal_ID,
        Fragment_Shader_Sea_ID
    );
    PushShaderPipeline(&Assets, Shader_Pipeline_Text_Outline_ID, 4,
        Vertex_Shader_Screen_ID,
        TESC_Bezier_ID,
        TESE_Bezier_ID,
        Fragment_Shader_Single_Color_ID
    );

#if GAME_RENDER_API_VULKAN
    PushShaderPipeline(&Assets, Shader_Pipeline_Vulkan_Test_ID, 2, Vertex_Shader_Vulkan_Test_ID, Fragment_Shader_Vulkan_Test_ID);
#endif
    
    // Compute
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Compute\\OutlineInit.comp", Compute_Shader_Outline_Init_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Compute\\JumpFlood.comp", Compute_Shader_Jump_Flood_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Compute\\Test.comp", Compute_Shader_Test_ID);
    PushShader(&Assets, "..\\GameAssets\\Shader\\Files\\Compute\\Kernel.comp", Compute_Shader_Kernel_ID);

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
        LoadShader(Platform, &AssetArena, &Assets.Shader[i]);
    }

    // Compute shaders
    for (int i = 0; i < game_compute_shader_id_count; i++) {
        LoadComputeShader(Platform, &AssetArena, &Assets.ComputeShader[i]);
    }

    // Shader pipelines vertex and uniform layouts
    LoadShaderPipelines(&Assets);

    game_assets* OutputAssets = (game_assets*)FileMemory;
    if (OutputAssets) *OutputAssets = Assets;
    Platform->WriteEntireFile(Path, sizeof(game_assets) + Assets.TotalSize, FileMemory);

    Log(Info, "Finished writing assets file.");

    VirtualFree(FileMemory, 0, MEM_RELEASE);
}

void LoadAssetsFromFile(platform_read_entire_file Read, game_assets* Assets, const char* Path) {
    read_file_result AssetsFile = Read(Path);

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
            } break;

            case Asset_Type_Mesh: {
                game_mesh* Mesh = GetAsset(Assets, Asset.ID.Mesh);
                Mesh->Vertices = (void*)(Assets->Memory + Asset.Offset);
                vertex_layout Layout = Assets->VertexLayouts[Mesh->LayoutID];
                Mesh->Faces = (uint32*)((uint8*)Mesh->Vertices + Layout.Stride * Mesh->nVertices);
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
