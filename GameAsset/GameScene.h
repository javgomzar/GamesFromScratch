#ifndef GAME_SCENE
#define GAME_SCENE

#include "GameMesh.h"
#include "GameMaterial.h"

struct game_scene_node {
    int nChildren;
    int* Children;
    matrix4 Matrix;
    int Mesh;
};

struct game_scene {
    int nNodes;
    int* Nodes;
};

#endif