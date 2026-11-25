#pragma once

#include <common.h>
#include <platform.h>
#include <data_structures.h>


typedef uint32 Asset_ID;

ENUM(Asset_Type,
    Asset_Scene,
    Asset_Texture,
    Asset_Sound,
    Asset_Mesh,
    Asset_Material,
    Asset_Animation,
    Asset_Font
);

struct Asset_Handle {
    Asset_ID id;
    uint32 generation;
};

struct Asset {
    bool processed;
    bool loaded;
    virtual void Process() = 0;
    virtual void Load() = 0;
    virtual void Unload() = 0;
};

struct Asset_Metadata {
    File_Info source_file;
    File_Info binary_file;
    Asset_Handle handle;
    Asset_Type type;
};

struct Asset_Manager {
    Asset_ID next_id;
    Hash_Table<Asset_ID, Asset_Metadata> metadata;
    void Initialize();
    Asset_Handle AddAsset(String path);
    Asset& GetAsset(Asset_Handle handle);
};
