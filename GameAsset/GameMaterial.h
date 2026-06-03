#ifndef GAME_MATERIAL
#define GAME_MATERIAL

#include "GameColor.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Materials                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum class alpha_mode {
    opaque,
    mask,
    blend
};

struct game_material {
    string Name;
    v4 BaseColorFactor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    v3 EmissiveFactor = V3(0.0f, 0.0f, 0.0f);
    color Color = color::WHITE;
    alpha_mode AlphaMode = alpha_mode::opaque;
    float AlphaCutoff = 0.5f;
    float Metallicity = 1.0f;
    float Roughness = 1.0f;
    int BaseColorTexture = -1;
    int MetallicRoughnessTexture = -1;
    int NormalTexture = -1;
    int OcclusionTexture = -1;
    int EmissiveTexture = -1;
    bool DoubleSided = false;
};

#endif