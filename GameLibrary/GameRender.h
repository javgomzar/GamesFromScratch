#ifndef GAME_RENDER_H
#define GAME_RENDER_H

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
    uint32 Offset;
    uint32 Count;
    uint32* Pointer;
};

struct vertex_buffer {
    vertex_layout* VertexLayouts;
    memory_arena Vertices[vertex_layout_id_count];
    memory_arena Elements;
    uint32 VertexCount[vertex_layout_id_count];
    uint32 ElementCount;
};

/* Initializes several vertex buffers and element buffers. Returns total memory used.*/
inline memory_index InitializeVertexBuffer(
    memory_arena* Arena,
    vertex_buffer* Buffer,
    vertex_layout* VertexLayouts
) {
    memory_index TotalSize = 0;
    memory_index Size = 0;

    Buffer->VertexLayouts = VertexLayouts;

    // Vertex buffers (one per layout)
    for (int i = 0; i < vertex_layout_id_count; i++) {
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
    
    memory_index Size = VertexCount * VertexBuffer->VertexLayouts[VertexLayoutID].Stride;
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

const memory_index TEXT_BUFFER_SIZE = Kilobytes(16);

struct text_buffer {
    memory_arena Instances[game_font_id_count][FONT_CHARACTERS_COUNT];
    uint32 Count[game_font_id_count][FONT_CHARACTERS_COUNT];
};

void InitializeTextBuffer(memory_arena* Arena, text_buffer* TextBuffer) {
    for (int Font = 0; Font < game_font_id_count; Font++) {
        for (int c = 0; c < FONT_CHARACTERS_COUNT; c++) {
            TextBuffer->Instances[Font][c] = SuballocateMemoryArena(Arena, TEXT_BUFFER_SIZE);
            TextBuffer->Count[Font][c] = 0;
        }
    }
}

void ClearTextBuffer(text_buffer* TextBuffer) {
    for (int Font = 0; Font < game_font_id_count; Font++) {
        for (int c = 0; c < FONT_CHARACTERS_COUNT; c++) {
            ClearArena(&TextBuffer->Instances[Font][c]);
            TextBuffer->Count[Font][c] = 0;
        }
    }
}

void PushTextEntry(text_buffer* TextBuffer, game_font_id FontID, char Char, v2 Pen, float Size, color Color = White) {
    memory_arena* Arena = &TextBuffer->Instances[FontID][Char - '!'];
    float* Data = PushArray(Arena, 7, float);
    *Data++ = Pen.X; *Data++ = Pen.Y;
    *Data++ = Size;
    *Data++ = Color.R; *Data++ = Color.G; *Data++ = Color.B; *Data++ = Color.Alpha;
    TextBuffer->Count[FontID][Char - '!']++;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Render entries                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_RENDER_ENTRIES = 16384;

ENUM(render_command_type,
    render_clear,
    render_draw_primitive,
    render_text,
    render_mesh,
    render_shader_pass,
    render_compute,
    render_target
);

struct render_command {
    render_command_type Type;
    float Priority;
    uint32 Index;
};

ENUM(render_group_target,
    Target_None,
    Target_World,
    Target_Outline,
    Target_Postprocessing_Outline,
    Target_PingPong,
    Target_Fluid,
    Target_Output
);

struct render_clear_command {
    render_group_target Target;
    color Color;
};

ENUM(render_primitive,
    render_primitive_point,
    render_primitive_line,
    render_primitive_line_strip,
    render_primitive_triangle,
    render_primitive_triangle_strip,
    render_primitive_patches
);

FLAGS(render_flags,
    DEPTH_TEST_FLAG,
    STENCIL_TEST_FLAG,
    OVERWRITE_ALPHA_FLAG,

    TEXT_OUTLINE_FLAG,

    DEBUG_BONES_FLAG,

    SKY_FLAG,
    WATER_FLAG
);

struct render_primitive_options {
    render_flags Flags;
    float Thickness = 2.0f;
    transform Transform = IdentityTransform;
    game_bitmap* Texture = nullptr;
    game_heightmap* Heightmap = nullptr;
    game_font* Font = nullptr;
    v2 Pen;
    int PatchParameter = 4;
    float TextSize = 0;
};

struct render_primitive_command {
    render_primitive Primitive = render_primitive_point;
    render_primitive_options Options;
    color Color;
    vertex_buffer_entry VertexEntry = {0};
    float* Vertices = 0;
    element_buffer_entry ElementEntry = {0};
};

struct render_text_options {
    color Color        = White;
    game_font_id Font  = Font_Menlo_Regular_ID;
    bool Outline       = false;
    color OutlineColor = Black;
    float OutlineWidth = 2.0f;
    float Points       = 20.0f;
};

struct render_mesh_options {
    armature* Armature = nullptr;
    color Color = White;
    game_bitmap_id TextureID = Bitmap_Empty_ID;
    transform Transform = IdentityTransform;
    bool Outline = false;
};

struct render_mesh_command {
    game_mesh_id MeshID;
    render_mesh_options Options;
};

ENUM(color_format,
    Color_Format_R,
    Color_Format_RG,
    Color_Format_RGB,
    Color_Format_RGBA
);

struct render_group_target_description {
    render_group_target Target;
    color_format Format;
    bool Multisample;
    bool Depth;
    bool Stencil;
};

ENUM(shader_pass_type,
    shader_pass_empty
);

struct render_shader_pass_command {
    vertex_buffer_entry VertexEntry;
    shader_pass_type Type;
    render_group_target Source;
    render_group_target Target;
    color Color;
    int Level;
    float Width;
    bool ClearTarget;
};

const int COMPUTE_GROUP_SIZE = 16;

ENUM(compute_type,
    compute_kernel,
    compute_outline_init,
    compute_jump_flood,
    compute_outline,
    compute_fft
);

struct render_compute_command {
    compute_type Type;
    render_group_target Source;
    render_group_target Target;
    iv3 nGroups;
    matrix3 Kernel;
    color Color;
    int Level;
    float Width;
};

struct render_target_command {
    vertex_buffer_entry VertexEntry;
    render_group_target Source;
    render_group_target Target;
    bool DebugAttachment;
    bool Attachment; // The source's attachment will also be rendered to the target's attachment
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

const int MAX_CLEAR_COMMANDS = 16;
const int MAX_PRIMITIVE_COMMANDS = 8192;
const int MAX_MESH_COMMANDS = 128;
const int MAX_HEIGHTMAP_COMMANDS = 8;
const int MAX_SHADER_PASS_COMMANDS = 32;
const int MAX_COMPUTE_COMMANDS = 32;
const int MAX_COMPUTE_SHADER_PASS_COMMANDS = 32;
const int MAX_RENDER_TARGET_COMMANDS = 16;

struct render_group {
    render_command Entries[MAX_RENDER_ENTRIES];
    render_clear_command ClearCommands[MAX_CLEAR_COMMANDS];
    render_primitive_command PrimitiveCommands[MAX_PRIMITIVE_COMMANDS];
    render_mesh_command MeshCommands[MAX_MESH_COMMANDS];
    render_shader_pass_command ShaderPassCommands[MAX_SHADER_PASS_COMMANDS];
    render_compute_command ComputeCommands[MAX_COMPUTE_COMMANDS];
    render_target_command TargetCommands[MAX_RENDER_TARGET_COMMANDS];
    render_group_target_description RenderTargets[render_group_target_count];
    vertex_buffer VertexBuffer;
    text_buffer TextBuffer;
    light Light;
    game_assets* Assets;
    game_font* DebugFont;
    int32 Width;
    int32 Height;
    uint32 EntryCount;
    uint32 nClearCommands;
    uint32 nMeshCommands;
    uint32 nPrimitiveCommands;
    uint32 nShaderPassCommands;
    uint32 nComputeCommands;
    uint32 nTargets;
    bool Debug;
    bool DebugNormals;
    bool DebugBones;
    bool DebugColliders;
    bool PushOutline;
    bool RenderText;
};

void InitializeRenderGroup(
    memory_arena* Arena,
    render_group* Group, 
    game_assets* Assets,
    int32 Width,
    int32 Height
) {
    Group->Width = Width;
    Group->Height = Height;

    Group->Assets = Assets;
    Group->DebugFont = GetAsset(Assets, Font_Menlo_Regular_ID);
    
    Group->Debug = false;
    Group->DebugNormals = false;
    Group->DebugColliders = false;
    Group->DebugBones = false;

    // Lighting
    Group->Light = Light(V3(-0.5, -1, 1), White);

    // Vertex & element buffers
    InitializeVertexBuffer(Arena, &Group->VertexBuffer, Assets->VertexLayout);

    // Text buffer
    InitializeTextBuffer(Arena, &Group->TextBuffer);

    // Render targets
    Group->RenderTargets[Target_None] = {};

    Group->RenderTargets[Target_World] = {
        .Target = Target_World,
        .Format = Color_Format_RGBA,
        .Multisample = true,
        .Depth = true,
        .Stencil = false
    };

    Group->RenderTargets[Target_Outline] = {
        .Target = Target_Outline,
        .Format = Color_Format_RGBA,
        .Multisample = true,
        .Depth = true,
        .Stencil = false
    };

    Group->RenderTargets[Target_Postprocessing_Outline] = {
        .Target = Target_Postprocessing_Outline,
        .Format = Color_Format_RGBA,
        .Multisample = false,
        .Depth = false,
        .Stencil = false
    };

    Group->RenderTargets[Target_Output] = {
        .Target = Target_Output,
        .Format = Color_Format_RGB,
        .Multisample = false,
        .Depth = true,
        .Stencil = false
    };

    Group->RenderTargets[Target_PingPong] = {
        .Target = Target_PingPong,
        .Format = Color_Format_RGBA,
        .Multisample = false,
        .Depth = true,
        .Stencil = false
    };

    Group->RenderTargets[Target_Fluid] = {
        .Target = Target_Fluid,
        .Format = Color_Format_RGB,
        .Multisample = false,
        .Depth = true,
        .Stencil = false
    };
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
            if (Command.Index >= MAX_CLEAR_COMMANDS) {
                Raise("Invalid target for clearing");
            }
        } break;
        case render_draw_primitive: {
            if (Command.Index >= MAX_PRIMITIVE_COMMANDS) {
                Raise("Primitive draw command overflow.");
            }
        } break;
        case render_mesh: {
            if (Command.Index >= MAX_MESH_COMMANDS) {
                Raise("Mesh command overflow.");
            }
        } break;
        case render_shader_pass: {
            if (Command.Index >= MAX_SHADER_PASS_COMMANDS) {
                Raise("Shader pass command overflow.");
            }
        } break;
        case render_compute: {
            if (Command.Index >= MAX_COMPUTE_COMMANDS) {
                Raise("Compute command overflow.");
            }
        } break;
        case render_target: {
            if (Command.Index >= MAX_RENDER_TARGET_COMMANDS) {
                Raise("Render target command overflow.");
            }
        } break;
        case render_text:
            if (!Group->RenderText) {
                Group->RenderText = true;
                break;
            }
        default: Raise("Invalid render command type.");
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
                continue;
            }
            else break;
        }
    }
    Group->EntryCount++;
}

