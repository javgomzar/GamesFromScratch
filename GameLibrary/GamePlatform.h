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

// Services that the platform layer provides for the game
struct read_file_result {
    uint32 ContentSize;
    void* Content;
};

#define PLATFORM_READ_ENTIRE_FILE(name) read_file_result name(const char* Filename)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_WRITE_ENTIRE_FILE(name) bool name(const char* Filename, uint64 MemorySize, void* Memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(platform_write_entire_file);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_FREE_FILE_MEMORY(platform_free_file_memory);

struct platform_api {
    platform_read_entire_file* ReadEntireFile;
    platform_free_file_memory* FreeFileMemory;
    platform_write_entire_file* WriteEntireFile;
};