#pragma once

#include <stdint.h>
#include <cstring>
#include <intrin.h>
#include <cassert>
#include <stdarg.h>
#include <math.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef size_t memory_size;

#define Kilobytes(value) ((value)*1024)
#define Megabytes(value) (Kilobytes(value)*1024)
#define Gigabytes(value) ((uint64)Megabytes(value)*1024)

#define INTROSPECT

#define ENUM_START(Name) enum Name {
#define ENUM_END(Name) Name##_Count }
#define ENUM(Name, ...) ENUM_START(Name) __VA_ARGS__, ENUM_END(Name);

#define FLAGS(...)

#define ArrayCount(arr) (sizeof((arr)) / sizeof((arr)[0]))

uint16 BigEndian(uint16 little_endian);
uint32 BigEndian(uint32 little_endian);
uint64 BigEndian(uint64 little_endian);
int16 BigEndian(int16 little_endian);
int32 BigEndian(int32 little_endian);
int64 BigEndian(int64 little_endian);
uint8 MSB32(uint32 x);
uint8 MSB64(uint64 x);

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Memory arenas                                                                                                                   |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

void ZeroSize(memory_size size, void* memory);

struct Arena {
    memory_size capacity;
    memory_size used;
    uint8* base;

    Arena(memory_size size);
    Arena(memory_size size, void* memory);
    Arena(Arena& arena, memory_size size);
    ~Arena();
    void Clear();
    void Free();
    void* PushSize(memory_size size);
    void PopSize(memory_size size);

    template <typename T>
    T* Push() {
        return (T*)PushSize(sizeof(T));
    }

    template <typename T>
    void Pop() {
        PopSize(sizeof(T));
    }

    template <typename T>
    T* PushArray(memory_size count) {
        return (T*)PushSize(count * sizeof(T));
    }

    template <typename T>
    void PopArray(memory_size count) {
        PopSize(count * sizeof(T));
    }
};

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Strings                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

class String {
public:
    int64 length;
    const char* content;

    String();
    String(const char* str);
    String(const char* str, int64 len);

    char operator[](int64 index) const;
    bool operator==(String rhs) const;

    String Slice(int start, int end) const;
    static char* Copy(Arena& arena, String string);
    static String Persist(Arena& arena, String string);
    static String Read(void* memory);
    static String Split(String& input, char separator);
    static String Join(Arena& arena, String separator, int count, ...);
    static String Format(Arena& arena, const char* format, int input_count, ...);
};
