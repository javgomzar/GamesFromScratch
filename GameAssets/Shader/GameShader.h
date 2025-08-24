#include "GamePlatform.h"
#include "Tokenizer.h"
#include <stdio.h>

#include "Win32Debug.h"

#ifndef GAME_SHADER
#define GAME_SHADER

enum game_shader_id {
    // Vertex shaders
    Vertex_Shader_Passthrough_ID,
    Vertex_Shader_Screen_ID,
    Vertex_Shader_Screen_Texture_ID,
    Vertex_Shader_Perspective_ID,
    Vertex_Shader_Bones_ID,
    Vertex_Shader_Barycentric_ID,
#if GAME_RENDER_API_VULKAN
    Vertex_Shader_Vulkan_Test_ID,
#endif

    // Tessellation control shaders
    TESC_Heightmap_ID,
    TESC_Bezier_ID,

    // Tessellation evaluation shaders
    TESE_Heightmap_ID,
    TESE_Trochoidal_ID,
    TESE_Bezier_ID,

    // Geometry shaders
    Geometry_Shader_Test_ID,
    Geometry_Shader_Debug_Normals_ID,

    // Fragment shaders
    Fragment_Shader_Antialiasing_ID,
    Fragment_Shader_Single_Color_ID,
    Fragment_Shader_Texture_ID,
    Fragment_Shader_Framebuffer_Attachment_ID,
    Fragment_Shader_Outline_ID,
    Fragment_Shader_Kernel_ID,
    Fragment_Shader_Mesh_ID,
    Fragment_Shader_Sphere_ID,
    Fragment_Shader_Jump_Flood_ID,
    Fragment_Shader_Heightmap_ID,
    Fragment_Shader_Sea_ID,
    Fragment_Shader_Bezier_Exterior_ID,
    Fragment_Shader_Bezier_Interior_ID,
#if GAME_RENDER_API_VULKAN
    Fragment_Shader_Vulkan_Test_ID,
#endif

    game_shader_id_count
};

enum game_shader_pipeline_id {
    Shader_Pipeline_Antialiasing_ID,
    Shader_Pipeline_Framebuffer_ID,
    Shader_Pipeline_Screen_Single_Color_ID,
    Shader_Pipeline_World_Single_Color_ID,
    Shader_Pipeline_Bones_Single_Color_ID,
    Shader_Pipeline_Texture_ID,
    Shader_Pipeline_Mesh_ID,
    Shader_Pipeline_Mesh_Bones_ID,
    Shader_Pipeline_Sphere_ID,
    Shader_Pipeline_Jump_Flood_ID,
    Shader_Pipeline_Outline_ID,
    Shader_Pipeline_Heightmap_ID,
    Shader_Pipeline_Trochoidal_ID,
    Shader_Pipeline_Text_Outline_ID,
    Shader_Pipeline_Debug_Normals_ID,
    Shader_Pipeline_Bezier_Exterior_ID,
    Shader_Pipeline_Bezier_Interior_ID,
    Shader_Pipeline_Solid_Text_ID,
#if GAME_RENDER_API_VULKAN
    Shader_Pipeline_Vulkan_Test_ID,
#endif

    game_shader_pipeline_id_count
};

enum game_compute_shader_id {
    Compute_Shader_Outline_Init_ID,
    Compute_Shader_Jump_Flood_ID,
    Compute_Shader_Kernel_ID,
    Compute_Shader_Test_ID,

    game_compute_shader_id_count
};

// +-------------------------------------------------------------------------------------------------------------------------------------------+
// | Vertex layouts                                                                                                                            |
// +-------------------------------------------------------------------------------------------------------------------------------------------+

enum shader_type {
    shader_type_float,
    shader_type_vec2,
    shader_type_vec3,
    shader_type_vec4,
    shader_type_int,
    shader_type_ivec2,
    shader_type_ivec3,
    shader_type_ivec4,
    shader_type_mat2,
    shader_type_mat3,
    shader_type_mat4,

    shader_type_count
};

const char* ShaderTypeTokens[shader_type_count] = {
    "float",
    "vec2",
    "vec3",
    "vec4",
    "int",
    "ivec2",
    "ivec3",
    "ivec4",
    "mat2",
    "mat3",
    "mat4"
};

