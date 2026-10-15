#include "GamePlatform.h"
#include "GameMath.h"
#include "tokenizer.h"


ENUM(Vertex_Type,
    Vertex_Type_empty,
    Vertex_Type_float,
    Vertex_Type_V2,
    Vertex_Type_V3,
    Vertex_Type_V4,
    Vertex_Type_int,
    Vertex_Type_IV2,
    Vertex_Type_IV3,
    Vertex_Type_IV4,
    Vertex_Type_Matrix2,
    Vertex_Type_Matrix3,
    Vertex_Type_Matrix4
);

bool IsFloatType(Vertex_Type type) {
    return 
        type == Vertex_Type_float ||
        type == Vertex_Type_V2 ||
        type == Vertex_Type_V3 ||
        type == Vertex_Type_V4 ||
        type == Vertex_Type_Matrix2 ||
        type == Vertex_Type_Matrix3 ||
        type == Vertex_Type_Matrix4;
}

bool IsIntegerType(Vertex_Type type) {
    return
        type == Vertex_Type_int ||
        type == Vertex_Type_IV2 ||
        type == Vertex_Type_IV3 ||
        type == Vertex_Type_IV4;
}

uint32 GetVertexTypeSizeInBytes(Vertex_Type type) {
    switch (type) {
        case Vertex_Type_float: { return 4; } break;
        case Vertex_Type_V2:  { return 8; } break;
        case Vertex_Type_V3:  { return 12; } break;
        case Vertex_Type_V4:  { return 16; } break;
        case Vertex_Type_int:   { return 4; } break;
        case Vertex_Type_IV2: { return 8; } break;
        case Vertex_Type_IV3: { return 12; } break;
        case Vertex_Type_IV4: { return 16; } break;
        case Vertex_Type_Matrix2:  { return 16; } break;
        case Vertex_Type_Matrix3:  { return 36; } break;
        case Vertex_Type_Matrix4:  { return 64; } break;
        default: Raise("Invalid vertex attribute type.");
    }
    return 0;
}

// Returns number of elements for a given shader type. For example, output is 3 for type `vec3`.
int GetVertexTypeSize(Vertex_Type type) {
    switch (type) {
        case Vertex_Type_float: { return 1; } break;
        case Vertex_Type_V2:  { return 2; } break;
        case Vertex_Type_V3:  { return 3; } break;
        case Vertex_Type_V4:  { return 4; } break;
        case Vertex_Type_int:   { return 1; } break;
        case Vertex_Type_IV2: { return 2; } break;
        case Vertex_Type_IV3: { return 3; } break;
        case Vertex_Type_IV4: { return 4; } break;
        case Vertex_Type_Matrix2:  { return 4; } break;
        case Vertex_Type_Matrix3:  { return 9; } break;
        case Vertex_Type_Matrix4:  { return 16; } break;
        default: Raise("Invalid vertex attribute type.");
    }
    return 0;
}

struct Vertex_Attribute {
    Vertex_Type type;
    uint32 location;
    uint32 size;
    uint32 offset;
};

bool operator!=(Vertex_Attribute attribute1, Vertex_Attribute attribute2) {
    return 
        attribute1.type != attribute2.type ||
        attribute1.location != attribute2.location ||
        attribute1.size != attribute2.size ||
        attribute1.offset != attribute2.offset;
}

ENUM(Vertex_Layout_ID,
    Vertex_Layout_V2_id,
    Vertex_Layout_V2_V2_id,
    Vertex_Layout_V3_id,
    Vertex_Layout_V3_V2_id,
    Vertex_Layout_V3_V2_V3_id,
    Vertex_Layout_V3_V4_id,
    Vertex_Layout_V4_id,
    Vertex_Layout_bones_id
);

const uint8 MAX_VERTEX_ATTRIBUTES = 16;
struct Vertex_Layout {
    Vertex_Layout_ID id;
    Vertex_Attribute attributes[MAX_VERTEX_ATTRIBUTES];
    uint32 stride;
    uint8 attribute_count;
};

void AddAttribute(Vertex_Layout* Vertex_Layout, Vertex_Type type) {
    Vertex_Attribute* attribute = &Vertex_Layout->attributes[Vertex_Layout->attribute_count];
    attribute->location = Vertex_Layout->attribute_count++;
    attribute->type = type;
    attribute->size = GetVertexTypeSizeInBytes(type);
    attribute->offset = Vertex_Layout->stride;
    Vertex_Layout->stride += attribute->size;
}

