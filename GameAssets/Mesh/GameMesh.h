#include "GamePlatform.h"
#include "GameMath.h"
#include "Shader/GameShader.h"

#ifndef GAME_MESH
#define GAME_MESH

enum game_mesh_id {
    Mesh_Enemy_ID,
    Mesh_Sphere_ID,
    Mesh_Body_ID,
    Mesh_Shield_ID,
    Mesh_Sword_ID,
    Mesh_Selector_ID,
    
    game_mesh_id_count
};

const int BONE_NAME_LENGTH = 32;

struct bone {
    int ID;
    char Name[BONE_NAME_LENGTH];
    v3 Head;
    v3 Tail;
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
    vertex_layout_id LayoutID;
    uint32 nVertices;
    uint32 nFaces;
    void* Vertices;
    uint32* Faces;
};

struct preprocessed_mesh {
    uint32 nVertices;
    uint32 nFaces;
    uint32 nBones;
};

#endif