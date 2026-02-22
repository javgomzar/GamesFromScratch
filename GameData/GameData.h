#ifndef GAME_DATA
#define GAME_DATA

#include "GamePlatform.h"


typedef uint32 game_data_slot_id;
struct game_data_slot {
    game_data_slot_id ID;
    uint32 Offset;
    uint16 Size;
    bool Deleted;
};

typedef uint32 game_data_page_id;

const int GAME_DATA_PAGE_SIZE = Kilobytes(4);

ENUM(game_data_page_type,
    blank_data_page,
    meta_data_page,
    row_data_page
);

struct game_data_page_header {
    game_data_page_type Type;
    game_data_page_id ID;
    uint32 FreeSpace;
    uint32 CheckSum;
    uint32 Size;
    uint32 nSlots;
    game_data_slot_id NextSlotID;
};

struct game_data_page {
    game_data_page_header* Header;
    game_data_slot* Slots;
};

game_data_page CreateDataPage(
    memory_arena* Arena, 
    game_data_page_type Type, 
    game_data_page_id ID
) {
    game_data_page Result = {};
    Result.Header = (game_data_page_header*)PushSize(Arena, GAME_DATA_PAGE_SIZE);
    Result.Header->Type = Type;
    Result.Header->ID = ID;
    Result.Header->CheckSum = 0;
    Result.Header->Size = sizeof(game_data_page_header);
    Result.Header->FreeSpace = GAME_DATA_PAGE_SIZE - Result.Header->Size;
    Result.Header->nSlots = 0;
    Result.Header->NextSlotID = 1;
    Result.Slots = (game_data_slot*)((uint8*)Result.Header + sizeof(game_data_page_header));

    return Result;
}

game_data_slot* AddSlot(game_data_page Page, uint16 Size) {
    Assert(Page.Header->Type != blank_data_page);
    Assert(Page.Header->FreeSpace > Size + sizeof(game_data_slot));

    game_data_slot* Result = Page.Slots + Page.Header->nSlots++;
    Page.Header->Size += sizeof(game_data_slot);
    Page.Header->FreeSpace -= sizeof(game_data_slot);
    
    Result->ID = Page.Header->NextSlotID++;
    Result->Size = Size;
    Result->Deleted = false;

    Page.Header->Size += Size;
    Page.Header->FreeSpace -= Size;
    Result->Offset = sizeof(game_data_page) + Page.Header->nSlots * sizeof(game_data_slot) + Page.Header->FreeSpace;

    return Result;
}

bool DeleteSlot(game_data_page Page, game_data_slot_id ID) {
    for (int i = 0; i < Page.Header->nSlots; i++) {
        if (Page.Slots[i].ID == ID) {
            Page.Slots[i].Deleted = true;
            return true;
        }
    }
    return false;
}

game_data_slot* GetSlot(game_data_page Page, game_data_slot_id ID) {
    for (int i = 0; i < Page.Header->nSlots; i++) {
        if (Page.Slots[i].ID == ID) {
            return &Page.Slots[i];
        }
    }
    return nullptr;
}

typedef uint32 game_data_file_id;
struct game_data_file_header {
    char MagicNumber[2];
    uint32 HeaderSize;
    game_data_file_id ID;
    uint32 nPages;
};

struct game_data_file {
    game_data_file_header Header;
    
    read_file_result File;
};

/*
    128-bit identifier for arbitrary data. Each of the members of an ID must be greater than zero.
*/
struct game_data_id {
    game_data_file_id FileID;
    game_data_page_id PageID;
    game_data_slot_id SlotID;

    operator bool() const { return FileID && PageID && SlotID; }
};

struct game_data_manager {
    memory_arena FilesArena;
    uint32 nFiles;
    game_data_file_id NextFileID;

};

game_data_manager InitializeDataManager(const char* Path) {
    if (Platform.FileExists(Path)) {

    }
    else {

    }

    return {};
}

void CloseDataManager(game_data_manager Manager) {

}

game_data_file* CreateDataFile(game_data_manager* Manager, const char* Path) {    
    if (Platform.FileExists(Path)) {
        game_data_file_header* Header = (game_data_file_header*)Platform.ReadFileChunk(Path, 0, sizeof(game_data_file_header));
        if (Header->MagicNumber[0] != 'G' || Header->MagicNumber[1] != 'D') {
            Log(Error, "File already exists and is not a valid data file.");
        }
    }
    else {

    }

    return nullptr;
}











enum game_data_type {
    data_type_bool,
    data_type_int,
    data_type_float,
    data_type_string
};

uint32 GetSize(game_data_type Type) {
    switch(Type) {
        case data_type_bool:   return 1;
        case data_type_int:    return 4;
        case data_type_float:  return 4;
        case data_type_string: return 0;

        default: Raise("Invalid data type.");
    }

    return 0;
}

#endif