#ifndef GAME_PLATFORM
#define GAME_PLATFORM

#include "stdint.h"
#include "string.h"

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef size_t memory_index;

inline void Assert(bool assertion, const char* Message = "") {
    if (!assertion) {
        int* i = 0;
        int j = *i;
    }
}

// Arrays
#define ArrayStructDefinition(Capacity, Type) struct Type##_array { uint32 Size = Capacity; uint32 Count = 0; Type Content[Capacity]; }
#define ArrayAppendDefinition(Capacity, Type) void Append(Type##_array* Array, Type Element) { Assert(Array->Count < Capacity); \
    Array->Content[Array->Count++] = Element; }
#define ArrayPopDefinition(Capacity, Type) Type Pop(Type##_array* Array) { Assert(Array->Count > 0); \
    Type Result = Array->Content[Array->Count]; Array->Content[Array->Count--] = {}; return Result; }
#define ArrayClearDefinition(Type) void Clear(Type##_array* Array) { for(int i = 0; i < Array->Count; i++) Array->Content[i] = {}; Array->Count = 0; }
#define ArrayDefinition(Capacity, Type) \
    ArrayStructDefinition(Capacity, Type); \
    ArrayAppendDefinition(Capacity, Type); \
    ArrayPopDefinition(Capacity, Type); \
    ArrayClearDefinition(Type)

#define ArrayCount(arr) (sizeof((arr)) / sizeof((arr)[0]))

// Memory Arenas
struct memory_arena {
    memory_index Size;
    uint8* Base;
    memory_index Used;
};

inline memory_arena MemoryArena(memory_index Size, uint8* Base) {
    memory_arena Result;
    Result.Size = Size;
    Result.Base = Base;
    Result.Used = 0;
    return Result;
}

inline void ZeroSize(memory_index Size, void* Ptr) {
    uint8* Byte = (uint8*)Ptr;
    while (Size--) {
        *Byte++ = 0;
    }
}

inline void ClearArena(memory_arena* Arena) {
    ZeroSize(Arena->Used, Arena->Base);
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, Count*sizeof(type))
#define PushSize(Arena, Size) (void*)PushSize_(Arena, Size)
inline void* PushSize_(memory_arena* Arena, memory_index Size) {
    Assert(Arena->Size >= Arena->Used + Size);
    void* Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return Result;
}

#define PopStruct(Arena, type) (type *)PopSize_(Arena, sizeof(type))
#define PopArray(Arena, Count, type) (type *)PopSize_(Arena, Count*sizeof(type))
#define PopSize(Arena, Size) (void*)PopSize_(Arena, Size)
inline void* PopSize_(memory_arena* Arena, memory_index Size) {
    memory_index BytesErased = Size < Arena->Used? Size : Arena->Used;
    void* Result = (void*)(Arena->Base + Arena->Used - BytesErased);
    ZeroSize(BytesErased, Result);
    Arena->Used -= BytesErased;
    return Result;
}

inline char* PushString(memory_arena* Arena, const char* String) {
    return PushArray(Arena, strlen(String) + 1, char);
}

// Services that the platform layer provides for the game
struct read_file_result {
    char Path[512];
    int64 Timestamp;
    uint32 ContentSize;
    void* Content;
};

// Multithreading
// struct thread_info {
//     int ID;
//     bool Running;
// };

// struct work_queue_entry {
//    const char* String;
// };

// int EntryCount = 0;
// int NextEntryToDo = 0;
// work_queue_entry Entries[1000];

// std::mutex Mutex;

// void PushWorkEntry(const char* String) {
//    Assert(EntryCount < 1000);

//    std::lock_guard<std::mutex> guard(Mutex);
//    work_queue_entry* Entry = &Entries[EntryCount++];
//    Entry->String = String;
// }

// void ThreadProc(thread_info* ThreadInfo) {
//    char Buffer[256];
//    sprintf_s(Buffer, "Thread %u started.\n", ThreadInfo->ID);
//    OutputDebugStringA(Buffer);

//    while(ThreadInfo->Running) {
//        Mutex.lock();

//        int CurrentEntryCount = EntryCount;
//        if (CurrentEntryCount > 0) {
//            int EntryIndex = NextEntryToDo++;
//            EntryCount--;
//            Mutex.unlock();

//            work_queue_entry* Entry = &Entries[EntryIndex];

//            sprintf_s(Buffer, "Thread %u: %s; EntryCount=%u, EntryIndex=%u\n", ThreadInfo->ID, Entry->String, CurrentEntryCount, EntryIndex);
//            OutputDebugStringA(Buffer);
//        }
//        else {
//            Mutex.unlock();
//            ThreadInfo->Running = false;
//        }
//    }

//    sprintf_s(Buffer, "Thread %u was shut down.\n", ThreadInfo->ID);
//    OutputDebugStringA(Buffer);
// }

// void TestThreads() {
//    std::thread Threads[5];
//    thread_info ThreadInfos[5];

//    for (int i = 0; i < 5; i++) {
//        thread_info* ThreadInfo = &ThreadInfos[i];
//        ThreadInfo->ID = i;
//        ThreadInfo->Running = true;
//        Threads[i] = std::thread(ThreadProc, ThreadInfo);
//    }

//    for (int i = 0; i < 5; i++) {
//        Threads[i].join();
//    }
// }

#define PLATFORM_READ_ENTIRE_FILE(name) read_file_result name(const char* Path)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_WRITE_ENTIRE_FILE(name) bool name(const char* Path, uint64 MemorySize, void* Memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);

#define PLATFORM_APPEND_TO_FILE(name) bool name(const char* Path, uint64 MemorySize, void* Memory)
typedef PLATFORM_APPEND_TO_FILE(platform_append_to_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

struct platform_api {
    platform_read_entire_file* ReadEntireFile;
    platform_free_file_memory* FreeFileMemory;
    platform_write_entire_file* WriteEntireFile;
    platform_append_to_file* AppendToFile;
};

#endif