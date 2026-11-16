#include "pch.h"
#include "vertex.h"


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
        default: Log::Fatal("Invalid vertex attribute type.");
    }
    return 0;
}

// Returns number of components for a given shader type. For example, returns 3 for type `vec3`.
int GetVertexTypeComponents(Vertex_Type type) {
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
        default: Log::Fatal("Invalid vertex attribute type.");
    }
    return 0;
}

bool operator!=(Vertex_Attribute attribute1, Vertex_Attribute attribute2) {
    return 
        attribute1.type != attribute2.type ||
        attribute1.location != attribute2.location ||
        attribute1.size != attribute2.size ||
        attribute1.offset != attribute2.offset;
}

void AddAttribute(Vertex_Layout* vertex_layout, Vertex_Type type) {
    Vertex_Attribute* attribute = &vertex_layout->attributes[vertex_layout->attribute_count];
    attribute->location = vertex_layout->attribute_count++;
    attribute->type = type;
    attribute->size = GetVertexTypeSizeInBytes(type);
    attribute->offset = vertex_layout->stride;
    vertex_layout->stride += attribute->size;
}

Vertex_Layout VertexLayout(Vertex_ID id, int attribute_count, ...) {
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

const Vertex_Layout VERTEX_LAYOUTS[Vertex_ID_Count] = {
    VertexLayout(Vertex_Position_ID, 1, Vertex_Type_V3),
    VertexLayout(Vertex_Joint_ID,    2, Vertex_Type_V4, Vertex_Type_IV4),
    VertexLayout(Vertex_Surface_ID,  3, Vertex_Type_V4, Vertex_Type_V3, Vertex_Type_V2),
    VertexLayout(Vertex_Text_ID,     4, Vertex_Type_V4, Vertex_Type_V2, Vertex_Type_float, Vertex_Type_float),
};

Vertex_Buffer_Entry PushVertices(Vertex_Buffer* buffer, uint64 count) {
    if (buffer->vertex_count + count >= buffer->capacity) {
        Log::Error("Vertex buffer overflow");
        return {};
    }
    
    Vertex_Buffer_Entry entry;
    entry.count = count;
    entry.offset = buffer->vertex_count;
    entry.pointer = buffer->memory.PushSize(count * buffer->stride);
    buffer->vertex_count += count;
    return entry;
}

void ClearVertexBuffer(Vertex_Buffer& buffer) {
    buffer.memory.Clear();
    buffer.vertex_count = 0;
}
