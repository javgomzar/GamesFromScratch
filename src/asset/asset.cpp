#include "asset.h"

Asset_Manager::Asset_Manager(Arena& permanent) : arena(permanent, Megabytes(1)) {
    next_id = 1;

    // game_asset* Test = AddAsset(&Result, "GameAsset/Files/Manifest/Test.man" , game_asset_tag::TEST);
}