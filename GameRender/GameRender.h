#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#pragma once
#include "GameAsset.h"
#include "GameColor.h"


// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Render entries                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(Sort_Key,
    Sort_Background,
    Sort_Terrain,
    Sort_Meshes,
    Sort_UI,
    Sort_Debug
);

ENUM(Render_Primitive,
    Render_Primitive_Point,
    Render_Primitive_Line,
    Render_Primitive_Line_Strip,
    Render_Primitive_Triangle,
    Render_Primitive_Triangle_Strip,
    Render_Primitive_Patches
);

ENUM(Render_Group_Target,
    Target_None,
    Target_World,
    Target_Outline,
    Target_Postprocessing_Outline,
    Target_PingPong,
    Target_Fluid,
    Target_Output
);

FLAGS(Render_Flag,
    depth_test,
    stencil_test,
    overwrite_alpha,

    text_outline,

    debug_bones,

    sky,
    water
);

struct Render_Command {
    Sort_Key key;
    Render_Primitive primitive;
    Render_Flag flags;
    // Pipeline pipeline;
    Vertex_Buffer_Entry positions;
    Vertex_Buffer_Entry attributes;
    Vertex_Buffer_Entry joints;
    Vertex_Buffer_Entry pose;
    Vertex_Buffer_Entry indices;
    Vertex_Buffer_Entry instances;
    Matrix4 transform;
    // Material material;
    Texture* texture;
    Color color;
    bool dynamic;
};

struct Render_Command_Options {
    Color color = Color::WHITE;
    Render_Flag flags;
    Vertex_Layout_ID instance_layout_id;
    Sort_Key order = Sort_Meshes;
    Texture* texture = nullptr;
    Transform transform = IdentityTransform;
    bool dynamic;
};

struct Render_Text_Options {
    Color color        = Color::WHITE;
    Font* Font         = nullptr;
    bool Outline       = false;
    Color OutlineColor = Color::BLACK;
    float OutlineWidth = 2.0f;
    float Points       = 20.0f;
};

ENUM(Color_Format,
    Color_Format_R,
    Color_Format_RG,
    Color_Format_RGB,
    Color_Format_RGBA
);

struct Render_Group_Target_Description {
    Render_Group_Target target;
    Color_Format format;
    bool multisample;
    bool depth;
    bool stencil;
};

const int COMPUTE_GROUP_SIZE = 16;

// struct render_compute_command {
//     compute_type Type;
//     Render_Group_Target Source;
//     Render_Group_Target Target;
//     IV3 nGroups;
//     Matrix3 Kernel;
//     Color color;
//     int Level;
//     float Width;
// };

enum wrap_mode {
    Wrap_Clamp,
    Wrap_Repeat,
    Wrap_Crop
};

struct Light {
    Color color;
    V3 direction;
    V3 camera_position;
    float ambient;
    float diffuse;
};

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Render group                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_RENDER_ENTRIES = 16384;

struct Render_Group {
    Render_Command entries[MAX_RENDER_ENTRIES];
    Render_Group_Target_Description targets[Render_Group_Target_count];
    Vertex_Buffer vertex_buffer;
    Text_Buffer text_buffer;
    Light light;
    Asset_Manager* asset_manager;
    Font* debug_font;
    Arena* transient;
    int32 width;
    int32 height;
    uint32 entry_count;
    bool Debug;
    bool DebugNormals;
    bool DebugBones;
    bool DebugColliders;
    bool PushOutline;
    bool RenderText;
};

void InitializeRenderGroup(
    Render_Group* group, 
    Arena* permanent,
    Arena* transient,
    Asset_Manager* asset_manager,
    int32 width,
    int32 height
) {
    group->transient = transient;

    group->width = width;
    group->height = height;

    group->asset_manager = asset_manager;
    // group->DebugFont = GetFont(AssetManager, "DejaVuSansMono");
    
    group->Debug = false;
    group->DebugNormals = false;
    group->DebugColliders = false;
    group->DebugBones = false;

    // Lighting
    group->light.color = Color::WHITE;
    group->light.direction = normalize(V3(-0.5, -1, 1));
    group->light.ambient = 0.5f;
    group->light.diffuse = 0.5f;

    // Vertex & element buffers
    InitializeVertexBuffer(&group->vertex_buffer, permanent);

    // Text buffer
    InitializeTextBuffer(permanent, &group->text_buffer);

    // Render targets
    group->targets[Target_None] = {};

    group->targets[Target_World] = {
        .target = Target_World,
        .format = Color_Format_RGBA,
        .multisample = true,
        .depth = true,
        .stencil = false
    };

    group->targets[Target_Outline] = {
        .target = Target_Outline,
        .format = Color_Format_RGBA,
        .multisample = true,
        .depth = true,
        .stencil = false
    };

    group->targets[Target_Postprocessing_Outline] = {
        .target = Target_Postprocessing_Outline,
        .format = Color_Format_RGBA,
        .multisample = false,
        .depth = false,
        .stencil = false
    };

    group->targets[Target_Output] = {
        .target = Target_Output,
        .format = Color_Format_RGB,
        .multisample = false,
        .depth = true,
        .stencil = false
    };

    group->targets[Target_PingPong] = {
        .target = Target_PingPong,
        .format = Color_Format_RGBA,
        .multisample = false,
        .depth = true,
        .stencil = false
    };

    group->targets[Target_Fluid] = {
        .target = Target_Fluid,
        .format = Color_Format_RGB,
        .multisample = false,
        .depth = true,
        .stencil = false
    };
}

void Swap(Render_Group* group, int i, int j) {
    Render_Command entry = group->entries[i];
    group->entries[i] = group->entries[j];
    group->entries[j] = entry;
}

void _PushCommand(Render_Group*group,
    Render_Primitive primitive,
    uint32 vertex_count,
    Render_Command_Options options
) {
    Render_Command command;
    command.key = options.order;
    command.primitive = primitive;
    command.flags = options.flags;
    command.transform = TRS_Matrix(options.transform);
    command.texture = options.texture;
    command.color = options.color;

    // Sort
    uint32 i = group->entry_count;
    group->entries[i] = command;
    if (group->entry_count > 0) {
        uint32 j = i - 1;
        while(i > 0) {
            Render_Command check = group->entries[j];
            if (check.key > command.key) {
                Swap(group, i, j);
                i = j;
                j = i - 1;
                continue;
            }
            break;
        }
    }
    ++group->entry_count;
}

