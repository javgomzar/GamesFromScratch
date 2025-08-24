#ifndef RENDER_GROUP
#define RENDER_GROUP

#pragma once
#include "GameAssets.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Vertex buffer                                                                                                                                |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const memory_index VERTEX_BUFFER_SIZE = Kilobytes(64);
const memory_index ELEMENT_BUFFER_SIZE = Kilobytes(8);

struct vertex_buffer_entry {
    uint32 Offset;
    uint32 Count;
    vertex_layout_id LayoutID;
    void* Pointer;
};

struct element_buffer_entry {
    memory_index Offset;
    uint32 Count;
    uint32* Pointer;
};

struct vertex_buffer {
    vertex_layout Layouts[vertex_layout_id_count];
    memory_arena Vertices[vertex_layout_id_count];
    memory_arena Elements;
    uint32 VertexCount[vertex_layout_id_count];
    uint32 ElementCount;
};

/* Initializes several vertex buffers and element buffers. Returns total memory used.*/
inline memory_index InitializeVertexBuffer(
    memory_arena* Arena,
    game_assets* Assets, 
    vertex_buffer* Buffer
) {
    memory_index TotalSize = 0;
    memory_index Size = 0;

    // Vertex buffers (one per layout)
    for (int i = 0; i < vertex_layout_id_count; i++) {
        Buffer->Layouts[i] = Assets->VertexLayouts[i];
        Buffer->VertexCount[i] = 0;
        Size = VERTEX_BUFFER_SIZE;
        Buffer->Vertices[i] = SuballocateMemoryArena(Arena, Size);

        TotalSize += Size;
    }

    // Element buffers
    Buffer->ElementCount = 0;
    Size = ELEMENT_BUFFER_SIZE;
    Buffer->Elements = SuballocateMemoryArena(Arena, Size);
    TotalSize += Size;

    return TotalSize;
}

vertex_buffer_entry PushVertexEntry(vertex_buffer* VertexBuffer, uint64 VertexCount, vertex_layout_id VertexLayoutID) {
    memory_arena* Arena = &VertexBuffer->Vertices[VertexLayoutID];
    
    vertex_buffer_entry Entry;
    Entry.Count = VertexCount;
    Entry.LayoutID = VertexLayoutID;
    Entry.Offset = VertexBuffer->VertexCount[VertexLayoutID];
    
    memory_index Size = VertexCount * VertexBuffer->Layouts[VertexLayoutID].Stride;
    void* Destination = PushSize(Arena, Size);
    Entry.Pointer = Destination;

    VertexBuffer->VertexCount[VertexLayoutID] += VertexCount;
    return Entry;
}

element_buffer_entry PushElementEntry(vertex_buffer* VertexBuffer, uint64 ElementCount) {
    element_buffer_entry Entry;
    Entry.Count = ElementCount;
    Entry.Offset = VertexBuffer->ElementCount;

    memory_index Size = ElementCount * sizeof(uint32);
    void* Destination = PushSize(&VertexBuffer->Elements, Size);
    Entry.Pointer = (uint32*)Destination;

    VertexBuffer->ElementCount += ElementCount;
    return Entry;
}

