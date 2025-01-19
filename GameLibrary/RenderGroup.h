#pragma once
#include "GameAssets.h"


// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Color                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct color {
    double Alpha;
    double R;
    double G;
    double B;
};

color Color(double R, double G, double B, double Alpha = 1.0) {
    return { Alpha, R, G, B };
}

color Color(color Color, double Alpha) {
    return { Alpha, Color.R, Color.G, Color.B };
}

color operator*(double Luminosity, color Color) {
    return {
        Color.Alpha,
        min(Luminosity * Color.R, 1.0),
        min(Luminosity * Color.G, 1.0),
        min(Luminosity * Color.B, 1.0),
    };
}

static int Attenuation = 100;
static color Black = { 1.0, 0.0, 0.0, 0.0 };
static color White = { 1.0, 1.0, 1.0, 1.0 };
static color Gray = { 1.0, 0.5, 0.5, 0.5 };
static color DarkGray = { 1.0, 0.1, 0.1, 0.1 };
static color Red = { 1.0, 1.0, 0.0, 0.0 };
static color Green = { 1.0, 0.0, 1.0, 0.0 };
static color Blue = { 1.0, 0.0, 0.0, 1.0 };
static color Magenta = { 1.0, 1.0, 0.0, 1.0 };
static color Yellow = { 1.0, 1.0, 1.0, 0.0 };
static color Cyan = { 1.0, 0.0, 1.0, 1.0 };
static color Orange = { 1.0, 1.0, 0.63, 0.0 };
static color BackgroundBlue = { 1.0, 0.4, 0.4, 0.8 };

uint32 GetColorBytes(color Color) {
    uint8 Alpha = Color.Alpha * 255.0;
    uint8 R = Color.R * 255.0;
    uint8 G = Color.G * 255.0;
    uint8 B = Color.B * 255.0;
    return (Alpha << 24) | (R << 16) | (G << 8) | B;
}

//color GetColor(uint32 Bytes, uint32 RedMask, uint32 GreenMask, uint32 BlueMask) {
//    uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
//    
//    uint32 RedShift;
//    uint32 GreenShift;
//    uint32 BlueShift;
//    uint32 AlphaShift;
//    _BitScanForward((DWORD*)&RedShift, RedMask);
//    _BitScanForward((DWORD*)&GreenShift, GreenMask);
//    _BitScanForward((DWORD*)&BlueShift, BlueMask);
//    _BitScanForward((DWORD*)&AlphaShift, AlphaMask);
//
//    color Color;
//    Color.R = (double)((RedMask & Bytes) >> RedShift) / 255.0;
//    Color.G = (double)((GreenMask & Bytes) >> GreenShift) / 255.0;
//    Color.B = (double)((BlueMask & Bytes) >> BlueShift) / 255.0;
//    Color.Alpha = (double)((AlphaMask & Bytes) >> AlphaShift) / 255.0;
//    return Color;
//}