void Clear(Render_Group* group) {
    ZeroSize(group->entry_count * sizeof(Render_Command), group->entries);
    group->entry_count = 0;

    group->PushOutline = false;
    group->RenderText = false;
    
    ClearVertexBuffer(&group->vertex_buffer);
    ClearTextBuffer(&group->text_buffer);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders & render targets                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

// void PushOutlineInitCompute(
//     Render_Group* group,
//     Render_Group_Target Target,
//     float Order = 0.0f
// ) {
//     render_command Command = {};
//     Command.Type = render_compute;
//     Command.Index = group->nComputeCommands;
//     Command.Priority = Order;

//     PushCommand(Group, Command);

//     render_compute_command ComputeCommand = {};
//     ComputeCommand.Type = compute_outline_init;
//     ComputeCommand.Target = Target_Postprocessing_Outline;
//     ComputeCommand.nGroups.X = (group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Y = (group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Z = 1;

//     group->ComputeCommands[group->nComputeCommands++] = ComputeCommand;
// }

// void PushJumpFloodCompute(
//     Render_Group* group,
//     int Level,
//     float Order = 0.0f
// ) {
//     render_command Command = {};
//     Command.Type = render_compute;
//     Command.Index = group->nComputeCommands;
//     Command.Priority = Order;

//     PushCommand(Group, Command);

//     render_compute_command ComputeCommand = {};
//     ComputeCommand.Type = compute_jump_flood;
//     ComputeCommand.Target = Target_Postprocessing_Outline;
//     ComputeCommand.nGroups.X = (group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Y = (group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Z = 1;
//     ComputeCommand.Level = Level;

//     group->ComputeCommands[group->nComputeCommands++] = ComputeCommand;
// }

// void PushOutlineCompute(
//     Render_Group* group,
//     Color color,
//     float Width,
//     float Order = 0.0f
// ) {
//     render_command Command = {};
//     Command.Type = render_compute;
//     Command.Index = group->nComputeCommands;
//     Command.Priority = Order;

//     PushCommand(Group, Command);

//     render_compute_command ComputeCommand = {};
//     ComputeCommand.Type = compute_outline;
//     ComputeCommand.Target = Target_Postprocessing_Outline;
//     ComputeCommand.color = color;
//     ComputeCommand.Width = Width;
//     ComputeCommand.nGroups.X = (group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Y = (group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Z = 1;

//     group->ComputeCommands[group->nComputeCommands++] = ComputeCommand;
// }

// void PushKernelCompute(
//     Render_Group* group,
//     Render_Group_Target Target,
//     matrix3 Kernel,
//     float Order = SORT_ORDER_SHADER_PASSES
// ) {
//     render_command Command;
//     Command.Type = render_compute;
//     Command.Index = group->nComputeCommands;
//     Command.Priority = Order;

//     PushCommand(Group, Command);

//     render_compute_command ComputeCommand;
//     ComputeCommand.Type = compute_kernel;
//     ComputeCommand.Target = Target;
//     ComputeCommand.Kernel = Kernel;
//     ComputeCommand.nGroups.X = (group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Y = (group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Z = 1;

//     group->ComputeCommands[group->nComputeCommands++] = ComputeCommand;
// }

// void PushBlur(
//     Render_Group* group,
//     Render_Group_Target Target,
//     float Order = SORT_ORDER_SHADER_PASSES
// ) {
//     matrix3 Kernel = {
//         1, 2, 1,
//         2, 4, 2,
//         1, 2, 1
//     };
//     Kernel *= 1.0f / 16.0f;

//     PushKernelCompute(Group, Target, Kernel, Order);
// }

// void PushFFT(
//     Render_Group* group, 
//     Render_Group_Target Source,
//     Render_Group_Target Target
// ) {
//     render_command Command;
//     Command.Type = render_compute;
//     Command.Index = group->nComputeCommands;
//     Command.Priority = SORT_ORDER_CLEAR;

//     PushCommand(Group, Command);

//     render_compute_command ComputeCommand;
//     ComputeCommand.Type = compute_fft;
//     ComputeCommand.Source = Source;
//     ComputeCommand.Target = Target;
//     ComputeCommand.nGroups.X = 1024 / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Y = 1024 / COMPUTE_GROUP_SIZE;
//     ComputeCommand.nGroups.Z = 1;

//     group->ComputeCommands[group->nComputeCommands++] = ComputeCommand;
// }

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Render commands                                                                                                                                                  |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

#define PushCommand(Group, Primitive, LayoutID, nVertices, ...) _PushCommand(Group, Primitive, LayoutID, nVertices, { __VA_ARGS__ })

void PushRect() {

}

void PushPoint(Render_Group* group, V2 point, Color color, Sort_Key Order = Sort_Debug) {
    float* Vertices = (float*)PushCommand(
        group,
        render_primitive_point, 
        vertex_layout_v2_id, 
        1,
        .color = color,
        .Order = Order
    )->VertexEntry.Pointer;
    Vertices[0] = Point.X;
    Vertices[1] = Point.Y;
}

void PushPoint(Render_Group* group, v3 Point, Color color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
    float* Vertices = PushPrimitiveCommand(
        Group, 
        render_primitive_point,
        vertex_layout_v3_id, 
        1,
        .color = color,
        .Flags = render_flag::depth_test,
        .Order = Order
    )->Vertices;
    Vertices[0] = Point.X;
    Vertices[1] = Point.Y;
    Vertices[2] = Point.Z;
}

void PushLine(
    Render_Group* group,
    v2 Start,
    v2 End,
    Color color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        vertex_layout_v2_id,
        2,
        .color = color,
        .Order = Order,
        .Thickness = Thickness
    )->Vertices;
    Vertices[0] = Start.X;
    Vertices[1] = Start.Y;
    Vertices[2] = End.X;
    Vertices[3] = End.Y;
}

void PushLine(
    Render_Group* group,
    v3 Start,
    v3 End,
    Color color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        vertex_layout_v3_id,
        2,
        .color = color,
        .Flags = render_flag::depth_test,
        .Order = Order,
        .Thickness = Thickness
    )->Vertices;
    Vertices[0] = {Start.X};
    Vertices[1] = {Start.Y};
    Vertices[2] = {Start.Z};
    Vertices[3] = {End.X};
    Vertices[4] = {End.Y};
    Vertices[5] = {End.Z};
}

void PushRay(
    Render_Group* group,
    ray Ray,
    Color color,
    float Thickness = 2.0f,
    float Length = 10.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    v3 Start = Ray.Point;
    v3 End = Start + Length * Ray.Direction;
    PushLine(Group, Start, End, color, Thickness, Order);
}

void PushTriangle(
    Render_Group* group,
    triangle3 Triangle,
    Color color,
    float Order = SORT_ORDER_MESHES
) {
    float* Vertices = PushPrimitiveCommand(
        Group, 
        render_primitive_triangle,
        vertex_layout_v3_id,
        3,
        .color = color,
        .Flags = render_flag::depth_test,
        .Order = Order
    )->Vertices;
    Vertices[0] = Triangle.Points[0].X;
    Vertices[1] = Triangle.Points[0].Y;
    Vertices[2] = Triangle.Points[0].Z;
    Vertices[3] = Triangle.Points[1].X;
    Vertices[4] = Triangle.Points[1].Y;
    Vertices[5] = Triangle.Points[1].Z;
    Vertices[6] = Triangle.Points[2].X;
    Vertices[7] = Triangle.Points[2].Y;
    Vertices[8] = Triangle.Points[2].Z;
}

void PushTriangle(
    Render_Group* group,
    triangle2 Triangle,
    Color color,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v2_id,
        3,
        .color = color,
        .Order = Order
    )->Vertices;

    Vertices[0] = Triangle.Points[0].X;
    Vertices[1] = Triangle.Points[0].Y;
    Vertices[2] = Triangle.Points[1].X;
    Vertices[3] = Triangle.Points[1].Y;
    Vertices[4] = Triangle.Points[2].X;
    Vertices[5] = Triangle.Points[2].Y;
}

void PushCircle(
    Render_Group* group,
    v2 Center,
    float Radius,
    Color color,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 30
) {
    int MAX_N = 62;
    int N = Clamp(nVertices, 14, MAX_N);

    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v2_id,
        N+1,
        .color = color,
        .nElements = 3*N,
        .Order = Order
    );

    v2* Vertices = (v2*)Command->VertexEntry.Pointer;
    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = Center;
    Vertices[1] = V2(Center.X, Center.Y - Radius);
    for (int i = 2; i < N+1; i++) {
        Vertices[i].X = Center.X + Radius * sin(Theta);
        Vertices[i].Y = Center.Y - Radius * cos(Theta);
        Theta += dTheta;
    }

    uint32* Elements = Command->ElementEntry.Pointer;
    uint32 Offset = Command->VertexEntry.Offset;
    for (int i = 0; i < N - 1; i++) {
        Elements[3*i] = Offset;
        Elements[3*i + 1] = Offset + i + 1;
        Elements[3*i + 2] = Offset + i + 2;
    }
    Elements[3*N - 3] = Offset + 0;
    Elements[3*N - 2] = Offset + N;
    Elements[3*N - 1] = Offset + 1;
}