void ClearVertexBuffer(vertex_buffer* Buffer) {
    for (int i = 0; i < vertex_layout_id_count; i++) {
        ClearArena(&Buffer->Vertices[i]);
        Buffer->VertexCount[i] = 0;
    }
    ClearArena(&Buffer->Elements);
    Buffer->ElementCount = 0;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Camera                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct camera {
    basis Basis;
    void* Entity;
    v3 Position;
    float Distance;
    float Pitch;
    float Angle;
    matrix4 View;
    bool OnAir;
};

basis GetCameraBasis(float Angle, float Pitch) {
    float cosA = cosf(Angle * Degrees);
    float sinA = sinf(Angle * Degrees);
    float cosP = cosf(Pitch * Degrees);
    float sinP = sinf(Pitch * Degrees);

    v3 X = V3(        cosA,  0.0,         sinA);
    v3 Y = V3(-sinA * sinP, cosP,  cosA * sinP);
    v3 Z = V3( sinA * cosP, sinP, -cosA * cosP);

    basis Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    return Result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Render entries                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_RENDER_ENTRIES = 16384;

enum render_command_type {
    render_clear,
    render_draw_primitive,
    render_shader_pass,
    render_compute_shader_pass,
    render_target,

    render_command_type_count
};

struct render_command {
    render_command_type Type;
    float Priority;
    uint32 Index;
};

struct render_clear_command {
    color Color;
};

enum render_primitive {
    render_primitive_point,
    render_primitive_line,
    render_primitive_line_strip,
    render_primitive_line_loop,
    render_primitive_triangle,
    render_primitive_triangle_fan,
    render_primitive_patches,

    render_primitive_count
};

typedef uint64 render_flags;
enum {
    DEPTH_TEST_RENDER_FLAG   = 1 << 0,
    STENCIL_TEST_RENDER_FLAG = 1 << 1,
};

struct render_primitive_options {
    render_flags Flags = 0;
    float Thickness = 2.0f;
    transform Transform = IdentityTransform;
    game_bitmap* Texture = NULL;
    game_mesh* Mesh = NULL;
    game_font* Font = NULL;
    armature* Armature = NULL;
    v2 Pen;
    int PatchParameter = 4;
    float TextSize = 0;
    bool Outline = false;
};

struct render_primitive_command {
    render_primitive Primitive = render_primitive_point;
    render_primitive_options Options;
    color Color;
    vertex_buffer_entry VertexEntry = {0};
    float* Vertices = 0;
    element_buffer_entry ElementEntry = {0};
    game_shader_pipeline* Shader = 0;
};

enum render_group_target {
    Target_None,
    Target_World,
    Target_Outline,
    Target_Postprocessing_Outline,
    Target_PingPong,
    Target_Output,

    render_group_target_count
};

struct render_shader_pass_command {
    vertex_buffer_entry VertexEntry;
    game_shader_pipeline* Shader;
    render_group_target Target;
    color Color;
    int Level;
    float Width;
};

struct render_compute_shader_pass_command {
    game_compute_shader* Shader;
    render_group_target Source;
    render_group_target Target;
    matrix3 Kernel;
};

struct render_target_command {
    vertex_buffer_entry VertexEntry;
    game_shader_pipeline* Shader;
    render_group_target Source;
    render_group_target Target;
    bool DebugAttachment;
};

enum wrap_mode {
    Wrap_Clamp,
    Wrap_Repeat,
    Wrap_Crop
};

struct light {
    float Ambient;
    float Diffuse;
    v3 Direction;
    color Color;
};

light Light(v3 Direction, color Color = White, float Ambient = 0.5f, float Diffuse = 0.5f) {
    return { Ambient, Diffuse, normalize(Direction), Color };
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Render group                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_FRAMEBUFFER_COUNT = 8;
const int MAX_PRIMITIVE_COMMANDS = 2048;
const int MAX_MESH_COMMANDS = 64;
const int MAX_HEIGHTMAP_COMMANDS = 8;
const int MAX_SHADER_PASS_COMMANDS = 32;
const int MAX_COMPUTE_SHADER_PASS_COMMANDS = 32;
const int MAX_RENDER_TARGET_COMMANDS = 16;

struct render_group {
    render_command Entries[MAX_RENDER_ENTRIES];
    render_clear_command Clears[render_group_target_count];
    render_primitive_command PrimitiveCommands[MAX_PRIMITIVE_COMMANDS];
    render_shader_pass_command ShaderPassCommands[MAX_SHADER_PASS_COMMANDS];
    render_compute_shader_pass_command ComputeShaderPassCommands[MAX_COMPUTE_SHADER_PASS_COMMANDS];
    render_target_command TargetCommands[MAX_RENDER_TARGET_COMMANDS];
    vertex_buffer VertexBuffer;
    light Light;
    camera* Camera;
    game_assets* Assets;
    int32 Width;
    int32 Height;
    uint32 EntryCount;
    uint32 nPrimitiveCommands;
    uint32 nShaderPassCommands;
    uint32 nComputeShaderPassCommands;
    uint32 nTargets;
    bool Debug;
    bool DebugNormals;
    bool DebugBones;
    bool DebugColliders;
    bool PushOutline;
};

void InitializeRenderGroup(
    memory_arena* Arena,
    render_group* Group, 
    game_assets* Assets
) {
    Group->Assets = Assets;

    // Lighting
    Group->Light = Light(V3(-0.5, -1, 1), White);

    // Vertex & element buffers
    InitializeVertexBuffer(Arena, Assets, &Group->VertexBuffer);
}

// Render entries sorting
float SORT_ORDER_CLEAR = 0.0f;
float SORT_ORDER_MESHES = 100.0f;
float SORT_ORDER_OUTLINED_MESHES = 150.0f;
float SORT_ORDER_DEBUG_OVERLAY = 200.0f;
float SORT_ORDER_SHADER_PASSES = 8000.0f;
float SORT_ORDER_PUSH_RENDER_TARGETS = 9000.0f;

void Swap(render_group* Group, int i, int j) {
    render_command Entry = Group->Entries[i];
    Group->Entries[i] = Group->Entries[j];
    Group->Entries[j] = Entry;
}

void PushCommand(render_group* Group, render_command Command) {
    // Size check
    switch(Command.Type) {
        case render_clear: {
            if (Command.Index >= render_group_target_count) {
                Raise("Invalid target for clearing");
            }
        } break;
        case render_draw_primitive: {
            if (Command.Index >= MAX_PRIMITIVE_COMMANDS) {
                Raise("Primitive draw command overflow.");
            }
        } break;
        case render_shader_pass: {
            if (Command.Index >= MAX_SHADER_PASS_COMMANDS) {
                Raise("Shader pass command overflow.");
            }
        } break;
        case render_compute_shader_pass: {
            if (Command.Index >= MAX_COMPUTE_SHADER_PASS_COMMANDS) {
                Raise("Compute shader pass command overflow.");
            }
        } break;
        case render_target: {
            if (Command.Index >= MAX_RENDER_TARGET_COMMANDS) {
                Raise("Render target command overflow.");
            }
        };
    }

    // Starting from the end, put it in order
    uint32 i = Group->EntryCount;
    Group->Entries[i] = Command;
    if (Group->EntryCount > 0) {
        uint32 j = i - 1;
        while(i > 0) {
            render_command Check = Group->Entries[j];
            if (Check.Priority > Command.Priority) {
                Swap(Group, i, j);
                i = j;
                j = i - 1;
            }
            else {
                break;
            }
        }
    }
    Group->EntryCount++;
}

void ClearEntries(render_group* Group) {
    ZeroSize(Group->EntryCount * sizeof(render_command), Group->Entries);
    ZeroSize(render_group_target_count * sizeof(render_clear_command), Group->Clears);
    ZeroSize(Group->nPrimitiveCommands * sizeof(render_primitive_command), Group->PrimitiveCommands);
    ZeroSize(Group->nShaderPassCommands * sizeof(render_shader_pass_command), Group->ShaderPassCommands);
    ZeroSize(Group->nComputeShaderPassCommands * sizeof(render_compute_shader_pass_command), Group->ComputeShaderPassCommands);
    ZeroSize(Group->nTargets * sizeof(render_target_command), Group->TargetCommands);
    Group->nPrimitiveCommands = 0;
    Group->nShaderPassCommands = 0;
    Group->nComputeShaderPassCommands = 0;
    Group->nTargets = 0;
    Group->EntryCount = 0;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Push methods                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushClear(render_group* Group, color Color, render_group_target Target = Target_Output) {
    render_command Command;
    Command.Index = Target;
    Command.Priority = 0.0f;
    Command.Type = render_clear;
    PushCommand(Group, Command);

    render_clear_command Clear;
    Clear.Color = Color;
    Group->Clears[Target] = Clear;
}

/*
    Pushes a primitive rendering command to the queue and returns a `float` pointer to which to write vertex data.
    If `nElements > 0`, it will also return a `uint32` pointer to which to write element data. In this case, the `VertexOffset`
    unsigned integer should be added to the element data. This return member will be valid even if `nElements == 0`.
*/
render_primitive_command* PushPrimitiveCommand(
    render_group* Group,
    render_primitive Primitive,
    color Color,
    game_shader_pipeline* Shader,
    vertex_layout_id LayoutID,
    uint32 nVertices,
    uint32 nElements = 0,
    float Order = 0.0,
    render_primitive_options Options = {}
) {
    TIMED_BLOCK;
    render_command Command;
    Command.Index = Group->nPrimitiveCommands;
    Command.Priority = Order;
    Command.Type = render_draw_primitive;

    PushCommand(Group, Command);

    render_primitive_command* PrimitiveCommand = &Group->PrimitiveCommands[Group->nPrimitiveCommands++];
    *PrimitiveCommand = {};
    PrimitiveCommand->Options = Options;
    PrimitiveCommand->Primitive = Primitive;
    PrimitiveCommand->Shader = Shader;
    PrimitiveCommand->Color = Color;

    if (nVertices > 0) {
        if (Options.Mesh != NULL || Options.Font != NULL) {
            PrimitiveCommand->VertexEntry.Count = nVertices;
            PrimitiveCommand->VertexEntry.LayoutID = LayoutID;
        }
        else {
            PrimitiveCommand->VertexEntry = PushVertexEntry(&Group->VertexBuffer, nVertices, LayoutID);
            PrimitiveCommand->Vertices = (float*)PrimitiveCommand->VertexEntry.Pointer;
        }
    }

    if (nElements > 0) {
        if (Options.Mesh != NULL || Options.Font != NULL) {
            PrimitiveCommand->ElementEntry.Count = nElements;
        }
        else {
            PrimitiveCommand->ElementEntry = PushElementEntry(&Group->VertexBuffer, nElements);
        }
    }

    return PrimitiveCommand;
}

void PushPoint(render_group* Group, v2 Point, color Color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    float* Vertices = (float*)PushPrimitiveCommand(
        Group,
        render_primitive_point, 
        Color, 
        Shader, 
        vertex_layout_vec2_id, 
        1,
        0,
        Order
    )->VertexEntry.Pointer;
    Vertices[0] = Point.X;
    Vertices[1] = Point.Y;
}

void PushPoint(render_group* Group, v3 Point, color Color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    float* Vertices = PushPrimitiveCommand(
        Group, 
        render_primitive_point,
        Color,
        Shader, 
        vertex_layout_vec3_id, 
        1,
        0,
        Order
    )->Vertices;
    Vertices[0] = Point.X;
    Vertices[1] = Point.Y;
    Vertices[2] = Point.Z;
}

void PushLine(
    render_group* Group,
    v2 Start,
    v2 End,
    color Color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Thickness = Thickness;
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        Color,
        Shader,
        vertex_layout_vec2_id,
        2,
        0,
        Order,
        Options
    )->Vertices;
    Vertices[0] = Start.X;
    Vertices[1] = Start.Y;
    Vertices[2] = End.X;
    Vertices[3] = End.Y;
}

void PushLine(
    render_group* Group,
    v3 Start,
    v3 End,
    color Color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {    
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    Options.Thickness = Thickness;
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        Color,
        Shader,
        vertex_layout_vec3_id,
        2,
        0,
        Order
    )->Vertices;
    Vertices[0] = {Start.X};
    Vertices[1] = {Start.Y};
    Vertices[2] = {Start.Z};
    Vertices[3] = {End.X};
    Vertices[4] = {End.Y};
    Vertices[5] = {End.Z};
}

void PushRay(
    render_group* Group,
    ray Ray,
    color Color,
    float Thickness = 2.0f,
    float Length = 10.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    v3 Start = Ray.Point;
    v3 End = Start + Length * Ray.Direction;
    PushLine(Group, Start, End, Color, Thickness, Order);
}

void PushTriangle(
    render_group* Group,
    triangle3 Triangle,
    color Color,
    float Order = SORT_ORDER_MESHES
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    float* Vertices = PushPrimitiveCommand(
        Group, 
        render_primitive_triangle,
        Color,
        Shader,
        vertex_layout_vec3_id,
        3,
        0,
        Order,
        Options
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
    render_group* Group,
    triangle2 Triangle,
    color Color,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        Shader,
        vertex_layout_vec2_id,
        3,
        0,
        Order
    )->Vertices;

    Vertices[0] = Triangle.Points[0].X;
    Vertices[1] = Triangle.Points[0].Y;
    Vertices[2] = Triangle.Points[1].X;
    Vertices[3] = Triangle.Points[1].Y;
    Vertices[4] = Triangle.Points[2].X;
    Vertices[5] = Triangle.Points[2].Y;
}

void PushCircle(
    render_group* Group,
    v2 Center,
    float Radius,
    color Color,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 30
) {
    int MAX_N = 62;
    int N = Clamp(nVertices, 14, MAX_N);

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_triangle_fan,
        Color,
        Shader,
        vertex_layout_vec2_id,
        N+2,
        0,
        Order
    )->Vertices;

    v2* Vertices = (v2*)Data;

    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = Center;
    Vertices[1] = V2(Center.X, Center.Y - Radius);
    for (int i = 2; i < N+1; i++) {
        Vertices[i].X = Center.X + Radius * sin(Theta);
        Vertices[i].Y = Center.Y - Radius * cos(Theta);
        Theta += dTheta;
    }
    Vertices[N+1] = Vertices[1];
}

void PushCircle(
    render_group* Group, 
    v3 Center,
    float Radius, 
    v3 Normal,
    color Color,
    float Order = SORT_ORDER_MESHES,
    int nVertices = 32
) {
    int MAX_N = 62;
    int N = Clamp(nVertices, 14, MAX_N);
    basis Basis = Complete(Normal);

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    float* Data = (float*)PushPrimitiveCommand(
        Group,
        render_primitive_triangle_fan,
        Color,
        Shader,
        vertex_layout_vec3_id,
        N+2,
        0,
        Order,
        Options
    )->VertexEntry.Pointer;
    
    v3* Vertices = (v3*)Data;

    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = Center;
    Vertices[1] = Center - Radius * Basis.Y;
    for (int i = 2; i < N+1; i++) {
        Vertices[i] = Center + Radius * (sin(Theta) * Basis.X - cos(Theta) * Basis.Y);
        Theta += dTheta;
    }
    Vertices[N+1] = Vertices[1];
}

void PushCircunference(
    render_group* Group,
    v2 Center,
    float Radius,
    color Color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 32
) {
    int MAX_N = 64;
    int N = Clamp(nVertices, 16, MAX_N);

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Thickness = Thickness;
    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_loop,
        Color,
        Shader,
        vertex_layout_vec2_id,
        N,
        0,
        Order,
        Options
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
}

void PushCircunference(
    render_group* Group,
    v3 Center,
    v3 Normal,
    float Radius,
    color Color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 32
) {
    int MAX_N = 64;
    int N = Clamp(nVertices, 16, MAX_N);

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    Options.Thickness = Thickness;
    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_loop,
        Color,
        Shader,
        vertex_layout_vec3_id,
        N,
        0,
        Order,
        Options
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
}

/*
    Pushes an arc of circunference to the renderer. Basis will determine the plane in which the circunference is contained.
    Basis.Z will be the normal to this plane, and Basis.X will be the offset from the center where the arc will be started.
*/
void PushArc(
    render_group* Group,
    v3 Center,
    basis Basis,
    float Radius,
    double Angle,
    color Color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    int nVertices = 32
) {
    int MAX_N = 64;
    int N = Clamp(nVertices, 16, MAX_N);

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    Options.Thickness = Thickness;
    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        Color,
        Shader,
        vertex_layout_vec3_id,
        N,
        0,
        Order,
        Options
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
    render_group* Group,
    rectangle Rect,
    color Color,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        Shader,
        vertex_layout_vec2_id,
        4,
        6,
        Order
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
    render_group* Group,
    v3 LeftTop,
    v3 WidthAxis,
    v3 HeightAxis,
    float Width,
    float Height,
    color Color,
    float Order = SORT_ORDER_MESHES
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        Shader,
        vertex_layout_vec3_id,
        4,
        6,
        Order
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
    render_group* Group,
    rectangle Rect,
    color Color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Thickness = Thickness;
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        render_primitive_line_loop,
        Color,
        Shader,
        vertex_layout_vec2_id,
        4,
        0,
        Order
    )->Vertices;
    
    Vertices[0] = { Rect.Left             , Rect.Top               };
    Vertices[1] = { Rect.Left + Rect.Width, Rect.Top               };
    Vertices[2] = { Rect.Left + Rect.Width, Rect.Top + Rect.Height };
    Vertices[3] = { Rect.Left             , Rect.Top + Rect.Height };
}

void PushBitmap(
    render_group* Group, 
    game_bitmap* Bitmap, 
    rectangle Rect, 
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    wrap_mode WrapMode = Wrap_Clamp,
    v2 Size = V2(1.0f, 1.0f),
    v2 Offset = V2(0.0f , 0.0f),
    bool Refresh = false
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Texture_ID);
    render_primitive_options Options = {};
    Options.Texture = Bitmap;
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        White,
        Shader,
        vertex_layout_vec2_vec2_id,
        4,
        6,
        Order,
        Options
    );

    int Width = Bitmap->Header.Width;
    int Height = Bitmap->Header.Height;
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
    Elements[0] = 0;
    Elements[1] = 1;
    Elements[2] = 2;
    Elements[3] = 3;
    Elements[4] = 2;
    Elements[5] = 1;
}

