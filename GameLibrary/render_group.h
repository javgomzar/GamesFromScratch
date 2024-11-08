#pragma once
#include "Assets.h"

const int MAX_ENTRIES = 10000;

enum render_group_entry_type {
    group_type_render_entry_clear,
    group_type_render_entry_line,
    group_type_render_entry_triangle,
    group_type_render_entry_rect,
    group_type_render_entry_textured_rect,
    group_type_render_entry_text,
    group_type_render_entry_button,
    group_type_render_entry_video,
    group_type_render_entry_mesh,
    group_type_render_entry_mesh_outline,
    group_type_render_entry_shader_pass,
    group_type_render_entry_render_target,
};

enum render_group_target {
    Background,
    Screen,
    World,
    Outline,
    Postprocessing_Background,
    Postprocessing_Screen,
    Postprocessing_World,
    Postprocessing_Outline
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
    color Color;
    v3 Start;
    v3 Finish;
    float Thickness;
};

struct render_entry_triangle {
    render_group_header Header;
    game_triangle Triangle;
    color Color;
    basis Basis;
};

struct render_entry_rect {
    render_group_header Header;
    game_rect Rect;
    color Color;
};

struct render_entry_textured_rect {
    render_group_header Header;
    basis Basis;
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
    game_rect Rect;
    color Color;
};

struct render_entry_text {
    render_group_header Header;
    character* Characters;
    v3 Position;
    color Color;
    int Points;
    string String;
    bool Wrapped;
};

struct render_entry_button {
    render_group_header Header;
    character* Characters;
    button* Button;
};

struct render_entry_video {
    render_group_header Header;
    game_video* Video;
    game_rect Rect;
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
    uint32 TargetIndex;
    uint32 SourceIndex;
};

const int MAX_FRAME_BUFFER_COUNT = 16;
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

