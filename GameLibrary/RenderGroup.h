#pragma once
#include "GameAssets.h"

const int MAX_RENDER_ENTRIES = 10000;

enum render_group_entry_type {
    group_type_render_entry_clear,
    group_type_render_entry_line,
    group_type_render_entry_triangle,
    group_type_render_entry_rect,
    group_type_render_entry_textured_rect,
    group_type_render_entry_text,
    group_type_render_entry_button,
    group_type_render_entry_mesh,
    group_type_render_entry_mesh_outline,
    group_type_render_entry_shader_pass,
    group_type_render_entry_render_target,
};

enum render_group_target {
    World,
    Output,
    Outline,
    Postprocessing_Outline,
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
    double Z;
};

bool LessThan(sort_key Key1, sort_key Key2) {
    return Key1.Z < Key2.Z;
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
    coordinate_system Coordinates;
    game_rect Rect;
    color Color;
};

struct render_entry_textured_rect {
    render_group_header Header;
    coordinate_system Coordinates;
    game_rect Rect;
    loaded_bmp* Texture;
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
    coordinate_system Coordinates;
    character* Characters;
    v3 Position;
    color Color;
    int Points;
    string String;
    bool Wrapped;
};

struct render_entry_button {
    render_group_header Header;
    coordinate_system Coordinates;
    character* Characters;
    button* Button;
};

struct light {
    double Ambient;
    v3 Direction;
    color Color;
};

struct render_entry_mesh {
    render_group_header Header;
    mesh* Mesh;
    transform Transform;
    shader* Shader;
    light Light;
    color Color;
};

struct render_entry_mesh_outline {
    render_group_header Header;
    shader* JumpFloodShader;
    int StartingLevel;
    color Color;
    int Width;
    int Passes;
};

struct render_entry_shader_pass {
    render_group_header Header;
    shader* Shader;
    uint32 TargetIndex;
    color Color;
    float Kernel[9];
    double Width;
    double Time;
};

struct render_entry_render_target {
    render_group_header Header;
    shader* Shader;
    int TargetIndex;
    int SourceIndex;
};