void PushBitmap(
    render_group* Group, 
    game_bitmap_id ID, 
    rectangle Rect, 
    float Order = SORT_ORDER_DEBUG_OVERLAY,
    wrap_mode Mode = Wrap_Clamp,
    v2 Size = V2(1.0, 1.0),
    v2 Offset = V2(0,0)
) {
    game_bitmap* Bitmap = GetAsset(Group->Assets, ID);
    PushBitmap(Group, Bitmap, Rect, Order, Mode, Size, Offset, false);
}

void PushText(
    render_group* Group,
    v2 Position,
    game_font_id FontID,
    const char* String,
    color Color = White,
    float Points = 20.0f,
    bool Wrapped = false,
    bool Outline = false,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    uint32 nCharacters = 0;
    uint32 StringLength = strlen(String);
    for (int i = 0; i < StringLength; i++) {
        if (String[i] == '\0') break;
        if (String[i] >= '!' && String[i] <= '~') nCharacters++;
    }

    game_font* Font = GetAsset(Group->Assets, FontID);
    game_shader_pipeline* OutlineShader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Text_Outline_ID);
    game_shader_pipeline* InteriorShader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Bezier_Interior_ID);
    game_shader_pipeline* ExteriorShader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Bezier_Exterior_ID);
    game_shader_pipeline* SolidShader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Solid_Text_ID);
    
    v2 Pen = Position;
    float DPI = 96;
    float Size = Points * (DPI / 72.0f) / Font->UnitsPerEm;
    float LineJump = 2 * Font->LineJump * Size;

    for (int i = 0; i < StringLength; i++) {
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
            Pen.X += Font->SpaceAdvance * Size;
        }

        // Character
        else if ('!' <= c && c <= '~') {
            game_font_character* pCharacter = Font->Characters + (c - '!');
            float HorizontalAdvance = pCharacter->Width * Size;
            if (Wrapped && (Pen.X + HorizontalAdvance > Group->Width)) {
                Pen.X = Position.X;
                Pen.Y += LineJump;
            }

            if (pCharacter->nContours > 0) {
                render_primitive_options Options = {};
                Options.Font = Font;
                Options.TextSize = Size;
                Options.PatchParameter = 3;
                Options.Outline = Outline;
                Options.Pen = Pen;

                if (Outline) {
                    render_primitive_command* Command = PushPrimitiveCommand(
                        Group,
                        render_primitive_patches,
                        Color,
                        OutlineShader,
                        vertex_layout_vec2_vec2_id,
                        3 * pCharacter->nOnCurve,
                        0,
                        SORT_ORDER_DEBUG_OVERLAY,
                        Options
                    );

                    Command->VertexEntry.Offset = pCharacter->VertexOffset;
                }

                if (pCharacter->nInteriorCurves > 0) {
                    render_primitive_command* Command = PushPrimitiveCommand(
                        Group,
                        render_primitive_triangle,
                        Color,
                        InteriorShader,
                        vertex_layout_vec2_vec2_id,
                        0,
                        3 * pCharacter->nInteriorCurves,
                        SORT_ORDER_DEBUG_OVERLAY,
                        Options
                    );

                    Command->ElementEntry.Offset = pCharacter->InteriorCurvesOffset;
                }

                if (pCharacter->nExteriorCurves > 0) {
                    render_primitive_command* Command = PushPrimitiveCommand(
                        Group,
                        render_primitive_triangle,
                        Color,
                        ExteriorShader,
                        vertex_layout_vec2_vec2_id,
                        0,
                        3 * pCharacter->nExteriorCurves,
                        SORT_ORDER_DEBUG_OVERLAY,
                        Options
                    );
    
                    Command->ElementEntry.Offset = pCharacter->ExteriorCurvesOffset;
                }

                render_primitive_command* Command = PushPrimitiveCommand(
                    Group,
                    render_primitive_triangle,
                    Color,
                    SolidShader,
                    vertex_layout_vec2_vec2_id,
                    0,
                    3 * pCharacter->nSolidTriangles,
                    SORT_ORDER_DEBUG_OVERLAY,
                    Options
                );
    
                Command->ElementEntry.Offset = pCharacter->SolidTrianglesOffset;

                // glyph_contour_point Last = Contour.Points[Contour.nPoints-1];
                // for (int k = 0; k < Contour.nPoints; k++) {
                //     glyph_contour_point First = Contour.Points[k];
                //     if (First.OnCurve) {
                //         glyph_contour_point Second = {}, Third = {};
                //         Second = Contour.Points[(k + 1) % Contour.nPoints];
                //         if (Second.OnCurve) {
                //             Third = Second;
                //             Second.X = 0.5f * (First.X + Third.X);
                //             Second.Y = 0.5f * (First.Y + Third.Y);
                //         }
                //         else {
                //             Third = Contour.Points[(k + 2) % Contour.nPoints];
                //         }

                //         *Vertices++ = Pen.X + ((First.X) - pCharacter->Left) * Size;
                //         *Vertices++ = Pen.Y - First.Y * Size;
                //         *Vertices++ = Pen.X + ((Second.X) - pCharacter->Left) * Size;
                //         *Vertices++ = Pen.Y - Second.Y * Size;
                //         *Vertices++ = Pen.X + ((Third.X) - pCharacter->Left) * Size;
                //         *Vertices++ = Pen.Y - Third.Y * Size;
                //     }
                // }
            }

            Pen.X += pCharacter->Width * Size;
        }
    }

    /*

    float* Vertices = new float[4 * 5 * nCharacters];
    uint32* Elements = new uint32[6 * nCharacters];

    uint32 nVertices = 0;
    uint32 nElements = 0;

    for (int i = 0; i < StringLength; i++) {
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
            Pen.X += Font->SpaceAdvance * Size;
        }

        // Character
        else if ('!' <= c && c <= '~') {
            game_font_character* pCharacter = Font->Characters + (c - '!');
            float HorizontalAdvance = pCharacter->Advance * Size;
            if (Wrapped && (Pen.X + HorizontalAdvance > Group->Width)) {
                Pen.X = Position.X;
                Pen.Y += LineJump;
            }

            float Left = Pen.X + pCharacter->Left * Size;
            float Top = Pen.Y - pCharacter->Top * Size;
            float CharWidth = pCharacter->Width * Size;
            float CharHeight = pCharacter->Height * Size;

            float MinTexX = (float)(pCharacter->AtlasX) / (float)(Font->Bitmap.Header.Width);
            float MaxTexX = (float)(pCharacter->AtlasX + pCharacter->Width) / (float)(Font->Bitmap.Header.Width);
            float MinTexY = 1.0f - (float)(pCharacter->AtlasY + pCharacter->Height) / (float)(Font->Bitmap.Header.Height);
            float MaxTexY = 1.0f - (float)(pCharacter->AtlasY) / (float)(Font->Bitmap.Header.Height);

            Vertices[20*nVertices]   = Left;
            Vertices[20*nVertices+1] = Top;
            Vertices[20*nVertices+2] = 0;
            Vertices[20*nVertices+3] = MinTexX;
            Vertices[20*nVertices+4] = MaxTexY;

            Vertices[20*nVertices+5] = Left + CharWidth;
            Vertices[20*nVertices+6] = Top;
            Vertices[20*nVertices+7] = 0;
            Vertices[20*nVertices+8] = MaxTexX;
            Vertices[20*nVertices+9] = MaxTexY;

            Vertices[20*nVertices+10] = Left;
            Vertices[20*nVertices+11] = Top + CharHeight;
            Vertices[20*nVertices+12] = 0;
            Vertices[20*nVertices+13] = MinTexX;
            Vertices[20*nVertices+14] = MinTexY;

            Vertices[20*nVertices+15] = Left + CharWidth;
            Vertices[20*nVertices+16] = Top + CharHeight;
            Vertices[20*nVertices+17] = 0;
            Vertices[20*nVertices+18] = MaxTexX;
            Vertices[20*nVertices+19] = MinTexY;

            Elements[6*nElements]   = 4*nVertices;
            Elements[6*nElements+1] = 4*nVertices + 1;
            Elements[6*nElements+2] = 4*nVertices + 2;
            Elements[6*nElements+3] = 4*nVertices + 3;
            Elements[6*nElements+4] = 4*nVertices + 2;
            Elements[6*nElements+5] = 4*nVertices + 1;

            nVertices++;
            nElements++;

            Pen.X += pCharacter->Advance * Size;
        }
    }

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Texture_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_triangle,
        Shader,
        vertex_layout_vec3_vec2_id,
        4 * nCharacters,
        Vertices,
        0,
        Order,
        6 * nCharacters,
        Elements,
        &Font->Bitmap
    );

    delete [] Elements;
    delete [] Vertices;

    */
}