void PushCircle(
    Render_Group* group, 
    v3 Center,
    float Radius, 
    v3 Normal,
    Color color,
    float Order = SORT_ORDER_MESHES,
    int nVertices = 32
) {
    int MAX_N = 62;
    int N = Clamp(nVertices, 14, MAX_N);
    Normal = normalize(Normal);
    basis Basis = Complete(Normal);
    Basis.X = Basis.Z;
    Basis.Z = Normal;

    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v3_id,
        N+1,
        .color = color,
        .Flags = render_flag::depth_test,
        .nElements = 3*N,
        .Order = Order,
    );
    
    v3* Vertices = (v3*)Command->VertexEntry.Pointer;

    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = Center;
    Vertices[1] = Center - Radius * Basis.Y;
    for (int i = 2; i < N+1; i++) {
        Vertices[i] = Center + Radius * (sin(Theta) * Basis.X - cos(Theta) * Basis.Y);
        Theta += dTheta;
    }

    uint32* Elements = Command->ElementEntry.Pointer;
    uint32 Offset = Command->VertexEntry.Offset;
    for (int i = 0; i < N - 1; i++) {
        Elements[3*i] = Offset;
        Elements[3*i + 1] = Offset + i + 1;
        Elements[3*i + 2] = Offset + i + 2;
    }
    Elements[3*N - 3] = Offset + 0;
    Elements[3*N - 2] = Offset + N;
    Elements[3*N - 1] = Offset + 1;
}

void PushCircunference(
    Render_Group* group,
    v2 Center,
    float Radius,
    Color color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 32
) {
    int MAX_N = 64;
    int N = Clamp(nVertices, 16, MAX_N);

    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        vertex_layout_v2_id,
        N+1,
        .color = color,
        .Order = Order,
        .Thickness = Thickness
    )->Vertices;

    v2* Vertices = (v2*)Data;

    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = V2(Center.X, Center.Y - Radius);
    for (int i = 1; i < N; i++) {
        Vertices[i].X = Center.X + Radius * sin(Theta);
        Vertices[i].Y = Center.Y - Radius * cos(Theta);
        Theta += dTheta;
    }
    Vertices[N] = V2(Center.X, Center.Y - Radius);
}

void PushCircunference(
    Render_Group* group,
    v3 Center,
    v3 Normal,
    float Radius,
    Color color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 32
) {
    int MAX_N = 64;
    int N = Clamp(nVertices, 16, MAX_N);

    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        vertex_layout_v3_id,
        N+1,
        .color = color,
        .Flags = render_flag::depth_test,
        .Order = Order
    )->Vertices;

    v3* Vertices = (v3*)Data;

    basis Basis = Complete(Normal);
    Basis.X = Basis.Z;
    Basis.Z = Normal;

    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = Center - Radius * Basis.Y;
    for (int i = 1; i < N; i++) {
        Vertices[i] = Center + Radius * (sin(Theta) * Basis.X - cos(Theta) * Basis.Y);
        Theta += dTheta;
    }
    Vertices[N] = Center - Radius * Basis.Y;
}

/*
    Pushes an arc of circunference to the renderer. Basis will determine the plane in which the circunference is contained.
    Basis.Z will be the normal to this plane, and Basis.X will be the offset from the center where the arc will be started.
*/
void PushArc(
    Render_Group* group,
    v3 Center,
    basis Basis,
    float Radius,
    double Angle,
    Color color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 32
) {
    int MAX_N = 64;
    int N = Clamp(nVertices, 16, MAX_N);

    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        vertex_layout_v3_id,
        N,
        .color = color,
        .Flags = render_flag::depth_test,
        .Order = Order
    )->Vertices;

    v3* Vertices = (v3*)Data;

    double dTheta = Angle * Degrees / (N-1);
    double Theta = dTheta;
    Vertices[0] = Center + Radius * Basis.X;
    for (int i = 1; i < N; i++) {
        Vertices[i] = Center + Radius * (cos(Theta) * Basis.X + sin(Theta) * Basis.Y);
        Theta += dTheta;
    }
}

void PushRect(
    Render_Group* group,
    rectangle Rect,
    Color color,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v2_id,
        4,
        .color = color,
        .nElements = 6,
        .Order = Order
    );

    v2* Vertices = (v2*)Result->Vertices;
    uint32* Elements = Result->ElementEntry.Pointer;
    uint32 Offset = (uint32)Result->VertexEntry.Offset;

    Vertices[0] = { Rect.Left             , Rect.Top               };
    Vertices[1] = { Rect.Left + Rect.Width, Rect.Top               };
    Vertices[2] = { Rect.Left             , Rect.Top + Rect.Height };
    Vertices[3] = { Rect.Left + Rect.Width, Rect.Top + Rect.Height };

    Elements[0] = Offset + 0;
    Elements[1] = Offset + 1;
    Elements[2] = Offset + 2;
    Elements[3] = Offset + 3;
    Elements[4] = Offset + 2;
    Elements[5] = Offset + 1;
}

void PushRect(
    Render_Group* group,
    v3 LeftTop,
    v3 WidthAxis,
    v3 HeightAxis,
    float Width,
    float Height,
    Color color,
    float Order = SORT_ORDER_MESHES
) {
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v3_id,
        4,
        .color = color,
        .Flags = render_flag::depth_test,
        .nElements = 6,
        .Order = Order
    );

    WidthAxis = normalize(WidthAxis);
    HeightAxis = normalize(HeightAxis);
    
    v3 RightTop = LeftTop + Width * WidthAxis;
    v3 LeftBottom = LeftTop + Height * HeightAxis;
    v3 RightBottom = RightTop + Height * HeightAxis;

    v3* Vertices = (v3*)Result->Vertices;
    Vertices[0] = { LeftTop.X,     LeftTop.Y,     LeftTop.Z };
    Vertices[1] = { RightTop.X,    RightTop.Y,    RightTop.Z };
    Vertices[2] = { LeftBottom.X,  LeftBottom.Y,  LeftBottom.Z };
    Vertices[3] = { RightBottom.X, RightBottom.Y, RightBottom.Z };

    uint32 VertexOffset = Result->VertexEntry.Offset;

    uint32* Elements = Result->ElementEntry.Pointer;
    Elements[0] = VertexOffset + 0;
    Elements[1] = VertexOffset + 1;
    Elements[2] = VertexOffset + 2;
    Elements[3] = VertexOffset + 3;
    Elements[4] = VertexOffset + 2;
    Elements[5] = VertexOffset + 1;
}

void PushRectOutline(
    Render_Group* group,
    rectangle Rect,
    Color color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        vertex_layout_v2_id,
        5,
        .color = color,
        .Order = Order,
        .Thickness = Thickness
    )->Vertices;
    
    Vertices[0] = { Rect.Left             , Rect.Top               };
    Vertices[1] = { Rect.Left + Rect.Width, Rect.Top               };
    Vertices[2] = { Rect.Left + Rect.Width, Rect.Top + Rect.Height };
    Vertices[3] = { Rect.Left             , Rect.Top + Rect.Height };
    Vertices[4] = { Rect.Left             , Rect.Top               };
}