void Clear(render_group* Group) {
    ZeroSize(Group->EntryCount * sizeof(render_command), Group->Entries);
    ZeroSize(Group->nClearCommands * sizeof(render_clear_command), Group->ClearCommands);
    ZeroSize(Group->nMeshCommands * sizeof(render_mesh_command), Group->MeshCommands);
    ZeroSize(Group->nPrimitiveCommands * sizeof(render_primitive_command), Group->PrimitiveCommands);
    ZeroSize(Group->nShaderPassCommands * sizeof(render_shader_pass_command), Group->ShaderPassCommands);
    ZeroSize(Group->nComputeCommands * sizeof(render_compute_command), Group->ComputeCommands);
    ZeroSize(Group->nTargets * sizeof(render_target_command), Group->TargetCommands);

    Group->nClearCommands = 0;
    Group->nPrimitiveCommands = 0;
    Group->nMeshCommands = 0;
    Group->nShaderPassCommands = 0;
    Group->nComputeCommands = 0;
    Group->nTargets = 0;
    Group->EntryCount = 0;

    Group->PushOutline = false;
    Group->RenderText = false;
    
    ClearVertexBuffer(&Group->VertexBuffer);
    ClearTextBuffer(&Group->TextBuffer);
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders & render targets                                                                                                                                        |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushRenderTarget(
    render_group* Group,
    render_group_target Source,
    render_group_target Target,
    float Order = SORT_ORDER_PUSH_RENDER_TARGETS
) {
    render_command Command;
    Command.Index = Group->nTargets;
    Command.Priority = Order;
    Command.Type = render_target;

    PushCommand(Group, Command);

    render_target_command TargetCommand;
    TargetCommand.Source = Source;
    TargetCommand.Target = Target;
    TargetCommand.DebugAttachment = false;
    TargetCommand.Attachment = Group->RenderTargets[Source].Depth || Group->RenderTargets[Source].Stencil;

    TargetCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_v2_v2_id);
    float* Data = (float*)TargetCommand.VertexEntry.Pointer;
    
    *Data++ = -1.0f; *Data++ = -1.0f; *Data++ = 0.0f; *Data++ = 0.0f;
    *Data++ =  1.0f; *Data++ = -1.0f; *Data++ = 1.0f; *Data++ = 0.0f;
    *Data++ =  1.0;  *Data++ =  1.0f; *Data++ = 1.0f; *Data++ = 1.0f;
    *Data++ = -1.0f; *Data++ = -1.0f; *Data++ = 0.0f; *Data++ = 0.0f;
    *Data++ =  1.0f; *Data++ =  1.0f; *Data++ = 1.0f; *Data++ = 1.0f;
    *Data++ = -1.0f; *Data++ =  1.0f; *Data++ = 0.0f; *Data++ = 1.0f;

    Group->TargetCommands[Group->nTargets++] = TargetCommand;
}