void PushFillbar(
    render_group* Group,
    char* Description,
    float FillPercentage,
    rectangle Rect,
    color Color = Red
) {
    PushRect(Group, Rect, DarkGray);
    rectangle SmallRect = Rect;
    SmallRect.Width *= FillPercentage;
    PushRect(Group, SmallRect, Color);

    v2 Position = LeftTop(Rect);
    PushText(Group, Position + V2(5.0f, 15.0f), Font_Menlo_Regular_ID, Description, White, 8);

    int Points = 8;
    float Width, Height;
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    char Buffer[16];
    sprintf_s(Buffer, "%.2f%%", 100 * FillPercentage);
    GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
    PushText(Group, Position + V2(Rect.Width - Width - 5.0f, 15.0f), Font_Menlo_Regular_ID, Buffer, White, 8);
}

void PushFillbar(
    render_group* Group,
    char* Description,
    int Used,
    int Max,
    rectangle Rect,
    color Color = Red
) {
    float FillPercentage = (float)Used / (float)Max;
    PushRect(Group, Rect, DarkGray);
    rectangle SmallRect = Rect;
    SmallRect.Width *= FillPercentage;
    PushRect(Group, Rect, Color);

    v2 Position = LeftTop(Rect);
    PushText(Group, Position + V2(5.0f, 15.0f), Font_Menlo_Regular_ID, Description, White, 8);

    int Points = 8;
    float Width, Height;
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    char Buffer[16];
    sprintf_s(Buffer, "%d/%d", Used, Max);
    GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
    PushText(Group, Position + V2(Rect.Width - Width - 5.0f, 15.0f), Font_Menlo_Regular_ID, Buffer, White, 8);
}