void PushBitmap(
    Render_Group* group, 
    Texture* texture, 
    rectangle Rect, 
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    wrap_mode WrapMode = Wrap_Clamp,
    v2 Size = V2(1.0f, 1.0f),
    v2 Offset = V2(0.0f , 0.0f),
    bool Refresh = false
) {
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v2_v2_id,
        4,
        .nElements = 6,
        .Order = Order,
        .texture = texture
    );

    int Width = texture->width;
    int Height = texture->height;
    float MinTexX = 0.0f;
    float MaxTexX = 1.0f;
    float MinTexY = 0.0f;
    float MaxTexY = 1.0f;
    switch (WrapMode) {
        case Wrap_Clamp: {
            MinTexX = Size.X < 0 ? 1.0f : 0.0f;
            MaxTexX = Size.X < 0 ? 0.0f : 1.0f;
            MinTexY = Size.Y < 0 ? 1.0f : 0.0f;
            MaxTexY = Size.Y < 0 ? 0.0f : 1.0f;
        } break;

        case Wrap_Crop: {
            float MinX = Offset.X / Size.X / Width;
            float MinY = 1.0 - (Rect.Height + Offset.Y) / Size.Y / Height;
            float MaxX = (Rect.Width + Offset.X) / Size.X / Width;
            float MaxY = 1.0 - Offset.Y / Size.Y / Height;
            MinTexX = Size.X < 0 ? MaxX : MinX;
            MaxTexX = Size.X < 0 ? MinX : MaxX;
            MinTexY = Size.Y < 0 ? MaxY : MinY;
            MaxTexY = Size.Y < 0 ? MinY : MaxY;
        } break;

        case Wrap_Repeat: {
            float MinX = 0.0;
            float MinY = -Rect.Height / (Size.Y * Height);
            float MaxX = Rect.Width / (Size.X * Width);
            float MaxY = 1.0;
            MinTexX = Size.X < 0 ? MaxX : MinX;
            MaxTexX = Size.X < 0 ? MinX : MaxX;
            MinTexY = Size.Y ? MaxY : MinY;
            MaxTexY = Size.Y ? MinY : MaxY;
        } break;

        default: { Assert(false); }
    }
    
    v4* Vertices = (v4*)Result->Vertices;
    Vertices[0] = { Rect.Left             , Rect.Top              , MinTexX, MaxTexY };
    Vertices[1] = { Rect.Left + Rect.Width, Rect.Top              , MaxTexX, MaxTexY };
    Vertices[2] = { Rect.Left             , Rect.Top + Rect.Height, MinTexX, MinTexY };
    Vertices[3] = { Rect.Left + Rect.Width, Rect.Top + Rect.Height, MaxTexX, MinTexY };

    uint32* Elements = Result->ElementEntry.Pointer;
    Elements[0] = Result->VertexEntry.Offset + 0;
    Elements[1] = Result->VertexEntry.Offset + 1;
    Elements[2] = Result->VertexEntry.Offset + 2;
    Elements[3] = Result->VertexEntry.Offset + 3;
    Elements[4] = Result->VertexEntry.Offset + 2;
    Elements[5] = Result->VertexEntry.Offset + 1;
}

void _PushText(
    Render_Group* group,
    v2 Position,
    string String,
    render_text_options Options = {}
) {
    if (!group->RenderText) {
        render_command Command = {};
        Command.Type = render_text;
        Command.Index = 0;
        Command.Priority = SORT_ORDER_DEBUG_OVERLAY + 7000.0f;

        PushCommand(Group, Command);
    }

    uint32 nCharacters = 0;
    for (int i = 0; i < String.Length; i++) {
        if (String[i] == '\0') break;
        if (String[i] >= '!' && String[i] <= '~') nCharacters++;
    }

    Color color = Options.color;
    
    v2 Pen = Position;
    float DPI = 96;
    float Size = Options.Points * (DPI / 72.0f) / Options.Font->UnitsPerEm;
    float LineJump = Options.Font->LineJump * Size;

    for (int i = 0; i < String.Length; i++) {
        char c = String[i];

        if (
            c == '\0' || 
            (c == '#' && String[i+1] == '#')
        ) break;

        // Carriage returns
        if (c == '\n') {
            Pen.X = Position.X;
            Pen.Y += LineJump;
        }

        // Space
        else if (c == ' ') {
            Pen.X += Options.Font->SpaceAdvance * Size;
        }

        // Character
        else if ('!' <= c && c <= '~') {
            Font_character* pCharacter = Options.Font->Characters + (c - '!');
            float HorizontalAdvance = pCharacter->Width * Size;
            // if (Options.Wrapped && (Pen.X + HorizontalAdvance > group->Width)) {
            //     Pen.X = Position.X;
            //     Pen.Y += LineJump;
            // }

            if (pCharacter->nContours > 0) {
                PushTextEntry(&group->text_buffer, Options.Font, c, Pen, Size, Options.color);

                if (Options.Outline) {
                    // TODO: Add text outline back
                    // render_primitive_command* Command = PushPrimitiveCommand(
                    //     Group,
                    //     render_primitive_patches,
                    //     Options.OutlineColor,
                    //     vertex_layout_v2_v2_id,
                    //     3 * pCharacter->nOnCurve,
                    //     TEXT_OUTLINE_FLAG,
                    //     SORT_ORDER_DEBUG_OVERLAY,
                    //     PrimitiveOptions
                    // );

                    // Command->VertexEntry.Offset = pCharacter->VertexOffset;
                }
            }

            Pen.X += pCharacter->Width * Size;
        }
    }
}

#define PushText(Group, Position, String, ...) _PushText(Group, Position, String, { __VA_ARGS__ })

void PushFillbar(
    Render_Group* group,
    string Description,
    float FillPercentage,
    rectangle Rect,
    Color color = Color::RED
) {
    PushRect(Group, Rect, Color::DARK_GRAY);
    rectangle SmallRect = Rect;
    SmallRect.Width *= FillPercentage;
    PushRect(Group, SmallRect, color);

    v2 Position = LeftTop(Rect);
    PushText(Group, Position + V2(5.0f, 15.0f), Description, .Points = 10);

    int Points = 8;
    string Text = Format(group->Transient, "{f2}", 1, 100.0f * FillPercentage);
    float Width = GetTextWidth(Text, group->DebugFont, Points);
    PushText(Group, Position + V2(Rect.Width - Width - 5.0f, 15.0f), Text, .Points = 8);
}

void PushFillbar(
    Render_Group* group,
    char* Description,
    int Used,
    int Max,
    rectangle Rect,
    Color color = Color::RED
) {
    float FillPercentage = (float)Used / (float)Max;
    float Points = 8;
    PushRect(Group, Rect, Color::DARK_GRAY);
    rectangle SmallRect = Rect;
    SmallRect.Width *= FillPercentage;
    PushRect(Group, Rect, color);

    v2 Position = LeftTop(Rect);
    PushText(Group, Position + V2(5.0f, 15.0f), Description, .Points = Points);

    string Text = Format(group->Transient, "{i}/{i}", Used, Max);
    float Width = GetTextWidth(Text, group->DebugFont, Points);
    PushText(Group, Position + V2(Rect.Width - Width - 5.0f, 15.0f), Text, .Points = Points);
}

void PushFillbar(
    Render_Group* group,
    char* Description,
    int Used,
    int Max,
    v3 LeftTop,
    v3 WidthAxis,
    v3 HeightAxis,
    float Width,
    float Height,
    Color color = Color::RED
) {
    float FillPercentage = (float)Used / (float)Max;
    PushRect(Group, LeftTop, WidthAxis, HeightAxis, Width, Height, Color::DARK_GRAY);
    float SmallWidth = FillPercentage * Width;
    PushRect(Group, LeftTop, WidthAxis, HeightAxis, SmallWidth, Height, Color::RED);
}