color operator+(color Color1, color Color2) {
    return Color(
        Color1.R + Color2.R,
        Color1.G + Color2.G,
        Color1.B + Color2.B
    );
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Camera                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct camera {
    basis Basis;
    vector_plane Plane;
    v3 Position;
    v3 Velocity;
    double Distance;
    double Pitch;
    double Angle;
};

basis GetCameraBasis(double Angle, double Pitch) {
    v3 X = V3(
        cos(Angle * Degrees),
        0.0,
        sin(Angle * Degrees)
    );
    v3 Y = V3(
        -sin(Angle * Degrees) * sin(Pitch * Degrees),
        cos(Pitch * Degrees),
        cos(Angle * Degrees) * sin(Pitch * Degrees)
    );
    v3 Z = V3(
        sin(Angle * Degrees) * cos(Pitch * Degrees),
        sin(Pitch * Degrees),
        -cos(Angle * Degrees) * cos(Pitch * Degrees)
    );

    return { X, Y, Z };
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Colliders                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct rect_collider {
    v3 Center;
    double Width;
    double Height;
};

struct cube_collider {
    v3 Center;
    scale Size;
};

struct sphere_collider {
    v3 Center;
    double Radius;
};

bool Collide(rect_collider Collider, v3 Position) {
    return fabs(Position.X - Collider.Center.X) < (double)Collider.Width / 2.0 &&
        fabs(Position.Y - Collider.Center.Y) < (double)Collider.Height / 2.0;
}

bool Collide(cube_collider Collider, v3 Position) {
    return fabs(Position.X - Collider.Center.X) < Collider.Size.X / 2.0 &&
           fabs(Position.Y - Collider.Center.Y) < Collider.Size.Y / 2.0 &&
           fabs(Position.Z - Collider.Center.Z) < Collider.Size.Z / 2.0;
}

bool Collide(sphere_collider Collider, v3 Position) {
    return module(Position - Collider.Center) < Collider.Radius;
}

/*
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/
bool HitBoundingBox(double minB[3], double maxB[3], double origin[3], double dir[3], double coord[3])
/* double minB[NUMDIM], maxB[NUMDIM];		box */
/* double origin[NUMDIM], dir[NUMDIM];		ray */
/* double coord[NUMDIM];			hit point   */
{
    bool inside = true;
    char quadrant[3];
    register int i;
    int whichPlane;
    double maxT[3];
    double candidatePlane[3];
    char LEFT = 1;
    char RIGHT = 0;
    char MIDDLE = 2;

    /* Find candidate planes; this loop can be avoided if
    rays cast all from the eye(assume perpsective view) */
    for (i = 0; i < 3; i++)
        if (origin[i] < minB[i]) {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = false;
        }
        else if (origin[i] > maxB[i]) {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = false;
        }
        else {
            quadrant[i] = MIDDLE;
        }

    /* Ray origin inside bounding box */
    if (inside) {
        coord = origin;
        return true;
    }


    /* Calculate T distances to candidate planes */
    for (i = 0; i < 3; i++)
        if (quadrant[i] != MIDDLE && dir[i] != 0.)
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        else
            maxT[i] = -1.;

    /* Get largest of the maxT's for final choice of intersection */
    whichPlane = 0;
    for (i = 1; i < 3; i++)
        if (maxT[whichPlane] < maxT[i])
            whichPlane = i;

    /* Check final candidate actually inside box */
    if (maxT[whichPlane] < 0.) return false;
    for (i = 0; i < 3; i++)
        if (whichPlane != i) {
            coord[i] = origin[i] + maxT[whichPlane] * dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i])
                return false;
        }
        else {
            coord[i] = candidatePlane[i];
        }
    return true;				/* ray hits box */
}

bool Raycast(v3 Origin, v3 Direction, cube_collider Collider) {
    double minB[3] = { 0 };
    minB[0] = Collider.Center.X - Collider.Size.X / 2.0;
    minB[1] = Collider.Center.Y - Collider.Size.Y / 2.0;
    minB[2] = Collider.Center.Z - Collider.Size.Z / 2.0;
    double maxB[3] = { 0 };
    maxB[0] = Collider.Center.X + Collider.Size.X / 2.0;
    maxB[1] = Collider.Center.Y + Collider.Size.Y / 2.0;
    maxB[2] = Collider.Center.Z + Collider.Size.Z / 2.0;
    double origin[3] = { Origin.X, Origin.Y, Origin.Z };
    double dir[3] = { Direction.X, Direction.Y, Direction.Z };
    double coord[3] = { 0,0,0 };

    return HitBoundingBox(minB, maxB, origin, dir, coord);
}

bool Raycast(camera* Camera, double Width, double Height, v2 Mouse, cube_collider Collider) {
    v3 ScreenOffset =
        (2.0 * Mouse.X / Width - 1.0) * Camera->Basis.X +
        (Height - 2.0 * Mouse.Y) / Width * Camera->Basis.Y - Camera->Basis.Z;
    return Raycast(Camera->Position + Camera->Distance * Camera->Basis.Z, ScreenOffset, Collider);
}

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