uint32 GetSizeOf(render_group_entry_type Type) {
    switch (Type) {
        case group_type_render_entry_clear:
        {
            return sizeof(render_entry_clear);
        } break;

        case group_type_render_entry_line:
        {
            return sizeof(render_entry_line);
        } break;

        case group_type_render_entry_triangle:
        {
            return sizeof(render_entry_triangle);
        } break;

        case group_type_render_entry_rect:
        {
            return sizeof(render_entry_rect);
        } break;

        case group_type_render_entry_text:
        {
            return sizeof(render_entry_text);
        } break;

        case group_type_render_entry_button:
        {
            return sizeof(render_entry_button);
        } break;

        case group_type_render_entry_textured_rect:
        {
            return sizeof(render_entry_textured_rect);
        } break;

        case group_type_render_entry_mesh:
        {
            return sizeof(render_entry_mesh);
        } break;

        case group_type_render_entry_render_target:
        {
            return sizeof(render_entry_render_target);
        } break;

        case group_type_render_entry_shader_pass:
        {
            return sizeof(render_entry_shader_pass);
        } break;

        case group_type_render_entry_mesh_outline:
        {
            return sizeof(render_entry_mesh_outline);
        } break;

        default:
        {
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
        Result->Target = Output;
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

void PushClear(render_group* Group, color Color, render_group_target Target = Output) {
    render_entry_clear* Entry = PushRenderElement(Group, render_entry_clear);
    Entry->Header.Key.Z = -1.0;
    Entry->Header.Target = Target;
    Entry->Color = Color;
}

void PushLine(render_group* Group, v3 Start, v3 Finish, color Color, int Thickness, coordinate_system Coordinates = Screen_Coordinates) {
    render_entry_line* Entry = PushRenderElement(Group, render_entry_line);
    Entry->Header.Key.Z = max(Start.Z, Finish.Z);
    Entry->Color = Color;
    Entry->Start = Start;
    Entry->Finish = Finish;
    Entry->Thickness = Thickness;
    Entry->Coordinates = Coordinates;
}

void PushTriangle(render_group* Group, game_triangle Triangle, color Color, coordinate_system Coordinates = Screen_Coordinates) {
    render_entry_triangle* Entry = PushRenderElement(Group, render_entry_triangle);
    Entry->Header.Key.Z = max(Triangle.Points[0].Z, Triangle.Points[1].Z, Triangle.Points[2].Z);
    Entry->Color = Color;
    Entry->Triangle = Triangle;
    Entry->Coordinates = Coordinates;
}

void PushCircle(render_group* Group, v3 Center, double Radius, color Color, coordinate_system Coordinates = Screen_Coordinates) {
    int N = max(12*log(Radius), 10);

    double dTheta = Tau / N;
    double Theta = 0;
    for (int i = 0; i < N; i++) {
        game_triangle Triangle;
        Triangle.Points[0] = Center;
        Triangle.Points[1] = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        Theta += dTheta;
        Triangle.Points[2] = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        PushTriangle(Group, Triangle, Color, Coordinates);
    }
}

void PushRect(render_group* Group, game_rect Rect, color Color, double Z, coordinate_system Coordinates = Screen_Coordinates) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Z = Z;
    Entry->Rect = Rect;
    Entry->Color = Color;
    Entry->Coordinates = Coordinates;
}

void PushTexturedRectClamp(
    render_group* Group,
    loaded_bmp* Texture,
    game_rect Rect,
    double Z = 0.0,
    color Color = White,
    bool FlipX = false,
    bool FlipY = false
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Mode = Clamp;

    Entry->MinTexX = FlipX ? 1.0 : 0.0;
    Entry->MaxTexX = FlipX ? 0.0 : 1.0;
    Entry->MinTexY = FlipY ? 1.0 : 0.0;
    Entry->MaxTexY = FlipY ? 0.0 : 1.0;
}

void PushTexturedRectRepeat(
    render_group* Group,
    loaded_bmp* Texture,
    game_rect Rect,
    double Z = 0.0,
    color Color = White,
    scale Scale_ = Scale()
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Mode = Repeat;

    double MinX = 0.0;
    double MinY = -Rect.Height / (Scale_.Y * (double)Texture->Header.Height);
    double MaxX = Rect.Width / (Scale_.X * (double)Texture->Header.Width);
    double MaxY = 1.0;
    Entry->MinTexX = Scale_.X < 0? MaxX : MinX;
    Entry->MaxTexX = Scale_.X < 0? MinX : MaxX;
    Entry->MinTexY = Scale_.Y ? MaxY : MinY;
    Entry->MaxTexY = Scale_.Y ? MinY : MaxY;
}

void PushTexturedRectCrop(
    render_group* Group,
    loaded_bmp* Texture,
    game_rect Rect,
    v3 Offset = V3(0,0,0),
    double Z = 0.0,
    basis Basis = Identity(1.0),
    color Color = White,
    bool FlipX = false, bool FlipY = false
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Mode = Crop;

    double ScaleX = module(Basis.X);
    double ScaleY = module(Basis.Y);

    double MinX = Offset.X / ScaleX / (double)Texture->Header.Width;
    double MinY = 1.0 - (Rect.Height + Offset.Y) / ScaleY / (double)Texture->Header.Height;
    double MaxX = (Rect.Width + Offset.X) / ScaleX / (double)Texture->Header.Width;
    double MaxY = 1.0 - Offset.Y / ScaleY / (double)Texture->Header.Height;
    Entry->MinTexX = FlipX ? MaxX : MinX;
    Entry->MaxTexX = FlipX ? MinX : MaxX;
    Entry->MinTexY = FlipY ? MaxY : MinY;
    Entry->MaxTexY = FlipY ? MinY : MaxY;
}

void PushText(render_group* Group, v3 Position, character* Characters, color Color, int Points, string String, bool Wrapped) {
    render_entry_text* Entry = PushRenderElement(Group, render_entry_text);
    Entry->Header.Key.Z = Position.Z;
    Entry->Position = Position;
    Entry->Characters = Characters;
    Entry->Color = Color;
    Entry->Points = Points;
    Entry->String = String;
    Entry->Wrapped = Wrapped;
}

void PushButton(render_group* Group, character* Characters, button* Button) {
    render_entry_button* Entry = PushRenderElement(Group, render_entry_button);
    Entry->Header.Key.Z = 0;
    Entry->Button = Button;
    Entry->Characters = Characters;
}

void PushMesh(
    render_group* Group,
    mesh* Mesh,
    transform Transform,
    light Light,
    shader* Shader,
    color Color = White,
    render_group_target Target = Output
) {
    render_entry_mesh* Entry = PushRenderElement(Group, render_entry_mesh);
    Entry->Header.Key.Z = Transform.Translation.Z;
    Entry->Header.Target = Target;

    Entry->Transform = Transform;
    Entry->Mesh = Mesh;
    Entry->Light = Light;
    Entry->Shader = Shader;
    Entry->Color = Color;
}

void PushRectOutline(render_group* Group, game_rect Rect, color Color) {
    v3 A = V3(Rect.Left, Rect.Top, 0);
    v3 B = V3(Rect.Left + Rect.Width, Rect.Top, 0);
    v3 C = V3(Rect.Left, Rect.Top + Rect.Height, 0);
    v3 D = V3(Rect.Left + Rect.Width, Rect.Top + Rect.Height, 0);

    PushLine(Group, A, B, Color, 2.0, Screen_Coordinates);
    PushLine(Group, A, C, Color, 2.0, Screen_Coordinates);
    PushLine(Group, B, D, Color, 2.0, Screen_Coordinates);
    PushLine(Group, C, D, Color, 2.0, Screen_Coordinates);
}

void PushRectOutline(render_group* Group, rect_collider Collider, color Color) {
    game_rect Rect = {
        Collider.Center.X - Collider.Width / 2.0,
        Collider.Center.Y - Collider.Height / 2.0,
        Collider.Width,
        Collider.Height
    };
    PushRectOutline(Group, Rect, Color);
}

void PushCubeOutline(render_group* Group, v3 Position, v3 Size, color Color) {
    v3 A = Position + V3(0.0, Size.Y, Size.Z);
    v3 B = Position + V3(Size.X, Size.Y, Size.Z);
    v3 C = Position + V3(0.0, 0.0, Size.Z);
    v3 D = Position + V3(Size.X, 0.0, Size.Z);
    v3 E = Position + V3(Size.X, 0.0, 0.0);
    v3 F = Position + V3(Size.X, Size.Y, 0.0);
    v3 G = Position + V3(0.0, Size.Y, 0.0);
    v3 H = Position + V3(0.0, 0.0, 0.0);
    PushLine(Group, A, B, Color, 1.0, World_Coordinates);
    PushLine(Group, A, C, Color, 1.0, World_Coordinates);
    PushLine(Group, A, G, Color, 1.0, World_Coordinates);
    PushLine(Group, F, G, Color, 1.0, World_Coordinates);
    PushLine(Group, B, F, Color, 1.0, World_Coordinates);
    PushLine(Group, B, D, Color, 1.0, World_Coordinates);
    PushLine(Group, E, F, Color, 1.0, World_Coordinates);
    PushLine(Group, C, H, Color, 1.0, World_Coordinates);
    PushLine(Group, C, D, Color, 1.0, World_Coordinates);
    PushLine(Group, D, E, Color, 1.0, World_Coordinates);
    PushLine(Group, E, H, Color, 1.0, World_Coordinates);
    PushLine(Group, G, H, Color, 1.0, World_Coordinates);
}

void PushCubeOutline(render_group* Group, cube_collider Collider, color Color) {
    PushCubeOutline(Group, Collider.Center - 0.5 * Collider.Size, Collider.Size, Color);
}

void PushCircleOutline(render_group* Group, v3 Center, double Radius, color Color, double Width = 1.0) {
    int N = max(12 * log(Radius), 10);

    double dTheta = Tau / N;
    double Theta = 0;
    for (int i = 0; i < N; i++) {
        v3 Start = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        Theta += dTheta;
        v3 Finish = Center + Radius * V3(cos(Theta), sin(Theta), 0);
        PushLine(Group, Start, Finish, Color, Width, Screen_Coordinates);
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | UI                                                                                                                                                               |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
void PushSlider(render_group* Group, slider Slider) {
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
    PushLine(Group, Slider.Position, Slider.Position + V3(0.0, UpperLineFinish, 0.0), Slider.Color, 2.0, Screen_Coordinates);
    PushCircleOutline(Group, V3(Slider.Position.X, CircleCenter, 0), Radius, Slider.Color, 2.0);
    PushLine(Group, Slider.Position + V3(0.0, LowerLineStart, 0.0), Slider.Position + V3(0.0, 60.0, 0.0), Slider.Color, 2.0, Screen_Coordinates);
}

void PushUI(render_group* Group, UI* UserInterface) {
    PushSlider(Group, UserInterface->Slider1);
    PushSlider(Group, UserInterface->Slider2);
    PushSlider(Group, UserInterface->Slider3);
    PushSlider(Group, UserInterface->Slider4);
    PushSlider(Group, UserInterface->Slider5);
    PushSlider(Group, UserInterface->Slider6);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Targets & Shaders                                                                                                                                                |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

uint32 GetTargetIndex(render_group_target Target) {
    switch (Target) {
        case World: { return 0; } break;
        case Outline: { return 1; } break;
        case Postprocessing_Outline: { return 2; } break;
        case Output: { return 3; } break;
        default: { Assert(false); } break;
    }
}

void PushRenderTarget(render_group* Group, render_group_target Target, shader* Shader, double Z = 99999) {
    render_entry_render_target* Entry = PushRenderElement(Group, render_entry_render_target);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Target;

    Entry->SourceIndex = GetTargetIndex(Target);
    Entry->TargetIndex = -1;
    if (Target == World) Entry->TargetIndex = GetTargetIndex(Output);
    else if (Target == Outline) Entry->TargetIndex = GetTargetIndex(Postprocessing_Outline);
    else if (Target == Postprocessing_Outline) Entry->TargetIndex = GetTargetIndex(Output);

    Entry->Shader = Shader;
}

void PushShaderPass(
    render_group* Group,
    shader* Shader,
    render_group_target Target,
    double Z = 1000,
    color Color = White,
    double Width = 0.0,
    double Time = 0.0
) {
    render_entry_shader_pass* Entry = PushRenderElement(Group, render_entry_shader_pass);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Target;

    Entry->Shader = Shader;
    Entry->TargetIndex = GetTargetIndex(Target);
    Entry->Color = Color;
    Entry->Width = Width;
    Entry->Time = Time;
}

void PushKernelShaderPass(
    render_group* Group,
    shader* Shader,
    render_group_target Target,
    matrix3 Kernel,
    double Z = 1000
) {
    render_entry_shader_pass* Entry = PushRenderElement(Group, render_entry_shader_pass);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Target;

    Entry->Shader = Shader;
    Entry->TargetIndex = GetTargetIndex(Target);
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
    int StartingLevel,
    double Time
) {
    PushRenderTarget(Group, Outline, &Group->Assets->FramebufferShader, 500);
    PushShaderPass(Group, &Group->Assets->OutlineInitShader, Postprocessing_Outline, 550);

    render_entry_mesh_outline* Entry = PushRenderElement(Group, render_entry_mesh_outline);
    Entry->Header.Key.Z = 1000;
    Entry->Header.Target = Postprocessing_Outline;

    Entry->JumpFloodShader = &Group->Assets->JumpFloodShader;

    Entry->Passes = Passes;
    Entry->Width = Width;
    Entry->StartingLevel = StartingLevel;

    PushShaderPass(Group, &Group->Assets->OutlineShader, Postprocessing_Outline, 575, Color, Width, Time);

    PushRenderTarget(Group, Postprocessing_Outline, &Group->Assets->FramebufferShader, 600);
}
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Debug                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushDebugArena(render_group* Group, character* Characters, memory_arena Arena, v3 Position, double Alpha = 1.0) {
    double ArenaPercentage = (double)Arena.Used / (double)Arena.Size;
    PushRect(Group, { Position.X, Position.Y, 120.0, 20.0 }, Color(DarkGray, Alpha), Position.Z + 0.1);
    PushRect(Group, { Position.X, Position.Y, 120.0 * ArenaPercentage, 20.0 }, Color(Red, Alpha), Position.Z + 0.2);
    PushText(Group, { Position.X, Position.Y + 15.0, Position.Z + 0.3 }, Characters, Color(White, Alpha), 8, Arena.Name, false);
    sprintf_s(Arena.Percentage.Content, 7, "%.02f%%", ArenaPercentage * 100.0);
    PushText(Group, { Position.X + 125.0, Position.Y + 15.0, Position.Z + 0.3 }, Characters, Color(White, Alpha), 8, Arena.Percentage, false);
}

void PushDebugVector(render_group* Group, v3 Vector, v3 Position, coordinate_system Coordinates, color Color = White) {
    double Width = Group->Width;
    double Height = Group->Height;

    // Debug axis
    PushLine(Group, Position, Position + Vector, Color, 0.0025 * Height, Coordinates);

    double Length = module(Vector);
    v2 CameraCoordinates = perp(V2(dot(Vector, Group->Camera.Basis.X), dot(Vector, Group->Camera.Basis.Y)));
    v3 Orthogonal = CameraCoordinates.X * Group->Camera.Basis.X + CameraCoordinates.Y * Group->Camera.Basis.Y;
    game_triangle Triangle = {
        Position + Vector,
        Position + 0.875 * Length * Vector,
        Position + 0.8 * Length * Vector - (Length / 15.0) * Orthogonal,
    };
    PushTriangle(Group, Triangle, Color, Coordinates);
    Triangle = {
        Position + Vector,
        Position + 0.875 * Length * Vector,
        Position + 0.8 * Length * Vector + (Length / 15.0) * Orthogonal, 
    };
    PushTriangle(Group, Triangle, Color, Coordinates);
    //PushLine(Group, Position, Position + Vector, White, 1.0, Coordinates);
}

void PushDebugNormals(render_group* Group, mesh Mesh, transform Transform) {
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