void PushCubeOutline(
    Render_Group* group,
    v3 Position,
    scale Scale = GetScale(1.0),
    Color color = Color::WHITE,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        vertex_layout_v3_id,
        8,
        .color = color,
        .Flags = render_flag::depth_test,
        .nElements = 24,
        .Order = Order
    );

    v3* Vertices = (v3*)Result->Vertices;
    Vertices[0] = Position + V3(0.0, Scale.Y, Scale.Z);
    Vertices[1] = Position + V3(Scale.X, Scale.Y, Scale.Z);
    Vertices[2] = Position + V3(0.0, 0.0, Scale.Z);
    Vertices[3] = Position + V3(Scale.X, 0.0, Scale.Z);
    Vertices[4] = Position + V3(Scale.X, 0.0, 0.0);
    Vertices[5] = Position + V3(Scale.X, Scale.Y, 0.0);
    Vertices[6] = Position + V3(0.0, Scale.Y, 0.0);
    Vertices[7] = Position + V3(0.0, 0.0, 0.0);

    uint32 VertexOffset = Result->VertexEntry.Offset;

    uint32* Elements = Result->ElementEntry.Pointer;
    Elements[0]  = VertexOffset + 0;
    Elements[1]  = VertexOffset + 1;
    Elements[2]  = VertexOffset + 0;
    Elements[3]  = VertexOffset + 2;
    Elements[4]  = VertexOffset + 0;
    Elements[5]  = VertexOffset + 6;
    Elements[6]  = VertexOffset + 5;
    Elements[7]  = VertexOffset + 6;
    Elements[8]  = VertexOffset + 1;
    Elements[9]  = VertexOffset + 5;
    Elements[10] = VertexOffset + 1;
    Elements[11] = VertexOffset + 3;
    Elements[12] = VertexOffset + 4;
    Elements[13] = VertexOffset + 5;
    Elements[14] = VertexOffset + 2;
    Elements[15] = VertexOffset + 7;
    Elements[16] = VertexOffset + 2;
    Elements[17] = VertexOffset + 3;
    Elements[18] = VertexOffset + 3;
    Elements[19] = VertexOffset + 4;
    Elements[20] = VertexOffset + 4;
    Elements[21] = VertexOffset + 7;
    Elements[22] = VertexOffset + 6;
    Elements[23] = VertexOffset + 7;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Video                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

// void PushVideo(
//     Render_Group* group, 
//     game_video_id VideoID, 
//     rectangle Rect, 
//     double SecondsElapsed, 
//     float Order = SORT_ORDER_DEBUG_OVERLAY
// ) {
//     game_video* Video = GetAsset(group->Assets, VideoID);
//     int Width = roundf(Rect.Width);
//     int Height = roundf(Rect.Height);
//     if (!Video->VideoContext.Ended) {
//         TIMED_BLOCK;
//         Video->TimeElapsed += SecondsElapsed;
//         char Text[256];
//         sprintf_s(Text, "%.2f Time elapsed | %.2f Time played", Video->TimeElapsed, Video->VideoContext.PTS * Video->VideoContext.TimeBase);
//         Log(Info, Text);

//         render_entry_rect* Entry = PushRenderElement(Group, render_entry_rect);
//         Entry->Header.Target = World;
//         Entry->Header.Key.Order = Order;
   
//         Entry->Texture = &Video->Texture;
//         Entry->WrapMode = Wrap_Clamp;
//         Entry->Header.Key.Order = Order;
//         Entry->Rect = Rect;
//         Entry->RefreshTexture = true;
//         Entry->Color = color::WHITE;

//         if (Video->TimeElapsed > Video->VideoContext.PTS * Video->VideoContext.TimeBase) {
//             LoadFrame(&Video->VideoContext);
//             Entry->MinTexX = 0.0;
//             Entry->MinTexY = Clamp(Rect.Height / (float)Video->Height, 0.0f, 1.0f);
//             Entry->MaxTexX = Clamp(Rect.Width / (float)Video->Width, 0.0f, 1.0f);
//             Entry->MaxTexY = 0.0;
            
//             Video->Texture.Header.Width = Width;
//             Video->Texture.Header.Height = Height;
//         }
//         WriteFrame(&Video->VideoContext, Width, Height);
//     }
// }

// void PushVideoLoop(
//     Render_Group* group, 
//     game_video_id VideoID, 
//     rectangle Rect, 
//     double SecondsElapsed, 
//     int64_t StartOffset, 
//     int64_t EndOffset,
//     float Order = SORT_ORDER_DEBUG_OVERLAY
// ) {
//     PushVideo(Group, VideoID, Rect, SecondsElapsed, Order);
//     game_video* Video = GetAsset(group->Assets, VideoID);
//     auto& VideoContext = Video->VideoContext;
//     auto& FormatContext = VideoContext.FormatContext;
//     auto& CodecContext = VideoContext.CodecContext;
//     auto& StreamIndex = VideoContext.VideoStreamIndex;
//     auto& PTS = VideoContext.PTS; // Presentation time-stamp (in time-base units)

//     if (PTS >= EndOffset) {
//         av_seek_frame(FormatContext, StreamIndex, StartOffset, AVSEEK_FLAG_BACKWARD);
//         do { LoadFrame(&Video->VideoContext); } while (Video->VideoContext.PTS < StartOffset - 1000);
//         Video->TimeElapsed = Video->VideoContext.PTS * Video->VideoContext.TimeBase;
//     }
// }

// void PushSlider(Render_Group* group, ui_slider Slider, v2 Position, color Color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
//     Assert(Slider.MinValue != Slider.MaxValue);
//     double Range = Slider.MaxValue - Slider.MinValue;
//     double CircleCenter = Position.Y + 60.0 * (Slider.MaxValue - Slider.Value) / Range;
//     double Radius = 6.0;
//     double UpperLineFinish = 0.0;

//     double Percentage = 0;
//     if (Slider.MaxValue == 0.0) Percentage = 1.0 - Slider.Value / Slider.MinValue;
//     else Percentage = (Slider.Value - Slider.MinValue) / Range;

//     if (Percentage < 0.85) {
//         UpperLineFinish = (0.85 - (Slider.Value - Slider.MinValue) / Range) * 60;
//     }
//     double LowerLineStart = 60.0;
//     if (Slider.Value > Slider.MinValue + 0.15 * Range) {
//         LowerLineStart = (1.15 - Slider.Value / Range) * 60;
//     }
//     PushLine(Group, Position, Position + V2(0.0, UpperLineFinish), Color, 2.0, Order);
//     PushCircunference(Group, V2(Position.X, CircleCenter), Radius, Color, 2.0, Order);
//     PushLine(Group, Position + V2(0.0, LowerLineStart), Position + V2(0.0, 60.0), Color, 2.0, Order);
// }

