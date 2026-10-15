#include "GamePlatform.h"
#include "GameMath.h"
#include "GameVertex.h"

#ifndef GAME_MESH
#define GAME_MESH

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Meshes                                                                                                                                       |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

const int BONE_NAME_LENGTH = 32;

INTROSPECT
struct Bone {
    int ID;
    char Name[BONE_NAME_LENGTH];
    Segment3 Segment;
    Transform transform;
};

const int MAX_ARMATURE_BONES = 32;

INTROSPECT
struct armature {
    uint32 nBones;
    Bone Bones[MAX_ARMATURE_BONES];
};

INTROSPECT
struct Mesh {
    String Name;
    armature Armature;
    Vertex_Layout_ID VertexLayoutID;
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
    uint32 nVertices;
    uint32 nEdges;
    uint32 nFaces;
    uint32 nBones;
};

preprocessed_mesh PreprocessMesh(void* FileContent) {
    preprocessed_mesh Result = {};
    Tokenizer tokenizer = InitTokenizer(FileContent);

    Token token = RequireToken(tokenizer, Token_Identifier);
    while (token.type == Token_Identifier) {
        if (token == "nV")   Result.nVertices = Parseuint32(tokenizer);
        else if (token == "nE") Result.nEdges = Parseuint32(tokenizer);
        else if (token == "nF") Result.nFaces = Parseuint32(tokenizer);
        else if (token == "nB") Result.nBones = Parseuint32(tokenizer);
        token = GetToken(tokenizer);
    }
    return Result;
}

uint32 GetMeshVerticesSize(uint32 nVertices, bool HasArmature) {
    uint32 VertexSize = HasArmature ? 10 * sizeof(float) + 2 * sizeof(int32) : 8 * sizeof(float);
    return VertexSize * nVertices;
}

Mesh LoadMesh(Arena* arena, preprocessed_mesh* Preprocessed, void* FileContent) {
    Mesh Result = {};
    Tokenizer tokenizer = InitTokenizer(FileContent);
    AdvanceUntilLine(tokenizer, 2);

    Result.nVertices = Preprocessed->nVertices;
    Result.nEdges = Preprocessed->nEdges;
    Result.nFaces = Preprocessed->nFaces;
    Result.Armature.nBones = Preprocessed->nBones;
    bool HasArmature = Result.Armature.nBones > 0;
    uint32 VerticesSize = GetMeshVerticesSize(Preprocessed->nVertices, HasArmature);
    Result.Vertices = PushSize(arena, VerticesSize);
    Result.VertexLayoutID = HasArmature ? Vertex_Layout_bones_id : Vertex_Layout_V3_V2_V3_id;
    if (Preprocessed->nEdges > 0) {
        Result.Edges = PushArray(arena, 2 * Preprocessed->nEdges, uint32);
    }
    if (Preprocessed->nFaces > 0) {
        Result.Faces = PushArray(arena, 3 * Preprocessed->nFaces, uint32);
    }

    Result.MinX = FLT_MAX;
    Result.MinY = FLT_MAX;
    Result.MinZ = FLT_MAX;
    Result.MaxX = -FLT_MAX;
    Result.MaxY = -FLT_MAX;
    Result.MaxZ = -FLT_MAX;

    float* pOutV = (float*)Result.Vertices;
    for (int i = 0; i < Result.nVertices; i++) {
        V3 Position = ParseV3(tokenizer);
        V3 Normal = ParseV3(tokenizer);
        V2 Texture = ParseV2(tokenizer);

        if (Position.x < Result.MinX) {
            Result.MinX = Position.x;
        }
        if (Position.x > Result.MaxX) {
            Result.MaxX = Position.x;
        }
        if (Position.y < Result.MinY) {
            Result.MinY = Position.y;
        }
        if (Position.y > Result.MaxY) {
            Result.MaxY = Position.y;
        }
        if (Position.z < Result.MinZ) {
            Result.MinZ = Position.z;
        }
        if (Position.z > Result.MaxZ) {
            Result.MaxZ = Position.z;
        }

        *pOutV++ = Position.x; *pOutV++ = Position.y; *pOutV++ = Position.z;
        *pOutV++ = Texture.x;  *pOutV++ = Texture.y;
        *pOutV++ = Normal.x;   *pOutV++ = Normal.y;   *pOutV++ = Normal.z;

        if (HasArmature) {
            IV2 BoneIDs = ParseIV2(tokenizer);
            V2 Weights = ParseV2(tokenizer);
            int32* pOutB = (int32*)pOutV;
            *pOutB++ = BoneIDs.x;
            *pOutB++ = BoneIDs.y;

            pOutV = (float*)pOutB;
            *pOutV++ = Weights.x;
            *pOutV++ = Weights.y;
        }
    }

    uint32* pOutE = Result.Edges;
    for (int i = 0; i < Result.nEdges; i++) {
        UV2 Edge = ParseUV2(tokenizer);
        *pOutE++ = Edge.x;
        *pOutE++ = Edge.y;
    }

    uint32* pOutF = Result.Faces;
    for (int i = 0; i < Result.nFaces; i++) {
        UV3 Face = ParseUV3(tokenizer);
        *pOutF++ = Face.x;
        *pOutF++ = Face.y;
        *pOutF++ = Face.z;
    }

    Token token;
    for (int i = 0; i < Result.Armature.nBones; i++) {
        Bone bone = {};
        bone.ID = Parseuint32(tokenizer);
        token = RequireToken(tokenizer, Token_Identifier);
        int Length = token.length < BONE_NAME_LENGTH ? token.length : BONE_NAME_LENGTH;
        for (int j = 0; j < Length; j++) {
            bone.Name[j] = token.text[j];
        }
        bone.Segment.head = ParseV3(tokenizer);
        bone.Segment.tail = ParseV3(tokenizer);
        Result.Armature.Bones[bone.ID] = bone;
    }

    return Result;
}

#endif