shader_type GetShaderType(token Token) {
    for (int i = 0; i < shader_type_count; i++) {
        if (Token == ShaderTypeTokens[i]) {
            return (shader_type)i;
        }
    }
    char ErrorBuffer[256];
    sprintf_s(ErrorBuffer, "Invalid shader attribute type token `%s`.", Token.Text);
    Raise(ErrorBuffer);
    return shader_type_count;
}

uint32 GetShaderTypeSizeInBytes(shader_type Type) {
    switch (Type) {
        case shader_type_float: { return 4; } break;
        case shader_type_vec2:  { return 8; } break;
        case shader_type_vec3:  { return 12; } break;
        case shader_type_vec4:  { return 16; } break;
        case shader_type_int:   { return 4; } break;
        case shader_type_ivec2: { return 8; } break;
        case shader_type_ivec3: { return 12; } break;
        case shader_type_ivec4: { return 16; } break;
        case shader_type_mat2:  { return 16; } break;
        case shader_type_mat3:  { return 36; } break;
        case shader_type_mat4:  { return 64; } break;
        default: Raise("Invalid vertex attribute type `shader_type_count`.");
    }
    return 0;
}

// Returns number of elements for a given shader type. For example, output is 3 for type `vec3`.
int GetShaderTypeSize(shader_type Type) {
    switch (Type) {
        case shader_type_float: { return 1; } break;
        case shader_type_vec2:  { return 2; } break;
        case shader_type_vec3:  { return 3; } break;
        case shader_type_vec4:  { return 4; } break;
        case shader_type_int:   { return 1; } break;
        case shader_type_ivec2: { return 2; } break;
        case shader_type_ivec3: { return 3; } break;
        case shader_type_ivec4: { return 4; } break;
        case shader_type_mat2:  { return 4; } break;
        case shader_type_mat3:  { return 9; } break;
        case shader_type_mat4:  { return 16; } break;
        default: Raise("Invalid vertex attribute type `shader_type_count`.");
    }
    return 0;
}

struct vertex_attribute {
    shader_type Type;
    uint32 Location;
    uint32 Size;
    uint32 Offset;
};

bool operator!=(vertex_attribute Attribute1, vertex_attribute Attribute2) {
    return 
        Attribute1.Type != Attribute2.Type ||
        Attribute1.Location != Attribute2.Location ||
        Attribute1.Size != Attribute2.Size ||
        Attribute1.Offset != Attribute2.Offset;
}

enum vertex_layout_id {
    vertex_layout_vec2_id,
    vertex_layout_vec2_vec2_id,
    vertex_layout_vec3_id,
    vertex_layout_vec3_vec2_id,
    vertex_layout_vec3_vec2_vec3_id,
    vertex_layout_vec3_vec4_id,
    vertex_layout_vec4_id,
    vertex_layout_bones_id,

    vertex_layout_id_count
};

const uint8 MAX_VERTEX_ATTRIBUTES = 16;
struct vertex_layout {
    vertex_layout_id ID;
    vertex_attribute Attributes[MAX_VERTEX_ATTRIBUTES];
    uint32 Stride;
    uint8 nAttributes;
};

void AddAttribute(vertex_layout* VertexLayout, shader_type Type) {
    vertex_attribute* Attribute = &VertexLayout->Attributes[VertexLayout->nAttributes];
    Attribute->Location = VertexLayout->nAttributes++;
    Attribute->Type = Type;
    Attribute->Size = GetShaderTypeSizeInBytes(Type);
    Attribute->Offset = VertexLayout->Stride;
    VertexLayout->Stride += Attribute->Size;
}

vertex_layout VertexLayout(uint8 nAttributes, ...) {
    vertex_layout Result = {};
    Result.Stride = 0;
    Result.nAttributes = 0;

    va_list Types;
    va_start(Types, nAttributes);

    for (int i = 0; i < nAttributes; i++) {
        shader_type Type = va_arg(Types, shader_type);
        AddAttribute(&Result, Type);
    }
    return Result;
}

