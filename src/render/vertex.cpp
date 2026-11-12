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