void PushFillbar(
    render_group* Group,
    char* Description,
    int Used,
    int Max,
    v3 LeftTop,
    v3 WidthAxis,
    v3 HeightAxis,
    float Width,
    float Height,
    color Color = Red
) {
    float FillPercentage = (float)Used / (float)Max;
    PushRect(Group, LeftTop, WidthAxis, HeightAxis, Width, Height, DarkGray);
    float SmallWidth = FillPercentage * Width;
    PushRect(Group, LeftTop, WidthAxis, HeightAxis, SmallWidth, Height, Red);

    // v2 Position = LeftTop(Rect);
    // PushText(Group, Position + V2(5.0f, 15.0f), Font_Menlo_Regular_ID, Description, White, 8);

    // int Points = 8;
    // float Width, Height;
    // game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    // char Buffer[16];
    // sprintf_s(Buffer, "%d/%d", Used, Max);
    // GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
    // PushText(Group, Position + V2(350.0f - Width - 5.0f, 15.0f), Font_Menlo_Regular_ID, Buffer, White, 8);
}

void PushCubeOutline(
    render_group* Group,
    v3 Position,
    scale Size = Scale(1.0),
    color Color = White,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        Color,
        Shader,
        vertex_layout_vec3_id,
        8,
        24,
        Order,
        Options
    );

    v3* Vertices = (v3*)Result->Vertices;
    Vertices[0] = Position + V3(0.0, Size.Y, Size.Z);
    Vertices[1] = Position + V3(Size.X, Size.Y, Size.Z);
    Vertices[2] = Position + V3(0.0, 0.0, Size.Z);
    Vertices[3] = Position + V3(Size.X, 0.0, Size.Z);
    Vertices[4] = Position + V3(Size.X, 0.0, 0.0);
    Vertices[5] = Position + V3(Size.X, Size.Y, 0.0);
    Vertices[6] = Position + V3(0.0, Size.Y, 0.0);
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
//     render_group* Group, 
//     game_video_id VideoID, 
//     rectangle Rect, 
//     double SecondsElapsed, 
//     float Order = SORT_ORDER_DEBUG_OVERLAY
// ) {
//     game_video* Video = GetAsset(Group->Assets, VideoID);
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
//         Entry->Color = White;

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
//     render_group* Group, 
//     game_video_id VideoID, 
//     rectangle Rect, 
//     double SecondsElapsed, 
//     int64_t StartOffset, 
//     int64_t EndOffset,
//     float Order = SORT_ORDER_DEBUG_OVERLAY
// ) {
//     PushVideo(Group, VideoID, Rect, SecondsElapsed, Order);
//     game_video* Video = GetAsset(Group->Assets, VideoID);
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