void _PushMesh(
    Render_Group* group,
    Mesh* Mesh,
    render_mesh_options Options
) {
    
    render_command Command = {};
    Command.Type = render_mesh;
    Command.Priority = Options.outline ? SORT_ORDER_OUTLINED_MESHES : SORT_ORDER_MESHES;
    Command.Index = group->nMeshCommands++;
    PushCommand(Group, Command);

    render_mesh_command* MeshCommand = &group->MeshCommands[Command.Index];
    MeshCommand->Mesh = Mesh;
    MeshCommand->Options = Options;
    MeshCommand->Options.outline = false;

    // Outlines
    if (Options.outline) {
        render_command OutlineCommand = {};
        OutlineCommand.Type = render_mesh;
        OutlineCommand.Priority = SORT_ORDER_CLEAR + 1.0f;
        OutlineCommand.Index = group->nMeshCommands++;
        PushCommand(Group, OutlineCommand);

        render_mesh_command* OutlineMeshCommand = &group->MeshCommands[OutlineCommand.Index];
        OutlineMeshCommand->Mesh = Mesh;
        OutlineMeshCommand->Options.transform = Options.transform;
        OutlineMeshCommand->Options.color = Color::WHITE;
        OutlineMeshCommand->Options.outline = true;

        if (!group->PushOutline) {
            PushRenderTarget(Group, Target_Outline, Target_Postprocessing_Outline, SORT_ORDER_CLEAR + 1.0f);
            PushOutlineInitCompute(Group, Target_Postprocessing_Outline, SORT_ORDER_CLEAR + 2.0f);

            int Shifts = 12;
            int Level = 1 << Shifts;
            float JumpOrder = SORT_ORDER_CLEAR + 3.0f;

            // Compute version
            for (int i = 0; i <= Shifts; i++) {
                JumpOrder += 1.0f;
                PushJumpFloodCompute(Group, Level, JumpOrder);
                Level >>= 1;
            }
            PushOutlineCompute(Group, Color::WHITE, 4.0f, JumpOrder + 1.0f);

            PushRenderTarget(Group, Target_Postprocessing_Outline, Target_World, SORT_ORDER_OUTLINED_MESHES - 0.1f);
            group->PushOutline = true;
        }
    }

    // Debug bones rendering
    if (group->Debug && group->DebugBones && Options.armature) {
        v3* Vertices = (v3*)PushPrimitiveCommand(
            Group, 
            render_primitive_line,
            vertex_layout_v3_id,
            2 * Options.armature->nBones,
            .color = Color::BLACK,
            .Flags = render_flag::debug_bones,
            .Order = SORT_ORDER_DEBUG_OVERLAY,
            .Thickness = 2.5f
        )->Vertices;

        for (int i = 0; i < Options.armature->nBones; i++) {
            bone Bone = Options.armature->Bones[i];
            transform BoneTransform = Bone.Transform;
            v3 Head = BoneTransform * Options.transform * Bone.Segment.Head;
            v3 Tail = BoneTransform * Options.transform * Bone.Segment.Tail;
            Vertices[2*i] = Head;
            Vertices[2*i+1] = Tail;
        }
    }
}

#define PushMesh(Group, Mesh, ...) _PushMesh(Group, Mesh, { __VA_ARGS__ })

void PushHeightmap(
    Render_Group* group, 
    Texture* heightmap,
    v3 LeftBottom,
    scale S,
    float Order = SORT_ORDER_MESHES
) {
    int nVertices = HEIGHTMAP_RESOLUTION*HEIGHTMAP_RESOLUTION;
    int nElements = 4*(HEIGHTMAP_RESOLUTION-1)*(HEIGHTMAP_RESOLUTION-1);

    PushPrimitiveCommand(
        Group, 
        render_primitive_patches,
        vertex_layout_v3_v2_id, 
        nVertices,
        .Flags = render_flag::depth_test,
        .nElements = nElements,
        .Order = Order,
        .texture = heightmap,
        .Transform = GetTransform(LeftBottom, Quaternion(1.0f), S),
    );
}

void PushWater(Render_Group* group, v3 Position, scale S) {
    int nVertices = HEIGHTMAP_RESOLUTION*HEIGHTMAP_RESOLUTION;
    int nElements = 4*(HEIGHTMAP_RESOLUTION-1)*(HEIGHTMAP_RESOLUTION-1);

    PushPrimitiveCommand(
        Group,
        render_primitive_patches,
        vertex_layout_v3_v2_id, 
        nVertices,
        .Flags = render_flag::depth_test | render_flag::water,
        .nElements = nElements,
        .Order = SORT_ORDER_MESHES,
        .Transform = GetTransform(Position, Quaternion(1.0f), S),
    );
}

void GenerateHeightmapVertices(float* Vertices) {
    float L = 1.0f / (float)HEIGHTMAP_RESOLUTION;
    for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
    for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
        *Vertices++ = i * L;
        *Vertices++ = j * L;
    }}
}

void GenerateHeightmapElements(uint32* Elements) {
    for (int i = 0; i < HEIGHTMAP_RESOLUTION - 1; i++) {
    for (int j = 0; j < HEIGHTMAP_RESOLUTION - 1; j++) {
        *Elements++ = j       + HEIGHTMAP_RESOLUTION * i;
        *Elements++ = j       + HEIGHTMAP_RESOLUTION * (i + 1);
        *Elements++ = (j + 1) + HEIGHTMAP_RESOLUTION * i;
        *Elements++ = (j + 1) + HEIGHTMAP_RESOLUTION * (i + 1);
    }}
}

void PushSky(Render_Group* group) {
    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v3_id,
        8,
        .Flags = render_flag::sky | render_flag::depth_test,
        .nElements = 36,
        .Order = SORT_ORDER_MESHES
    );

    v3* Vertices = (v3*)Command->Vertices;
    *Vertices++ = V3(-1.0f, -1.0f, -1.0f);
    *Vertices++ = V3( 1.0f, -1.0f, -1.0f);
    *Vertices++ = V3(-1.0f, -1.0f,  1.0f);
    *Vertices++ = V3( 1.0f, -1.0f,  1.0f);
    *Vertices++ = V3(-1.0f,  1.0f, -1.0f);
    *Vertices++ = V3( 1.0f,  1.0f, -1.0f);
    *Vertices++ = V3(-1.0f,  1.0f,  1.0f);
    *Vertices++ = V3( 1.0f,  1.0f,  1.0f);

    uint32* Elements = Command->ElementEntry.Pointer;
    uint32 Offset = Command->VertexEntry.Offset;
    *Elements++ = Offset + 0; *Elements++ = Offset + 1; *Elements++ = Offset + 2;
    *Elements++ = Offset + 1; *Elements++ = Offset + 2; *Elements++ = Offset + 3;
    *Elements++ = Offset + 0; *Elements++ = Offset + 1; *Elements++ = Offset + 4;
    *Elements++ = Offset + 1; *Elements++ = Offset + 4; *Elements++ = Offset + 5;
    *Elements++ = Offset + 0; *Elements++ = Offset + 2; *Elements++ = Offset + 4;
    *Elements++ = Offset + 2; *Elements++ = Offset + 4; *Elements++ = Offset + 6;
    *Elements++ = Offset + 1; *Elements++ = Offset + 3; *Elements++ = Offset + 5;
    *Elements++ = Offset + 3; *Elements++ = Offset + 5; *Elements++ = Offset + 7;
    *Elements++ = Offset + 4; *Elements++ = Offset + 5; *Elements++ = Offset + 6;
    *Elements++ = Offset + 5; *Elements++ = Offset + 6; *Elements++ = Offset + 7;
    *Elements++ = Offset + 2; *Elements++ = Offset + 6; *Elements++ = Offset + 7;
    *Elements++ = Offset + 2; *Elements++ = Offset + 7; *Elements++ = Offset + 3;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushCollider(Render_Group* group, collider Collider, transform Transform, Color color) {
    v3 Position = Transform.Translation + Collider.Offset;
    switch (Collider.Type) {
        case Rect_Collider: {
            PushRectOutline(Group, Rectangle(Collider), color);
        } break;

        case Cube_Collider: {
            PushCubeOutline(Group, Position, GetScale(Collider.Cube.HalfWidth,Collider.Cube.HalfHeight,Collider.Cube.HalfDepth), color);
        } break;

        case Sphere_Collider: {
            PushCircunference(Group, Position, V3(1,0,0), Collider.Sphere.Radius, color);
            PushCircunference(Group, Position, V3(0,1,0), Collider.Sphere.Radius, color);
            PushCircunference(Group, Position, V3(0,0,1), Collider.Sphere.Radius, color);
        } break;

        case Capsule_Collider: {
            v3 Head = Transform * Collider.Capsule.Segment.Head;
            v3 Tail = Transform * Collider.Capsule.Segment.Tail;

            segment3 TransformedSegment = { Head, Tail };
            transform ST = SegmentTransform(TransformedSegment);
            Basis basis = ST * Identity3;
            v3 D = normalize(Tail - Head);

            // Top part
            PushArc(Group, Tail, Basis, Collider.Capsule.Distance, 180, color);
            v3 Temp = Basis.X;
            Basis.X = Basis.Z;
            Basis.Z = Temp;
            PushArc(Group, Tail, Basis, Collider.Capsule.Distance, 180, color);
            PushCircunference(Group, Tail, D, Collider.Capsule.Distance, color);

            // Vertical lines
            v3 Offset[4] = { Basis.X, -Basis.X, Basis.Z, -Basis.Z };
            for (int i = 0; i < 4; i++) {
                PushLine(
                    Group, 
                    Tail + Collider.Capsule.Distance * Offset[i], 
                    Head + Collider.Capsule.Distance * Offset[i],
                    color
                );
            }

            // Bottom part
            PushCircunference(Group, Head, D, Collider.Capsule.Distance, color);
            Basis = ST * Identity3;
            Basis.X = -Basis.X;
            Basis.Y = -Basis.Y;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, color);
            Temp = Basis.X;
            Basis.X = Basis.Z;
            Basis.Z = Temp;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, color);

            Collider.Capsule.Segment = Transform * Collider.Capsule.Segment;
        } break;

        default: {
            Raise("Invalid collider type");
        }
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Debug                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const float DEBUG_ENTRIES_TEXT_POINTS = 10.0f;