void PushShaderPass(
    render_group* Group,
    shader_pass_type Type,
    render_group_target Source,
    render_group_target Target,
    color Color,
    float Order = SORT_ORDER_SHADER_PASSES
) {
    render_command Command;
    Command.Type = render_shader_pass;
    Command.Index = Group->nShaderPassCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_shader_pass_command ShaderCommand;
    ShaderCommand.Type = Type;
    ShaderCommand.Color = Color;
    ShaderCommand.Source = Source;
    ShaderCommand.Target = Target;
    
    ShaderCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_v3_v2_id);

    float* Data = (float*)ShaderCommand.VertexEntry.Pointer;
    Data[0] = -1.0f;  Data[1] = -1.0f;  Data[2] = 0.0f;  Data[3] = 0.0f;  Data[4] = 0.0f;
    Data[5] = 1.0f;   Data[6] = -1.0f;  Data[7] = 0.0f;  Data[8] = 1.0f;  Data[9] = 0.0f;
    Data[10] = 1.0;   Data[11] = 1.0f;  Data[12] = 0.0f; Data[13] = 1.0f; Data[14] = 1.0f;
    Data[15] = -1.0f; Data[16] = -1.0f; Data[17] = 0.0f; Data[18] = 0.0f; Data[19] = 0.0f;
    Data[20] = 1.0f;  Data[21] = 1.0f;  Data[22] = 0.0f; Data[23] = 1.0f; Data[24] = 1.0f;
    Data[25] = -1.0f; Data[26] = 1.0f;  Data[27] = 0.0f; Data[28] = 0.0f; Data[29] = 1.0f;
    
    Group->ShaderPassCommands[Group->nShaderPassCommands++] = ShaderCommand;
}

