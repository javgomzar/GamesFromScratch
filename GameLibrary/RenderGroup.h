#ifndef RENDER_GROUP
#define RENDER_GROUP

#pragma once
#include "GameAssets.h"
#include "GameEntity.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Vertex buffer                                                                                                                                |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const memory_index MAX_VERTEX_BUFFER_COUNT = Kilobytes(16);
const memory_index MAX_ELEMENT_BUFFER_COUNT = Kilobytes(16);

struct vertex_buffer_entry {
    memory_index Offset;
    uint64 Count;
    vertex_layout_id LayoutID;
    void* Pointer;
};

struct element_buffer_entry {
    memory_index Offset;
    uint64 Count;
    uint32* Pointer;
};

struct vertex_buffer {
    vertex_layout Layouts[vertex_layout_id_count];
    memory_arena VertexArena[vertex_layout_id_count];
    memory_arena ElementArena;
    uint64 VertexCount[vertex_layout_id_count];
    uint64 ElementCount;
};

/* Initializes a vertex and element buffer. Returns total size used.*/
memory_index InitializeVertexBuffer(vertex_layout* Layouts, vertex_buffer* Buffer, uint8* Start) {
    memory_index TotalSize = 0;

    memory_index Size = 0;
    for (int i=0; i < vertex_layout_id_count; i++) {
        Buffer->Layouts[i] = Layouts[i];
        Buffer->VertexCount[i] = 0;
        Size = MAX_VERTEX_BUFFER_COUNT * Layouts[i].Stride;
        Buffer->VertexArena[i] = MemoryArena(Size, Start);
        Start += Size;
        TotalSize += Size;
    }
    Buffer->ElementCount = 0;
    Size = MAX_ELEMENT_BUFFER_COUNT * sizeof(uint32);
    Buffer->ElementArena = MemoryArena(Size, Start);
    TotalSize += Size;
    return TotalSize;
}

vertex_buffer_entry PushVertexEntry(vertex_buffer* VertexBuffer, uint64 VertexCount, vertex_layout_id VertexLayoutID, void* Data) {
    memory_arena* Arena = &VertexBuffer->VertexArena[VertexLayoutID];
    
    vertex_buffer_entry Entry;
    Entry.Count = VertexCount;
    Entry.LayoutID = VertexLayoutID;
    Entry.Offset = VertexBuffer->VertexCount[VertexLayoutID];
    
    memory_index Size = VertexCount * VertexBuffer->Layouts[VertexLayoutID].Stride;
    void* Destination = PushSize(Arena, Size);
    memcpy(Destination, Data, Size);
    Entry.Pointer = Destination;

    VertexBuffer->VertexCount[VertexLayoutID] += VertexCount;
    return Entry;
}

element_buffer_entry PushElementEntry(vertex_buffer* VertexBuffer, uint64 ElementCount, void* Data) {
    element_buffer_entry Entry;
    Entry.Count = ElementCount;
    Entry.Offset = VertexBuffer->ElementCount;

    memory_index Size = ElementCount * sizeof(uint32);
    void* Destination = PushSize(&VertexBuffer->ElementArena, Size);
    memcpy(Destination, Data, Size);
    Entry.Pointer = (uint32*)Destination;

    VertexBuffer->ElementCount += ElementCount;
    return Entry;
}

