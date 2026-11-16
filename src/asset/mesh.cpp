#include "pch.h"
#include "mesh.h"

preprocessed_mesh PreprocessMesh(void* FileContent) {
    preprocessed_mesh Result = {};
    Tokenizer tokenizer = Tokenizer(FileContent);

    Token token = RequireToken(tokenizer, Token_Identifier);
    while (token.type == Token_Identifier) {
        if (token == "nV")   Result.nVertices = Parseuint32(tokenizer);
        else if (token == "nE") Result.nEdges = Parseuint32(tokenizer);
        else if (token == "nF") Result.nFaces = Parseuint32(tokenizer);
        else if (token == "nB") Result.nBones = Parseuint32(tokenizer);
        token = tokenizer.GetToken();
    }
    return Result;
}

uint32 GetMeshVerticesSize(uint32 nVertices, bool HasArmature) {
    uint32 VertexSize = HasArmature ? 10 * sizeof(float) + 2 * sizeof(int32) : 8 * sizeof(float);
    return VertexSize * nVertices;
}