struct sort_entry {
    sort_key Key;
    uint32 PushBufferOffset;
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


#define PushRenderElement(Group, type) (type*)PushRenderElement_(Group, sizeof(type), group_type_##type)
render_group_header* PushRenderElement_(render_group* Group, uint32 Size, render_group_entry_type Type) {
    render_group_header* Result = 0;

    if ((Group->PushBufferSize + Size) < Group->MaxPushBufferSize) {
        Result = (render_group_header*)(Group->PushBufferBase + Group->PushBufferSize);
        Result->Key = { 0 }; // Must be set when pushed
        Result->Target = Background;
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

        case group_type_render_entry_video:
        {
            return sizeof(render_entry_video);
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

void PushClear(render_group* Group, color Color, render_group_target Target = Background) {
    render_entry_clear* Entry = PushRenderElement(Group, render_entry_clear);
    Entry->Header.Key.Z = -1.0;
    Entry->Header.Target = Target;
    Entry->Color = Color;
}

void PushLine(render_group* Group, v3 Start, v3 Finish, color Color, int Thickness, render_group_target Target = Background) {
    render_entry_line* Entry = PushRenderElement(Group, render_entry_line);
    Entry->Header.Key.Z = max(Start.Z, Finish.Z);
    Entry->Header.Target = Target;
    Entry->Color = Color;
    Entry->Start = Start;
    Entry->Finish = Finish;
    Entry->Thickness = Thickness;
}

void PushTriangle(render_group* Group, game_triangle Triangle, color Color, basis Basis = Identity()) {
    render_entry_triangle* Entry = PushRenderElement(Group, render_entry_triangle);
    Entry->Header.Key.Z = max(Triangle.Points[0].Z, Triangle.Points[1].Z, Triangle.Points[2].Z);
    Entry->Header.Target = Screen;
    Entry->Color = Color;
    Entry->Triangle = Triangle;
    Entry->Basis = Basis;
}

void PushCircle(render_group* Group, v3 Center, double Radius, color Color, basis Basis = Identity()) {
    int N = max(12*log(Radius), 10);

    double dTheta = Tau / N;
    double Theta = 0;
    for (int i = 0; i < N; i++) {
        game_triangle Triangle;
        Triangle.Points[0] = Center;
        Triangle.Points[1] = Center + Radius * (cos(Theta) * Basis.X + sin(Theta) * Basis.Y);
        Theta += dTheta;
        Triangle.Points[2] = Center + Radius * (cos(Theta) * Basis.X + sin(Theta) * Basis.Y);
        PushTriangle(Group, Triangle, Color);
    }
}

void PushRect(render_group* Group, game_rect Rect, color Color, double Z) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Screen;
    Entry->Rect = Rect;
    Entry->Color = Color;
}

void PushTexturedRectClamp(
    render_group* Group,
    loaded_bmp* Texture,
    game_rect Rect,
    double Z = 0.0,
    basis Basis = Identity(1.0),
    color Color = White,
    bool FlipX = false, bool FlipY = false
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Screen;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Basis = normalize(Basis);
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
    basis Basis = Identity(1.0),
    color Color = White,
    bool FlipX = false, bool FlipY = false
) {
    render_entry_textured_rect* Entry = PushRenderElement(Group, render_entry_textured_rect);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Screen;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Basis = normalize(Basis);
    Entry->Mode = Repeat;

    double ScaleX = module(Basis.X);
    double ScaleY = module(Basis.Y);

    double MinX = 0.0;
    double MinY = -Rect.Height / (ScaleY * (double)Texture->Header.Height);
    double MaxX = Rect.Width / (ScaleX * (double)Texture->Header.Width);
    double MaxY = 1.0;
    Entry->MinTexX = FlipX ? MaxX : MinX;
    Entry->MaxTexX = FlipX ? MinX : MaxX;
    Entry->MinTexY = FlipY ? MaxY : MinY;
    Entry->MaxTexY = FlipY ? MinY : MaxY;
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
    Entry->Header.Target = Screen;

    Entry->Rect = Rect;
    Entry->Texture = Texture;
    Entry->Color = Color;
    Entry->Basis = normalize(Basis);
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

void PushRectOutline(render_group* Group, game_rect Rect, color Color) {
    v3 A = V3(Rect.Left, Rect.Top, 0);
    v3 B = V3(Rect.Left + Rect.Width, Rect.Top, 0);
    v3 C = V3(Rect.Left, Rect.Top + Rect.Height, 0);
    v3 D = V3(Rect.Left + Rect.Width, Rect.Top + Rect.Height, 0);

    PushLine(Group, A, B, Color, 2.0, Screen);
    PushLine(Group, A, C, Color, 2.0, Screen);
    PushLine(Group, B, D, Color, 2.0, Screen);
    PushLine(Group, C, D, Color, 2.0, Screen);
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

void PushText(render_group* Group, v3 Position, character* Characters, color Color, int Points, string String, bool Wrapped) {
    render_entry_text* Entry = PushRenderElement(Group, render_entry_text);
    Entry->Header.Key.Z = Position.Z;
    Entry->Header.Target = Screen;
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
    Entry->Header.Target = Screen;
    Entry->Button = Button;
    Entry->Characters = Characters;
}

void _PushVideo(render_group* Group, game_video* Video, game_rect Rect, int Z) {
    render_entry_video* Entry = PushRenderElement(Group, render_entry_video);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Screen;
    Entry->Video = Video;
    Entry->Rect = Rect;
}

void PushMesh(
    render_group* Group, 
    mesh* Mesh, 
    transform Transform, 
    light Light, 
    shader* Shader, 
    color Color = White,
    render_group_target Target = World
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

void PushDebugArena(render_group* Group, character* Characters, memory_arena Arena, v3 Position, double Alpha = 1.0) {
    double ArenaPercentage = (double)Arena.Used / (double)Arena.Size;
    PushRect(Group, { Position.X, Position.Y, 120.0, 20.0 }, Color(DarkGray, Alpha), Position.Z + 0.1);
    PushRect(Group, { Position.X, Position.Y, 120.0 * ArenaPercentage, 20.0 }, Color(Red, Alpha), Position.Z + 0.2);
    PushText(Group, { Position.X, Position.Y + 15.0, Position.Z + 0.3 }, Characters, Color(White, Alpha), 8, Arena.Name, false);
    sprintf_s(Arena.Percentage.Content, 7, "%.02f%%", ArenaPercentage * 100.0);
    PushText(Group, { Position.X + 125.0, Position.Y + 15.0, Position.Z + 0.3}, Characters, Color(White, Alpha), 8, Arena.Percentage, false);
}

void PushDebugVector(render_group* Group, v3 Vector, v3 Position, render_group_target Target = World) {
    PushLine(Group, Position, Position + Vector, White, 1.0, Target);
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
        PushDebugVector(Group, Normal, Position);
    }
}

uint32 GetTargetIndex(render_group_target Target) {
    switch (Target) {
        case Background: { return 0; } break;
        case World: { return 1; } break;
        case Screen: { return 2; } break;
        case Outline: { return 3; } break;
        case Postprocessing_Background: { return 4; } break;
        case Postprocessing_World: { return 5; } break;
        case Postprocessing_Screen: { return 6; } break;
        case Postprocessing_Outline: { return 7; } break;
        default: { Assert(false); } break;
    }
}

void PushRenderTarget(render_group* Group, render_group_target Target, shader* Shader, double Z = 99999) {
    render_entry_render_target* Entry = PushRenderElement(Group, render_entry_render_target);
    Entry->Header.Key.Z = Z;
    Entry->Header.Target = Target;
    
    Entry->SourceIndex = GetTargetIndex(Target);

    Entry->TargetIndex = -1;
    if (Target == Background) {
        Entry->TargetIndex = 4;
    }
    else if (Target == World) {
        Entry->TargetIndex = 5;
    }
    else if (Target == Screen) {
        Entry->TargetIndex = 6;
    }
    else if (Target == Outline) {
        Entry->TargetIndex = 7;
    }

    Entry->Shader = Shader;
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
    PushLine(Group, A, B, Color, 1.0, World);
    PushLine(Group, A, C, Color, 1.0, World);
    PushLine(Group, A, G, Color, 1.0, World);
    PushLine(Group, F, G, Color, 1.0, World);
    PushLine(Group, B, F, Color, 1.0, World);
    PushLine(Group, B, D, Color, 1.0, World);
    PushLine(Group, E, F, Color, 1.0, World);
    PushLine(Group, C, H, Color, 1.0, World);
    PushLine(Group, C, D, Color, 1.0, World);
    PushLine(Group, D, E, Color, 1.0, World);
    PushLine(Group, E, H, Color, 1.0, World);
    PushLine(Group, G, H, Color, 1.0, World);
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
        PushLine(Group, Start, Finish, Color, Width, Screen);
    }
}


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
    PushLine(Group, Slider.Position, Slider.Position + V3(0.0, UpperLineFinish, 0.0), Slider.Color, 2.0, Screen);
    PushCircleOutline(Group, V3(Slider.Position.X, CircleCenter, 0), Radius, Slider.Color, 2.0);
    PushLine(Group, Slider.Position + V3(0.0, LowerLineStart, 0.0), Slider.Position + V3(0.0, 60.0, 0.0), Slider.Color, 2.0, Screen);
}

void PushUI(render_group* Group, UI* UserInterface) {
    PushSlider(Group, UserInterface->Slider1);
    PushSlider(Group, UserInterface->Slider2);
    PushSlider(Group, UserInterface->Slider3);
    PushSlider(Group, UserInterface->Slider4);
    PushSlider(Group, UserInterface->Slider5);
    PushSlider(Group, UserInterface->Slider6);
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

    PushLine(Group, Position, Position + l_ + t_ + fv, White, 2.0, World);
    PushLine(Group, Position, Position + r_ + t_ + fv, White, 2.0, World);
    PushLine(Group, Position, Position + l_ + b_ + fv, White, 2.0, World);
    PushLine(Group, Position, Position + r_ + b_ + fv, White, 2.0, World);

    PushLine(Group, Position + r_ + b_ + fv, Position + l_ + b_ + fv, White, 2.0, World);
    PushLine(Group, Position + r_ + t_ + fv, Position + l_ + t_ + fv, White, 2.0, World);
    PushLine(Group, Position + r_ + b_ + fv, Position + r_ + t_ + fv, White, 2.0, World);
    PushLine(Group, Position + l_ + b_ + fv, Position + l_ + t_ + fv, White, 2.0, World);

    PushLine(Group, Position + rv + bv + nv, Position + lv + bv + nv, White, 2.0, World);
    PushLine(Group, Position + rv + tv + nv, Position + lv + tv + nv, White, 2.0, World);
    PushLine(Group, Position + rv + bv + nv, Position + rv + tv + nv, White, 2.0, World);
    PushLine(Group, Position + lv + bv + nv, Position + lv + tv + nv, White, 2.0, World);
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
    game_assets* Assets,
    float Width,
    color Color,
    int Passes,
    int StartingLevel,
    double Time
) {
    PushRenderTarget(Group, Outline, &Assets->FramebufferShader, 500);
    PushShaderPass(Group, &Assets->OutlineInitShader, Postprocessing_Outline, 510);

    render_entry_mesh_outline* Entry = PushRenderElement(Group, render_entry_mesh_outline);
    Entry->Header.Key.Z = 1000;
    Entry->Header.Target = Postprocessing_Outline;

    Entry->JumpFloodShader = &Assets->JumpFloodShader;

    Entry->Passes = Passes;
    Entry->Width = Width;
    Entry->StartingLevel = StartingLevel;

    PushShaderPass(Group, &Assets->OutlineShader, Postprocessing_Outline, 1010, Color, Width, Time);
}

void PushFace(render_group* Group, game_input* Input, face Face, transform PieceTransform, iv3 PiecePosition, v3 CubePosition) {
    transform Transform = Face.Transform * PieceTransform;
    Transform.Translation += CubePosition;
    color Color = Face.Color;
    if (Face.Selected) {
        Color = Color + White;
    }
    PushMesh(Group, &Group->Assets->FaceMesh, Transform, { 0 }, &Group->Assets->HeightShader, Color, World);
    PushMesh(Group, &Group->Assets->FaceMesh, Transform, { 0 }, &Group->Assets->SingleColorShader, White, Outline);
}

void PushPiece(render_group* Group, game_input* Input, center_piece Piece, v3 Position) {
    PushFace(Group, Input, Piece.Face, Piece.Transform, Piece.Position, Position);
}

void PushPiece(render_group* Group, game_input* Input, edge_piece Piece, v3 Position) {
    PushFace(Group, Input, Piece.Faces[0], Piece.Transform, Piece.Position, Position);
    PushFace(Group, Input, Piece.Faces[1], Piece.Transform, Piece.Position, Position);
}

void PushPiece(render_group* Group, game_input* Input, corner_piece Piece, v3 Position) {
    PushFace(Group, Input, Piece.Faces[0], Piece.Transform, Piece.Position, Position);
    PushFace(Group, Input, Piece.Faces[1], Piece.Transform, Piece.Position, Position);
    PushFace(Group, Input, Piece.Faces[2], Piece.Transform, Piece.Position, Position);
}

void PushCube(render_group* Group, game_input* Input, rubiks_cube* Cube) {
    for (int i = 0; i < 12; i++) {
        if (i < 6) PushPiece(Group, Input, Cube->Centers[i], Cube->Position);
        PushPiece(Group, Input, Cube->Edges[i], Cube->Position);
        if (i < 8) PushPiece(Group, Input, Cube->Corners[i], Cube->Position);
    }
}

// Render entries sorting
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


// +------------------------------------------------------------------------------------------------------------------+
// |  Renders                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------+


void Clear(loaded_bmp* OutputTarget, color Color) {
    uint32 ColorBytes = GetColorBytes(Color);
    uint8* Row = (uint8*)OutputTarget->Content;
    for (int Y = 0; Y < OutputTarget->Header.Height; Y++) {
        uint32* Pixel = (uint32*)Row;
        for (int X = 0; X < OutputTarget->Header.Width; X++) {
            *Pixel++ = ColorBytes;
        }
        Row += OutputTarget->Pitch;
    }
}

//uint32* GetPixel(loaded_bmp* Bitmap, game_screen_position Position) {
//    if (Position.X > Bitmap->Header.Width || Position.Y > Bitmap->Header.Height) {
//        return (uint32*)Bitmap->Content;
//    }
//    return (uint32*)Bitmap->Content + Position.X + Position.Y * Bitmap->Header.Width;
//}

bool IsInside(v3 Position, game_rect Rect) {
    bool A = Position.X >= Rect.Left && Position.X <= Rect.Left + Rect.Width;
    bool B = Position.Y >= Rect.Top && Position.Y <= Rect.Top + Rect.Height;
    return A && B;
}

int CheckLineSide(v2 P0, v2 P1, v2 Position) {
    // 1 means to the right, -1 to the left, 0, is on the line
    if (P0.X == P1.X) {
        if (Position.X == P0.X) {
            return 0;
        }
        else {
            return (Position.X > P0.X) ? 1 : -1;
        }
    }

    // if the line is horizontal, 1 is up, -1 is down
    if (P0.Y == P1.Y) {
        if (Position.Y == P0.Y) {
            return 0;
        }
        else {
            return (Position.Y > P0.Y) ? -1 : 1;
        }
    }

    double X = (Position.X - P0.X) / (P1.X - P0.X);
    double Y = (Position.Y - P0.Y) / (P1.Y - P0.Y);

    if (X == Y) {
        return 0;
    }
    else {
        return (X > Y) ? 1 : -1;
    }
}

void RenderLine(loaded_bmp* OutputTarget, color Color, v3 Start, v3 Finish) {
    // Deciding if we need to render at all
    game_rect Rect = { 0 };
    Rect.Width = OutputTarget->Header.Width;
    Rect.Height = OutputTarget->Header.Height;

    if (!IsInside(Start, Rect) &&
        !IsInside(Finish, Rect) &&
        !IsInside({ Start.X, Finish.Y }, Rect) &&
        !IsInside({ Finish.X, Start.Y }, Rect)) {
        return;
    }

    // int32 Thickness = 1; // TODO: Add thickness

    int32 DX = Sign((double)(Finish.X - Start.X));
    int32 DY = Sign((double)(Finish.Y - Start.Y));

    v2 P0 = V2(Start.X, Start.Y);
    v2 P1 = V2(Finish.X, Finish.Y);

    int32 X0 = Start.X;
    int32 Y0 = Start.Y;

    uint32 ColorBytes = GetColorBytes(Color);
    uint32* Row = OutputTarget->Content + X0 + Y0 * OutputTarget->Header.Width;
    while (true) {
        uint32* Pixel = Row;

        if (X0 > 0 && X0 < OutputTarget->Header.Width &&
            Y0 > 0 && Y0 < OutputTarget->Header.Height) {
            *Pixel = ColorBytes;
        }

        if (X0 == Finish.X && Y0 == Finish.Y) {
            break;
        }

        //int A11 = CheckLineSide(P0, P1, V2(X0, Y0));
        int A12 = CheckLineSide(P0, P1, V2(X0 + DX, Y0));
        int A21 = CheckLineSide(P0, P1, V2(X0, Y0 + DY));
        int A22 = CheckLineSide(P0, P1, V2(X0 + DX, Y0 + DY));

        if (A22 != A12 || Start.Y == Finish.Y) {
            X0 += DX;
            Row += DX;
        }

        if (A22 != A21 || Start.X == Finish.X) {
            Y0 += DY;
            Row += DY * OutputTarget->Header.Width;
        }
    }
}


void RenderRectangle(loaded_bmp* OutputTarget, game_rect Rect, color Color) {
    int32 TargetWidth = (int32)OutputTarget->Header.Width;
    int32 TargetHeight = (int32)OutputTarget->Header.Height;
    int32 BytesPerPixel = OutputTarget->Header.BitsPerPixel >> 3;

    // Crop extra pixels
    int MinX = Rect.Left;
    if (Rect.Left < 0) {
        MinX = 0;
    }
    else if (MinX > TargetWidth) {
        return;
    }

    int MaxX = MinX + Rect.Width;
    if (MaxX > TargetWidth) {
        MaxX = TargetWidth;
    }

    int MinY = Rect.Top;
    if (MinY < 0) {
        MinY = 0;
    }
    else if (MinY > TargetHeight) {
        return;
    }

    int MaxY = MinY + Rect.Height;
    if (MaxY > TargetHeight) {
        MaxY = TargetHeight;
    }

    uint32 ColorBytes = GetColorBytes(Color);
    uint8* Row = (uint8*)OutputTarget->Content + MinX * BytesPerPixel + MinY * OutputTarget->Pitch;
    for (int Y = MinY; Y < MaxY; Y++) {
        uint32* Pixel = (uint32*)Row;
        for (int X = MinX; X < MaxX; X++) {
            *Pixel++ = ColorBytes;
        }
        Row += OutputTarget->Pitch;
    }
}


void RenderBMP(loaded_bmp* OutputTarget, loaded_bmp* BMP, v3 Position) {
    int32 BMPWidth = BMP->Header.Width;
    int32 BMPHeight = BMP->Header.Height;

    int32 TargetWidth = (int32)OutputTarget->Header.Width;
    int32 TargetHeight = (int32)OutputTarget->Header.Height;

    if (Position.X + BMPWidth > 0 && Position.X < TargetWidth &&
        Position.Y + BMPHeight > 0 && Position.Y < TargetHeight) {
        int32 BlitWidth;
        int32 BlitHeight;
        v3 SourcePosition;
        v3 DestinationPosition;

        // Cropping
        if (Position.X < 0) {
            BlitWidth = BMPWidth + Position.X;
            SourcePosition.X = -Position.X;
            DestinationPosition.X = 0;
        }
        else {
            SourcePosition.X = 0;
            DestinationPosition.X = Position.X;
            if (Position.X + BMPWidth > TargetWidth) {
                BlitWidth = TargetWidth - Position.X;
            }
            else {
                BlitWidth = BMPWidth;
            }
        }

        if (Position.Y < 0) {
            BlitHeight = BMPHeight + Position.Y;
            SourcePosition.Y = -Position.Y;
            DestinationPosition.Y = 0;
        }
        else {
            SourcePosition.Y = 0;
            DestinationPosition.Y = Position.Y;
            if (Position.Y + BMPHeight > TargetHeight) {
                BlitHeight = TargetHeight - Position.Y;
            }
            else {
                BlitHeight = BMPHeight;
            }
        }

        // BMP starts on the last row
        uint32* SourceRow = BMP->Content + BMP->Header.Width * (BMP->Header.Height - (int)SourcePosition.Y - 1) + (int)SourcePosition.X;
        uint8* DestinationRow = (uint8*)(OutputTarget->Content + (int)DestinationPosition.X) + (int)DestinationPosition.Y * OutputTarget->Pitch;
        for (int32 Y = 0; Y < BlitHeight; Y++) {
            uint32* Destination = (uint32*)DestinationRow;
            uint32* Source = SourceRow;
            for (int32 X = 0; X < BlitWidth; X++) {
                color BMPColor = GetColor(*Source++, BMP->Header.RedMask, BMP->Header.GreenMask, BMP->Header.BlueMask);
                color BackgroundColor = GetColor(*Destination, 0x00ff0000, 0x0000ff00, 0x000000ff);

                if (BMPColor.R != BackgroundColor.R || BMPColor.G != BackgroundColor.G || BMPColor.B != BackgroundColor.B) {
                    //*Destination++ = GetColorBytes(Mix(BMPColor, BackgroundColor));
                }
                else {
                    Destination++;
                }
            }
            SourceRow -= BMP->Header.Width;
            DestinationRow += OutputTarget->Pitch;
        }
    }
}

//void RenderWhiteNoise(game_offscreen_buffer* Buffer) {
//    uint8* Row = (uint8*)Buffer->Memory;
//    for (int Y = 0; Y < Buffer->Height; ++Y) {
//        uint32* Pixel = (uint32*)Row;
//        for (int X = 0; X < Buffer->Width; ++X) {
//            uint8 Gray = rand() % 255;
//            uint8 Red = Gray;
//            uint8 Green = Gray;
//            uint8 Blue = Gray;
//
//            uint32 RGB_color = (Red << 16) | (Green << 8) | Blue;
//
//            *Pixel++ = RGB_color;
//        }
//        Row += Buffer->Pitch;
//    }
//}

//void RenderWeirdGradient(game_offscreen_buffer* Buffer, game_state* pGameState) {
//    //int XOffset = pGameState->XOffset;
//    //int YOffset = pGameState->YOffset;
//
//    uint8* Row = (uint8*)Buffer->Memory;
//    for (int Y = 0; Y < Buffer->Height; ++Y) {
//        uint32* Pixel = (uint32*)Row;
//        for (int X = 0; X < Buffer->Width; ++X) {
//            uint8 Red = 0xff;
//            //uint8 Green = X + XOffset;
//            uint8 Blue = Y + YOffset;
//
//            uint32 RGB_color = (Red << 16) | (Green << 8) | Blue;
//
//            *Pixel++ = RGB_color;
//        }
//        Row += Buffer->Pitch;
//    }
//}

// Text

//void RenderText(loaded_bmp* OutputTarget, memory_arena* Arena, FT_Face* Font, game_screen_position Position, text Text) {
//    FT_Error error;
//
//    error = FT_Set_Char_Size(*Font, 0, Text.Points * 64, 128, 128);
//    if (error) {
//        Assert(false);
//    }
//    else {
//        FT_GlyphSlot Slot = (*Font)->glyph;
//        int PenX = Position.X;
//        int PenY = Position.Y;
//
//        error = FT_Load_Char(*Font, '\n', FT_LOAD_RENDER);
//        if (error) {
//            Assert(false);
//        }
//
//        int LineJump = (int)(0.023f * (float)Slot->metrics.height); // 0.023 because height is in 64ths of pixel
//
//        for (int i = 0; i < Text.Length; i++) {
//            error = FT_Load_Char(*Font, Text.Content[i], FT_LOAD_RENDER);
//            if (error) {
//                Assert(false);
//            }
//
//            // Carriage returns
//            if (Text.Content[i] == '\n') {
//                PenY += LineJump;
//                PenX = Position.X;
//            }
//            else {
//                if (Text.Wrapped && PenX + (Slot->metrics.width >> 6) > OutputTarget->Header.Width) {
//                    PenX = Position.X;
//                    PenY += LineJump;
//                }
//                FT_Bitmap FTBMP = Slot->bitmap;
//                loaded_bmp BMP = MakeEmptyBitmap(Arena, FTBMP.width, FTBMP.rows, true);
//                LoadFTBMP(&FTBMP, &BMP);
//                RenderBMP(OutputTarget, &BMP, { (double)(PenX + Slot->bitmap_left), (double)(PenY - Slot->bitmap_top), 0 });
//                PopSize(Arena, BMP.Header.FileSize / 8);
//                PenX += Slot->advance.x >> 6;
//            }
//        }
//    }
//}

/*
void RenderGroupToOutput(render_group* Group, loaded_bmp* OutputTarget) {
    v2 ScreenCenter = { 0.5f * (float)OutputTarget->Header.Width, 0.5f * (float)OutputTarget->Header.Height };

    for (uint32 BaseAddress = 0; BaseAddress < Group->PushBufferSize; ) {
        render_group_header* Header = (render_group_header*)(Group->PushBufferBase + BaseAddress);
        switch (Header->Type) {
            case group_type_render_entry_clear: {
                render_entry_clear* Entry = (render_entry_clear*)Header;
                Clear(OutputTarget, Entry->Color);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_line: {
                render_entry_line* Entry = (render_entry_line*)Header;
                RenderLine(OutputTarget, Entry->Color, Entry->Start, Entry->Finish);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_rect: {
                render_entry_rect* Entry = (render_entry_rect*)Header;
                RenderRectangle(OutputTarget, Entry->Rect, Entry->Color);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_bmp: {
                render_entry_bmp* Entry = (render_entry_bmp*)Header;
                RenderBMP(OutputTarget, Entry->Bitmap, Entry->Position);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_text: {
                render_entry_text* Entry = (render_entry_text*)Header;
                RenderText(OutputTarget, Entry->Arena, Entry->Font, Entry->Position, Entry->Text);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_button: {
                render_entry_button* Entry = (render_entry_button*)Header;
                button* Button = Entry->Button;
                RenderBMP(OutputTarget, Button->Clicked ? &Button->ClickedImage : &Button->Image, { Button->Collider.Left, Button->Collider.Top, 0 });
                RenderText(OutputTarget, Entry->Arena, Button->Face, {
                    Button->Collider.Left + Button->Image.Header.Width / 2,
                    Button->Collider.Top + Button->Image.Header.Height / 2,
                    0 }, Button->Text);

                BaseAddress += sizeof(*Entry);
            } break;
            case group_type_render_entry_rect_outline: {
                render_entry_rect_outline* Entry = (render_entry_rect_outline*)Header;

                game_screen_position P11 = { Entry->Rect.Left, Entry->Rect.Top, 0};
                game_screen_position P12 = { Entry->Rect.Left + Entry->Rect.Width, Entry->Rect.Top, 0 };
                game_screen_position P21 = { Entry->Rect.Left, Entry->Rect.Top + Entry->Rect.Height, 0 };
                game_screen_position P22 = { Entry->Rect.Left + Entry->Rect.Width, Entry->Rect.Top + Entry->Rect.Height, 0 };

                RenderLine(OutputTarget, Entry->Color, P11, P12);
                RenderLine(OutputTarget, Entry->Color, P12, P22);
                RenderLine(OutputTarget, Entry->Color, P22, P21);
                RenderLine(OutputTarget, Entry->Color, P21, P11);

                BaseAddress += sizeof(*Entry);
            } break;
            default: {
                // Invalid code path
                Assert(false);
            } break;
        }
    }
}
*/