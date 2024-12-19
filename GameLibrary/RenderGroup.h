#pragma once
#include "../../GameAssets/GameAssets.h"

/*
    TODO:
        - Put circles in its own group_type (too many lines)
*/

const int MAX_RENDER_ENTRIES = 10000;

enum render_group_entry_type {
    group_type_render_entry_clear,
    group_type_render_entry_line,
    group_type_render_entry_triangle,
    group_type_render_entry_rect,
    group_type_render_entry_textured_rect,
    group_type_render_entry_text,
    group_type_render_entry_video,
    group_type_render_entry_mesh,
    group_type_render_entry_mesh_outline,
    group_type_render_entry_shader_pass,
    group_type_render_entry_render_target,
    group_type_render_entry_debug_grid
};

enum render_group_target {
    World,
    Outline,
    Postprocessing_Outline,
    PingPong,
    Output,

    render_group_target_count
};

enum coordinate_system {
    World_Coordinates,
    Screen_Coordinates
};

enum wrap_mode {
    Clamp,
    Repeat,
    Crop
};

struct sort_key {
    double Order;
};

bool LessThan(sort_key Key1, sort_key Key2) {
    return Key1.Order < Key2.Order;
}

struct render_group_header {
    render_group_entry_type Type;
    render_group_target Target;
    sort_key Key;
    uint32 PushBufferOffset;
};

struct render_entry_clear {
    render_group_header Header;
    color Color;
};

struct render_entry_line {
    render_group_header Header;
    coordinate_system Coordinates;
    color Color;
    v3 Start;
    v3 Finish;
    float Thickness;
};

struct render_entry_triangle {
    render_group_header Header;
    coordinate_system Coordinates;
    game_triangle Triangle;
    color Color;
};

struct render_entry_rect {
    render_group_header Header;
    game_rect Rect;
    color Color;
};

struct render_entry_textured_rect {
    render_group_header Header;
    game_rect Rect;
    game_bitmap* Texture;
    color Color;
    wrap_mode Mode;
    double MinTexX;
    double MinTexY;
    double MaxTexX;
    double MaxTexY;
};

struct render_entry_rect_outline {
    render_group_header Header;
    coordinate_system Coordinates;
    game_rect Rect;
    color Color;
};

struct render_entry_text {
    render_group_header Header;
    game_font* Font;
    string String;
    color Color;
    v2 Position;
    int Points;
    bool Wrapped;
};

struct render_entry_button {
    render_group_header Header;
    game_font* Characters;
    //button* Button;
};

struct render_entry_video {
    render_group_header Header;
    game_video* Video;
    game_rect Rect;
};

struct light {
    double Ambient;
    double Diffuse;
    v3 Direction;
    color Color;
};

light Light(v3 Direction, color Color = White, double Ambient = 0.5, double Diffuse = 0.5) {
    return { Ambient, Diffuse, normalize(Direction), Color };
}

struct render_entry_mesh {
    render_group_header Header;
    game_mesh* Mesh;
    game_bitmap* Texture;
    transform Transform;
    game_shader_id ShaderID;
    light Light;
    color Color;
};

struct render_entry_mesh_outline {
    render_group_header Header;
    int StartingLevel;
    color Color;
    int Width;
    int Passes;
};

struct render_entry_shader_pass {
    render_group_header Header;
    game_shader_id ShaderID;
    render_group_target Target;
    color Color;
    float Kernel[9];
    double Width;
    double Time;
};

struct render_entry_render_target {
    render_group_header Header;
    game_shader_id ShaderID;
    render_group_target Target;
};

struct render_entry_debug_grid {
    render_group_header Header;
    double Alpha;
};

