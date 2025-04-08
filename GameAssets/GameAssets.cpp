
#include <iostream>
#include "GameAssets.h"


/*
    TODO:
        - Make fonts load bitmaps into a single bitmap that can be cropped to select a certain character (Atlas).
        - Fix video. Possibly extract all frames and load them to the asset file.
        - Asset hot reloading
*/

int main() {
    void* Memory = VirtualAlloc(0, Megabytes(100), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    memory_arena Arena = MemoryArena(Megabytes(100), (uint8*)Memory);

    game_asset SpainAsset = {};
    SpainAsset.File = PlatformReadEntireFile("..\\..\\Assets\\Bitmaps\\spain.bmp");

    game_bitmap SpainBMP = AssetLoadBitmap(&Arena, &SpainAsset);

    return 0;
}