bool operator==(vertex_layout Layout1, vertex_layout Layout2) {
    if (Layout1.Stride == Layout2.Stride && Layout1.nAttributes == Layout2.nAttributes) {
        for (int i = 0; i < Layout1.nAttributes; i++) {
            if (Layout1.Attributes[i] != Layout2.Attributes[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// +-------------------------------------------------------------------------------------------------------------------------------------------+
// | Shader uniforms                                                                                                                           |
// +-------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_SHADER_UNIFORM_BLOCK_MEMBERS = 16;
const int SHADER_SETS = 3;
const int MAX_SHADER_SET_BINDINGS = 8;
const int MAX_SHADER_UBOS = 8;
const int MAX_SHADER_SAMPLERS = 4;

struct shader_uniform_block {
    uint32 Set;
    uint32 Binding;
    uint32 nMembers;
    shader_type Member[MAX_SHADER_UNIFORM_BLOCK_MEMBERS];
};

bool operator==(shader_uniform_block UBO1, shader_uniform_block UBO2) {
    if (UBO1.Set == UBO2.Set && UBO1.Binding == UBO2.Binding && UBO1.nMembers == UBO2.nMembers) {
        for (int i = 0; i < UBO1.nMembers; i++) {
            if (UBO1.Member[i] != UBO2.Member[i]) return false;
        }
        return true;
    }
    return false;
}

bool operator!=(shader_uniform_block UBO1, shader_uniform_block UBO2) {
    if (UBO1.Set != UBO2.Set || UBO1.Binding != UBO2.Binding || UBO1.nMembers != UBO2.nMembers) {
        return true;
    }
    else {
        for (int i = 0; i < UBO1.nMembers; i++) {
            if (UBO1.Member[i] != UBO2.Member[i]) return true;
        }
    }
    return false;
}

struct shader_uniform_sampler {
    uint32 Set;
    uint32 Binding;
};

const int SHADER_UNIFORM_BLOCKS = 9;

struct alignas(16) global_uniforms {
    matrix4 projection;
    matrix4 view;
    v2 resolution;
    float time;
};

struct alignas(16) light_uniforms {
    alignas(16) v3 direction;
    alignas(16) v3 color;
    float ambient;
    float diffuse;
};

struct alignas(16) color_uniforms {
    v4 color;
};

struct alignas(16) model_uniforms {
    matrix4 model;
    matrix4 normal;
};

struct alignas(16) bone_uniforms {
    matrix4 bone_transforms[32];
    matrix4 bone_normal_transforms[32];
    alignas(16) int n_bones;
};

struct alignas(16) outline_uniforms {
    float width;
    int level;
};

struct alignas(16) kernel_uniforms {
    float XX, XY, XZ, _Pad0;
    float YX, YY, YZ, _Pad1;
    float ZX, ZY, ZZ, _Pad2;
};

struct alignas(16) antialiasing_uniforms {
    int samples;
};

struct alignas(16) text_uniforms {
    v2 Pen;
    float Size;
};

// +-------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders                                                                                                                                   |
// +-------------------------------------------------------------------------------------------------------------------------------------------+

enum game_shader_type {
    Vertex_Shader,
    Tessellation_Control_Shader,
    Tessellation_Evaluation_Shader,
    Geometry_Shader,
    Fragment_Shader,

    game_shader_type_count
};

struct game_shader {
    game_shader_id ID;
    game_shader_type Type;
    read_file_result File;
    char* Code;
    uint64 BinarySize;
    uint32* Binary;
    vertex_layout VertexLayout;
    uint32 nUBOs;
    shader_uniform_block UBO[MAX_SHADER_UBOS];
    uint32 nSamplers;
    shader_uniform_sampler Sampler[MAX_SHADER_SAMPLERS];
};

struct game_shader_pipeline {
    vertex_layout_id VertexLayoutID;
    game_shader_pipeline_id ID;
    game_shader_id Pipeline[game_shader_type_count];
    bool IsProvided[game_shader_type_count];
    bool Bindings[SHADER_SETS][MAX_SHADER_SET_BINDINGS];
};

struct game_compute_shader {
    game_compute_shader_id ID;
    uint32 Size;
    char* Code;
};

void LoadShader(platform_api* Platform, memory_arena* Arena, game_shader* Shader);
void LoadComputeShader(platform_api* Platform, memory_arena* Arena, game_compute_shader* Shader);

#endif