void ClearVertexBuffer(vertex_buffer* Buffer) {
    for (int i = 0; i < vertex_layout_id_count; i++) {
        ClearArena(&Buffer->VertexArena[i]);
        Buffer->VertexCount[i] = 0;
    }
    ClearArena(&Buffer->ElementArena);
    Buffer->ElementCount = 0;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Render entries                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_RENDER_ENTRIES = 16384;

enum render_command_type {
    render_clear,
    render_draw_primitive,
    render_draw_mesh,
    render_draw_heightmap,
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


struct render_primitive_command {
    color Color = White;
    vertex_buffer_entry VertexEntry = {0};
    element_buffer_entry ElementEntry = {0};
    game_bitmap* Texture = 0;
    game_shader_pipeline* Shader = 0;
    render_flags Flags = 0;
    render_primitive Primitive = render_primitive_point;
    float Thickness = 2.0f;
};

struct render_mesh_command {
    color Color;
    transform Transform;
    game_shader_pipeline* Shader;
    game_mesh* Mesh;
    game_bitmap* Texture;
    armature* Armature;
    bool Outline;
};

struct render_heightmap_command {
    transform Transform;
    game_shader_pipeline* Shader;
    game_heightmap* Heightmap;
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
const int MAX_PRIMITIVE_COMMANDS = 1024;
const int MAX_MESH_COMMANDS = 64;
const int MAX_HEIGHTMAP_COMMANDS = 8;
const int MAX_SHADER_PASS_COMMANDS = 32;
const int MAX_COMPUTE_SHADER_PASS_COMMANDS = 32;
const int MAX_RENDER_TARGET_COMMANDS = 16;

struct render_group {
    render_command Entries[MAX_RENDER_ENTRIES];
    render_clear_command Clears[render_group_target_count];
    render_primitive_command PrimitiveCommands[MAX_PRIMITIVE_COMMANDS];
    render_mesh_command MeshCommands[MAX_MESH_COMMANDS];
    render_heightmap_command HeightmapCommands[MAX_HEIGHTMAP_COMMANDS];
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
    uint32 nMeshCommands;
    uint32 nHeightmapCommands;
    uint32 nShaderPassCommands;
    uint32 nComputeShaderPassCommands;
    uint32 nTargets;
    bool Debug;
    bool DebugNormals;
    bool DebugBones;
    bool DebugColliders;
    bool PushOutline;
};

memory_index InitializeRenderGroup(render_group* Group, game_assets* Assets, uint8* VertexBufferStart) {
    Group->Assets = Assets;

    // Lighting
    Group->Light = Light(V3(-0.5, -1, 1), White);

    // Vertex & element buffers
    memory_index MemoryUsed = InitializeVertexBuffer(Assets->VertexLayouts, &Group->VertexBuffer, VertexBufferStart);

    return MemoryUsed;
}

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
        case render_draw_mesh: {
            if (Command.Index >= MAX_MESH_COMMANDS) {
                Raise("Mesh draw command overflow.");
            }
        } break;
        case render_draw_heightmap: {
            if (Command.Index >= MAX_HEIGHTMAP_COMMANDS) {
                Raise("Heightmap draw command overflow.");
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
    ZeroSize(Group->nMeshCommands * sizeof(render_mesh_command), Group->MeshCommands);
    ZeroSize(Group->nHeightmapCommands * sizeof(render_heightmap_command), Group->HeightmapCommands);
    ZeroSize(Group->nShaderPassCommands * sizeof(render_shader_pass_command), Group->ShaderPassCommands);
    ZeroSize(Group->nComputeShaderPassCommands * sizeof(render_compute_shader_pass_command), Group->ComputeShaderPassCommands);
    ZeroSize(Group->nTargets * sizeof(render_target_command), Group->TargetCommands);
    Group->nPrimitiveCommands = 0;
    Group->nMeshCommands = 0;
    Group->nHeightmapCommands = 0;
    Group->nShaderPassCommands = 0;
    Group->nComputeShaderPassCommands = 0;
    Group->nTargets = 0;
    Group->EntryCount = 0;
}

// Render entries sorting
float SORT_ORDER_CLEAR = 0.0;
float SORT_ORDER_MESHES = 100.0;
float SORT_ORDER_OUTLINED_MESHES = 150.0;
float SORT_ORDER_DEBUG_OVERLAY = 200.0;
float SORT_ORDER_SHADER_PASSES = 8000.0;
float SORT_ORDER_PUSH_RENDER_TARGETS = 9000.0;

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

void PushPrimitiveCommand(
    render_group* Group,
    color Color,
    render_primitive Primitive,
    game_shader_pipeline* Shader,
    vertex_layout_id LayoutID,
    uint32 nVertices,
    float* Vertices,
    render_flags Flags = 0,
    float Order = 0.0,
    uint32 nElements = 0,
    uint32* Elements = 0,
    game_bitmap* Texture = 0,
    float Thickness = 2.0f
) {
    TIMED_BLOCK;
    render_command Command;
    Command.Index = Group->nPrimitiveCommands;
    Command.Priority = Order;
    Command.Type = render_draw_primitive;

    PushCommand(Group, Command);

    render_primitive_command PrimitiveCommand;
    PrimitiveCommand.Color = Color;
    PrimitiveCommand.Flags = Flags;
    PrimitiveCommand.Primitive = Primitive;
    PrimitiveCommand.Shader = Shader;
    PrimitiveCommand.Texture = Texture;
    PrimitiveCommand.Thickness = Thickness;
    
    PrimitiveCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, nVertices, LayoutID, Vertices);
    uint32 VertexOffset = PrimitiveCommand.VertexEntry.Offset;

    if (nElements > 0) {
        for (int i = 0; i < nElements; i++) {
            Elements[i] = VertexOffset + Elements[i];
        }
        PrimitiveCommand.ElementEntry = PushElementEntry(&Group->VertexBuffer, nElements, Elements);
    }

    Group->PrimitiveCommands[Group->nPrimitiveCommands++] = PrimitiveCommand;
}

void PushPoint(render_group* Group, v3 Point, color Color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    float Vertices[3] = { Point.X, Point.Y, Point.Z };
    PushPrimitiveCommand(
        Group, 
        Color, 
        render_primitive_point, 
        Shader, 
        vertex_layout_vec3_id, 
        1, 
        Vertices,
        0,
        Order,
        0, 0, 0
    );
}

void PushLine(
    render_group* Group,
    v2 Start,
    v2 End,
    color Color,
    int Thickness = 2.0f,
    float Order = 0.0
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    float Data[6] = { Start.X, Start.Y, 0, End.X, End.Y, 0 };
    PushPrimitiveCommand(
        Group, 
        Color, 
        render_primitive_line,
        Shader,
        vertex_layout_vec3_id,
        2,
        Data,
        0,
        Order,
        0,0,0,
        Thickness
    );
}

void PushLine(
    render_group* Group,
    v3 Start,
    v3 End,
    color Color,
    float Thickness = 2.0f,
    float Order = 0.0
) {    
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    float Data[6] = { Start.X, Start.Y, Start.Z, End.X, End.Y, End.Z };
    PushPrimitiveCommand(
        Group, 
        Color,
        render_primitive_line,
        Shader,
        vertex_layout_vec3_id,
        2,
        Data,
        DEPTH_TEST_RENDER_FLAG,
        Order,
        0,0,0,
        Thickness
    );
}

void PushRay(
    render_group* Group,
    ray Ray,
    color Color,
    float Thickness = 2.0f,
    float Length = 10.0f,
    float Order = 0.0
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    v3 Start = Ray.Point;
    v3 End = Start + Length * Ray.Direction;
    PushLine(Group, Start, End, Color, Thickness, Order);
}

void PushTriangle(
    render_group* Group,
    triangle Triangle,
    color Color,
    float Order = SORT_ORDER_MESHES
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    float Data[9] = { 
        Triangle.Points[0].X, Triangle.Points[0].Y, Triangle.Points[0].Z,
        Triangle.Points[1].X, Triangle.Points[1].Y, Triangle.Points[1].Z,
        Triangle.Points[2].X, Triangle.Points[2].Y, Triangle.Points[2].Z
    };
    PushPrimitiveCommand(
        Group, 
        Color, 
        render_primitive_triangle,
        Shader,
        vertex_layout_vec3_id,
        3,
        Data,
        DEPTH_TEST_RENDER_FLAG,
        Order
    );
}

void PushTriangle(
    render_group* Group,
    v2 Vertex1,
    v2 Vertex2,
    v2 Vertex3,
    color Color,
    float Order = 0.0
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    float Data[9] = { 
        Vertex1.X, Vertex1.Y, 0,
        Vertex2.X, Vertex2.Y, 0,
        Vertex3.X, Vertex3.Y, 0
    };
    PushPrimitiveCommand(
        Group, 
        Color, 
        render_primitive_triangle,
        Shader,
        vertex_layout_vec3_id,
        3,
        Data,
        0,
        Order
    );
}

void PushCircle(
    render_group* Group,
    v2 Center,
    float Radius,
    color Color,
    float Order = 0.0,
    int nVertices = 30
) {
    int MAX_N = 62;
    int N = Clamp(nVertices, 14, MAX_N);
    float* Data = new float[3*(N+2)];
    v3* Vertices = (v3*)Data;

    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = V3(Center, 0);
    Vertices[1] = V3(Center.X, Center.Y - Radius, 0);
    for (int i = 2; i < N+1; i++) {
        Vertices[i].X = Center.X + Radius * sin(Theta);
        Vertices[i].Y = Center.Y - Radius * cos(Theta);
        Vertices[i].Z = 0;
        Theta += dTheta;
    }
    Vertices[N+1] = Vertices[1];

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_triangle_fan,
        Shader,
        vertex_layout_vec3_id,
        N+2,
        Data,
        0,
        Order
    );

    delete [] Data;
}

void PushCircle(
    render_group* Group, 
    v3 Center,
    double Radius, 
    v3 Normal,
    color Color,
    float Order = 0.0,
    int nVertices = 32
) {
    int MAX_N = 62;
    int N = Clamp(nVertices, 14, MAX_N);
    basis Basis = Complete(Normal);
    
    float* Data = new float[3*(N+2)];
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

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_triangle_fan,
        Shader,
        vertex_layout_vec3_id,
        N+2,
        Data,
        DEPTH_TEST_RENDER_FLAG,
        Order
    );

    delete [] Data;
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
    float* Data = new float[3*N];
    v3* Vertices = (v3*)Data;

    double dTheta = Tau / N;
    double Theta = dTheta;
    Vertices[0] = V3(Center.X, Center.Y - Radius, 0);
    for (int i = 1; i < N; i++) {
        Vertices[i].X = Center.X + Radius * sin(Theta);
        Vertices[i].Y = Center.Y - Radius * cos(Theta);
        Vertices[i].Z = 0;
        Theta += dTheta;
    }

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_line_loop,
        Shader,
        vertex_layout_vec3_id,
        N,
        Data,
        0,
        Order,
        0,0,0,
        Thickness
    );

    delete [] Data;
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
    float* Data = new float[3*N];
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

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_line_loop,
        Shader,
        vertex_layout_vec3_id,
        N,
        Data,
        DEPTH_TEST_RENDER_FLAG,
        Order,
        0,0,0,
        Thickness
    );

    delete [] Data;
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
    float* Data = new float[3*N];
    v3* Vertices = (v3*)Data;

    double dTheta = Angle * Degrees / (N-1);
    double Theta = dTheta;
    Vertices[0] = Center + Radius * Basis.X;
    for (int i = 1; i < N; i++) {
        Vertices[i] = Center + Radius * (cos(Theta) * Basis.X + sin(Theta) * Basis.Y);
        Theta += dTheta;
    }

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_line_strip,
        Shader,
        vertex_layout_vec3_id,
        N,
        Data,
        DEPTH_TEST_RENDER_FLAG,
        Order,
        0,0,0,
        Thickness
    );

    delete [] Data;
}

