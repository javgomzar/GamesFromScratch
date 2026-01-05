#include "GamePlatform.h"
#include "GameMath.h"

#ifndef GAME_MESH
#define GAME_MESH

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Vertex layouts                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

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

bool FindCompatibleVertexLayout(vertex_layout* VertexLayouts, vertex_layout VertexLayout, vertex_layout_id* Result = nullptr) {
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

void AddAttribute(vertex_layout* VertexLayout, vertex_type Type) {
    vertex_attribute* Attribute = &VertexLayout->Attributes[VertexLayout->nAttributes];
    Attribute->Location = VertexLayout->nAttributes++;
    Attribute->Type = Type;
    Attribute->Size = GetVertexTypeSizeInBytes(Type);
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
        vertex_type Type = va_arg(Types, vertex_type);
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

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Meshes                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(game_mesh_id,
    Mesh_Tetrahedron_ID,
    Mesh_Cube_ID,
    Mesh_Octahedron_ID,
    Mesh_Icosahedron_ID,
    Mesh_Dodecahedron_ID,
    Mesh_Horns_ID,
    Mesh_Dog_ID,
    Mesh_Dyno_ID,
    Mesh_Squid_ID,
    Mesh_Sphere_ID,
    Mesh_Body_ID,
    Mesh_Shield_ID,
    Mesh_Sword_ID,
    Mesh_Knife_ID,
    Mesh_Staff_ID,
    Mesh_Bow_ID,
    Mesh_Selector_ID
);

const int BONE_NAME_LENGTH = 32;

INTROSPECT
struct bone {
    int ID;
    char Name[BONE_NAME_LENGTH];
    segment3 Segment;
    transform Transform;
};

const int MAX_ARMATURE_BONES = 32;
INTROSPECT
struct armature {
    uint32 nBones;
    bone Bones[MAX_ARMATURE_BONES];
};

struct game_mesh {
    armature Armature;
    game_mesh_id ID;
    vertex_layout_id VertexLayoutID;
    uint32 nVertices;
    void* Vertices;
    uint32* Edges;
    uint32* Faces;
    uint32 nEdges;
    uint32 nFaces;
    float MinX, MaxX;
    float MinY, MaxY;
    float MinZ, MaxZ;
};

struct preprocessed_mesh {
    read_file_result File;
    uint32 nVertices;
    uint32 nEdges;
    uint32 nFaces;
    uint32 nBones;
};

preprocessed_mesh PreprocessMesh(read_file_result File) {
    preprocessed_mesh Result = {};
    Result.File = File;

    if (File.ContentSize > 0) {
        tokenizer Tokenizer = InitTokenizer(File.Content);

        token Token = RequireToken(Tokenizer, Token_Identifier);
        while (Token.Type == Token_Identifier) {
            if (Token == "nV")   Result.nVertices = Parseuint32(Tokenizer);
            else if (Token == "nE") Result.nEdges = Parseuint32(Tokenizer);
            else if (Token == "nF") Result.nFaces = Parseuint32(Tokenizer);
            else if (Token == "nB") Result.nBones = Parseuint32(Tokenizer);
            Token = GetToken(Tokenizer);
        }
    }
    return Result;
}

uint32 GetMeshVerticesSize(uint32 nVertices, bool HasArmature) {
    uint32 VertexSize = HasArmature ? 10 * sizeof(float) + 2 * sizeof(int32) : 8 * sizeof(float);
    return VertexSize * nVertices;
}

game_mesh LoadMesh(memory_arena* Arena, preprocessed_mesh* Preprocessed) {
    game_mesh Result = {};

    if (Preprocessed->File.ContentSize > 0) {
        tokenizer Tokenizer = InitTokenizer(Preprocessed->File.Content);
        AdvanceUntilLine(Tokenizer, 2);

        Result.nVertices = Preprocessed->nVertices;
        Result.nEdges = Preprocessed->nEdges;
        Result.nFaces = Preprocessed->nFaces;
        Result.Armature.nBones = Preprocessed->nBones;
        bool HasArmature = Result.Armature.nBones > 0;
        uint32 VerticesSize = GetMeshVerticesSize(Preprocessed->nVertices, HasArmature);
        Result.Vertices = PushSize(Arena, VerticesSize);
        Result.VertexLayoutID = HasArmature ? vertex_layout_bones_id : vertex_layout_v3_v2_v3_id;
        if (Preprocessed->nEdges > 0) {
            Result.Edges = PushArray(Arena, 2 * Preprocessed->nEdges, uint32);
        }
        if (Preprocessed->nFaces > 0) {
            Result.Faces = PushArray(Arena, 3 * Preprocessed->nFaces, uint32);
        }

        Result.MinX = FLT_MAX;
        Result.MinY = FLT_MAX;
        Result.MinZ = FLT_MAX;
        Result.MaxX = -FLT_MAX;
        Result.MaxY = -FLT_MAX;
        Result.MaxZ = -FLT_MAX;

        float* pOutV = (float*)Result.Vertices;
        for (int i = 0; i < Result.nVertices; i++) {
            v3 Position = ParseV3(Tokenizer);
            v3 Normal = ParseV3(Tokenizer);
            v2 Texture = ParseV2(Tokenizer);

            if (Position.X < Result.MinX) {
                Result.MinX = Position.X;
            }
            if (Position.X > Result.MaxX) {
                Result.MaxX = Position.X;
            }
            if (Position.Y < Result.MinY) {
                Result.MinY = Position.Y;
            }
            if (Position.Y > Result.MaxY) {
                Result.MaxY = Position.Y;
            }
            if (Position.Z < Result.MinZ) {
                Result.MinZ = Position.Z;
            }
            if (Position.Z > Result.MaxZ) {
                Result.MaxZ = Position.Z;
            }

            *pOutV++ = Position.X; *pOutV++ = Position.Y; *pOutV++ = Position.Z;
            *pOutV++ = Texture.X;  *pOutV++ = Texture.Y;
            *pOutV++ = Normal.X;   *pOutV++ = Normal.Y;   *pOutV++ = Normal.Z;

            if (HasArmature) {
                iv2 BoneIDs = ParseIV2(Tokenizer);
                v2 Weights = ParseV2(Tokenizer);
                int32* pOutB = (int32*)pOutV;
                *pOutB++ = BoneIDs.X;
                *pOutB++ = BoneIDs.Y;

                pOutV = (float*)pOutB;
                *pOutV++ = Weights.X;
                *pOutV++ = Weights.Y;
            }
        }

        uint32* pOutE = Result.Edges;
        for (int i = 0; i < Result.nEdges; i++) {
            uv2 Edge = ParseUV2(Tokenizer);
            *pOutE++ = Edge.X;
            *pOutE++ = Edge.Y;
        }

        uint32* pOutF = Result.Faces;
        for (int i = 0; i < Result.nFaces; i++) {
            uv3 Face = ParseUV3(Tokenizer);
            *pOutF++ = Face.X;
            *pOutF++ = Face.Y;
            *pOutF++ = Face.Z;
        }

        token Token;
        for (int i = 0; i < Result.Armature.nBones; i++) {
            bone Bone = {};
            Bone.ID = Parseuint32(Tokenizer);
            Token = RequireToken(Tokenizer, Token_Identifier);
            int Length = min(Token.Length, BONE_NAME_LENGTH);
            for (int j = 0; j < Length; j++) {
                Bone.Name[j] = Token.Text[j];
            }
            Bone.Segment.Head = ParseV3(Tokenizer);
            Bone.Segment.Tail = ParseV3(Tokenizer);
            Result.Armature.Bones[Bone.ID] = Bone;
        }
    }

    return Result;
}

#endif