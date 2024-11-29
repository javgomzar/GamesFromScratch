
#include <iostream>
#include "GameAssets.h"
#include "windows.h"
#include "../Win32PlatformLayer/Win32PlatformLayer.h"


void PushAsset(game_assets* Assets, const char* Path) {
    game_asset Asset = {};
    Asset.File = PlatformReadEntireFile(Path);

    // Get asset type from file extension

    Asset.Type = Bitmap;
    
    switch (Asset.Type) {
        case Bitmap: {
            bitmap_header* Header = (bitmap_header*)Asset.File.Content;
            Asset.Size = Header->Width * Header->Height * Header->BitsPerPixel >> 3;
            Assert(Asset.Size + Header->BitmapOffset == Header->FileSize);
        } break;

        default: {
            Asset.Size = Asset.File.ContentSize;
        }
    }

    Asset.ID = Assets->nAssets++;
    Assets->Asset[Asset.ID] = Asset;
    Assets->n[Asset.Type]++;
    Assets->TotalSize += Asset.Size;
    Assets->Size[Asset.Type] += Asset.Size;
}

void PushDir(game_assets* Assets, const char* Path) {
    WIN32_FIND_DATAA Data;
    HANDLE hFind = FindFirstFileA(Path, &Data);

    if (hFind == INVALID_HANDLE_VALUE) {
        char Buffer[256] = "Invalid asset directory: ";
        strcat_s(Buffer, Path);
        OutputDebugStringA(Buffer);
        Assert(false);
    }

    do {
        if (strcmp(Data.cFileName, ".") != 0 && strcmp(Data.cFileName, "..") != 0) {
            uint64 DestSize = strlen(Path) + strlen(Data.cFileName) + 1;
            char* Buffer = new char[DestSize];
            strncpy_s(Buffer, DestSize, Path, strlen(Path) - 1);
            strcat_s(Buffer, DestSize, Data.cFileName);
            PushAsset(Assets, Buffer);
        }
    } while (FindNextFileA(hFind, &Data));
}

int main() {
    game_assets Assets = {};

    // Fonts

    // Text

    // Bitmaps
    PushDir(&Assets, "..\\..\\Assets\\Bitmaps\\*");
    
    // Sound

    // Meshes

    // Video

    // Shaders

    // Calculate amount of memory necessary

    // Output file

    OutputDebugStringA("FINISHED\n");
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
