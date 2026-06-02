#ifndef GAME_VERTEX
#define GAME_VERTEX

#include "GamePlatform.h"
#include "GameMath.h"
#include "Tokenizer.h"


ENUM(vertex_type,
    vertex_type_empty,
    vertex_type_float,
    vertex_type_v2,
    vertex_type_v3,
    vertex_type_v4,
    vertex_type_int,
    vertex_type_iv2,
    vertex_type_iv3,
    vertex_type_iv4,
    vertex_type_mat2,
    vertex_type_mat3,
    vertex_type_mat4
);

bool IsFloatType(vertex_type Type) {
    return 
        Type == vertex_type_float ||
        Type == vertex_type_v2 ||
        Type == vertex_type_v3 ||
        Type == vertex_type_v4 ||
        Type == vertex_type_mat2 ||
        Type == vertex_type_mat3 ||
        Type == vertex_type_mat4;
}

bool IsIntegerType(vertex_type Type) {
    return
        Type == vertex_type_int ||
        Type == vertex_type_iv2 ||
        Type == vertex_type_iv3 ||
        Type == vertex_type_iv4;
}

uint32 GetVertexTypeSizeInBytes(vertex_type Type) {
    switch (Type) {
        case vertex_type_float: { return 4; } break;
        case vertex_type_v2:  { return 8; } break;
        case vertex_type_v3:  { return 12; } break;
        case vertex_type_v4:  { return 16; } break;
        case vertex_type_int:   { return 4; } break;
        case vertex_type_iv2: { return 8; } break;
        case vertex_type_iv3: { return 12; } break;
        case vertex_type_iv4: { return 16; } break;
        case vertex_type_mat2:  { return 16; } break;
        case vertex_type_mat3:  { return 36; } break;
        case vertex_type_mat4:  { return 64; } break;
        default: Raise("Invalid vertex attribute type.");
    }
    return 0;
}

// Returns number of elements for a given shader type. For example, output is 3 for type `vec3`.
int GetVertexTypeSize(vertex_type Type) {
    switch (Type) {
        case vertex_type_float: { return 1; } break;
        case vertex_type_v2:  { return 2; } break;
        case vertex_type_v3:  { return 3; } break;
        case vertex_type_v4:  { return 4; } break;
        case vertex_type_int:   { return 1; } break;
        case vertex_type_iv2: { return 2; } break;
        case vertex_type_iv3: { return 3; } break;
        case vertex_type_iv4: { return 4; } break;
        case vertex_type_mat2:  { return 4; } break;
        case vertex_type_mat3:  { return 9; } break;
        case vertex_type_mat4:  { return 16; } break;
        default: Raise("Invalid vertex attribute type.");
    }
    return 0;
}