// void PushSlider(render_group* Group, ui_slider Slider, v2 Position, color Color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
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

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders & render targets                                                                                                                                        |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushRenderTarget(
    render_group* Group,
    render_group_target Target,
    float Order = SORT_ORDER_PUSH_RENDER_TARGETS
) {
    render_command Command;
    Command.Index = Group->nTargets;
    Command.Priority = Order;
    Command.Type = render_target;

    PushCommand(Group, Command);

    render_target_command TargetCommand;
    TargetCommand.Source = Target;
    TargetCommand.DebugAttachment = false;

    game_shader_pipeline_id ShaderID = 
        Target == Target_World || Target == Target_Outline ? 
        Shader_Pipeline_Antialiasing_ID : 
        Shader_Pipeline_Framebuffer_ID;
    TargetCommand.Shader = GetShaderPipeline(Group->Assets, ShaderID);
    
    if (Target == Target_World || Target == Target_Postprocessing_Outline) TargetCommand.Target = Target_Output;
    else if (Target == Target_Outline) TargetCommand.Target = Target_Postprocessing_Outline;
    else if (Target == Target_Output) TargetCommand.Target = Target_None;
    else Raise("Target shouldn't be rendered out.");

    TargetCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_vec3_vec2_id);
    float* Data = (float*)TargetCommand.VertexEntry.Pointer;
    
    Data[0] = -1.0f;  Data[1] = -1.0f;  Data[2] = 0.0f;  Data[3] = 0.0f;  Data[4] = 0.0f;
    Data[5] = 1.0f;   Data[6] = -1.0f;  Data[7] = 0.0f;  Data[8] = 1.0f;  Data[9] = 0.0f;
    Data[10] = 1.0;   Data[11] = 1.0f;  Data[12] = 0.0f; Data[13] = 1.0f; Data[14] = 1.0f;
    Data[15] = -1.0f; Data[16] = -1.0f; Data[17] = 0.0f; Data[18] = 0.0f; Data[19] = 0.0f;
    Data[20] = 1.0f;  Data[21] = 1.0f;  Data[22] = 0.0f; Data[23] = 1.0f; Data[24] = 1.0f;
    Data[25] = -1.0f; Data[26] = 1.0f;  Data[27] = 0.0f; Data[28] = 0.0f; Data[29] = 1.0f;

    Group->TargetCommands[Group->nTargets++] = TargetCommand;
}