void PushDebugVector(Render_Group* group, v2 Vector, v2 Position, Color color) {
    v2 Orthogonal = perp(normalize(V2(Vector.X, Vector.Y)));
    float OrthogonalLength = 0.005333f * group->Height;

    int Thickness = fmax(1.0f, 0.0025f * group->Height);
    PushLine(Group, Position, Position + 0.875 * Vector, color, Thickness, SORT_ORDER_DEBUG_OVERLAY);
    triangle2 Arrowhead1 = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector - OrthogonalLength * Orthogonal
    };
    PushTriangle(
        Group,
        Arrowhead1,
        color, 
        SORT_ORDER_DEBUG_OVERLAY
    );
    triangle2 Arrowhead2 = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector + OrthogonalLength * Orthogonal,
    };
    PushTriangle(
        Group,
        Arrowhead2,
        color, 
        SORT_ORDER_DEBUG_OVERLAY
    );
}

void PushDebugVector(Render_Group* group, basis CameraBasis, v3 Vector, v3 Position, Color color) {
    v2 CameraCoordinates = perp(V2(dot(Vector, CameraBasis.X), dot(Vector, CameraBasis.Y)));
    v3 Orthogonal = normalize(CameraCoordinates.X * CameraBasis.X + CameraCoordinates.Y * CameraBasis.Y);
    float OrthogonalLength = (modulus(Vector) / 15.0f);

    int Thickness = fmax(1.0f, 0.0025f *  group->Height);
    PushLine(Group, Position, Position + 0.875f * Vector, color, Thickness, SORT_ORDER_MESHES);
    
    // Arrow head
    triangle3 Triangle = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector - OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, color, SORT_ORDER_MESHES);
    Triangle = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector + OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, color, SORT_ORDER_MESHES);
}

void PushDebugFustrum(
    Render_Group* group,
    basis CameraBasis,
    v3 Position,
    double l, double r, double b, double t, double n, double f
) {
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        vertex_layout_v3_v2_id,
        9,
        .Flags = render_flag::depth_test,
        .nElements = 24,
        .Order = SORT_ORDER_DEBUG_OVERLAY,
    );

    v3 nv = -n * CameraBasis.Z;
    v3 rv = r * CameraBasis.X;
    v3 lv = -l * CameraBasis.X;
    v3 tv = t * CameraBasis.Y * ((double)group->Height / (double)group->Width);
    v3 bv = -b * CameraBasis.Y * ((double)group->Height / (double)group->Width);
    v3 fv = -f * CameraBasis.Z;

    v3 l_ = f * lv;
    v3 r_ = f * rv;
    v3 t_ = f * tv;
    v3 b_ = f * bv;

    v3* Vertices = (v3*)Result->Vertices;
    Vertices[0] = Position;
    Vertices[1] = Position + l_ + t_ + fv;
    Vertices[2] = Position + r_ + t_ + fv;
    Vertices[3] = Position + l_ + b_ + fv;
    Vertices[4] = Position + r_ + b_ + fv;
    Vertices[5] = Position + rv + bv + nv;
    Vertices[6] = Position + rv + tv + nv;
    Vertices[7] = Position + lv + bv + nv;
    Vertices[8] = Position + lv + tv + nv;

    uint32* Elements = Result->ElementEntry.Pointer;
    *Elements++ = 0; *Elements++ = 1;
    *Elements++ = 0; *Elements++ = 2;
    *Elements++ = 0; *Elements++ = 3;
    *Elements++ = 0; *Elements++ = 4;
    *Elements++ = 4; *Elements++ = 3;
    *Elements++ = 2; *Elements++ = 1;
    *Elements++ = 4; *Elements++ = 2;
    *Elements++ = 3; *Elements++ = 1;
    *Elements++ = 5; *Elements++ = 7;
    *Elements++ = 6; *Elements++ = 8;
    *Elements++ = 5; *Elements++ = 6;
    *Elements++ = 7; *Elements++ = 8;
}

void PushDebugGrid(Render_Group* group, float Alpha) {
    const int nVertices = 404;

    v3* Vertices = (v3*)PushPrimitiveCommand(
        Group,
        render_primitive_line,
        vertex_layout_v3_id,
        nVertices,
        .color = ChangeAlpha(Color::WHITE, 0.2f),
        .Flags = render_flag::depth_test | render_flag::overwrite_alpha,
        .Order = SORT_ORDER_MESHES,
        .Thickness = 1.0f
    )->Vertices;

    for (int i = 0; i <= 100; i++) {
        Vertices[4*i  ] = V3(50-i, 0, -50);
        Vertices[4*i+1] = V3(50-i, 0, 50);
        Vertices[4*i+2] = V3(-50, 0, 50-i);
        Vertices[4*i+3] = V3(50, 0, 50-i);
    }
}

void PushDebugTarget(Render_Group* group, Render_Group_Target Target, bool Attachment = false) {
    render_command Command;
    Command.Index = group->nTargets;
    Command.Priority = SORT_ORDER_PUSH_RENDER_TARGETS + 0.1f;
    Command.Type = render_target;

    PushCommand(Group, Command);

    render_target_command TargetCommand = {};
    TargetCommand.Source = Target;
    TargetCommand.Target = Target_Output;
    TargetCommand.DebugAttachment = Attachment;
    TargetCommand.Attachment = false;
    TargetCommand.VertexEntry = PushVertexEntry(&group->VertexBuffer, 6, vertex_layout_v2_v2_id);

    float* Vertices = (float*)TargetCommand.VertexEntry.Pointer;
    *Vertices++ = -1.0f; *Vertices++ = -1.0f; *Vertices++ = 0.0f; *Vertices++ = 0.0f,
    *Vertices++ = -0.5f; *Vertices++ = -1.0f; *Vertices++ = 1.0f; *Vertices++ = 0.0f;
    *Vertices++ = -0.5f; *Vertices++ = -0.5f; *Vertices++ = 1.0f; *Vertices++ = 1.0f;
    *Vertices++ = -1.0f; *Vertices++ = -1.0f; *Vertices++ = 0.0f; *Vertices++ = 0.0f;
    *Vertices++ = -0.5f; *Vertices++ = -0.5f; *Vertices++ = 1.0f; *Vertices++ = 1.0f;
    *Vertices++ = -1.0f; *Vertices++ = -0.5f; *Vertices++ = 0.0f; *Vertices++ = 1.0f;

    group->TargetCommands[group->nTargets++] = TargetCommand;
}