void PushRect(
    render_group* Group,
    rectangle Rect,
    color Color,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    float Vertices[12] = {
        Rect.Left             , Rect.Top              , 0,
        Rect.Left + Rect.Width, Rect.Top              , 0,
        Rect.Left             , Rect.Top + Rect.Height, 0,
        Rect.Left + Rect.Width, Rect.Top + Rect.Height, 0
    };

    uint32 Elements[6] = { 0, 1, 2, 3, 2, 1 };

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_triangle,
        Shader,
        vertex_layout_vec3_id,
        4,
        Vertices,
        0,
        Order,
        6,
        Elements
    );
}

void PushRectOutline(
    render_group* Group,
    rectangle Rect,
    color Color,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    float Vertices[12] = {
        Rect.Left             , Rect.Top              , 0,
        Rect.Left + Rect.Width, Rect.Top              , 0,
        Rect.Left + Rect.Width, Rect.Top + Rect.Height, 0,
        Rect.Left             , Rect.Top + Rect.Height, 0,
    };
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_line_loop,
        Shader,
        vertex_layout_vec3_id,
        4,
        Vertices,
        0,
        Order
    );
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
            double MinX = 0.0;
            double MinY = -Rect.Height / (Size.Y * Height);
            double MaxX = Rect.Width / (Size.X * Width);
            double MaxY = 1.0;
            MinTexX = Size.X < 0 ? MaxX : MinX;
            MaxTexX = Size.X < 0 ? MinX : MaxX;
            MinTexY = Size.Y ? MaxY : MinY;
            MaxTexY = Size.Y ? MinY : MaxY;
        } break;

        default: { Assert(false); }
    }
    
    float Vertices[20] = {
        Rect.Left             , Rect.Top              , 0, MinTexX, MaxTexY,
        Rect.Left + Rect.Width, Rect.Top              , 0, MaxTexX, MaxTexY,
        Rect.Left             , Rect.Top + Rect.Height, 0, MinTexX, MinTexY,
        Rect.Left + Rect.Width, Rect.Top + Rect.Height, 0, MaxTexX, MinTexY
    };
    uint32 Elements[6] = { 0, 1, 2, 3, 2, 1 };
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Texture_ID);
    PushPrimitiveCommand(
        Group,
        White,
        render_primitive_triangle,
        Shader,
        vertex_layout_vec3_vec2_id,
        4,
        Vertices,
        0,
        Order,
        6,
        Elements,
        Bitmap
    );
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
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    uint32 nCharacters = 0;
    uint32 StringLength = strlen(String);
    for (int i = 0; i < StringLength; i++) {
        if (String[i] == '\0') break;
        if (String[i] >= '!' && String[i] <= '~') nCharacters++;
    }

    game_font* Font = GetAsset(Group->Assets, FontID);

    v2 Pen = Position;
    float Size = 0.05f * (float)Points;
    float LineJump = Font->LineJump * Size;

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
}