void PushShaderPass(
    render_group* Group,
    game_shader_pipeline_id ShaderID,
    render_group_target Target,
    color Color,
    int Level,
    float Width,
    float Order = SORT_ORDER_SHADER_PASSES
) {
    render_command Command;
    Command.Type = render_shader_pass;
    Command.Index = Group->nShaderPassCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_shader_pass_command ShaderCommand;
    ShaderCommand.Color = Color;
    ShaderCommand.Shader = GetShaderPipeline(Group->Assets, ShaderID);
    ShaderCommand.Target = Target;
    ShaderCommand.Width = Width;
    ShaderCommand.Level = Level;
    
    ShaderCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_vec3_vec2_id);

    float* Data = (float*)ShaderCommand.VertexEntry.Pointer;
    Data[0] = -1.0f;  Data[1] = -1.0f;  Data[2] = 0.0f;  Data[3] = 0.0f;  Data[4] = 0.0f;
    Data[5] = 1.0f;   Data[6] = -1.0f;  Data[7] = 0.0f;  Data[8] = 1.0f;  Data[9] = 0.0f;
    Data[10] = 1.0;   Data[11] = 1.0f;  Data[12] = 0.0f; Data[13] = 1.0f; Data[14] = 1.0f;
    Data[15] = -1.0f; Data[16] = -1.0f; Data[17] = 0.0f; Data[18] = 0.0f; Data[19] = 0.0f;
    Data[20] = 1.0f;  Data[21] = 1.0f;  Data[22] = 0.0f; Data[23] = 1.0f; Data[24] = 1.0f;
    Data[25] = -1.0f; Data[26] = 1.0f;  Data[27] = 0.0f; Data[28] = 0.0f; Data[29] = 1.0f;
    
    Group->ShaderPassCommands[Group->nShaderPassCommands++] = ShaderCommand;
}

void PushShaderPass(
    render_group* Group,
    game_compute_shader_id ShaderID,
    render_group_target Source,
    render_group_target Target,
    float Order = SORT_ORDER_SHADER_PASSES
) {
    render_command Command;
    Command.Type = render_compute_shader_pass;
    Command.Index = Group->nComputeShaderPassCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_compute_shader_pass_command ComputeShaderCommand;
    ComputeShaderCommand.Shader = GetShader(Group->Assets, ShaderID);
    ComputeShaderCommand.Source = Source;
    ComputeShaderCommand.Target = Target;

    Group->ComputeShaderPassCommands[Group->nComputeShaderPassCommands++] = ComputeShaderCommand;
}

void PushKernelShaderPass(
    render_group* Group,
    game_compute_shader_id ShaderID,
    render_group_target Source,
    render_group_target Target,
    matrix3 Kernel,
    float Order = SORT_ORDER_SHADER_PASSES
) {
    render_command Command;
    Command.Type = render_compute_shader_pass;
    Command.Index = Group->nComputeShaderPassCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_compute_shader_pass_command ComputeShaderCommand;
    ComputeShaderCommand.Shader = GetShader(Group->Assets, ShaderID);
    ComputeShaderCommand.Source = Source;
    ComputeShaderCommand.Target = Target;
    ComputeShaderCommand.Kernel = Kernel;

    Group->ComputeShaderPassCommands[Group->nComputeShaderPassCommands++] = ComputeShaderCommand;
}

void PushBlur(
    render_group* Group,
    render_group_target Target,
    float Order = SORT_ORDER_SHADER_PASSES
) {
    matrix3 Kernel = {
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    };
    Kernel *= 1.0f / 16.0f;

    PushKernelShaderPass(Group, Compute_Shader_Kernel_ID, Target, Target, Kernel, Order);
}

