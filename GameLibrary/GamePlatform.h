#ifndef GAME_PLATFORM
#define GAME_PLATFORM

#include "stdint.h"
#include "string.h"

#ifndef INTROSPECT
#define INTROSPECT
#endif

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef size_t memory_index;

uint16 BigEndian(uint16 LittleEndian) {
    return (LittleEndian << 8) | (LittleEndian >> 8);
}

uint32 BigEndian(uint32 LittleEndian) {
    return ((LittleEndian >> 24) & 0x000000FF) |
           ((LittleEndian >> 8)  & 0x0000FF00) |
           ((LittleEndian << 8)  & 0x00FF0000) |
           ((LittleEndian << 24) & 0xFF000000);
}

uint64 BigEndian(uint64 LittleEndian) {
    return ((LittleEndian >> 56) & 0x00000000000000FF) |
           ((LittleEndian >> 40) & 0x000000000000FF00) |
           ((LittleEndian >> 24) & 0x0000000000FF0000) |
           ((LittleEndian >> 8)  & 0x00000000FF000000) |
           ((LittleEndian << 8)  & 0x000000FF00000000) |
           ((LittleEndian << 24) & 0x0000FF0000000000) |
           ((LittleEndian << 40) & 0x00FF000000000000) |
           ((LittleEndian << 56) & 0xFF00000000000000);
}

int16 BigEndian(int16 LittleEndian) {
    uint16 Unsigned = BigEndian(*(uint16*)&LittleEndian);
    return *(int16*)&Unsigned;
}

int32 BigEndian(int32 LittleEndian) {
    uint32 Unsigned = BigEndian(*(uint32*)&LittleEndian);
    return *(int32*)&Unsigned;
}

int64 BigEndian(int64 LittleEndian) {
    uint64 Unsigned = BigEndian(*(uint64*)&LittleEndian);
    return *(int64*)&Unsigned;
}

inline uint8 MSB64(uint32 X) {
	unsigned long Result = 0;
	_BitScanReverse(&Result, X);
	return (uint32)Result;
}

inline uint8 MSB32(uint64 X) {
	unsigned long Result = 0;
	_BitScanReverse64(&Result, X);
	return (uint32)Result;
}

inline void Assert(bool assertion, const char* Message = "") {
    if (!assertion) {
        int* i = 0;
        int j = *i;
    }
}

// Memory Arenas
struct memory_arena {
    memory_index Size;
    memory_index Used;
    uint8* Base;
};

inline void ZeroSize(memory_index Size, void* Memory) {
    memset(Memory, 0, Size);
}

inline memory_arena MemoryArena(memory_index Size, uint8* Base) {
    memory_arena Result;
    Result.Size = Size;
    Result.Base = Base;
    Result.Used = 0;
    return Result;
}

inline memory_arena AllocateMemoryArena(memory_index Size) {
    uint8* Base = (uint8*)calloc(1, Size);
    return MemoryArena(Size, Base);
}