struct render_entry_circle {
    render_group_header Header;
    coordinate_system Coordinates;
    v3 Center;
    v3 Normal;
    double Radius;
    color Color;
    double Thickness;
    bool Fill;
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
    game_bitmap* Texture;
    wrap_mode Mode;
    v2 Offset;
    double MinTexX;
    double MinTexY;
    double MaxTexX;
    double MaxTexY;
    bool Outline;
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
    //game_video* Video;
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
    game_shader_pipeline_id ShaderID;
    light Light;
    color Color;
};

struct render_entry_heightmap {
    render_group_header Header;
    game_heightmap* Heightmap;
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

struct render_entry_render_target {
    render_group_header Header;
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

        case group_type_render_entry_debug_grid: {
            return sizeof(render_entry_debug_grid);
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
    Entry->Normal = V3(0.0, 0.0, -1.0);
    Entry->Radius = Radius;
    Entry->Color = Color;
}

void PushCircle(
    render_group* Group, 
    v3 Center,
    v3 Normal,
    double Radius, 
    color Color,
    double Order = 0.0
) {
    render_entry_circle* Entry = PushRenderElement(Group, render_entry_circle);
    Entry->Header.Target = World;
    Entry->Coordinates = World_Coordinates;
    Entry->Header.Key.Order = Order;
    Entry->Fill = true;
    Entry->Center = Center;
    Entry->Normal = normalize(Normal);
    Entry->Radius = Radius;
    Entry->Color = Color;
}

void PushCircunference(
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
    Entry->Fill = false;
    Entry->Center = V3(Center.X, Center.Y, 0.0);
    Entry->Normal = V3(0.0,0.0,-1.0);
    Entry->Radius = Radius;
    Entry->Color = Color;
}

void PushCircunference(
    render_group* Group,
    v3 Center,
    v3 Normal,
    double Radius,
    color Color,
    double Order = 0.0
) {
    render_entry_circle* Entry = PushRenderElement(Group, render_entry_circle);
    Entry->Header.Target = World;
    Entry->Coordinates = World_Coordinates;
    Entry->Header.Key.Order = Order;
    Entry->Fill = false;
    Entry->Center = Center;
    Entry->Normal = normalize(Normal);
    Entry->Radius = Radius;
    Entry->Color = Color;
}

void PushRect(
    render_group* Group,
    game_rect Rect,
    color Color,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;
    Entry->Rect = Rect;
    Entry->Color = Color;
}

void PushRectOutline(
    render_group* Group,
    game_rect Rect,
    color Color,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;
    Entry->Rect = Rect;
    Entry->Color = Color;
    Entry->Outline = true;
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

void PushBitmap(
    render_group* Group, 
    game_bitmap* Bitmap, 
    game_rect Rect, 
    wrap_mode Mode = Clamp,
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

    int Width = Entry->Texture->Header.Width;
    int Height = Entry->Texture->Header.Height;

    switch (Entry->Mode) {
        case Clamp: {
            Entry->MinTexX = Size.X < 0 ? 1.0 : 0.0;
            Entry->MaxTexX = Size.X < 0 ? 0.0 : 1.0;
            Entry->MinTexY = Size.Y < 0 ? 1.0 : 0.0;
            Entry->MaxTexY = Size.Y < 0 ? 0.0 : 1.0;
        } break;

        case Crop: {
            double MinX = Offset.X / Size.X / Width;
            double MinY = 1.0 - (Rect.Height + Offset.Y) / Size.Y / Height;
            double MaxX = (Rect.Width + Offset.X) / Size.X / Width;
            double MaxY = 1.0 - Offset.Y / Size.Y / Height;
            Entry->MinTexX = Size.X < 0 ? MaxX : MinX;
            Entry->MaxTexX = Size.X < 0 ? MinX : MaxX;
            Entry->MinTexY = Size.Y < 0 ? MaxY : MinY;
            Entry->MaxTexY = Size.Y < 0 ? MinY : MaxY;
        } break;

        case Repeat: {
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
    game_rect Rect, 
    wrap_mode Mode = Clamp,
    v2 Size = V2(1.0, 1.0),
    v2 Offset = V2(0,0),
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_bitmap* Bitmap = GetAsset(Group->Assets, ID);
    PushBitmap(Group, Bitmap, Rect, Mode, Size, Offset, Order);
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

void PushCubeOutline(
    render_group* Group,
    cube_collider Collider,
    color Color,
    double Order = SORT_ORDER_DEBUG_OVERLAY
) {
    PushCubeOutline(Group, Collider.Center - 0.5 * Collider.Size * V3(1.0, 1.0, 1.0), Collider.Size, Color, Order);
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

struct slider {
    double Value;
    v3 Position;
    color Color;
    rect_collider Collider;
};

struct UI {
    slider Slider1;
    slider Slider2;
    slider Slider3;
    slider Slider4;
    slider Slider5;
    slider Slider6;
};

void InitSlider(slider* Slider, double Value, color Color) {
    *Slider = { 0 };
    
    if (Value > 1.0 || Value < 0.0) Assert(false);
    else Slider->Value = Value;

    Slider->Color = Color;
}

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
    PushCircunference(Group, V2(Slider.Position.X, CircleCenter), Radius, Slider.Color, Order);
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
    //PushShaderPass(Group, Shader_Outline_Init_ID, Postprocessing_Outline, White, SORT_ORDER_SHADER_PASSES);

    render_entry_mesh_outline* Entry = PushRenderElement(Group, render_entry_mesh_outline);
    Entry->Header.Key.Order = SORT_ORDER_SHADER_PASSES + 10.0;
    Entry->Header.Target = Postprocessing_Outline;

    Entry->Passes = Passes;
    Entry->Width = Width;
    Entry->StartingLevel = StartingLevel;

    //PushShaderPass(Group, Shader_Outline_ID, Postprocessing_Outline, Color, Width, 0, SORT_ORDER_SHADER_PASSES + 20.0);

    PushRenderTarget(Group, Postprocessing_Outline, SORT_ORDER_PUSH_RENDER_TARGETS - 10.0);
}

void PushMesh(
    render_group* Group,
    game_mesh_id MeshID,
    transform Transform,
    light Light,
    game_shader_pipeline_id ShaderID,
    game_bitmap_id TextureID = Bitmap_Empty_ID,
    color Color = White,
    double Order = SORT_ORDER_MESHES,
    bool Outlined = false
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
        OutlineEntry->Light = Light;
        OutlineEntry->ShaderID = Single_Color_Shader_Pipeline_ID;
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
    Entry->Texture = pTexture;
    Entry->Light = Light;
    Entry->ShaderID = ShaderID;
    Entry->Color = Color;
}

void PushHeightmap(render_group* Group, game_heightmap_id ID, double Order = SORT_ORDER_MESHES) {
    render_entry_heightmap* Entry = PushRenderElement(Group, render_entry_heightmap);
    Entry->Header.Key.Order = Order;
    Entry->Header.Target = World;

    Entry->Heightmap = GetAsset(Group->Assets, ID);
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
    double ArenaPercentage = (double)Arena.Used / (double)Arena.Size;
    game_rect Rect = { Position.X, Position.Y, 120.0, 20.0 };
    PushRect(Group, Rect, Color(DarkGray, Alpha), Order + 0.1);
    Rect.Width *= ArenaPercentage;
    PushRect(Group, Rect, Color(Red, Alpha), Order + 0.2);
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

    int Thickness = max(1.0, 0.0025 * Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, Coordinates, Order);
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