void PushMesh(
    render_group* Group,
    game_mesh_id MeshID,
    transform Transform,
    game_shader_pipeline_id ShaderID,
    game_bitmap_id TextureID = Bitmap_Empty_ID,
    color Color = White,
    armature* Armature = NULL,
    bool Outline = false,
    float Order = SORT_ORDER_MESHES
) {
    game_mesh* Mesh = GetAsset(Group->Assets, MeshID);
    render_primitive_options Options = {};
    Options.Mesh = Mesh;
    Options.Armature = Armature;
    Options.Texture = GetAsset(Group->Assets, TextureID);
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    Options.Outline = Outline;
    
    PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        GetShaderPipeline(Group->Assets, ShaderID),
        Armature != NULL ? vertex_layout_vec3_vec2_vec3_id : vertex_layout_bones_id,
        Mesh->nVertices,
        3 * Mesh->nFaces,
        SORT_ORDER_MESHES,
        Options
    );

    // Deal with outlines: Add necessary shader passes
    if (Outline && !Group->PushOutline) {
        PushRenderTarget(Group, Target_Outline, SORT_ORDER_SHADER_PASSES - 10.0f);
        PushShaderPass(Group, Compute_Shader_Outline_Init_ID, Target_Postprocessing_Outline, Target_Postprocessing_Outline, SORT_ORDER_SHADER_PASSES);

        int Shifts = 11;
        int Level = 1 << Shifts;
        float JumpOrder = SORT_ORDER_SHADER_PASSES;

        for (int i = 0; i <= Shifts; i++) {
            JumpOrder += 1.0f;
            PushShaderPass(Group, Shader_Pipeline_Jump_Flood_ID, Target_Postprocessing_Outline, White, Level, 0.0f, JumpOrder);
            Level >>= 1;
        }

        PushShaderPass(Group, Shader_Pipeline_Outline_ID, Target_Postprocessing_Outline, White, 0, 4.0f, JumpOrder);
        PushBlur(Group, Target_Postprocessing_Outline, JumpOrder + 1.0f);

        PushRenderTarget(Group, Target_Postprocessing_Outline, SORT_ORDER_SHADER_PASSES + 30.0f);
        Group->PushOutline = true;
    }

    // Debug bones rendering
    if (Group->Debug && Group->DebugBones && Armature != NULL) {
        game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
        Options = {};
        Options.Thickness = 2.5f;
        v3* Vertices = (v3*)PushPrimitiveCommand(
            Group, 
            render_primitive_line,
            Black,
            Shader,
            vertex_layout_vec3_id,
            2 * Armature->nBones,
            0,
            SORT_ORDER_DEBUG_OVERLAY,
            Options
        )->Vertices;

        for (int i = 0; i < Armature->nBones; i++) {
            bone Bone = Armature->Bones[i];
            transform BoneTransform = Bone.Transform;
            Vertices[2*i] = BoneTransform * Transform * Bone.Segment.Head;
            Vertices[2*i+1] = BoneTransform * Transform * Bone.Segment.Tail;
        }
    }
}

void PushHeightmap(
    render_group* Group, 
    game_heightmap* Heightmap, 
    game_shader_pipeline_id ShaderID,
    float Order = SORT_ORDER_MESHES
) {
    render_primitive_options Options = {};
    Options.Texture = &Heightmap->Bitmap;
    Options.PatchParameter = 4;
    Options.Flags = DEPTH_TEST_RENDER_FLAG;

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, ShaderID);

    // TODO: Render heightmaps with elements

    float* Vertices = PushPrimitiveCommand(
        Group, 
        render_primitive_patches,
        White,
        Shader, 
        vertex_layout_vec3_vec2_id, 
        Heightmap->nVertices,
        0,
        Order,
        Options
    )->Vertices;

    memcpy(Vertices, Heightmap->Vertices, Heightmap->nVertices * 5 * sizeof(float));
}

void PushHeightmap(
    render_group* Group, 
    game_heightmap_id ID, 
    game_shader_pipeline_id ShaderID,
    float Order = SORT_ORDER_MESHES
) {
    game_heightmap* Heightmap = GetAsset(Group->Assets, ID);
    PushHeightmap(Group, Heightmap, ShaderID, Order);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushCollider(render_group* Group, collider Collider, transform T, color Color) {
    v3 Position = T.Translation + Collider.Offset;
    switch (Collider.Type) {
        case Rect_Collider: {
            PushRectOutline(Group, Rectangle(Collider), Color);
        } break;

        case Cube_Collider: {
            PushCubeOutline(Group, Position, Scale(Collider.Cube.HalfWidth,Collider.Cube.HalfHeight,Collider.Cube.HalfDepth), Color);
        } break;

        case Sphere_Collider: {
            PushCircunference(Group, Position, V3(1,0,0), Collider.Sphere.Radius, Color);
            PushCircunference(Group, Position, V3(0,1,0), Collider.Sphere.Radius, Color);
            PushCircunference(Group, Position, V3(0,0,1), Collider.Sphere.Radius, Color);
        } break;

        case Capsule_Collider: {
            v3 Head = T * Collider.Capsule.Segment.Head;
            v3 Tail = T * Collider.Capsule.Segment.Tail;

            segment3 TransformedSegment = { Head, Tail };
            transform ST = SegmentTransform(TransformedSegment);
            basis Basis = ST * Identity3;
            v3 D = normalize(Tail - Head);

            // Top part
            PushArc(Group, Tail, Basis, Collider.Capsule.Distance, 180, Color);
            v3 Temp = Basis.X;
            Basis.X = Basis.Z;
            Basis.Z = Temp;
            PushArc(Group, Tail, Basis, Collider.Capsule.Distance, 180, Color);
            PushCircunference(Group, Tail, D, Collider.Capsule.Distance, Color);

            // Vertical lines
            v3 Offset[4] = { Basis.X, -Basis.X, Basis.Z, -Basis.Z };
            for (int i = 0; i < 4; i++) {
                PushLine(
                    Group, 
                    Tail + Collider.Capsule.Distance * Offset[i], 
                    Head + Collider.Capsule.Distance * Offset[i],
                    Color
                );
            }

            // Bottom part
            PushCircunference(Group, Head, D, Collider.Capsule.Distance, Color);
            Basis = ST * Identity3;
            Basis.X = -Basis.X;
            Basis.Y = -Basis.Y;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, Color);
            Temp = Basis.X;
            Basis.X = Basis.Z;
            Basis.Z = Temp;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, Color);

            Collider.Capsule.Segment = T * Collider.Capsule.Segment;
        } break;

        default: Assert(false);
    }
}

#endif