struct vertex_attribute {
    vertex_type Type;
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

ENUM(vertex_layout_id,
    vertex_layout_v2_id,
    vertex_layout_v2_v2_id,
    vertex_layout_v3_id,
    vertex_layout_v3_v2_id,
    vertex_layout_v3_v2_v3_id,
    vertex_layout_v3_v4_id,
    vertex_layout_v4_id,
    vertex_layout_bones_id
);

const uint8 MAX_VERTEX_ATTRIBUTES = 16;
struct vertex_layout {
    vertex_layout_id ID;
    vertex_attribute Attributes[MAX_VERTEX_ATTRIBUTES];
    uint32 Stride;
    uint8 nAttributes;
};

void AddAttribute(vertex_layout* VertexLayout, vertex_type Type) {
    vertex_attribute* Attribute = &VertexLayout->Attributes[VertexLayout->nAttributes];
    Attribute->Location = VertexLayout->nAttributes++;
    Attribute->Type = Type;
    Attribute->Size = GetVertexTypeSizeInBytes(Type);
    Attribute->Offset = VertexLayout->Stride;
    VertexLayout->Stride += Attribute->Size;
}

vertex_layout VertexLayout(vertex_layout_id ID, int nAttributes, ...) {
    vertex_layout Result = {};
    Result.ID = ID;
    Result.Stride = 0;
    Result.nAttributes = 0;

    va_list Types;
    va_start(Types, nAttributes);

    for (int i = 0; i < nAttributes; i++) {
        vertex_type Type = va_arg(Types, vertex_type);
        AddAttribute(&Result, Type);
    }
    return Result;
}

vertex_layout VertexLayouts[vertex_layout_id_count] = {
    VertexLayout(vertex_layout_v2_id,       1, vertex_type_v2),
    VertexLayout(vertex_layout_v2_v2_id,    2, vertex_type_v2, vertex_type_v2),
    VertexLayout(vertex_layout_v3_id,       1, vertex_type_v3),
    VertexLayout(vertex_layout_v3_v2_id,    2, vertex_type_v3, vertex_type_v2),
    VertexLayout(vertex_layout_v3_v2_v3_id, 3, vertex_type_v3, vertex_type_v2, vertex_type_v3),
    VertexLayout(vertex_layout_v3_v4_id,    2, vertex_type_v3, vertex_type_v4),
    VertexLayout(vertex_layout_v4_id,       1, vertex_type_v4),
    VertexLayout(vertex_layout_bones_id,    5, vertex_type_v3, vertex_type_v2, vertex_type_v3, vertex_type_iv2, vertex_type_v2),
};

bool FindCompatibleVertexLayout(vertex_layout VertexLayout, vertex_layout_id* Result = nullptr) {
    for (int i = 0; i < vertex_layout_id_count; i++) {
        vertex_layout TestLayout = VertexLayouts[i];
        if (TestLayout.nAttributes == VertexLayout.nAttributes) {
            bool Compatible = true;
            for (int j = 0; j < TestLayout.nAttributes; j++) {
                vertex_attribute Attribute = TestLayout.Attributes[j];
                vertex_attribute ShaderAttribute = VertexLayout.Attributes[j];

                if (ShaderAttribute.Type != vertex_type_empty && ShaderAttribute.Type != Attribute.Type) {
                    Compatible = false;
                    break;
                }
            }

            if (Compatible) {
                if (Result) *Result = TestLayout.ID;
                return true;
            }
        }
    }

    Result = nullptr;
    return false;
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

v2 ParseV2(tokenizer& Tokenizer) {
    v2 Result;
    Result.X = ParseFloat(Tokenizer);
    Result.Y = ParseFloat(Tokenizer);
    return Result;
}

v3 ParseV3(tokenizer& Tokenizer) {
    v3 Result;
    Result.X = ParseFloat(Tokenizer);
    Result.Y = ParseFloat(Tokenizer);
    Result.Z = ParseFloat(Tokenizer);
    return Result;
}

v4 ParseV4(tokenizer& Tokenizer) {
    v4 Result;
    Result.X = ParseFloat(Tokenizer);
    Result.Y = ParseFloat(Tokenizer);
    Result.Z = ParseFloat(Tokenizer);
    Result.W = ParseFloat(Tokenizer);
    return Result;
}

quaternion ParseQuaternion(tokenizer& Tokenizer) {
    quaternion Result;
    Result.c = ParseFloat(Tokenizer);
    Result.i = ParseFloat(Tokenizer);
    Result.j = ParseFloat(Tokenizer);
    Result.k = ParseFloat(Tokenizer);
    return Result;
}

iv2 ParseIV2(tokenizer& Tokenizer) {
    iv2 Result;
    Result.X = ParseInt(Tokenizer);
    Result.Y = ParseInt(Tokenizer);
    return Result;
}

iv3 ParseIV3(tokenizer& Tokenizer) {
    iv3 Result;
    Result.X = ParseInt(Tokenizer);
    Result.Y = ParseInt(Tokenizer);
    Result.Z = ParseInt(Tokenizer);
    return Result;
}

uv2 ParseUV2(tokenizer& Tokenizer) {
    uv2 Result;
    Result.X = Parseuint32(Tokenizer);
    Result.Y = Parseuint32(Tokenizer);
    return Result;
}

uv3 ParseUV3(tokenizer& Tokenizer) {
    uv3 Result;
    Result.X = Parseuint32(Tokenizer);
    Result.Y = Parseuint32(Tokenizer);
    Result.Z = Parseuint32(Tokenizer);
    return Result;
}

#endif