void PushDebugPlot(
    Render_Group* group,
    int N,
    float* Data,
    v2 Position,
    int dx,
    Color color = Color::WHITE,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        vertex_layout_v2_id,
        N,
        .color = color,
        .Order = Order,
        .Thickness = Thickness
    )->Vertices;

    float X = 0;
    for (int i = 0; i < N; i++) {
        Vertices[i] = Position + V2(X, -Data[i]);
        X += dx;
    }
}

inline uint16 ComputeTimeRecordsColWidths(
    Arena* Arena,
    Font* Font, float Points,
    uint16 nRecords, time_record* TimeRecords,
    float* FunctionColWidth,
    float* HitsColWidth,
    float* MCyclesColWidth,
    float* FileColWidth
) {
    uint16 TotalRecords = 0;
    string Text;
    float Width = 0;
    for (int i = 0; i < nRecords; i++) {
        time_record* Record = TimeRecords + i;
        if (Record->HitCount > 0) {
            TotalRecords += 1;
            Width = GetTextWidth(Record->FunctionName, Font, Points);
            if (Width > *FunctionColWidth) *FunctionColWidth = Width;
            Text = Format(Arena, "{i}", 1, Record->HitCount);
            Width = GetTextWidth(Text, Font, Points);
            if (Width > *HitsColWidth) *HitsColWidth = Width;
            Text = Format(Arena, "{f2}", 1, Record->CycleCount / 1000000.0f);
            Width = GetTextWidth(Text, Font, Points);
            if (Width > *MCyclesColWidth) *MCyclesColWidth = Width;
            Text = Format(Arena, "{s}:{i}", 2, Record->FileName, Record->LineNumber);
            Width = GetTextWidth(Text, Font, Points);
            if (Width > *FileColWidth) *FileColWidth = Width;
        }
    }
    return TotalRecords;
}

void PushTimeRecordsPartial(
    Render_Group* group,
    time_record* TimeRecords,
    uint16 nTimeRecords,
    float* X, float* Y,
    float TotalWidth, float RecordHeight,
    float FunctionColWidth,
    float HitsColWidth,
    float MCyclesColWidth,
    float FileColWidth,
    float HMargin, float VMargin
) {
    string Text;
    float Width = 0;
    for (int i = 0; i < nTimeRecords; i++) {
        time_record* Record = TimeRecords + i;

        if (Record->HitCount > 0) {
            PushText(Group, V2(*X, *Y), Record->FunctionName, .Font = group->DebugFont, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X += FunctionColWidth + HMargin;

            Text = Format(group->Transient, "{i}", 1, Record->HitCount);
            Width = GetTextWidth(Text, group->DebugFont, DEBUG_ENTRIES_TEXT_POINTS);
            *X += HitsColWidth - Width;
            PushText(Group, V2(*X, *Y), Text, .Font = group->DebugFont, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X += Width + HMargin;
            
            Text = Format(group->Transient, "{f2}", 1, Record->CycleCount / 1000000.0f);
            Width = GetTextWidth(Text, group->DebugFont, DEBUG_ENTRIES_TEXT_POINTS);
            *X += MCyclesColWidth - Width;
            PushText(Group, V2(*X, *Y), Text, .Font = group->DebugFont, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X += Width + HMargin;

            Text = Format(group->Transient, "{s}:{i}", 2, Record->FileName, Record->LineNumber);
            PushText(Group, V2(*X, *Y), Text, .Font = group->DebugFont, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X = group->Width - TotalWidth + HMargin;

            *Y += RecordHeight;

            Record->HitCount = 0;
            Record->CycleCount = 0;
        }
    }
    *X = group->Width - TotalWidth + HMargin;
}

void PushTimeRecords(
    Render_Group* group, 
    uint16 nTimeRecordsLibrary,
    time_record* TimeRecordsLibrary,
    uint16 nTimeRecordsPlatform,
    time_record* TimeRecordsPlatform
) {
    char Buffer[512];
    const float Points = DEBUG_ENTRIES_TEXT_POINTS;
    Font* Font = group->DebugFont;
    
    float FunctionHeaderWidth = GetTextWidth("Function", Font, Points);
    float FunctionColWidth    = FunctionHeaderWidth;
    float HitsHeaderWidth     = GetTextWidth("Hits", Font, Points);
    float HitsColWidth        = HitsHeaderWidth;
    float MCyclesHeaderWidth  = GetTextWidth("MCycles", Font, Points);
    float MCyclesColWidth     = MCyclesHeaderWidth;
    float FileHeaderWidth     = GetTextWidth("File", Font, Points);
    float FileColWidth        = FileHeaderWidth;

    // Compute column widths
    uint16 nTotalRecords = 0;
    nTotalRecords += ComputeTimeRecordsColWidths(group->Transient, Font, Points, nTimeRecordsLibrary, TimeRecordsLibrary, 
        &FunctionColWidth, &HitsColWidth, &MCyclesColWidth, &FileColWidth);
    nTotalRecords += ComputeTimeRecordsColWidths(group->Transient, Font, Points, nTimeRecordsPlatform, TimeRecordsPlatform, 
        &FunctionColWidth, &HitsColWidth, &MCyclesColWidth, &FileColWidth);

    float HMargin = 10.0f;
    float VMargin = 10.0f;

    float TotalWidth = FunctionColWidth + HitsColWidth + MCyclesColWidth + FileColWidth + 5 * HMargin;
    float RecordHeight = GetCharMaxHeight(Font, Points);
    float TotalHeight = (RecordHeight) * (nTotalRecords + 1) + 2 * VMargin;

    PushRect(Group, { group->Width - TotalWidth, group->Height - TotalHeight, TotalWidth, TotalHeight }, ChangeAlpha(Color::BLACK, 0.7f));

    float RecordX = group->Width - TotalWidth + HMargin;
    float RecordY = group->Height - TotalHeight + RecordHeight + 0.5f * VMargin;

    // Push header
    float X = RecordX + 0.5f * (FunctionColWidth - FunctionHeaderWidth);
    PushText(Group, V2(X, RecordY), "Function", .Font = group->DebugFont, .Points = Points);
    RecordX += FunctionColWidth + HMargin;

    X = RecordX + 0.5f * (HitsColWidth - HitsHeaderWidth);
    PushText(Group, V2(X, RecordY), "Hits", .Font = group->DebugFont, .Points = Points);
    RecordX += HitsColWidth + HMargin;

    X = RecordX + 0.5f * (MCyclesColWidth - MCyclesHeaderWidth);
    PushText(Group, V2(X, RecordY), "MCycles", .Font = group->DebugFont, .Points = Points);
    RecordX += MCyclesColWidth + HMargin;

    X = RecordX + 0.5f * (FileColWidth - FileHeaderWidth);
    PushText(Group, V2(X, RecordY), "File", .Font = group->DebugFont, .Points = Points);

    RecordX = group->Width - TotalWidth + HMargin;
    RecordY += RecordHeight + 0.5f * VMargin;

    // Push table
    PushTimeRecordsPartial(Group, TimeRecordsLibrary, nTimeRecordsLibrary, &RecordX, &RecordY, TotalWidth, RecordHeight, 
        FunctionColWidth, HitsColWidth, MCyclesColWidth, RecordHeight, HMargin, VMargin);
    PushTimeRecordsPartial(Group, TimeRecordsPlatform, nTimeRecordsLibrary, &RecordX, &RecordY, TotalWidth, RecordHeight, 
        FunctionColWidth, HitsColWidth, MCyclesColWidth, RecordHeight, HMargin, VMargin);
}

#endif