void PushText(
    render_group* Group,
    v2 Position,
    game_font_id FontID,
    string String,
    color Color = White,
    float Points = 20.0f,
    bool Wrapped = false,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    PushText(Group, Position, FontID, String.Content, Color, Points, Wrapped, Order);
}

void PushFillBar(
    render_group* Group,
    rectangle Rect,
    color Color,
    float FillPercent
) {
    PushRect(Group, Rect, DarkGray);
    Rect.Width *= FillPercent;
    PushRect(Group, Rect, Color);
}

void PushCubeOutline(
    render_group* Group,
    v3 Position,
    scale Size = Scale(1.0),
    color Color = White,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    v3 Vertices[8] = {
        Position + V3(0.0, Size.Y, Size.Z),
        Position + V3(Size.X, Size.Y, Size.Z),
        Position + V3(0.0, 0.0, Size.Z),
        Position + V3(Size.X, 0.0, Size.Z),
        Position + V3(Size.X, 0.0, 0.0),
        Position + V3(Size.X, Size.Y, 0.0),
        Position + V3(0.0, Size.Y, 0.0),
        Position + V3(0.0, 0.0, 0.0)
    };

    uint32 Elements[24] = { 0, 1, 0, 2, 0, 6, 5, 6, 1, 5, 1, 3, 4, 5, 2, 7, 2, 3, 3, 4, 4, 7, 6, 7 };
    
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_line,
        Shader,
        vertex_layout_vec3_id,
        8,
        (float*)Vertices,
        DEPTH_TEST_RENDER_FLAG,
        Order,
        24,
        Elements
    );
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
//         sprintf_s(Text, "%.02f Time elapsed | %.02f Time played", Video->TimeElapsed, Video->VideoContext.PTS * Video->VideoContext.TimeBase);
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

    float Data[30] = {
       -1.0f,-1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,-1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
       -1.0f,-1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
       -1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    TargetCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_vec3_vec2_id, Data);

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

    float Data[30] = {
       -1.0f,-1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,-1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
       -1.0f,-1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
       -1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    ShaderCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_vec3_vec2_id, Data);

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
    armature* Armature = 0,
    bool Outlined = false,
    float Order = SORT_ORDER_MESHES
) {
    render_command Command;
    Command.Index = Group->nMeshCommands;
    Command.Priority = Order;
    Command.Type = render_draw_mesh;

    PushCommand(Group, Command);

    render_mesh_command MeshCommand;
    MeshCommand.Mesh = GetAsset(Group->Assets, MeshID);
    MeshCommand.Transform = Transform;
    MeshCommand.Color = Color;
    MeshCommand.Texture = GetAsset(Group->Assets, TextureID);
    MeshCommand.Shader = GetShaderPipeline(Group->Assets, ShaderID);
    MeshCommand.Armature = Armature;
    MeshCommand.Outline = Outlined;

    Group->MeshCommands[Group->nMeshCommands++] = MeshCommand;

    if (Outlined && !Group->PushOutline) {
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

    if (Group->Debug && Group->DebugBones && Armature) {
        v3* Vertices = new v3[2 * Armature->nBones];

        for (int i = 0; i < Armature->nBones; i++) {
            bone Bone = Armature->Bones[i];
            transform BoneTransform = Bone.Transform;
            Vertices[2*i] = BoneTransform * Transform * Bone.Head;
            Vertices[2*i+1] = BoneTransform * Transform * Bone.Tail;
        }

        game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
        PushPrimitiveCommand(
            Group, 
            Black, 
            render_primitive_line,
            Shader,
            vertex_layout_vec3_id,
            2 * Armature->nBones,
            (float*)Vertices,
            0,
            SORT_ORDER_DEBUG_OVERLAY,
            0,0,0,
            2.5f
        );

        delete [] Vertices;
    }

}

void PushHeightmap(
    render_group* Group, 
    game_heightmap_id ID, 
    game_shader_pipeline_id ShaderID,
    transform Transform,
    float Order = SORT_ORDER_MESHES
) {
    render_command Command;
    Command.Index = Group->nHeightmapCommands;
    Command.Priority = Order;
    Command.Type = render_draw_heightmap;

    PushCommand(Group, Command);

    render_heightmap_command HeightmapCommand;
    HeightmapCommand.Heightmap = GetAsset(Group->Assets, ID);
    HeightmapCommand.Transform = Transform;
    HeightmapCommand.Shader = GetShaderPipeline(Group->Assets, ShaderID);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                                         |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushCollider(render_group* Group, game_entity* Entity, color Color) {
    collider Collider = Entity->Collider;
    v3 Position = Entity->Transform.Translation + Collider.Offset;
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
            v3 Head = Entity->Transform * Collider.Capsule.Segment.Head;
            v3 Tail = Entity->Transform * Collider.Capsule.Segment.Tail;

            segment TransformedSegment = { Head, Tail };
            transform T = SegmentTransform(TransformedSegment);
            basis Basis = T * Identity3;
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
            Basis = T * Identity3;
            Basis.X = -Basis.X;
            Basis.Y = -Basis.Y;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, Color);
            Temp = Basis.X;
            Basis.X = Basis.Z;
            Basis.Z = Temp;
            PushArc(Group, Head, Basis, Collider.Capsule.Distance, 180, Color);

            Collider.Capsule.Segment = Entity->Transform * Collider.Capsule.Segment;
        } break;

        default: Assert(false);
    }
}

