#include "GamePlatform.h"
#include "GameMath.h"
#include "Shader/GameShader.h"

#ifndef GAME_MESH
#define GAME_MESH

INTROSPECT
enum game_mesh_id {
    Mesh_Horns_ID,
    Mesh_Dog_ID,
    Mesh_Sphere_ID,
    Mesh_Body_ID,
    Mesh_Shield_ID,
    Mesh_Sword_ID,
    Mesh_Selector_ID,
    Mesh_Dyno_ID,
    
    game_mesh_id_count
};

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
    vertex_layout_id LayoutID;
    uint32 nVertices;
    uint32 nFaces;
    void* Vertices;
    uint32* Faces;
    float MinX;
    float MaxX;
    float MinY;
    float MaxY;
    float MinZ;
    float MaxZ;
};

struct preprocessed_mesh {
    read_file_result File;
    uint32 nVertices;
    uint32 nFaces;
    uint32 nBones;
};

preprocessed_mesh PreprocessMesh(read_file_result File);
game_mesh LoadMesh(memory_arena* Arena, preprocessed_mesh* Preprocessed);
uint32 GetMeshVerticesSize(uint32 nVertices, bool HasArmature);

#endif