Vertex_Layout VertexLayout(Vertex_Layout_ID id, int attribute_count, ...) {
    Vertex_Layout result = {};
    result.id = id;
    result.stride = 0;
    result.attribute_count = 0;

    va_list types;
    va_start(types, attribute_count);

    for (int i = 0; i < attribute_count; i++) {
        Vertex_Type type = va_arg(types, Vertex_Type);
        AddAttribute(&result, type);
    }
    return result;
}

Vertex_Layout VERTEX_LAYOUTS[Vertex_Layout_ID_count] = {
    VertexLayout(Vertex_Layout_V2_id,       1, Vertex_Type_V2),
    VertexLayout(Vertex_Layout_V2_V2_id,    2, Vertex_Type_V2, Vertex_Type_V2),
    VertexLayout(Vertex_Layout_V3_id,       1, Vertex_Type_V3),
    VertexLayout(Vertex_Layout_V3_V2_id,    2, Vertex_Type_V3, Vertex_Type_V2),
    VertexLayout(Vertex_Layout_V3_V2_V3_id, 3, Vertex_Type_V3, Vertex_Type_V2, Vertex_Type_V3),
    VertexLayout(Vertex_Layout_V3_V4_id,    2, Vertex_Type_V3, Vertex_Type_V4),
    VertexLayout(Vertex_Layout_V4_id,       1, Vertex_Type_V4),
    VertexLayout(Vertex_Layout_bones_id,    5, Vertex_Type_V3, Vertex_Type_V2, Vertex_Type_V3, Vertex_Type_IV2, Vertex_Type_V2),
};

bool FindCompatibleVertexLayout(Vertex_Layout vertex_layout, Vertex_Layout_ID* result = nullptr) {
    for (int i = 0; i < Vertex_Layout_ID_count; i++) {
        Vertex_Layout test_layout = VERTEX_LAYOUTS[i];
        if (test_layout.attribute_count == vertex_layout.attribute_count) {
            bool Compatible = true;
            for (int j = 0; j < test_layout.attribute_count; j++) {
                Vertex_Attribute attribute = test_layout.attributes[j];
                Vertex_Attribute shader_attribute = vertex_layout.attributes[j];

                if (shader_attribute.type != Vertex_Type_empty && shader_attribute.type != attribute.type) {
                    Compatible = false;
                    break;
                }
            }

            if (Compatible) {
                if (result) *result = test_layout.id;
                return true;
            }
        }
    }

    result = nullptr;
    return false;
}

bool operator==(Vertex_Layout Layout1, Vertex_Layout Layout2) {
    if (Layout1.stride == Layout2.stride && Layout1.attribute_count == Layout2.attribute_count) {
        for (int i = 0; i < Layout1.attribute_count; i++) {
            if (Layout1.attributes[i] != Layout2.attributes[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

V2 ParseV2(Tokenizer& tokenizer) {
    V2 result;
    result.x = ParseFloat(tokenizer);
    result.y = ParseFloat(tokenizer);
    return result;
}

V3 ParseV3(Tokenizer& tokenizer) {
    V3 result;
    result.x = ParseFloat(tokenizer);
    result.y = ParseFloat(tokenizer);
    result.z = ParseFloat(tokenizer);
    return result;
}

V4 ParseV4(Tokenizer& tokenizer) {
    V4 result;
    result.x = ParseFloat(tokenizer);
    result.y = ParseFloat(tokenizer);
    result.z = ParseFloat(tokenizer);
    result.w = ParseFloat(tokenizer);
    return result;
}

Quaternion ParseQuaternion(Tokenizer& tokenizer) {
    Quaternion result;
    result.c = ParseFloat(tokenizer);
    result.i = ParseFloat(tokenizer);
    result.j = ParseFloat(tokenizer);
    result.k = ParseFloat(tokenizer);
    return result;
}

IV2 ParseIV2(Tokenizer& tokenizer) {
    IV2 result;
    result.x = ParseInt(tokenizer);
    result.y = ParseInt(tokenizer);
    return result;
}

IV3 ParseIV3(Tokenizer& tokenizer) {
    IV3 result;
    result.x = ParseInt(tokenizer);
    result.y = ParseInt(tokenizer);
    result.z = ParseInt(tokenizer);
    return result;
}

UV2 ParseUV2(Tokenizer& tokenizer) {
    UV2 result;
    result.x = Parseuint32(tokenizer);
    result.y = Parseuint32(tokenizer);
    return result;
}

UV3 ParseUV3(Tokenizer& tokenizer) {
    UV3 result;
    result.x = Parseuint32(tokenizer);
    result.y = Parseuint32(tokenizer);
    result.z = Parseuint32(tokenizer);
    return result;
}