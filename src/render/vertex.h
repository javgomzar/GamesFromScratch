#include "platform.h"
#include "mathematics.h"
#include "tokenizer.h"
#include "parse.h"
#include "asset.h"


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

bool IsFloatType(Vertex_Type type);
bool IsIntegerType(Vertex_Type type);
uint32 GetVertexTypeSizeInBytes(Vertex_Type type);
int GetVertexTypeComponents(Vertex_Type type);

ENUM(Vertex_ID,
    Vertex_Position_ID,
    Vertex_Joint_ID,
    Vertex_Surface_ID,
    Vertex_Text_ID
);

typedef V3 Vertex_Position;

struct Vertex_Joint {
    IV4 joints;
    V4 weights;
};

struct Vertex_Surface {
    Color color;
    V3 normal;
    V2 texture;
};

struct Vertex_Text {
    Color color;
    V2 pen;
    float depth;
    float size;
};

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

const uint8 MAX_VERTEX_ATTRIBUTES = 8;
struct Vertex_Layout {
    Vertex_ID id;
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

Vertex_Layout VERTEX_LAYOUTS[Vertex_ID_Count] = {
    VertexLayout(Vertex_Position_ID, 1, Vertex_Type_V3),
    VertexLayout(Vertex_Joint_ID,    2, Vertex_Type_V4, Vertex_Type_IV4),
    VertexLayout(Vertex_Surface_ID,  3, Vertex_Type_V4, Vertex_Type_V3, Vertex_Type_V2),
    VertexLayout(Vertex_Text_ID,     4, Vertex_Type_V4, Vertex_Type_V2, Vertex_Type_float, Vertex_Type_float),
};

// bool FindCompatibleVertexLayout(Vertex_Layout vertex_layout, Vertex_Layout_ID* result = nullptr) {
//     for (int i = 0; i < Vertex_Layout_ID_count; i++) {
//         Vertex_Layout test_layout = VERTEX_LAYOUTS[i];
//         if (test_layout.attribute_count == vertex_layout.attribute_count) {
//             bool Compatible = true;
//             for (int j = 0; j < test_layout.attribute_count; j++) {
//                 Vertex_Attribute attribute = test_layout.attributes[j];
//                 Vertex_Attribute shader_attribute = vertex_layout.attributes[j];

//                 if (shader_attribute.type != Vertex_Type_empty && shader_attribute.type != attribute.type) {
//                     Compatible = false;
//                     break;
//                 }
//             }

//             if (Compatible) {
//                 if (result) *result = test_layout.id;
//                 return true;
//             }
//         }
//     }

//     result = nullptr;
//     return false;
// }

struct Vertex_Buffer_Entry {
    uint32 offset;
    uint32 count;
    void* pointer;
};

struct Vertex_Buffer {
    Arena memory;
    uint32 capacity;
    uint32 vertex_count;
    uint32 stride;
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

// const memory_size TEXT_BUFFER_SIZE = Kilobytes(16);

// struct Text_Buffer {
//     Font* font;
//     Arena instances[FONT_CHARACTERS_COUNT];
//     uint32 count[FONT_CHARACTERS_COUNT];
// };

// void InitializeTextBuffer(Arena* Arena, Text_Buffer* text_buffer) {
//     for (int c = 0; c < FONT_CHARACTERS_COUNT; c++) {
//         text_buffer->instances[c] = SuballocateMemoryArena(Arena, TEXT_BUFFER_SIZE);
//         text_buffer->count[c] = 0;
//     }
// }

// void ClearTextBuffer(Text_Buffer* text_buffer) {
//     for (int c = 0; c < FONT_CHARACTERS_COUNT; c++) {
//         Clear(&text_buffer->instances[c]);
//         text_buffer->count[c] = 0;
//     }
// }

// void PushTextEntry(Text_Buffer* text_buffer, Font* Font, char Char, V2 Pen, float Size, Color color = Color::WHITE) {
//     Arena* Arena = &text_buffer->instances[Char - '!'];
//     float* Data = PushArray(Arena, 7, float);
//     *Data++ = Pen.x; *Data++ = Pen.y;
//     *Data++ = Size;
//     *Data++ = color.R; *Data++ = color.G; *Data++ = color.B; *Data++ = color.A;
//     text_buffer->count[Char - '!']++;
// }

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