inline void FreeMemoryArena(memory_arena* Arena) {
    free(Arena->Base);
    Arena = {};
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

inline memory_arena SuballocateMemoryArena(memory_arena* Arena, memory_index Size) {
    memory_arena Result = {};
    Result.Base = (uint8*)PushSize(Arena, Size);
    Result.Size = Size;
    return Result;
}

inline char* PushString(memory_arena* Arena, const char* String) {
    return PushArray(Arena, strlen(String) + 1, char);
}

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | Data structures                                                                                                                          |
// +------------------------------------------------------------------------------------------------------------------------------------------+

/*
    Stack. Memory for elements must be previously allocated.
*/

template <typename T> class stack {
private:
    T* Top;
    T* Base;
public:
    uint64 n;
    uint64 Capacity;

    stack(uint64 MaxSize = 32) {
        n = 0;
        T* Memory = (T*)calloc(32, sizeof(T));
        Top = Memory;
        Base = Memory;
        Capacity = MaxSize;
    }

    ~stack() {
        free(Top);
        n = 0;
        Capacity = 0;
    }

    void Push(T Element) {
        if (n < Capacity) {
            *Top++ = Element;
            n++;
        }
        else Assert(false, "Stack is full.");
    }

    T Pop() {
        if (n > 0) {
            Top--;
            T Result = *Top;
            *Top = {};
            n--;
            return Result;
        }
        return NULL;
    }

    void Clear() {
        ZeroSize(n * sizeof(T), Top - n);
        Top = Top - n;
        n = 0;
    }

    const T& operator[](uint32 i) const {
        Assert(0 <= i && i < n);
        return Base[i];
    }
};

struct link {
    link* Previous;
    link* Next;
    void* Data;
};

void Attach(link* Link1, link* Link2) {
    Assert(Link1 != NULL || Link2 != NULL, "Two empty links tried to be linked.");
    if (Link1 != NULL) {
        Link1->Next = Link2;
    }
    if (Link2 != NULL) {
        Link2->Previous = Link1;
    }
}

void Delete(link* ThisLink) {
    Attach(ThisLink->Previous, ThisLink->Next);
    ThisLink->Previous = NULL;
    ThisLink->Next = NULL;
}

/*
    Doubly-linked list. All links must have been allocated somewhere previously.
*/
struct linked_list {
    link* First;
    link* Last;

    void PushBack(link* Element) {
        if (First == NULL || Last == NULL) {
            First = Element;
        }
        else {
            Attach(Last, Element);
        }
        Last = Element;
    }

    void PushFront(link* Element) {
        if (First == NULL || Last == NULL) {
            Last = Element;
        }
        else {
            Attach(Element, First);
        }
        First = Element;
    }

    void MakeCircular() {
        Attach(Last, First);
    }

    void Break(link* Link) {
        if (First == Link) First = Link->Next;
        if (Last == Link)  Last  = Link->Previous;
        Delete(Link);
    }

    bool IsEmpty() {
        return First == NULL && Last == NULL;
    }
};

inline linked_list Concatenate(linked_list L1, linked_list L2) {
    linked_list Result = {};
    Result.First = L1.First;
    Attach(L1.Last, L2.First);
    Result.Last = L2.Last;
    return Result;
}

uint64 GetLength(linked_list List) {
	link* Link = List.First;
	uint64 Result = 0;
    do {
		Link = Link->Next;
        Result++;
    } while (Link && Link != List.First);
	return Result;
}

// Fixed length arrays

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


// Fixed length lists that track available slots

#define DefineFreeListRemove(type) void Remove(type##_list* List, int Index) { Assert(List->Count > 0); List->Count--; List->List[Index] = {}; List->FreeIDs[List->nFreeIDs++] = Index;}
#define DefineFreeList(maxNumber, type) struct type##_list {int nFreeIDs; int FreeIDs[maxNumber]; int Count; type List[maxNumber];}; DefineFreeListRemove(type)

// Naive implementation of exponential array (see https://www.youtube.com/watch?v=i-h95QIGchY)

const int MAX_XARRAY_CHUNKS = 32;

struct xarray_meta {
    uint8 Shift;          // Exponent for size of first chunk
    uint8 nChunks;
    uint32 ElementSize;
};

struct xarray_header {
    uint64 n;
    uint8* Chunks[];
};

static inline void* xar_get(xarray_header* Xar, xarray_meta Meta, uint64 i) {
    uint32 ChunkIndex = 0;
    uint64 ChunkSize = 1 << Meta.Shift;
    uint64 Element = i;

    uint64 nShifts = i >> Meta.Shift;
    if (nShifts > 0) {
        ChunkIndex = MSB64(nShifts);
        Element -= ChunkSize << ChunkIndex;
        ChunkIndex++;
    }

    return Xar->Chunks[ChunkIndex] + Element * Meta.ElementSize; 
}

template <typename T> class xarray {
private:
    xarray_meta Meta;
    xarray_header* Header;

    void NewChunk(memory_index Size) {
        if (Meta.nChunks >= MAX_XARRAY_CHUNKS) {
            Assert(false, "Xarray chunk index overflow.");
        }
        Header->Chunks[Meta.nChunks++] = (uint8*)malloc(Size);
    }

public:
    xarray() {
        Meta = { 4, 0, sizeof(T) };
        Header = (xarray_header*)calloc(MAX_XARRAY_CHUNKS + 1, sizeof(uint64));
        NewChunk(Meta.ElementSize * (1 << Meta.Shift));
    }

    ~xarray() {
        for (int i = 0; i < Meta.nChunks; i++) {
            free(Header->Chunks[i]);
        }

        free(Header);
    }

    const T& operator[] (uint64 i) const {
        Assert(0 <= i && i < Header->n, "Index out of range.");
        return *(T*)xar_get(Header, Meta, i);
    }

    T* Insert(const T& Element = {}) {
        uint64 TotalSize = Meta.ElementSize * (1 << (Meta.Shift + Meta.nChunks - 1));
        uint64 NewIndex = Header->n++;
        if (NewIndex * sizeof(T) >= TotalSize) {
            NewChunk(TotalSize);
        }

        T* Pointer = (T*)xar_get(Header, Meta, NewIndex);
        *Pointer = Element;
        return Pointer;
    }

    uint64 Size() {
        return Header->n;
    }

    void Clear() {
        Header->n = 0;
    }
};

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