void PushEntities(render_group* Group, game_entity_list* List, game_input* Input, float Time) {
    TIMED_BLOCK;
    basis Basis = Group->Camera->Basis;
    ray Ray = MouseRay(Group->Width, Group->Height, Group->Camera->Position + Group->Camera->Distance * Basis.Z, Basis, Input->Mouse.Cursor);
    int i = 0;
    int nEntities = 0;
    while (nEntities < List->nEntities) {
        game_entity* Entity = &List->Entities[i++];
        if (Entity->Active) nEntities++;
        else continue;
        collider Collider = Entity->Transform * Entity->Collider;
        Entity->Hovered = Raycast(Ray, Collider);
        switch(Entity->Type) {
            case Entity_Type_Character: {
                character* pCharacter = &List->Characters.List[Entity->Index];
                PushMesh(
                    Group, 
                    Mesh_Body_ID, 
                    Entity->Transform,
                    Shader_Pipeline_Mesh_Bones_ID, 
                    Bitmap_Empty_ID,
                    White,
                    &pCharacter->Armature,
                    Entity->Hovered
                );
            } break;
    
            case Entity_Type_Enemy: {
                PushMesh(
                    Group,
                    Mesh_Enemy_ID,
                    Entity->Transform,
                    Shader_Pipeline_Mesh_ID,
                    Bitmap_Enemy_ID,
                    White, 0,
                    Entity->Hovered
                );
            } break;

            case Entity_Type_Prop: {
                prop* pProp = &List->Props.List[Entity->Index];
                PushMesh(
                    Group,
                    pProp->MeshID,
                    Entity->Transform,
                    pProp->Shader,
                    Bitmap_Empty_ID,
                    pProp->Color
                );
            } break;

            case Entity_Type_Weapon: {
                weapon* pWeapon = &List->Weapons.List[Entity->Index];
                game_mesh_id MeshID;
                switch(pWeapon->Type) {
                    case Weapon_Sword: MeshID = Mesh_Sword_ID; break;
                    case Weapon_Shield: MeshID = Mesh_Shield_ID; break;
                    default: Assert(false);
                }

                PushMesh(Group, MeshID, Entity->Transform, Shader_Pipeline_Mesh_ID);
            } break;
        }

        if (Group->Debug && Group->DebugColliders && Entity->Type != Entity_Type_Camera) {
            PushCollider(Group, Entity, Entity->Collided ? Red : Yellow);
        }
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Debug                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushDebugVector(render_group* Group, v2 Vector, v2 Position, color Color) {
    v2 Orthogonal = perp(normalize(V2(Vector.X, Vector.Y)));
    float OrthogonalLength = 0.005333f * Group->Height;

    int Thickness = max(1.0f, 0.0025f * Group->Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, SORT_ORDER_DEBUG_OVERLAY);
    PushTriangle(
        Group, 
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector - OrthogonalLength * Orthogonal,
        Color, 
        SORT_ORDER_DEBUG_OVERLAY
    );
    PushTriangle(
        Group, 
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector + OrthogonalLength * Orthogonal,
        Color, 
        SORT_ORDER_DEBUG_OVERLAY
    );
}

void PushDebugVector(render_group* Group, v3 Vector, v3 Position, color Color) {
    float Height = Group->Height;

    v2 CameraCoordinates = perp(V2(dot(Vector, Group->Camera->Basis.X), dot(Vector, Group->Camera->Basis.Y)));
    v3 Orthogonal = normalize(CameraCoordinates.X * Group->Camera->Basis.X + CameraCoordinates.Y * Group->Camera->Basis.Y);
    float OrthogonalLength = (modulus(Vector) / 15.0f);

    int Thickness = max(1.0, 0.0025 * Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, SORT_ORDER_MESHES);
    triangle Triangle = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector - OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, Color, SORT_ORDER_MESHES);
    Triangle = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector + OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, Color, SORT_ORDER_MESHES);
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

    v3 Vertices[9] = {
        Position,
        Position + l_ + t_ + fv,
        Position + r_ + t_ + fv,
        Position + l_ + b_ + fv,
        Position + r_ + b_ + fv,
        Position + rv + bv + nv,
        Position + rv + tv + nv,
        Position + lv + bv + nv,
        Position + lv + tv + nv
    };

    uint32 Elements[24] = { 0, 1, 0, 2, 0, 3, 0, 4, 4, 3, 2, 1, 4, 2, 3, 1, 5, 7, 6, 8, 5, 6, 7, 8 };

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        White,
        render_primitive_line,
        Shader,
        vertex_layout_vec3_vec2_id,
        9,
        (float*)Vertices,
        DEPTH_TEST_RENDER_FLAG,
        SORT_ORDER_DEBUG_OVERLAY,
        24,
        Elements
    );
}

