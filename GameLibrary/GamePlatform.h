#pragma once
#include "stdint.h"

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
void Assert(bool assertion) {
    if (!assertion) {
        throw("Assert failed");
    }
}

// Memory Arenas
struct memory_arena {
    memory_index Size;
    uint8* Base;
    memory_index Used;
};

void InitializeArena(memory_arena* Arena, memory_index Size, uint8* Base) {
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

void ZeroSize(memory_index Size, void* Ptr) {
    uint8* Byte = (uint8*)Ptr;
    while (Size--) {
        *Byte++ = 0;
    }
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, Count*sizeof(type))
#define PushSize(Arena, Size) (void*)PushSize_(Arena, Size)
void* PushSize_(memory_arena* Arena, memory_index Size) {
    Assert(Arena->Size >= Arena->Used + Size);
    void* Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return Result;
}

#define PopStruct(Arena, type) (type *)PopSize_(Arena, sizeof(type))
#define PopArray(Arena, Count, type) (type *)PopSize_(Arena, Count*sizeof(type))
#define PopSize(Arena, Size) (void*)PopSize_(Arena, Size)
void* PopSize_(memory_arena* Arena, memory_index Size) {
    memory_index BytesErased = Size < Arena->Used? Size : Arena->Used;
    void* Result = (void*)(Arena->Base + Arena->Used - BytesErased);
    ZeroSize(BytesErased, Result);
    Arena->Used -= BytesErased;
    return Result;
}


// Services that the platform layer provides for the game
struct read_file_result {
    uint32 ContentSize;
    void* Content;
};

// File loading
// BMP
#pragma pack(push, 1)
struct bitmap_header {
    uint16 FileType;
    uint32 FileSize;
    uint16 Reserved1;
    uint16 Reserved2;
    uint32 BitmapOffset;
    uint32 Size;
    int32 Width;
    int32 Height;
    uint16 Planes;
    uint16 BitsPerPixel;
    uint32 Compression;
    uint32 SizeOfBitmap;
    int32 HorzResolution;
    int32 VertResolution;
    uint32 ColorUser;
    uint32 ColorsImportant;
    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
};
#pragma pack(pop)

struct loaded_bmp {
    bitmap_header Header;
    uint32 Handle;
    uint32 BytesPerPixel;
    uint32 Pitch;
    uint32 AlphaMask;
    uint32* Content;
};

struct Character {
    unsigned char Letter;
    signed long Advance;
    signed long Width;
    signed long Height;
    int Left;
    int Top;
    loaded_bmp* Bitmap;
};

#define PLATFORM_READ_ENTIRE_FILE(name) read_file_result name(const char* Filename)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_WRITE_ENTIRE_FILE(name) bool name(const char* Filename, uint64 MemorySize, void* Memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

typedef void platform_opengl_render(struct render_group* Group, int32 Width, int32 Height);

struct platform_api {
    platform_read_entire_file* ReadEntireFile;
    platform_free_file_memory* FreeFileMemory;
    platform_write_entire_file* WriteEntireFile;
    platform_opengl_render* OpenGLRender;
};