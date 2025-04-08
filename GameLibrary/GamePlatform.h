#ifndef GAME_PLATFORM
#define GAME_PLATFORM

#include <stdint.h>
#include <time.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef size_t memory_index;

// Assert
inline void Assert(bool assertion, const char* Message = "") {
    if (!assertion) {
        throw("Assert failed");
    }
}

// Strings
struct string {
    int Length;
    char* Content;
};

// Array count
#define ArrayCount(arr) (sizeof((arr)) / sizeof((arr)[0]))

// Memory Arenas
struct memory_arena {
    memory_index Size;
    uint8* Base;
    memory_index Used;
    string Name;
    string Percentage;
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

inline string PushString(memory_arena* Arena, int Length, const char* Content) {
    string String = { 0 };
    String.Length = Length;
    String.Content = PushArray(Arena, Length, char);

    for (int i = 0; i < Length; i++) {
        String.Content[i] = Content[i];
    }

    return String;
}

// Services that the platform layer provides for the game
struct read_file_result {
    char Path[512];
    int64 Timestamp;
    uint32 ContentSize;
    void* Content;
};

// Multithreading
struct thread_info {
    int ID;
    bool Running;
};

#define PLATFORM_READ_ENTIRE_FILE(name) read_file_result name(const char* Path)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_WRITE_ENTIRE_FILE(name) bool name(const char* Path, uint64 MemorySize, void* Memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);

#define PLATFORM_APPEND_TO_FILE(name) bool name(const char* Path, uint64 MemorySize, void* Memory)
typedef PLATFORM_APPEND_TO_FILE(platform_append_to_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

typedef void platform_opengl_render(struct render_group* Group);

struct platform_api {
    platform_read_entire_file* ReadEntireFile;
    platform_free_file_memory* FreeFileMemory;
    platform_write_entire_file* WriteEntireFile;
    platform_append_to_file* AppendToFile;
};

#endif