uint32 GetSizeOf(render_group_entry_type Type) {
    switch (Type) {
        case group_type_render_entry_clear: {
            return sizeof(render_entry_clear);
        } break;

        case group_type_render_entry_line: {
            return sizeof(render_entry_line);
        } break;

        case group_type_render_entry_triangle: {
            return sizeof(render_entry_triangle);
        } break;

        case group_type_render_entry_rect: {
            return sizeof(render_entry_rect);
        } break;

        case group_type_render_entry_textured_rect: {
            return sizeof(render_entry_textured_rect);
        } break;

        case group_type_render_entry_text: {
            return sizeof(render_entry_text);
        } break;

        case group_type_render_entry_video: {
            return sizeof(render_entry_video);
        } break;

        case group_type_render_entry_mesh: {
            return sizeof(render_entry_mesh);
        } break;

        case group_type_render_entry_render_target: {
            return sizeof(render_entry_render_target);
        } break;

        case group_type_render_entry_shader_pass: {
            return sizeof(render_entry_shader_pass);
        } break;

        case group_type_render_entry_mesh_outline: {
            return sizeof(render_entry_mesh_outline);
        } break;

        case group_type_render_entry_debug_grid: {
            return sizeof(render_entry_debug_grid);
        } break;

        default: {
            Assert(false);
        } break;
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Render group                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_FRAME_BUFFER_COUNT = 8;
struct render_group {
    game_assets* Assets;
    int32 Width;
    int32 Height;
    basis DefaultBasis;
    uint32 MaxPushBufferSize;
    uint32 PushBufferSize;
    uint32 PushBufferElementCount;
    uint8* PushBufferBase;
    uint8* SortedBufferBase;
    camera Camera;
    bool Debug;
    bool PushOutline;
};

render_group* AllocateRenderGroup(game_assets* Assets, memory_arena* Arena, memory_index MaxPushBufferSize) {
    render_group* Result = PushStruct(Arena, render_group);
    Result->PushBufferBase = (uint8*)PushSize(Arena, MaxPushBufferSize);
    Result->MaxPushBufferSize = MaxPushBufferSize;
    Result->PushBufferSize = 0;

    Result->SortedBufferBase = (uint8*)PushSize(Arena, MaxPushBufferSize);

    Result->Assets = Assets;

    Result->DefaultBasis.X = V3(1, 0, 0);
    Result->DefaultBasis.Y = V3(0, 1, 0);
    Result->DefaultBasis.Z = V3(0, 0, 1);
    Result->Camera = { 0 };
    Result->Camera.Distance = 15.0;
    Result->Camera.Velocity = V3(0, 0, 0);
    Result->Camera.Angle = 0;
    Result->Camera.Pitch = 0;

    return(Result);
}

// Render entries sorting
double SORT_ORDER_CLEAR = 0.0;
double SORT_ORDER_MESHES = 100.0;
double SORT_ORDER_OUTLINED_MESHES = 150.0;
double SORT_ORDER_DEBUG_OVERLAY = 200.0;
double SORT_ORDER_SHADER_PASSES = 8000.0;
double SORT_ORDER_PUSH_RENDER_TARGETS = 9000.0;


struct sort_entry {
    sort_key Key;
    uint32 PushBufferOffset;
};

void ClearEntries(render_group* Group) {
    Group->PushBufferElementCount = 0;
    Group->PushBufferSize = 0;
}

void SwapEntries(sort_entry* Entry1, sort_entry* Entry2) {
    sort_key Key1 = Entry1->Key;
    uint32 Offset1 = Entry1->PushBufferOffset;

    *Entry1 = *Entry2;
    *Entry2 = { Key1, Offset1 };
}

void SortEntries(render_group* RenderGroup) {
    uint32 Count = RenderGroup->PushBufferElementCount;

    sort_entry* Entries = (sort_entry*)RenderGroup->SortedBufferBase;

    uint32 BaseAddress = 0;
    for (int i = 0; i < Count; i++) {
        render_group_header* Header = (render_group_header*)(RenderGroup->PushBufferBase + BaseAddress);
        Entries[i] = { Header->Key, Header->PushBufferOffset };

        BaseAddress += GetSizeOf(Header->Type);
    }

    for (int i = 0; i < Count - 1; i++) {
        if (LessThan(Entries[i + 1].Key, Entries[i].Key))
        {
            int j = i;
            do {
                SwapEntries(&Entries[j], &Entries[j + 1]);
                j--;
            } while (j > 0 && LessThan(Entries[j + 1].Key, Entries[j].Key));
        }
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Push methods                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

#define PushRenderElement(Group, type) (type*)PushRenderElement_(Group, sizeof(type), group_type_##type)
render_group_header* PushRenderElement_(render_group* Group, uint32 Size, render_group_entry_type Type) {
    render_group_header* Result = 0;

    if ((Group->PushBufferSize + Size) < Group->MaxPushBufferSize) {
        Result = (render_group_header*)(Group->PushBufferBase + Group->PushBufferSize);
        Result->Key = { 0 }; // Must be set when pushed
        Result->Target = World;
        Result->Type = Type;
        Result->PushBufferOffset = Group->PushBufferSize;

        Group->PushBufferSize += Size;
        Group->PushBufferElementCount++;
    }
    else {
        // Invalid code path
        Assert(false);
    }

    return Result;
}

void PushClear(render_group* Group, color Color, render_group_target Target = Output, double Order = SORT_ORDER_CLEAR) {
    render_entry_clear* Entry = PushRenderElement(Group, render_entry_clear);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;
    Entry->Color = Color;
}

void PushLine(
    render_group* Group, 
    v3 Start, 
    v3 Finish, 
    color Color, 
    int Thickness, 
    coordinate_system Coordinates = Screen_Coordinates,
    double Order = 0.0
) {
    render_entry_line* Entry = PushRenderElement(Group, render_entry_line);
    Entry->Header.Key.Order = Order;
    Entry->Color = Color;
    Entry->Start = Start;
    Entry->Finish = Finish;
    Entry->Thickness = Thickness;
    Entry->Coordinates = Coordinates;
}

void PushTriangle(
    render_group* Group, 
    game_triangle Triangle, 
    color Color, 
    coordinate_system Coordinates = Screen_Coordinates,
    double Order = 0.0
) {
    render_entry_triangle* Entry = PushRenderElement(Group, render_entry_triangle);
    Entry->Header.Key.Order = Order;
    Entry->Color = Color;
    Entry->Triangle = Triangle;
    Entry->Coordinates = Coordinates;
}

void PushCircle(
    render_group* Group, 
    v3 Center, 
    double Radius, 
    color Color, 
    coordinate_system Coordinates = Screen_Coordinates,
    double Order = 0.0
) {
    int N = max(12*log(Radius), 10);

    double dTheta = Tau / N;
    double Theta = 0;
    for (int i = 0; i < N; i++) {
        game_triangle Triangle;
        Triangle.Points[0] = Center;
        Triangle.Points[1] = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        Theta += dTheta;
        Triangle.Points[2] = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        PushTriangle(Group, Triangle, Color, Coordinates, Order);
    }
}

void PushRect(
    render_group* Group, 
    game_rect Rect, 
    color Color,
    render_group_target Target,
    double Order = 0.0
) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;
    Entry->Rect = Rect;
    Entry->Color = Color;
}

void PushTexturedRect(
    render_group* Group,
    game_bitmap* Texture,
    game_rect Rect,
    wrap_mode Mode,
    color Color = White,
    double Order = 0.0,
    scale Size = Scale(),
    v2 Offset = V2(0, 0)
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Mode = Mode;

    switch (Entry->Mode) {
        case Clamp: {
            Entry->MinTexX = Size.X < 0 ? 1.0 : 0.0;
            Entry->MaxTexX = Size.X < 0 ? 0.0 : 1.0;
            Entry->MinTexY = Size.Y < 0 ? 1.0 : 0.0;
            Entry->MaxTexY = Size.Y < 0 ? 0.0 : 1.0;
        } break;

        case Crop: {
            double MinX = Offset.X / Size.X / (double)Texture->Header.Width;
            double MinY = 1.0 - (Rect.Height + Offset.Y) / Size.Y / (double)Texture->Header.Height;
            double MaxX = (Rect.Width + Offset.X) / Size.X / (double)Texture->Header.Width;
            double MaxY = 1.0 - Offset.Y / Size.Y / (double)Texture->Header.Height;
            Entry->MinTexX = Size.X < 0 ? MaxX : MinX;
            Entry->MaxTexX = Size.X < 0 ? MinX : MaxX;
            Entry->MinTexY = Size.Y < 0 ? MaxY : MinY;
            Entry->MaxTexY = Size.Y < 0 ? MinY : MaxY;
        } break;

        case Repeat: {
            double MinX = 0.0;
            double MinY = -Rect.Height / (Size.Y * (double)Texture->Header.Height);
            double MaxX = Rect.Width / (Size.X * (double)Texture->Header.Width);
            double MaxY = 1.0;
            Entry->MinTexX = Size.X < 0 ? MaxX : MinX;
            Entry->MaxTexX = Size.X < 0 ? MinX : MaxX;
            Entry->MinTexY = Size.Y ? MaxY : MinY;
            Entry->MaxTexY = Size.Y ? MinY : MaxY;
        } break;

        default: { Assert(false); }
    }
}

void PushText(
    render_group* Group, 
    v2 Position, 
    game_font* Font, 
    string String, 
    color Color = White, 
    int Points = 20, 
    bool Wrapped = false,
    double Order = 0.0
) {
    render_entry_text* Entry = PushRenderElement(Group, render_entry_text);
    Entry->Header.Target = World;
    Entry->Header.Key.Order = Order;

    Entry->Font = Font;
    Entry->Color = Color;
    Entry->Wrapped = Wrapped;
    Entry->Points = Points;
    Entry->String = String;
    Entry->Position = Position;
}

//void PushButton(render_group* Group, character* Characters, button* Button, double Order = 0.0) {
//    // TODO: Push bitmap and text
//}

void PushRectOutline(
    render_group* Group, 
    game_rect Rect, 
    color Color = White,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    v3 A = V3(Rect.Left, Rect.Top, 0);
    v3 B = V3(Rect.Left + Rect.Width, Rect.Top, 0);
    v3 C = V3(Rect.Left, Rect.Top + Rect.Height, 0);
    v3 D = V3(Rect.Left + Rect.Width, Rect.Top + Rect.Height, 0);

    PushLine(Group, A, B, Color, 2.0, Screen_Coordinates, Order);
    PushLine(Group, A, C, Color, 2.0, Screen_Coordinates, Order);
    PushLine(Group, B, D, Color, 2.0, Screen_Coordinates, Order);
    PushLine(Group, C, D, Color, 2.0, Screen_Coordinates, Order);
}

void PushRectOutline(
    render_group* Group, 
    rect_collider Collider, 
    color Color = White,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_rect Rect = {
        Collider.Center.X - Collider.Width / 2.0,
        Collider.Center.Y - Collider.Height / 2.0,
        Collider.Width,
        Collider.Height
    };
    PushRectOutline(Group, Rect, Color, Order);
}

void PushCubeOutline(
    render_group* Group, 
    v3 Position, 
    scale Size = Scale(),
    color Color = White,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    v3 A = Position + V3(0.0, Size.Y, Size.Z);
    v3 B = Position + V3(Size.X, Size.Y, Size.Z);
    v3 C = Position + V3(0.0, 0.0, Size.Z);
    v3 D = Position + V3(Size.X, 0.0, Size.Z);
    v3 E = Position + V3(Size.X, 0.0, 0.0);
    v3 F = Position + V3(Size.X, Size.Y, 0.0);
    v3 G = Position + V3(0.0, Size.Y, 0.0);
    v3 H = Position + V3(0.0, 0.0, 0.0);
    PushLine(Group, A, B, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, A, C, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, A, G, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, F, G, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, B, F, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, B, D, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, E, F, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, C, H, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, C, D, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, D, E, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, E, H, Color, 1.0, World_Coordinates, Order);
    PushLine(Group, G, H, Color, 1.0, World_Coordinates, Order);
}

void PushCubeOutline(
    render_group* Group,
    cube_collider Collider, 
    color Color,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    PushCubeOutline(Group, Collider.Center - 0.5 * Collider.Size * V3(1.0, 1.0, 1.0), Collider.Size, Color, Order);
}

void PushCircleOutline(
    render_group* Group, 
    v3 Center, 
    double Radius, 
    color Color, 
    double Width = 1.0, 
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    int N = max(12 * log(Radius), 10);

    double dTheta = Tau / N;
    double Theta = 0;
    for (int i = 0; i < N; i++) {
        v3 Start = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        Theta += dTheta;
        v3 Finish = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        PushLine(Group, Start, Finish, Color, Width, Screen_Coordinates, Order);
    }
}

void PushDebugGrid(render_group* Group, double Alpha) {
    render_entry_debug_grid* Entry = PushRenderElement(Group, render_entry_debug_grid);
    Entry->Header.Key.Order = SORT_ORDER_MESHES;
    Entry->Header.Target = World;
    Entry->Alpha = Alpha;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Video                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
void _PushVideo(render_group* Group, game_video* Video, game_rect Rect, double Order = SORT_ORDER_DEBUG_OVERLAY) {
    render_entry_video* Entry = PushRenderElement(Group, render_entry_video);
    Entry->Header.Target = World;

    Entry->Header.Key.Order = Order;
    Entry->Video = Video;
    Entry->Rect = Rect;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | UI                                                                                                                                                               |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
void PushSlider(render_group* Group, slider Slider, double Order = SORT_ORDER_DEBUG_OVERLAY) {
    double CircleCenter = Slider.Position.Y + 60.0 * (1.0 - Slider.Value);
    double Radius = 5.0;
    double UpperLineFinish = 0.0;
    if (Slider.Value < 0.85) {
        UpperLineFinish = (0.85 - Slider.Value) * 60;
    }
    double LowerLineStart = 60.0;
    if (Slider.Value > 0.15) {
        LowerLineStart = (1.15 - Slider.Value) * 60;
    }
    PushLine(Group, Slider.Position, Slider.Position + V3(0.0, UpperLineFinish, 0.0), Slider.Color, 2.0, Screen_Coordinates, Order);
    PushCircleOutline(Group, V3(Slider.Position.X, CircleCenter, 0), Radius, Slider.Color, 2.0, Order);
    PushLine(Group, Slider.Position + V3(0.0, LowerLineStart, 0.0), Slider.Position + V3(0.0, 60.0, 0.0), Slider.Color, 2.0, Screen_Coordinates, Order);
}

void PushUI(render_group* Group, UI* UserInterface) {
    PushSlider(Group, UserInterface->Slider1, SORT_ORDER_DEBUG_OVERLAY);
    PushSlider(Group, UserInterface->Slider2, SORT_ORDER_DEBUG_OVERLAY);
    PushSlider(Group, UserInterface->Slider3, SORT_ORDER_DEBUG_OVERLAY);
    PushSlider(Group, UserInterface->Slider4, SORT_ORDER_DEBUG_OVERLAY);
    PushSlider(Group, UserInterface->Slider5, SORT_ORDER_DEBUG_OVERLAY);
    PushSlider(Group, UserInterface->Slider6, SORT_ORDER_DEBUG_OVERLAY);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Targets & Shaders                                                                                                                                                |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushRenderTarget(
    render_group* Group, 
    render_group_target Target, 
    game_shader_id ShaderID, 
    double Order = SORT_ORDER_PUSH_RENDER_TARGETS
) {
    render_entry_render_target* Entry = PushRenderElement(Group, render_entry_render_target);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;

    if (Target == World) Entry->Target = Output;
    else if (Target == Outline) Entry->Target = Postprocessing_Outline;
    else if (Target == Postprocessing_Outline) Entry->Target = Output;

    Entry->ShaderID = ShaderID;
}

void PushShaderPass(
    render_group* Group,
    game_shader_id ShaderID,
    render_group_target Target,
    color Color = White,
    double Width = 0.0,
    double Time = 0.0,
    double Order = SORT_ORDER_SHADER_PASSES
) {
    render_entry_shader_pass* Entry = PushRenderElement(Group, render_entry_shader_pass);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;

    Entry->ShaderID = ShaderID;
    Entry->Target = Target;
    Entry->Color = Color;
    Entry->Width = Width;
    Entry->Time = Time;
}

void PushKernelShaderPass(
    render_group* Group,
    render_group_target Target,
    matrix3 Kernel,
    double Order = SORT_ORDER_SHADER_PASSES
) {
    render_entry_shader_pass* Entry = PushRenderElement(Group, render_entry_shader_pass);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;

    Entry->ShaderID = Shader_Kernel_ID;
    Entry->Target = Target;
    Entry->Color = White;
    Entry->Width = 0;

    for (int i = 0; i < 9; i++) {
        Entry->Kernel[i] = Kernel[i];
    }
}

void PushMeshOutline(
    render_group* Group,
    float Width,
    color Color,
    int Passes,
    int StartingLevel
) {
    PushRenderTarget(Group, Outline, Shader_Antialiasing_ID, SORT_ORDER_SHADER_PASSES - 10);
    PushShaderPass(Group, Shader_Outline_Init_ID, Postprocessing_Outline, White, SORT_ORDER_SHADER_PASSES);

    render_entry_mesh_outline* Entry = PushRenderElement(Group, render_entry_mesh_outline);
    Entry->Header.Key.Order = SORT_ORDER_SHADER_PASSES + 10.0;
    Entry->Header.Target = Postprocessing_Outline;

    Entry->Passes = Passes;
    Entry->Width = Width;
    Entry->StartingLevel = StartingLevel;

    PushShaderPass(Group, Shader_Outline_ID, Postprocessing_Outline, Color, Width, 0, SORT_ORDER_SHADER_PASSES + 20.0);
    
    PushRenderTarget(Group, Postprocessing_Outline, Shader_Framebuffer_ID, SORT_ORDER_PUSH_RENDER_TARGETS - 10.0);
}

void PushMesh(
    render_group* Group,
    game_asset_id MeshID,
    transform Transform,
    light Light,
    game_shader_id ShaderID,
    game_asset_id TextureID = Bitmap_Empty_ID,
    color Color = White,
    double Order = SORT_ORDER_MESHES,
    bool Outlined = false
) {
    render_entry_mesh* Entry = PushRenderElement(Group, render_entry_mesh);

    game_mesh* pMesh = GetAsset(Group->Assets, MeshID, game_mesh);
    game_bitmap* pTexture = GetAsset(Group->Assets, TextureID, game_bitmap);

    if (Outlined) {
        Order = SORT_ORDER_OUTLINED_MESHES;

        render_entry_mesh* OutlineEntry = PushRenderElement(Group, render_entry_mesh);
        OutlineEntry->Header.Key.Order = Order;
        OutlineEntry->Header.Target = Outline;

        OutlineEntry->Transform = Transform;
        OutlineEntry->Mesh = pMesh;
        OutlineEntry->Light = Light;
        OutlineEntry->ShaderID = Shader_Single_Color_ID;
        OutlineEntry->Color = White;
        OutlineEntry->Texture = GetAsset(Group->Assets, Bitmap_Empty_ID, game_bitmap);

        if (!Group->PushOutline) {
            int Passes = 15;
            PushMeshOutline(Group, 5.0, Color, Passes + 1, (1 << Passes));
            Group->PushOutline = true;
        }
    }

    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;

    Entry->Transform = Transform;
    Entry->Mesh = pMesh;
    Entry->Texture = pTexture;
    Entry->Light = Light;
    Entry->ShaderID = ShaderID;
    Entry->Color = Color;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Debug                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushDebugArena(
    render_group* Group, 
    memory_arena Arena,
    v2 Position,
    double Alpha = 1.0, 
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_font* Font = GetAsset(Group->Assets, Font_Cascadia_Mono_ID, game_font);
    double ArenaPercentage = (double)Arena.Used / (double)Arena.Size;
    game_rect Rect = { Position.X, Position.Y, 120.0, 20.0 };
    PushRect(Group, Rect, Color(DarkGray, Alpha), World, Order + 0.1);
    Rect.Width *= ArenaPercentage;
    PushRect(Group, Rect, Color(Red, Alpha), World, Order + 0.2);
    PushText(Group, Position + V2(0, 15.0), Font, Arena.Name, Color(White, Alpha), 8, false, Order + 0.3);
    sprintf_s(Arena.Percentage.Content, 7, "%.02f%%", ArenaPercentage * 100.0);
    PushText(Group, Position + V2(125.0, 15.0), Font, Arena.Percentage, Color(White, Alpha), 8, false, Order + 0.3);
}

void PushDebugVector(render_group* Group, v3 Vector, v3 Position, coordinate_system Coordinates, color Color = White) {
    double Width = Group->Width;
    double Height = Group->Height;

    double Length = module(Vector);
    double Order = 0.0;
    v3 Orthogonal = V3(0, 0, 0);
    double OrthogonalLength = 0.0;

    switch (Coordinates) {
        case World_Coordinates: {
            Order = SORT_ORDER_MESHES;
            v2 CameraCoordinates = perp(V2(dot(Vector, Group->Camera.Basis.X), dot(Vector, Group->Camera.Basis.Y)));
            Orthogonal = normalize(CameraCoordinates.X * Group->Camera.Basis.X + CameraCoordinates.Y * Group->Camera.Basis.Y);
            OrthogonalLength = (Length / 15.0);
        } break;

        case Screen_Coordinates: {
            Order = SORT_ORDER_DEBUG_OVERLAY;
            v2 OrthogonalPlane = perp(normalize(V2(Vector.X, Vector.Y)));
            Orthogonal = V3(OrthogonalPlane.X, OrthogonalPlane.Y, 0);
            OrthogonalLength = (0.005333 * Height);
        } break;

        default: {
            Assert(false);
        }
    }

    PushLine(Group, Position, Position + 0.875 * Vector, Color, 0.0025 * Height, Coordinates, Order);
    game_triangle Triangle = {
    Position + Vector,
    Position + 0.875 * Vector,
    Position + 0.8 * Vector - OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, Color, Coordinates, Order);
    Triangle = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector + OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, Color, Coordinates, Order);
}

void PushDebugNormals(render_group* Group, game_mesh Mesh, transform Transform) {
    for (int i = 0; i < Mesh.nFaces; i++) {
        vertex Vertex1 = ((vertex*)Mesh.Vertices)[Mesh.Faces[3 * i]];
        vertex Vertex2 = ((vertex*)Mesh.Vertices)[Mesh.Faces[3 * i + 1]];
        vertex Vertex3 = ((vertex*)Mesh.Vertices)[Mesh.Faces[3 * i + 2]];

        v3 TransformedVertex1 = Transform * Vertex1.Vertex;
        v3 TransformedVertex2 = Transform * Vertex2.Vertex;
        v3 TransformedVertex3 = Transform * Vertex3.Vertex;

        v3 Position = (TransformedVertex1 + TransformedVertex2 + TransformedVertex3) * (1 / 3.0);
        v3 Normal = Rotate(Vertex1.Normal, Transform.Rotation);
        PushDebugVector(Group, Normal, Position, World_Coordinates);
    }
}

void PushDebugFustrum(
    render_group* Group,
    v3 Position,
    double Angle, double Pitch,
    double l, double r, double b, double t, double n, double f
) {
    basis B = GetCameraBasis(Angle, Pitch);

    v3 nv = -n * B.Z;
    v3 rv = r * B.X;
    v3 lv = -l * B.X;
    v3 tv = t * B.Y * ((double)Group->Height / (double)Group->Width);
    v3 bv = -b * B.Y * ((double)Group->Height / (double)Group->Width);
    v3 fv = -f * B.Z;

    v3 l_ = f * lv;
    v3 r_ = f * rv;
    v3 t_ = f * tv;
    v3 b_ = f * bv;

    PushLine(Group, Position, Position + l_ + t_ + fv, White, 2.0, World_Coordinates);
    PushLine(Group, Position, Position + r_ + t_ + fv, White, 2.0, World_Coordinates);
    PushLine(Group, Position, Position + l_ + b_ + fv, White, 2.0, World_Coordinates);
    PushLine(Group, Position, Position + r_ + b_ + fv, White, 2.0, World_Coordinates);

    PushLine(Group, Position + r_ + b_ + fv, Position + l_ + b_ + fv, White, 2.0, World_Coordinates);
    PushLine(Group, Position + r_ + t_ + fv, Position + l_ + t_ + fv, White, 2.0, World_Coordinates);
    PushLine(Group, Position + r_ + b_ + fv, Position + r_ + t_ + fv, White, 2.0, World_Coordinates);
    PushLine(Group, Position + l_ + b_ + fv, Position + l_ + t_ + fv, White, 2.0, World_Coordinates);

    PushLine(Group, Position + rv + bv + nv, Position + lv + bv + nv, White, 2.0, World_Coordinates);
    PushLine(Group, Position + rv + tv + nv, Position + lv + tv + nv, White, 2.0, World_Coordinates);
    PushLine(Group, Position + rv + bv + nv, Position + rv + tv + nv, White, 2.0, World_Coordinates);
    PushLine(Group, Position + lv + bv + nv, Position + lv + tv + nv, White, 2.0, World_Coordinates);
}