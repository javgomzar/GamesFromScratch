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

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Files                                                                                                                           |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

typedef uint32 game_data_file_id;

struct game_data_file_header {
    char MagicNumber[2];
    game_data_file_id ID;
    uint32 HeaderSize;
    uint32 Offset;
    uint32 nPages;
};

bool IsValid(game_data_file_header Header) {
    return Header.MagicNumber[0] == 'G' && Header.MagicNumber[1] == 'D';
}

game_data_file_header CreateFileHeader(game_data_file_id ID) {
    game_data_file_header Result = {};
    Result.MagicNumber[0] = 'G';
    Result.MagicNumber[1] = 'D';
    Result.HeaderSize = sizeof(game_data_file_header);
    Result.Offset = sizeof(game_data_file_header);
    Result.nPages = 0;
    Result.ID = ID;
    return Result;
}

game_data_file_header GetFileHeader(const char* Path) {
    game_data_file_header Result = {};
    file_chunk_info Info = Platform.ReadFileChunk(Path, 0, sizeof(game_data_file_header), &Result);
    bool Valid = IsValid(Result);
    if (!Valid) {
        Log(Error, "File is corrupted.");
    }
    return Result;
}

struct game_data_file {
    game_data_file_header Header;
    file_info Info;
};

struct game_data_file_manager {
    char Directory[MAX_PATH_LENGTH];
    game_data_file* Files;
    uint32 Count;
    uint32 Size;
    game_data_file_id NextID;
};

game_data_file_manager CreateDataFileManager(const char* Directory) {
    game_data_file_manager Result = {};
    strcpy_s(Result.Directory, Directory);
    Result.Size = 32;
    Result.Files = (game_data_file*)calloc(Result.Size, sizeof(game_data_file));
    Result.Count = 0;
    Result.NextID = 1;

    return Result;
}

void SaveDataFileManager(game_data_file_manager Manager) {
    uint32 FileSize = 3 * sizeof(uint32) + Manager.Count * sizeof(game_data_file);
    void* Memory = calloc(1, FileSize);

    uint32* Pointer = (uint32*)Memory;
    *Pointer++ = Manager.Count;
    *Pointer++ = Manager.Size;
    *Pointer++ = Manager.NextID;
    memcpy(Pointer, Manager.Files, Manager.Count * sizeof(game_data_file));

    Platform.WriteEntireFile(Manager.Directory, FileSize, Memory);
    free(Memory);
}

game_data_file_manager ReadDataFileManager(const char* Directory) {
    game_data_file_manager Result = {};
    strcpy_s(Result.Directory, Directory);
    
    file_info File;
    void* ReadMemory = Platform.ReadEntireFile(Directory, &File);

    uint32* Pointer = (uint32*)ReadMemory;
    Result.Count = *Pointer++;
    Result.Size = *Pointer++;
    Result.NextID = *Pointer++;

    Result.Files = (game_data_file*)calloc(Result.Size, sizeof(game_data_file));
    memcpy(Result.Files, Pointer, Result.Count * sizeof(game_data_file));

    Platform.FreeMemory(ReadMemory);

    return Result;
}

game_data_file_manager InitializeDataFileManager(const char* Directory) {
    if (Platform.FileExists(Directory)) {
        return ReadDataFileManager(Directory);
    }
    return CreateDataFileManager(Directory);
}

void CloseDataFileManager(game_data_file_manager Manager) {
    SaveDataFileManager(Manager);
    free(Manager.Files);
}

game_data_file* AddDataFile(game_data_file_manager* Manager, const char* Path) {
    game_data_file* Result = nullptr;
    if (Manager->Count + 1 == Manager->Size) {
        Manager->Size *= 2;
        Manager->Files = (game_data_file*)realloc(Manager->Files, Manager->Size);
    }
    Result = Manager->Files + Manager->Count++;
    Result->Header = CreateFileHeader(Manager->NextID++);

    return Result;
}

game_data_file* GetDataFile(game_data_file_manager* Manager, game_data_file_id ID) {
    game_data_file* Result = nullptr;
    for (int i = 0; i < Manager->Count; i++) {
        game_data_file* File = Manager->Files + i;
        if (File->Header.ID == ID) {
            Result = File;
            break;
        }
    }
    return Result;
}

game_data_file* GetOrCreateDataFile(game_data_file_manager* Manager, const char* Path) {
    game_data_file* Result = nullptr;
    bool Exists = Platform.FileExists(Path);
    if (Exists) {
        game_data_file_header Header = GetFileHeader(Path);

        if (IsValid(Header)) {
            Result = GetDataFile(Manager, Header.ID);
            if (Result) {
                return Result;
            }
            Result = AddDataFile(Manager, Path);
            Header.ID = Result->Header.ID;
            Result->Header = Header;
            return Result;
        }
    }
    Result = AddDataFile(Manager, Path);
    Platform.WriteEntireFile(Path, sizeof(game_data_file_header), &Result->Header);
    Result->Info = Platform.GetFileInfo(Path);
    return Result;
}

/*
    128-bit identifier for arbitrary data. Each of the members of an ID must be greater than zero.
*/
struct game_data_id {
    game_data_file_id FileID;
    game_data_page_id PageID;
    game_data_slot_id SlotID;

    operator bool() const { return FileID && PageID && SlotID; }
};

struct game_data_frame {
    game_data_file_id FileID;
    game_data_page_id PageID;
    uint8 FrameIndex;
    uint8 References;
    bool Dirty;
    bool Pinned;
};

#define GAME_DATA_FILE_PATH_LENGTH 256

#define GAME_DATA_FRAME_POOL_SIZE 128

struct game_data_manager {
    game_data_file_manager FileManager;
    game_data_frame Frames[GAME_DATA_FRAME_POOL_SIZE];
    uint8* FramePool;
};

game_data_manager CreateDataManager() {
    game_data_manager Result = {};

    memory_index FramePoolSize = GAME_DATA_FRAME_POOL_SIZE * GAME_DATA_PAGE_SIZE;
    Result.FramePool = (uint8*)Platform.AllocateMemory(FramePoolSize);

    return Result;
}

void CloseDataManager(game_data_manager Manager) {
    Platform.FreeMemory(Manager.FramePool);
}

enum game_data_type {
    data_type_bool,
    data_type_int,
    data_type_float,
    data_type_string,
    data_type_binary
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