void PushOutlineInitCompute(
    render_group* Group,
    render_group_target Target,
    float Order = 0.0f
) {
    render_command Command = {};
    Command.Type = render_compute;
    Command.Index = Group->nComputeCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_compute_command ComputeCommand = {};
    ComputeCommand.Type = compute_outline_init;
    ComputeCommand.Target = Target_Postprocessing_Outline;
    ComputeCommand.nGroups.X = (Group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Y = (Group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Z = 1;

    Group->ComputeCommands[Group->nComputeCommands++] = ComputeCommand;
}

void PushJumpFloodCompute(
    render_group* Group,
    int Level,
    float Order = 0.0f
) {
    render_command Command = {};
    Command.Type = render_compute;
    Command.Index = Group->nComputeCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_compute_command ComputeCommand = {};
    ComputeCommand.Type = compute_jump_flood;
    ComputeCommand.Target = Target_Postprocessing_Outline;
    ComputeCommand.nGroups.X = (Group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Y = (Group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Z = 1;
    ComputeCommand.Level = Level;

    Group->ComputeCommands[Group->nComputeCommands++] = ComputeCommand;
}

void PushOutlineCompute(
    render_group* Group,
    color Color,
    float Width,
    float Order = 0.0f
) {
    render_command Command = {};
    Command.Type = render_compute;
    Command.Index = Group->nComputeCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_compute_command ComputeCommand = {};
    ComputeCommand.Type = compute_outline;
    ComputeCommand.Target = Target_Postprocessing_Outline;
    ComputeCommand.Color = Color;
    ComputeCommand.Width = Width;
    ComputeCommand.nGroups.X = (Group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Y = (Group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Z = 1;

    Group->ComputeCommands[Group->nComputeCommands++] = ComputeCommand;
}

void PushKernelCompute(
    render_group* Group,
    render_group_target Target,
    matrix3 Kernel,
    float Order = SORT_ORDER_SHADER_PASSES
) {
    render_command Command;
    Command.Type = render_compute;
    Command.Index = Group->nComputeCommands;
    Command.Priority = Order;

    PushCommand(Group, Command);

    render_compute_command ComputeCommand;
    ComputeCommand.Type = compute_kernel;
    ComputeCommand.Target = Target;
    ComputeCommand.Kernel = Kernel;
    ComputeCommand.nGroups.X = (Group->Width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Y = (Group->Height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Z = 1;

    Group->ComputeCommands[Group->nComputeCommands++] = ComputeCommand;
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

    PushKernelCompute(Group, Target, Kernel, Order);
}

void PushFFT(
    render_group* Group, 
    render_group_target Source,
    render_group_target Target
) {
    render_command Command;
    Command.Type = render_compute;
    Command.Index = Group->nComputeCommands;
    Command.Priority = SORT_ORDER_CLEAR;

    PushCommand(Group, Command);

    render_compute_command ComputeCommand;
    ComputeCommand.Type = compute_fft;
    ComputeCommand.Source = Source;
    ComputeCommand.Target = Target;
    ComputeCommand.nGroups.X = 1024 / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Y = 1024 / COMPUTE_GROUP_SIZE;
    ComputeCommand.nGroups.Z = 1;

    Group->ComputeCommands[Group->nComputeCommands++] = ComputeCommand;
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Render commands                                                                                                                                                  |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

void PushClear(render_group* Group, color Color, render_group_target Target = Target_Output) {
    render_command Command;
    Command.Index = Group->nClearCommands;
    Command.Priority = 0.0f;
    Command.Type = render_clear;
    PushCommand(Group, Command);

    render_clear_command Clear;
    Clear.Target = Target;
    Clear.Color = Color;
    
    Group->ClearCommands[Group->nClearCommands++] = Clear;
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
    vertex_layout_id LayoutID,
    uint32 nVertices,
    uint32 nElements = 0,
    float Order = 0.0,
    render_primitive_options Options = {}
) {
    render_command Command;
    Command.Index = Group->nPrimitiveCommands;
    Command.Priority = Order;
    Command.Type = render_draw_primitive;

    PushCommand(Group, Command);

    render_primitive_command* PrimitiveCommand = &Group->PrimitiveCommands[Group->nPrimitiveCommands++];
    *PrimitiveCommand = {};
    PrimitiveCommand->Options = Options;
    PrimitiveCommand->Primitive = Primitive;
    PrimitiveCommand->Color = Color;

    if (nVertices > 0) {
        if (Options.Heightmap || Options.Flags & WATER_FLAG) {
            PrimitiveCommand->VertexEntry.Count = nVertices;
            PrimitiveCommand->VertexEntry.LayoutID = LayoutID;
        }
        else {
            PrimitiveCommand->VertexEntry = PushVertexEntry(&Group->VertexBuffer, nVertices, LayoutID);
            PrimitiveCommand->Vertices = (float*)PrimitiveCommand->VertexEntry.Pointer;
        }
    }

    if (nElements > 0) {
        if (Options.Heightmap || Options.Flags & WATER_FLAG) {
            PrimitiveCommand->ElementEntry.Count = nElements;
        }
        else {
            PrimitiveCommand->ElementEntry = PushElementEntry(&Group->VertexBuffer, nElements);
        }
    }

    return PrimitiveCommand;
}

void PushPoint(render_group* Group, v2 Point, color Color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
    float* Vertices = (float*)PushPrimitiveCommand(
        Group,
        render_primitive_point, 
        Color,
        vertex_layout_v2_id, 
        1,
        0,
        Order
    )->VertexEntry.Pointer;
    Vertices[0] = Point.X;
    Vertices[1] = Point.Y;
}

void PushPoint(render_group* Group, v3 Point, color Color, float Order = SORT_ORDER_DEBUG_OVERLAY) {
    float* Vertices = PushPrimitiveCommand(
        Group, 
        render_primitive_point,
        Color,
        vertex_layout_v3_id, 
        1,
        0,
        Order,
        { .Flags = DEPTH_TEST_FLAG }
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
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        Color,
        vertex_layout_v2_id,
        2,
        0,
        Order,
        { .Thickness = Thickness }
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
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        Color,
        vertex_layout_v3_id,
        2,
        0,
        Order,
        { 
            .Flags = DEPTH_TEST_FLAG,
            .Thickness = Thickness
        }
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
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_FLAG;
    float* Vertices = PushPrimitiveCommand(
        Group, 
        render_primitive_triangle,
        Color,
        vertex_layout_v3_id,
        3,
        0,
        Order,
        { .Flags = DEPTH_TEST_FLAG }
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
    float* Vertices = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        vertex_layout_v2_id,
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

    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        vertex_layout_v2_id,
        N+1,
        3*N,
        Order
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

    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        vertex_layout_v3_id,
        N+1,
        3*N,
        Order,
        { .Flags = DEPTH_TEST_FLAG }
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

    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        Color,
        vertex_layout_v2_id,
        N+1,
        0,
        Order,
        { .Thickness = Thickness }
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

    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        Color,
        vertex_layout_v3_id,
        N+1,
        0,
        Order,
        { .Flags = DEPTH_TEST_FLAG }
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

    float* Data = PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        Color,
        vertex_layout_v3_id,
        N,
        0,
        Order,
        { .Flags = DEPTH_TEST_FLAG }
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
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        vertex_layout_v2_id,
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
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        vertex_layout_v3_id,
        4,
        6,
        Order,
        {
            .Flags = DEPTH_TEST_FLAG
        }
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
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        Color,
        vertex_layout_v2_id,
        5,
        0,
        Order,
        { .Thickness = Thickness }
    )->Vertices;
    
    Vertices[0] = { Rect.Left             , Rect.Top               };
    Vertices[1] = { Rect.Left + Rect.Width, Rect.Top               };
    Vertices[2] = { Rect.Left + Rect.Width, Rect.Top + Rect.Height };
    Vertices[3] = { Rect.Left             , Rect.Top + Rect.Height };
    Vertices[4] = { Rect.Left             , Rect.Top               };
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
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        White,
        vertex_layout_v2_v2_id,
        4,
        6,
        Order,
        { .Texture = Bitmap }
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
    Elements[0] = Result->VertexEntry.Offset + 0;
    Elements[1] = Result->VertexEntry.Offset + 1;
    Elements[2] = Result->VertexEntry.Offset + 2;
    Elements[3] = Result->VertexEntry.Offset + 3;
    Elements[4] = Result->VertexEntry.Offset + 2;
    Elements[5] = Result->VertexEntry.Offset + 1;
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

void _PushText(
    render_group* Group,
    v2 Position,
    const char* String,
    render_text_options Options = {}
) {
    if (!Group->RenderText) {
        render_command Command = {};
        Command.Type = render_text;
        Command.Index = 0;
        Command.Priority = SORT_ORDER_DEBUG_OVERLAY + 7000.0f;

        PushCommand(Group, Command);
    }

    uint32 nCharacters = 0;
    uint32 StringLength = strlen(String);
    for (int i = 0; i < StringLength; i++) {
        if (String[i] == '\0') break;
        if (String[i] >= '!' && String[i] <= '~') nCharacters++;
    }

    game_font* Font = GetAsset(Group->Assets, Options.Font);
    color Color = Options.Color;
    
    v2 Pen = Position;
    float DPI = 96;
    float Size = Options.Points * (DPI / 72.0f) / Font->UnitsPerEm;
    float LineJump = Font->LineJump * Size;

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
            // if (Options.Wrapped && (Pen.X + HorizontalAdvance > Group->Width)) {
            //     Pen.X = Position.X;
            //     Pen.Y += LineJump;
            // }

            if (pCharacter->nContours > 0) {
                render_primitive_options PrimitiveOptions = {};
                PrimitiveOptions.Font = Font;
                PrimitiveOptions.TextSize = Size;
                PrimitiveOptions.PatchParameter = 3;
                PrimitiveOptions.Pen = Pen;
                PrimitiveOptions.Thickness = Options.OutlineWidth;

                PushTextEntry(&Group->TextBuffer, Options.Font, c, Pen, Size, Options.Color);

                if (Options.Outline) {
                    render_primitive_command* Command = PushPrimitiveCommand(
                        Group,
                        render_primitive_patches,
                        Options.OutlineColor,
                        vertex_layout_v2_v2_id,
                        3 * pCharacter->nOnCurve,
                        TEXT_OUTLINE_FLAG,
                        SORT_ORDER_DEBUG_OVERLAY,
                        PrimitiveOptions
                    );

                    Command->VertexEntry.Offset = pCharacter->VertexOffset;
                }
            }

            Pen.X += pCharacter->Width * Size;
        }
    }
}

#define PushText(Group, Position, String, ...) _PushText(Group, Position, String, { __VA_ARGS__ })

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
    PushText(Group, Position + V2(5.0f, 15.0f), Description, .Points = 10);

    int Points = 8;
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    std::string Text = std::format("{:.2f}%", 100 * FillPercentage);
    float Width = GetTextWidth(Text.c_str(), Font, Points);
    PushText(Group, Position + V2(Rect.Width - Width - 5.0f, 15.0f), Text.c_str(), .Points = 8);
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
    float Points = 8;
    PushRect(Group, Rect, DarkGray);
    rectangle SmallRect = Rect;
    SmallRect.Width *= FillPercentage;
    PushRect(Group, Rect, Color);

    v2 Position = LeftTop(Rect);
    PushText(Group, Position + V2(5.0f, 15.0f), Description, .Points = Points);

    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    std::string Text = std::format("{}/{}", Used, Max);
    float Width = GetTextWidth(Text.c_str(), Font, Points);
    PushText(Group, Position + V2(Rect.Width - Width - 5.0f, 15.0f), Text.c_str(), .Points = Points);
}

void PushFillbar(
    render_group* Group,
    basis CameraBasis,
    int Used,
    int Max,
    v3 LeftTop,
    float Width,
    float Height,
    color Color = Red
) {
    float FillPercentage = (float)Used / (float)Max;
    PushRect(Group, LeftTop, CameraBasis.X, CameraBasis.Y, Width, Height, DarkGray);
    float SmallWidth = FillPercentage * Width;
    PushRect(Group, LeftTop + 0.01f * CameraBasis.Z, CameraBasis.X, CameraBasis.Y, SmallWidth, Height, Red);
}

void PushCubeOutline(
    render_group* Group,
    v3 Position,
    scale Scale = GetScale(1.0),
    color Color = White,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        Color,
        vertex_layout_v3_id,
        8,
        24,
        Order,
        { .Flags = DEPTH_TEST_FLAG }
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

void _PushMesh(
    render_group* Group,
    game_mesh_id MeshID,
    render_mesh_options Options
) {
    
    render_command Command = {};
    Command.Type = render_mesh;
    Command.Priority = Options.Outline ? SORT_ORDER_OUTLINED_MESHES : SORT_ORDER_MESHES;
    Command.Index = Group->nMeshCommands++;
    PushCommand(Group, Command);

    render_mesh_command* MeshCommand = &Group->MeshCommands[Command.Index];
    MeshCommand->MeshID = MeshID;
    MeshCommand->Options = Options;
    MeshCommand->Options.Outline = false;

    // Outlines
    if (Options.Outline) {
        render_command OutlineCommand = {};
        OutlineCommand.Type = render_mesh;
        OutlineCommand.Priority = SORT_ORDER_CLEAR + 1.0f;
        OutlineCommand.Index = Group->nMeshCommands++;
        PushCommand(Group, OutlineCommand);

        render_mesh_command* OutlineMeshCommand = &Group->MeshCommands[OutlineCommand.Index];
        OutlineMeshCommand->MeshID = MeshID;
        OutlineMeshCommand->Options.Transform = Options.Transform;
        OutlineMeshCommand->Options.Color = White;
        OutlineMeshCommand->Options.Outline = true;

        if (!Group->PushOutline) {
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
            PushOutlineCompute(Group, White, 4.0f, JumpOrder + 1.0f);

            PushRenderTarget(Group, Target_Postprocessing_Outline, Target_World, SORT_ORDER_OUTLINED_MESHES - 0.1f);
            Group->PushOutline = true;
        }
    }

    // Debug bones rendering
    if (Group->Debug && Group->DebugBones && Options.Armature) {
        render_primitive_options PrimitiveOptions = {};
        PrimitiveOptions.Thickness = 2.5f;
        PrimitiveOptions.Flags = DEBUG_BONES_FLAG;
        v3* Vertices = (v3*)PushPrimitiveCommand(
            Group, 
            render_primitive_line,
            Black,
            vertex_layout_v3_id,
            2 * Options.Armature->nBones,
            0,
            SORT_ORDER_DEBUG_OVERLAY,
            PrimitiveOptions
        )->Vertices;

        for (int i = 0; i < Options.Armature->nBones; i++) {
            bone Bone = Options.Armature->Bones[i];
            transform BoneTransform = Bone.Transform;
            Vertices[2*i] = BoneTransform * Options.Transform * Bone.Segment.Head;
            Vertices[2*i+1] = BoneTransform * Options.Transform * Bone.Segment.Tail;
        }
    }
}

#define PushMesh(Group, MeshID, ...) _PushMesh(Group, MeshID, { __VA_ARGS__ })

void PushHeightmap(
    render_group* Group, 
    game_heightmap* Heightmap,
    v3 LeftBottom,
    scale S,
    float Order = SORT_ORDER_MESHES
) {
    uint32 nVertices = HEIGHTMAP_RESOLUTION*HEIGHTMAP_RESOLUTION;
    uint32 nElements = 4*(HEIGHTMAP_RESOLUTION-1)*(HEIGHTMAP_RESOLUTION-1);

    PushPrimitiveCommand(
        Group, 
        render_primitive_patches,
        White,
        vertex_layout_v3_v2_id, 
        nVertices,
        nElements,
        Order,
        {
            .Flags = (render_flags)(DEPTH_TEST_FLAG),
            .Transform = GetTransform(LeftBottom, Quaternion(1.0f), S),
            .Heightmap = Heightmap,
            .PatchParameter = 4,
        }
    );
}

void PushHeightmap(
    render_group* Group, 
    game_heightmap_id ID,
    v3 LeftBottom,
    scale S,
    float Order = SORT_ORDER_MESHES
) {
    game_heightmap* Heightmap = GetAsset(Group->Assets, ID);
    PushHeightmap(Group, Heightmap, LeftBottom, S, Order);
}

void PushWater(render_group* Group, v3 Position, scale S) {
    uint32 nVertices = HEIGHTMAP_RESOLUTION*HEIGHTMAP_RESOLUTION;
    uint32 nElements = 4*(HEIGHTMAP_RESOLUTION-1)*(HEIGHTMAP_RESOLUTION-1);

    PushPrimitiveCommand(
        Group, 
        render_primitive_patches,
        White,
        vertex_layout_v3_v2_id, 
        nVertices,
        nElements,
        SORT_ORDER_MESHES,
        {
            .Flags = (render_flags)(DEPTH_TEST_FLAG | WATER_FLAG),
            .Transform = GetTransform(Position, Quaternion(1.0f), S),
            .PatchParameter = 4,
        }
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

void PushSky(render_group* Group) {
    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        White,
        vertex_layout_v3_id,
        8,
        36,
        SORT_ORDER_MESHES,
        {
            .Flags = (render_flags)(SKY_FLAG | DEPTH_TEST_FLAG),
        }
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

void PushCollider(render_group* Group, collider Collider, transform Transform, color Color) {
    v3 Position = Transform.Translation + Collider.Offset;
    switch (Collider.Type) {
        case Rect_Collider: {
            PushRectOutline(Group, Rectangle(Collider), Color);
        } break;

        case Cube_Collider: {
            scale Scale = GetScale(Collider.Cube.HalfWidth, Collider.Cube.HalfHeight, Collider.Cube.HalfDepth);
            transform Transform = GetTransform(Position);
            Transform.Scale = Scale;
            PushMesh(Group, Mesh_Cube_ID, .Color = Yellow, .Transform = Transform);
        } break;

        case Sphere_Collider: {
            PushCircunference(Group, Position, V3(1,0,0), Collider.Sphere.Radius, Color);
            PushCircunference(Group, Position, V3(0,1,0), Collider.Sphere.Radius, Color);
            PushCircunference(Group, Position, V3(0,0,1), Collider.Sphere.Radius, Color);
        } break;

        case Capsule_Collider: {
            v3 Head = Transform * Collider.Capsule.Segment.Head;
            v3 Tail = Transform * Collider.Capsule.Segment.Tail;

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

            Collider.Capsule.Segment = Transform * Collider.Capsule.Segment;
        } break;

        default: Assert(false);
    }
}

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Debug                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const float DEBUG_ENTRIES_TEXT_POINTS = 10.0f;

void PushDebugVector(render_group* Group, v2 Vector, v2 Position, color Color) {
    v2 Orthogonal = perp(normalize(V2(Vector.X, Vector.Y)));
    float OrthogonalLength = 0.005333f * Group->Height;

    int Thickness = max(1.0f, 0.0025f * Group->Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, SORT_ORDER_DEBUG_OVERLAY);
    triangle2 Arrowhead1 = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector - OrthogonalLength * Orthogonal
    };
    PushTriangle(
        Group,
        Arrowhead1,
        Color, 
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
        Color, 
        SORT_ORDER_DEBUG_OVERLAY
    );
}

void PushDebugVector(render_group* Group, basis CameraBasis, v3 Vector, v3 Position, color Color) {
    float Height = Group->Height;

    v2 CameraCoordinates = perp(V2(dot(Vector, CameraBasis.X), dot(Vector, CameraBasis.Y)));
    v3 Orthogonal = normalize(CameraCoordinates.X * CameraBasis.X + CameraCoordinates.Y * CameraBasis.Y);
    float OrthogonalLength = (modulus(Vector) / 15.0f);

    int Thickness = max(1.0, 0.0025 * Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, SORT_ORDER_MESHES);
    triangle3 Triangle = {
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
    basis CameraBasis,
    v3 Position,
    double l, double r, double b, double t, double n, double f
) {
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_FLAG;
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        White,
        vertex_layout_v3_v2_id,
        9,
        24,
        SORT_ORDER_DEBUG_OVERLAY,
        Options
    );

    v3 nv = -n * CameraBasis.Z;
    v3 rv = r * CameraBasis.X;
    v3 lv = -l * CameraBasis.X;
    v3 tv = t * CameraBasis.Y * ((double)Group->Height / (double)Group->Width);
    v3 bv = -b * CameraBasis.Y * ((double)Group->Height / (double)Group->Width);
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

void PushDebugGrid(render_group* Group, float Alpha) {
    const int nVertices = 404;

    v3* Vertices = (v3*)PushPrimitiveCommand(
        Group,
        render_primitive_line,
        ChangeAlpha(White, 0.2f),
        vertex_layout_v3_id,
        nVertices,
        0,
        SORT_ORDER_MESHES,
        {
            .Flags = (render_flags)(DEPTH_TEST_FLAG | OVERWRITE_ALPHA_FLAG),
            .Thickness = 1.0f
        }
    )->Vertices;

    for (int i = 0; i <= 100; i++) {
        Vertices[4*i  ] = V3(50-i, 0, -50);
        Vertices[4*i+1] = V3(50-i, 0, 50);
        Vertices[4*i+2] = V3(-50, 0, 50-i);
        Vertices[4*i+3] = V3(50, 0, 50-i);
    }
}

void PushDebugTarget(render_group* Group, render_group_target Target, bool Attachment = false) {
    render_command Command;
    Command.Index = Group->nTargets;
    Command.Priority = SORT_ORDER_PUSH_RENDER_TARGETS + 0.1f;
    Command.Type = render_target;

    PushCommand(Group, Command);

    render_target_command TargetCommand = {};
    TargetCommand.Source = Target;
    TargetCommand.Target = Target_Output;
    TargetCommand.DebugAttachment = Attachment;
    TargetCommand.Attachment = false;
    TargetCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_v2_v2_id);

    float* Vertices = (float*)TargetCommand.VertexEntry.Pointer;
    *Vertices++ = -1.0f; *Vertices++ = -1.0f; *Vertices++ = 0.0f; *Vertices++ = 0.0f,
    *Vertices++ = -0.5f; *Vertices++ = -1.0f; *Vertices++ = 1.0f; *Vertices++ = 0.0f;
    *Vertices++ = -0.5f; *Vertices++ = -0.5f; *Vertices++ = 1.0f; *Vertices++ = 1.0f;
    *Vertices++ = -1.0f; *Vertices++ = -1.0f; *Vertices++ = 0.0f; *Vertices++ = 0.0f;
    *Vertices++ = -0.5f; *Vertices++ = -0.5f; *Vertices++ = 1.0f; *Vertices++ = 1.0f;
    *Vertices++ = -1.0f; *Vertices++ = -0.5f; *Vertices++ = 0.0f; *Vertices++ = 1.0f;

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
    render_primitive_options Options = {};
    Options.Thickness = Thickness;
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        Color,
        vertex_layout_v2_id,
        N,
        0,
        Order,
        Options
    )->Vertices;

    float X = 0;
    for (int i = 0; i < N; i++) {
        Vertices[i] = Position + V2(X, -Data[i]);
        X += dx;
    }
}

inline uint16 ComputeTimeRecordsColWidths(
    game_font* Font, float Points,
    uint16 nRecords, time_record* TimeRecords,
    float* FunctionColWidth,
    float* HitsColWidth,
    float* MCyclesColWidth,
    float* FileColWidth
) {
    uint16 TotalRecords = 0;
    std::string Text;
    float Width = 0;
    for (int i = 0; i < nRecords; i++) {
        time_record* Record = TimeRecords + i;
        if (Record->HitCount > 0) {
            TotalRecords += 1;
            Width = GetTextWidth(Record->FunctionName, Font, Points);
            if (Width > *FunctionColWidth) *FunctionColWidth = Width;
            Text = std::format("{}", Record->HitCount);
            Width = GetTextWidth(Text.c_str(), Font, Points);
            if (Width > *HitsColWidth) *HitsColWidth = Width;
            Text = std::format("{:.2f}", Record->CycleCount / 1000000.0f);
            Width = GetTextWidth(Text.c_str(), Font, Points);
            if (Width > *MCyclesColWidth) *MCyclesColWidth = Width;
            Text = std::format("{}:{}", Record->FileName, Record->LineNumber);
            Width = GetTextWidth(Text.c_str(), Font, Points);
            if (Width > *FileColWidth) *FileColWidth = Width;
        }
    }
    return TotalRecords;
}

void PushTimeRecordsPartial(
    render_group* Group,
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
    std::string Text;
    float Width = 0;
    for (int i = 0; i < nTimeRecords; i++) {
        time_record* Record = TimeRecords + i;

        if (Record->HitCount > 0) {
            PushText(Group, V2(*X, *Y), Record->FunctionName, .Font = Group->DebugFont->ID, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X += FunctionColWidth + HMargin;

            Text = std::format("{}", Record->HitCount);
            Width = GetTextWidth(Text.c_str(), Group->DebugFont, DEBUG_ENTRIES_TEXT_POINTS);
            *X += HitsColWidth - Width;
            PushText(Group, V2(*X, *Y), Text.c_str(), .Font = Group->DebugFont->ID, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X += Width + HMargin;
            
            Text = std::format("{:.2f}", Record->CycleCount / 1000000.0f);
            Width = GetTextWidth(Text.c_str(), Group->DebugFont, DEBUG_ENTRIES_TEXT_POINTS);
            *X += MCyclesColWidth - Width;
            PushText(Group, V2(*X, *Y), Text.c_str(), .Font = Group->DebugFont->ID, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X += Width + HMargin;

            Text = std::format("{}:{}", Record->FileName, Record->LineNumber);
            PushText(Group, V2(*X, *Y), Text.c_str(), .Font = Group->DebugFont->ID, .Points = DEBUG_ENTRIES_TEXT_POINTS);
            *X = Group->Width - TotalWidth + HMargin;

            *Y += RecordHeight;

            Record->HitCount = 0;
            Record->CycleCount = 0;
        }
    }
    *X = Group->Width - TotalWidth + HMargin;
}

void PushTimeRecords(
    render_group* Group, 
    uint16 nTimeRecordsLibrary,
    time_record* TimeRecordsLibrary,
    uint16 nTimeRecordsPlatform,
    time_record* TimeRecordsPlatform
) {
    char Buffer[512];
    const float Points = DEBUG_ENTRIES_TEXT_POINTS;
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    
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
    nTotalRecords += ComputeTimeRecordsColWidths(Font, Points, nTimeRecordsLibrary, TimeRecordsLibrary, 
        &FunctionColWidth, &HitsColWidth, &MCyclesColWidth, &FileColWidth);
    nTotalRecords += ComputeTimeRecordsColWidths(Font, Points, nTimeRecordsPlatform, TimeRecordsPlatform, 
        &FunctionColWidth, &HitsColWidth, &MCyclesColWidth, &FileColWidth);

    float HMargin = 10.0f;
    float VMargin = 10.0f;

    float TotalWidth = FunctionColWidth + HitsColWidth + MCyclesColWidth + FileColWidth + 5 * HMargin;
    float RecordHeight = GetCharMaxHeight(Font, Points);
    float TotalHeight = (RecordHeight) * (nTotalRecords + 1) + 2 * VMargin;

    PushRect(Group, { Group->Width - TotalWidth, Group->Height - TotalHeight, TotalWidth, TotalHeight }, ChangeAlpha(Black, 0.7f));

    float RecordX = Group->Width - TotalWidth + HMargin;
    float RecordY = Group->Height - TotalHeight + RecordHeight + 0.5f * VMargin;

    // Push header
    float X = RecordX + 0.5f * (FunctionColWidth - FunctionHeaderWidth);
    PushText(Group, V2(X, RecordY), "Function", .Font = Group->DebugFont->ID, .Points = Points);
    RecordX += FunctionColWidth + HMargin;

    X = RecordX + 0.5f * (HitsColWidth - HitsHeaderWidth);
    PushText(Group, V2(X, RecordY), "Hits", .Font = Group->DebugFont->ID, .Points = Points);
    RecordX += HitsColWidth + HMargin;

    X = RecordX + 0.5f * (MCyclesColWidth - MCyclesHeaderWidth);
    PushText(Group, V2(X, RecordY), "MCycles", .Font = Group->DebugFont->ID, .Points = Points);
    RecordX += MCyclesColWidth + HMargin;

    X = RecordX + 0.5f * (FileColWidth - FileHeaderWidth);
    PushText(Group, V2(X, RecordY), "File", .Font = Group->DebugFont->ID, .Points = Points);

    RecordX = Group->Width - TotalWidth + HMargin;
    RecordY += RecordHeight + 0.5f * VMargin;

    // Push table
    PushTimeRecordsPartial(Group, TimeRecordsLibrary, nTimeRecordsLibrary, &RecordX, &RecordY, TotalWidth, RecordHeight, 
        FunctionColWidth, HitsColWidth, MCyclesColWidth, RecordHeight, HMargin, VMargin);
    PushTimeRecordsPartial(Group, TimeRecordsPlatform, nTimeRecordsLibrary, &RecordX, &RecordY, TotalWidth, RecordHeight, 
        FunctionColWidth, HitsColWidth, MCyclesColWidth, RecordHeight, HMargin, VMargin);
}

#endif