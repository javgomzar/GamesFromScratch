#pragma once
#include "GameAssets.h"
#include "GameEntity.h"
#include "GameUI.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Render entries                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_RENDER_ENTRIES = 10000;

enum render_group_entry_type {
    group_type_render_entry_clear,
    group_type_render_entry_line,
    group_type_render_entry_circle,
    group_type_render_entry_triangle,
    group_type_render_entry_rect,
    group_type_render_entry_text,
    group_type_render_entry_video,
    group_type_render_entry_mesh,
    group_type_render_entry_heightmap,
    group_type_render_entry_mesh_outline,
    group_type_render_entry_shader_pass,
    group_type_render_entry_compute_shader_pass,
    group_type_render_entry_render_target,
    group_type_render_entry_debug_grid,
    group_type_render_entry_debug_framebuffer,
    group_type_render_entry_debug_plot,
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
    Wrap_Clamp,
    Wrap_Repeat,
    Wrap_Crop
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

struct render_entry_circle {
    render_group_header Header;
    coordinate_system Coordinates;
    v3 Center;
    basis Basis;
    double Angle;
    double Radius;
    color Color;
    float Thickness;
    bool Fill;
};

struct render_entry_triangle {
    render_group_header Header;
    coordinate_system Coordinates;
    triangle Triangle;
    color Color;
};

struct render_entry_rect {
    render_group_header Header;
    rectangle Rect;
    color Color;
    game_bitmap* Texture;
    wrap_mode Mode;
    v2 Offset;
    double MinTexX;
    double MinTexY;
    double MaxTexX;
    double MaxTexY;
    bool Outline;
    bool RefreshTexture;
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

struct render_entry_debug_plot {
    render_group_header Header;
    v3 Position;
    color Color;
    uint32 Size;
    int dx;
    double* Data;
};

struct render_entry_button {
    render_group_header Header;
    game_font* Characters;
    //button* Button;
};

struct render_entry_video {
    render_group_header Header;
    //game_video* Video;
    rectangle Rect;
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
    armature* Armature;
    game_bitmap* Texture;
    transform Transform;
    game_shader_pipeline_id ShaderID;
    color Color;
};

struct render_entry_heightmap {
    render_group_header Header;
    game_heightmap* Heightmap;
    game_shader_pipeline* Shader;
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
    game_shader_pipeline_id ShaderID;
    render_group_target Target;
    color Color;
    float Kernel[9];
    double Width;
};

struct render_entry_compute_shader_pass {
    render_group_header Header;
    game_compute_shader_id ShaderID;
    color Color;
    float Kernel[9];
    double Width;
};

struct render_entry_render_target {
    render_group_header Header;
    render_group_target Target;
};

struct render_entry_debug_grid {
    render_group_header Header;
    double Alpha;
};

struct render_entry_debug_framebuffer {
    render_group_header Header;
    render_group_target Framebuffer;
    bool Attachment;
};

uint32 GetSizeOf(render_group_entry_type Type) {
    switch (Type) {
        case group_type_render_entry_clear: {
            return sizeof(render_entry_clear);
        } break;

        case group_type_render_entry_line: {
            return sizeof(render_entry_line);
        } break;

        case group_type_render_entry_circle: {
            return sizeof(render_entry_circle);
        } break;

        case group_type_render_entry_triangle: {
            return sizeof(render_entry_triangle);
        } break;

        case group_type_render_entry_rect: {
            return sizeof(render_entry_rect);
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

        case group_type_render_entry_heightmap: {
            return sizeof(render_entry_heightmap);
        } break;

        case group_type_render_entry_mesh_outline: {
            return sizeof(render_entry_mesh_outline);
        } break;

        case group_type_render_entry_render_target: {
            return sizeof(render_entry_render_target);
        } break;

        case group_type_render_entry_shader_pass: {
            return sizeof(render_entry_shader_pass);
        } break;

        case group_type_render_entry_compute_shader_pass: {
            return sizeof(render_entry_compute_shader_pass);
        } break;

        case group_type_render_entry_debug_grid: {
            return sizeof(render_entry_debug_grid);
        } break;

        case group_type_render_entry_debug_framebuffer: {
            return sizeof(render_entry_debug_framebuffer);
        } break;

        case group_type_render_entry_debug_plot: {
            return sizeof(render_entry_debug_plot);
        } break;

        default: {
            Assert(false);
        } break;
    }
    return 0;
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
    camera* Camera;
    light Light;
    bool Debug;
    bool DebugNormals;
    bool DebugBones;
    bool DebugColliders;
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

    // Lighting
    Result->Light = Light(V3(-0.5, -1, 1), White);

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
    TIMED_BLOCK;
    uint32 Count = RenderGroup->PushBufferElementCount;

    sort_entry* Entries = (sort_entry*)RenderGroup->SortedBufferBase;

    uint32 Offset = 0;
    for (int i = 0; i < Count; i++) {
        render_group_header* Header = (render_group_header*)(RenderGroup->PushBufferBase + Offset);
        Entries[i] = { Header->Key, Header->PushBufferOffset };

        Offset += GetSizeOf(Header->Type);
    }

    for (int i = 0; i < Count - 1; i++) {
        bool Swap = LessThan(Entries[i + 1].Key, Entries[i].Key);
        if (Swap) {
            int j = i;
            do {
                SwapEntries(&Entries[j], &Entries[j + 1]);
                j--;
            } while (j >= 0 && LessThan(Entries[j + 1].Key, Entries[j].Key));
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
    triangle Triangle,
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
    v2 Center,
    double Radius,
    color Color,
    double Order = 0.0
) {
    render_entry_circle* Entry = PushRenderElement(Group, render_entry_circle);
    Entry->Header.Target = World;
    Entry->Coordinates = Screen_Coordinates;
    Entry->Header.Key.Order = Order;
    Entry->Fill = true;
    Entry->Center = V3(Center.X, Center.Y, 0.0);
    Entry->Basis = Identity3;
    Entry->Radius = Radius;
    Entry->Color = Color;
    Entry->Angle = 360;
}

void PushCircle(
    render_group* Group, 
    v3 Center,
    double Radius, 
    basis Basis,
    color Color,
    double Order = 0.0
) {
    render_entry_circle* Entry = PushRenderElement(Group, render_entry_circle);
    Entry->Header.Target = World;
    Entry->Coordinates = World_Coordinates;
    Entry->Header.Key.Order = Order;
    Entry->Fill = true;
    Entry->Center = Center;
    Entry->Basis = Basis;
    Entry->Radius = Radius;
    Entry->Color = Color;
    Entry->Angle = 360;
}

void PushCircunference(
    render_group* Group,
    v2 Center,
    double Radius,
    color Color,
    float Thickness,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_circle* Entry = PushRenderElement(Group, render_entry_circle);
    Entry->Header.Target = World;
    Entry->Coordinates = Screen_Coordinates;
    Entry->Header.Key.Order = Order;
    Entry->Fill = false;
    Entry->Center = V3(Center.X, Center.Y, 0.0);
    Entry->Basis = Identity3;
    Entry->Radius = Radius;
    Entry->Color = Color;
    Entry->Thickness = Thickness;
    Entry->Angle = 360;
}

void PushCircunference(
    render_group* Group,
    v3 Center,
    v3 Normal,
    double Radius,
    color Color,
    float Thickness,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_circle* Entry = PushRenderElement(Group, render_entry_circle);
    Entry->Header.Target = World;
    Entry->Coordinates = World_Coordinates;
    Entry->Header.Key.Order = Order;
    Entry->Fill = false;
    Entry->Center = Center;
    basis Basis = Complete(Normal);
    Basis.X = Basis.Z;
    Basis.Z = Normal;
    Entry->Basis = Basis;
    Entry->Radius = Radius;
    Entry->Color = Color;
    Entry->Thickness = Thickness;
    Entry->Angle = 360;
}

/*
    Pushes an arc of circunference to the renderer. Basis will determine the plane in which the circunference is contained.
    Basis.Z will be the normal to this plane, and Basis.X will be the offset from the center where the arc will be started.
*/
void PushArc(
    render_group* Group,
    v3 Center,
    basis Basis,
    double Radius,
    double Angle,
    color Color,
    double Thickness,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_circle* Entry = PushRenderElement(Group, render_entry_circle);
    Entry->Header.Target = World;
    Entry->Coordinates = World_Coordinates;
    Entry->Header.Key.Order = Order;
    Entry->Fill = false;
    Entry->Center = Center;
    Entry->Basis = Basis;
    Entry->Radius = Radius;
    Entry->Angle = Angle;
    Entry->Color = Color;
    Entry->Thickness = Thickness;
}

void PushRect(
    render_group* Group,
    rectangle Rect,
    color Color,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;
    Entry->Rect = Rect;
    Entry->Color = Color;
    Entry->Texture = 0;
    Entry->Outline = false;
    Entry->RefreshTexture = false;
}

void PushRectOutline(
    render_group* Group,
    rectangle Rect,
    color Color,
    double Order = SORT_ORDER_DEBUG_OVERLAY,
    render_group_target Target = World
) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;
    Entry->Rect = Rect;
    Entry->Color = Color;
    Entry->Outline = true;
}

void PushBitmap(
    render_group* Group, 
    game_bitmap* Bitmap, 
    rectangle Rect, 
    wrap_mode Mode = Wrap_Clamp,
    bool Refresh = false,
    v2 Size = V2(1.0, 1.0),
    v2 Offset = V2(0,0),
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;
    Entry->Rect = Rect;
    Entry->Color = White;
    Entry->Texture = Bitmap;
    Entry->Mode = Mode;
    Entry->RefreshTexture = Refresh;

    int Width = Entry->Texture->Header.Width;
    int Height = Entry->Texture->Header.Height;

    switch (Entry->Mode) {
        case Wrap_Clamp: {
            Entry->MinTexX = Size.X < 0 ? 1.0 : 0.0;
            Entry->MaxTexX = Size.X < 0 ? 0.0 : 1.0;
            Entry->MinTexY = Size.Y < 0 ? 1.0 : 0.0;
            Entry->MaxTexY = Size.Y < 0 ? 0.0 : 1.0;
        } break;

        case Wrap_Crop: {
            double MinX = Offset.X / Size.X / Width;
            double MinY = 1.0 - (Rect.Height + Offset.Y) / Size.Y / Height;
            double MaxX = (Rect.Width + Offset.X) / Size.X / Width;
            double MaxY = 1.0 - Offset.Y / Size.Y / Height;
            Entry->MinTexX = Size.X < 0 ? MaxX : MinX;
            Entry->MaxTexX = Size.X < 0 ? MinX : MaxX;
            Entry->MinTexY = Size.Y < 0 ? MaxY : MinY;
            Entry->MaxTexY = Size.Y < 0 ? MinY : MaxY;
        } break;

        case Wrap_Repeat: {
            double MinX = 0.0;
            double MinY = -Rect.Height / (Size.Y * Height);
            double MaxX = Rect.Width / (Size.X * Width);
            double MaxY = 1.0;
            Entry->MinTexX = Size.X < 0 ? MaxX : MinX;
            Entry->MaxTexX = Size.X < 0 ? MinX : MaxX;
            Entry->MinTexY = Size.Y ? MaxY : MinY;
            Entry->MaxTexY = Size.Y ? MinY : MaxY;
        } break;

        default: { Assert(false); }
    }
}

void PushBitmap(
    render_group* Group, 
    game_bitmap_id ID, 
    rectangle Rect, 
    wrap_mode Mode = Wrap_Clamp,
    v2 Size = V2(1.0, 1.0),
    v2 Offset = V2(0,0),
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_bitmap* Bitmap = GetAsset(Group->Assets, ID);
    PushBitmap(Group, Bitmap, Rect, Mode, false, Size, Offset, Order);
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
    PushLine(Group, A, B, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, A, C, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, A, G, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, F, G, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, B, F, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, B, D, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, E, F, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, C, H, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, C, D, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, D, E, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, E, H, Color, 2.0, World_Coordinates, Order);
    PushLine(Group, G, H, Color, 2.0, World_Coordinates, Order);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Video                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
//void _PushVideo(render_group* Group, game_video* Video, game_rect Rect, double Order = SORT_ORDER_DEBUG_OVERLAY) {
//    render_entry_video* Entry = PushRenderElement(Group, render_entry_video);
//    Entry->Header.Target = World;
//
//    Entry->Header.Key.Order = Order;
//    Entry->Video = Video;
//    Entry->Rect = Rect;
//}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | UI                                                                                                                                                               |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushSlider(render_group* Group, ui_slider Slider, v3 Position, color Color, double Order = SORT_ORDER_DEBUG_OVERLAY) {
    Assert(Slider.MinValue != Slider.MaxValue);
    double Range = Slider.MaxValue - Slider.MinValue;
    double CircleCenter = Position.Y + 60.0 * (Slider.MaxValue - Slider.Value) / Range;
    double Radius = 6.0;
    double UpperLineFinish = 0.0;

    double Percentage = 0;
    if (Slider.MaxValue == 0.0) Percentage = 1.0 - Slider.Value / Slider.MinValue;
    else Percentage = (Slider.Value - Slider.MinValue) / Range;

    if (Percentage < 0.85) {
        UpperLineFinish = (0.85 - (Slider.Value - Slider.MinValue) / Range) * 60;
    }
    double LowerLineStart = 60.0;
    if (Slider.Value > Slider.MinValue + 0.15 * Range) {
        LowerLineStart = (1.15 - Slider.Value / Range) * 60;
    }
    PushLine(Group, Position, Position + V3(0.0, UpperLineFinish, 0.0), Color, 2.0, Screen_Coordinates, Order);
    PushCircunference(Group, V2(Position.X, CircleCenter), Radius, Color, 2.0, Order);
    PushLine(Group, Position + V3(0.0, LowerLineStart, 0.0), Position + V3(0.0, 60.0, 0.0), Color, 2.0, Screen_Coordinates, Order);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders & render targets                                                                                                                                        |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushRenderTarget(
    render_group* Group,
    render_group_target Target,
    double Order = SORT_ORDER_PUSH_RENDER_TARGETS
) {
    render_entry_render_target* Entry = PushRenderElement(Group, render_entry_render_target);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;

    if (Target == World) Entry->Target = Output;
    else if (Target == Outline) Entry->Target = Postprocessing_Outline;
    else if (Target == Postprocessing_Outline) Entry->Target = Output;
}

void PushShaderPass(
    render_group* Group,
    game_shader_pipeline_id ShaderID,
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
}

void PushShaderPass(
    render_group* Group,
    game_compute_shader_id ShaderID,
    render_group_target Target,
    color Color = White,
    double Width = 2.0,
    int Level = 1,
    double Order = SORT_ORDER_SHADER_PASSES
) {
    render_entry_compute_shader_pass* Entry = PushRenderElement(Group, render_entry_compute_shader_pass);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = Target;
    Entry->Color = Color;
    Entry->Width = Width;
    Entry->ShaderID = ShaderID;
}

//void PushKernelShaderPass(
//    render_group* Group,
//    render_group_target Target,
//    matrix3 Kernel,
//    double Order = SORT_ORDER_SHADER_PASSES
//) {
//    render_entry_shader_pass* Entry = PushRenderElement(Group, render_entry_shader_pass);
//    Entry->Header.Key.Order = Order;
//    Entry->Header.Target = Target;
//
//    Entry->ShaderID = Shader_Kernel_ID;
//    Entry->Target = Target;
//    Entry->Color = White;
//    Entry->Width = 0;
//
//    for (int i = 0; i < 9; i++) {
//        Entry->Kernel[i] = Kernel[i];
//    }
//}

void PushMeshOutline(
    render_group* Group,
    float Width,
    color Color,
    int Passes,
    int StartingLevel
) {
    PushRenderTarget(Group, Outline, SORT_ORDER_SHADER_PASSES - 10);
    PushShaderPass(Group, Outline_Init_Compute_Shader_ID, Postprocessing_Outline);

    render_entry_mesh_outline* Entry = PushRenderElement(Group, render_entry_mesh_outline);
    Entry->Header.Key.Order = SORT_ORDER_SHADER_PASSES + 10.0;
    Entry->Header.Target = Postprocessing_Outline;

    Entry->Passes = Passes;
    Entry->Width = Width;
    Entry->StartingLevel = StartingLevel;

    PushShaderPass(Group, Shader_Pipeline_Outline_ID, Postprocessing_Outline, Color, Width, 0, SORT_ORDER_SHADER_PASSES + 20.0);

    PushRenderTarget(Group, Postprocessing_Outline, SORT_ORDER_SHADER_PASSES + 30.0);
}

void PushMesh(
    render_group* Group,
    game_mesh_id MeshID,
    transform Transform,
    game_shader_pipeline_id ShaderID,
    game_bitmap_id TextureID = Bitmap_Empty_ID,
    color Color = White,
    armature* Armature = 0,
    bool Outlined = false,
    double Order = SORT_ORDER_MESHES
) {
    render_entry_mesh* Entry = PushRenderElement(Group, render_entry_mesh);

    game_mesh* pMesh = GetAsset(Group->Assets, MeshID);
    game_bitmap* pTexture = GetAsset(Group->Assets, TextureID);

    if (Outlined) {
        Order = SORT_ORDER_OUTLINED_MESHES;

        render_entry_mesh* OutlineEntry = PushRenderElement(Group, render_entry_mesh);
        OutlineEntry->Header.Key.Order = Order;
        OutlineEntry->Header.Target = Outline;

        OutlineEntry->Transform = Transform;
        OutlineEntry->Mesh = pMesh;
        OutlineEntry->Armature = Armature;
        OutlineEntry->ShaderID = Shader_Pipeline_Single_Color_ID;
        OutlineEntry->Color = White;
        OutlineEntry->Texture = GetAsset(Group->Assets, Bitmap_Empty_ID);

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
    Entry->Armature = Armature;
    Entry->Texture = pTexture;
    Entry->ShaderID = ShaderID;
    Entry->Color = Color;
}

void PushHeightmap(render_group* Group, game_heightmap_id ID, game_shader_pipeline_id ShaderID, double Order = SORT_ORDER_MESHES) {
    render_entry_heightmap* Entry = PushRenderElement(Group, render_entry_heightmap);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;
    Entry->Shader = GetShaderPipeline(Group->Assets, ShaderID);

    Entry->Heightmap = GetAsset(Group->Assets, ID);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushCollider(render_group* Group, game_entity Entity, color Color) {
    collider Collider = Entity.Collider;
    Collider.Offset += Entity.Transform.Translation;
    switch (Collider.Type) {
        case Rect_Collider: {
            PushRectOutline(Group, Rectangle(Collider), Color);
        } break;

        case Cube_Collider: {
            PushCubeOutline(Group, Collider.Offset, Scale(Collider.Cube.HalfWidth,Collider.Cube.HalfHeight,Collider.Cube.HalfDepth), Color);
        } break;

        case Sphere_Collider: {
            PushCircunference(Group, Collider.Offset, V3(1,0,0), Collider.Sphere.Radius, Color, 2.0f);
            PushCircunference(Group, Collider.Offset, V3(0,1,0), Collider.Sphere.Radius, Color, 2.0f);
            PushCircunference(Group, Collider.Offset, V3(0,0,1), Collider.Sphere.Radius, Color, 2.0f);
        } break;

        case Capsule_Collider: {
            v3 Head = Entity.Transform * Collider.Capsule.Segment.Head;
            v3 Tail = Entity.Transform * Collider.Capsule.Segment.Tail;

            segment TransformedSegment = { Head, Tail };
            transform T = SegmentTransform(TransformedSegment);
            basis Basis = T * Identity3;
            v3 D = normalize(Tail - Head);

            // Top part
            PushArc(Group, Tail, Basis, Collider.Capsule.Distance, 180, Color, 2.0f);
            v3 Temp = Basis.X;
            Basis.X = Basis.Z;
            Basis.Z = Temp;
            PushArc(Group, Tail, Basis, Collider.Capsule.Distance, 180, Color, 2.0f);
            PushCircunference(Group, Tail, D, Collider.Capsule.Distance, Color, 2.0f);

            // Vertical lines
            v3 Offset[4] = { Basis.X, -Basis.X, Basis.Z, -Basis.Z };
            for (int i = 0; i < 4; i++) {
                PushLine(
                    Group, 
                    Tail + Collider.Capsule.Distance * Offset[i], 
                    Head + Collider.Capsule.Distance * Offset[i], 
                    Color, 2.0f, World_Coordinates
                );
            }

            // Bottom part
            PushCircunference(Group, Head, D, Collider.Capsule.Distance, Color, 2.0f);
            Basis = T * Identity3;
            Basis.X = -Basis.X;
            Basis.Y = -Basis.Y;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, Color, 2.0f);
            Temp = Basis.X;
            Basis.X = Basis.Z;
            Basis.Z = Temp;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, Color, 2.0f);

            Collider.Capsule.Segment = Entity.Transform * Collider.Capsule.Segment;
        } break;

        default: Assert(false);
    }
}

void PushEntities(render_group* Group, game_entity_list* List) {
    TIMED_BLOCK;
    for (int i = 0; i < List->nEntities; i++) {
        game_entity Entity = List->Entities[i];
        switch(Entity.Type) {
            case Character: {
                character* pCharacter = &List->Characters.List[Entity.Index];
                PushMesh(
                    Group, 
                    Mesh_Body_ID, 
                    Entity.Transform,
                    Shader_Pipeline_Mesh_ID, 
                    Bitmap_Empty_ID,
                    White,
                    &pCharacter->Armature
                );
            } break;
    
            case Enemy: {
                PushMesh(
                    Group,
                    Mesh_Enemy_ID,
                    Entity.Transform,
                    Shader_Pipeline_Mesh_ID,
                    Bitmap_Enemy_ID
                );
            } break;

            case Prop: {
                prop* pProp = &List->Props.List[Entity.Index];
                PushMesh(
                    Group,
                    pProp->MeshID,
                    Entity.Transform,
                    pProp->Shader,
                    Bitmap_Empty_ID,
                    pProp->Color
                );
            } break;

            case Weapon: {
                weapon* pWeapon = &List->Weapons.List[Entity.Index];
                game_mesh_id MeshID;
                switch(pWeapon->Type) {
                    case Sword: MeshID = Mesh_Sword_ID; break;
                    case Shield: MeshID = Mesh_Shield_ID; break;
                    default: Assert(false);
                }

                PushMesh(Group, MeshID, Entity.Transform, Shader_Pipeline_Mesh_ID);
            } break;
        }

        if (Group->Debug && Group->DebugColliders && Entity.Type != Camera) {
            PushCollider(Group, Entity, Entity.Collided ? Red : Yellow);
        }
    }
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
    game_font* Font = GetAsset(Group->Assets, Font_Cascadia_Mono_ID);
    float ArenaPercentage = (float)Arena.Used / (float)Arena.Size;
    float RectWidth = 250.0f;
    rectangle Rect = { Position.X, Position.Y, RectWidth, 20.0f };
    PushRect(Group, Rect, Color(DarkGray, Alpha), Order + 0.1);
    Rect.Width *= ArenaPercentage;
    PushRect(Group, Rect, Color(Red, Alpha), Order + 0.2);
    PushText(Group, Position + V2(0, 15.0), Font, Arena.Name, Color(White, Alpha), 8, false, Order + 0.3);
    sprintf_s(Arena.Percentage.Content, 7, "%.02f%%", ArenaPercentage * 100.0);
    PushText(Group, Position + V2(RectWidth - 55.0, 15.0), Font, Arena.Percentage, Color(White, Alpha), 8, false, Order + 0.3);
}

void PushDebugVector(render_group* Group, v3 Vector, v3 Position, coordinate_system Coordinates, color Color = White) {
    double Width = Group->Width;
    double Height = Group->Height;

    double Length = modulus(Vector);
    double Order = 0.0;
    v3 Orthogonal = V3(0, 0, 0);
    double OrthogonalLength = 0.0;

    switch (Coordinates) {
        case World_Coordinates: {
            Order = SORT_ORDER_MESHES;
            v2 CameraCoordinates = perp(V2(dot(Vector, Group->Camera->Basis.X), dot(Vector, Group->Camera->Basis.Y)));
            Orthogonal = normalize(CameraCoordinates.X * Group->Camera->Basis.X + CameraCoordinates.Y * Group->Camera->Basis.Y);
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

    int Thickness = max(1.0, 0.0025 * Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, Coordinates, Order);
    triangle Triangle = {
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

void PushDebugGrid(render_group* Group, double Alpha) {
    render_entry_debug_grid* Entry = PushRenderElement(Group, render_entry_debug_grid);
    Entry->Header.Key.Order = SORT_ORDER_MESHES;
    Entry->Header.Target = World;
    Entry->Alpha = Alpha;
}

void PushDebugFramebuffer(render_group* Group, render_group_target Framebuffer, bool Attachment = false) {
    render_entry_debug_framebuffer* Entry = PushRenderElement(Group, render_entry_debug_framebuffer);
    Entry->Header.Key.Order = SORT_ORDER_PUSH_RENDER_TARGETS - 0.1;
    Entry->Header.Target = Output;
    Entry->Framebuffer = Framebuffer;
    Entry->Attachment = Attachment;
}

void PushDebugPlot(
    render_group* Group, 
    int N, 
    double* Data, 
    v3 Position, 
    int dx, 
    color Color = White, 
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_debug_plot* Entry = PushRenderElement(Group, render_entry_debug_plot);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;
    Entry->Size = N;
    Entry->dx = dx;
    Entry->Color = Color;
    Entry->Data = Data;
    Entry->Position = Position;
}
