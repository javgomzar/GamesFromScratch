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
ENUM(game_mesh_id,
    Mesh_Sphere_ID,
    Mesh_Tetrahedron_ID,
    Mesh_Cube_ID,
    Mesh_Octahedron_ID,
    Mesh_Icosahedron_ID,
    Mesh_Dodecahedron_ID,
    Mesh_Enemy_ID,
    Mesh_Body_ID,
    Mesh_Shield_ID,
    Mesh_Sword_ID,
    Mesh_Selector_ID
);

const int BONE_NAME_LENGTH = 32;

struct bone {
    int ID;
    char Name[BONE_NAME_LENGTH];
    segment3 Segment;
    transform Transform;
};

const int MAX_ARMATURE_BONES = 32;
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
    file_info FileInfo;
    void* FileContent;
    uint32 nVertices;
    uint32 nEdges;
    uint32 nFaces;
    uint32 nBones;
};

preprocessed_mesh PreprocessMesh(file_info FileInfo, void* FileContent) {
    preprocessed_mesh Result = {};
    Result.FileInfo = FileInfo;
    Result.FileContent = FileContent;

    if (Result.FileInfo.Size > 0) {
        tokenizer Tokenizer = InitTokenizer(Result.FileContent);

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

    if (Preprocessed->FileInfo.Size > 0) {
        tokenizer Tokenizer = InitTokenizer(Preprocessed->FileContent);
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
            int Length = Token.Length < BONE_NAME_LENGTH ? Token.Length : BONE_NAME_LENGTH;
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