void PushDebugGrid(render_group* Group, float Alpha) {
    const int nVertices = 404;
    v3 Vertices[nVertices];

    for (int i = 0; i <= 100; i++) {
        Vertices[4*i  ] = V3(50-i, 0, -50);
        Vertices[4*i+1] = V3(50-i, 0, 50);
        Vertices[4*i+2] = V3(-50, 0, 50-i);
        Vertices[4*i+3] = V3(50, 0, 50-i);
    }
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color(White, 0.5f),
        render_primitive_line,
        Shader,
        vertex_layout_vec3_id,
        nVertices,
        (float*)Vertices,
        DEPTH_TEST_RENDER_FLAG,
        SORT_ORDER_DEBUG_OVERLAY-2.0f,
        0,0,0,
        1.0f
    );
}

void PushDebugFramebuffer(render_group* Group, render_group_target Framebuffer, bool Attachment = false) {
    render_command Command;
    Command.Index = Group->nTargets;
    Command.Priority = SORT_ORDER_PUSH_RENDER_TARGETS - 0.1f;
    Command.Type = render_target;

    PushCommand(Group, Command);

    render_target_command TargetCommand = {};
    TargetCommand.Source = Framebuffer;
    TargetCommand.Target = Target_Output;
    TargetCommand.DebugAttachment = Attachment;
    
    if (Framebuffer == Target_World || Framebuffer == Target_Outline) {
        TargetCommand.Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Antialiasing_ID);
    }
    else {
        TargetCommand.Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Framebuffer_ID);
    }

    float Vertices[30] = {
        0.5f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
        0.5f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f
    };
    TargetCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_vec3_vec2_id, Vertices);

    Group->TargetCommands[Group->nTargets++] = TargetCommand;
}

void PushDebugPlot(
    render_group* Group,
    int N,
    float* Data,
    v2 Position,
    int dx,
    color Color = White,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    v3* Vertices = new v3[N];
    float X = 0;
    for (int i = 0; i < N; i++) {
        Vertices[N] = V3(Position, 0) + V3(X, -Data[i], 0);
        X += dx;
    }

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    PushPrimitiveCommand(
        Group,
        Color,
        render_primitive_line_strip,
        Shader,
        vertex_layout_vec3_id,
        N,
        (float*)Vertices,
        0,
        Order,
        0,0,0,
        Thickness
    );

    delete [] Vertices;
}

#endif