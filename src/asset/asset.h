#pragma once

#include "stb_image.h"

#include "platform.h"
#include "data_structures.h"
#include "math.h"
#include "tokenizer.h"
#include <color.h>

typedef uint32 Asset_ID;

#include "animation.h"
#include "font.h"
#include "texture.h"
#include "audio.h"
#include "mesh.h"
// #include "video.h"


struct Asset_Header {
    uint32 magic_number;
    uint32 mesh_count;
    uint32 material_count;
    uint32 texture_count;
    uint32 skeleton_count;
    uint32 animation_count;
    uint32 sound_count;
};

struct Asset {
    String path;
    Array<Asset_ID> sounds;
    Array<Asset_ID> textures;
    Array<Asset_ID> meshes;
    Array<Asset_ID> materials;
    Array<Asset_ID> skeletons;
    Array<Asset_ID> animations;
};

struct Catalog {
    Asset test;
};

constexpr int MAX_ASSETS = 128;

struct Asset_Manager {
    Asset_ID next_id;
    Arena arena;
    Catalog catalog;
    Hash_Table<Asset_ID, Texture*> textures;
    // hash_table<Asset_ID, Mesh*> meshes;
    // hash_table<Asset_ID, Material*> materials;
    void Initialize(Arena& permanent);
    Asset_ID CreateID();
    void AddAsset(Asset& asset, String path, bool force_process, int file_count, ...);
    Texture* AddTexture();
};
