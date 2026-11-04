#ifndef GAME_PLATFORM
#define GAME_PLATFORM

#include <pch.h>

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

uint16 BigEndian(uint16 little_endian) {
    return (little_endian << 8) | (little_endian >> 8);
}

uint32 BigEndian(uint32 little_endian) {
    return ((little_endian >> 24) & 0x000000FF) |
           ((little_endian >> 8)  & 0x0000FF00) |
           ((little_endian << 8)  & 0x00FF0000) |
           ((little_endian << 24) & 0xFF000000);
}

uint64 BigEndian(uint64 little_endian) {
    return ((little_endian >> 56) & 0x00000000000000FF) |
           ((little_endian >> 40) & 0x000000000000FF00) |
           ((little_endian >> 24) & 0x0000000000FF0000) |
           ((little_endian >> 8)  & 0x00000000FF000000) |
           ((little_endian << 8)  & 0x000000FF00000000) |
           ((little_endian << 24) & 0x0000FF0000000000) |
           ((little_endian << 40) & 0x00FF000000000000) |
           ((little_endian << 56) & 0xFF00000000000000);
}

int16 BigEndian(int16 little_endian) {
    uint16 Unsigned = BigEndian(*(uint16*)&little_endian);
    return *(int16*)&Unsigned;
}

int32 BigEndian(int32 little_endian) {
    uint32 Unsigned = BigEndian(*(uint32*)&little_endian);
    return *(int32*)&Unsigned;
}

int64 BigEndian(int64 little_endian) {
    uint64 Unsigned = BigEndian(*(uint64*)&little_endian);
    return *(int64*)&Unsigned;
}

inline uint8 MSB32(uint32 x) {
	unsigned long result = 0;
#ifdef _WIN32
	_BitScanReverse(&result, x);
#else
    result = 31 - __builtin_clz(x);
#endif
	return (uint32)result;
}

inline uint8 MSB64(uint64 x) {
	unsigned long result = 0;
#ifdef _WIN32
	_BitScanReverse64(&result, x);
#else
    result = 31 - __builtin_clzll(x);
#endif
	return (uint32)result;
}

inline void Assert(bool assertion, const char* message = "") {
#ifdef _DEBUG
    if (!assertion)
        throw message;
#endif
}

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Memory arenas                                                                                                                   |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

struct Arena {
    memory_size size;
    memory_size used;
    uint8* base;
};

inline void ZeroSize(memory_size size, void* memory) {
    if (size > 0) {
        memset(memory, 0, size);
    }
}

inline Arena MemoryArena(memory_size size, void* base) {
    Arena result;
    result.size = size;
    result.base = (uint8*)base;
    result.used = 0;
    return result;
}

inline Arena AllocateMemoryArena(memory_size size) {
    uint8* base = (uint8*)calloc(1, size);
    return MemoryArena(size, base);
}

inline void Free(Arena& arena) {
    free(arena.base);
    arena.base = nullptr;
    arena.size = 0;
    arena.used = 0;
}

inline void Clear(Arena& arena) {
    ZeroSize(arena.used, arena.base);
    arena.used = 0;
}

#define PushStruct(arena, type) (type *)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)PushSize_(arena, count*sizeof(type))
#define PushSize(arena, size) (void*)PushSize_(arena, size)
inline void* PushSize_(Arena& arena, memory_size size) {
    Assert(arena.size >= arena.used + size);
    void* result = arena.base + arena.used;
    arena.used += size;
    return result;
}

#define PopStruct(arena, type) (type *)PopSize_(arena, sizeof(type))
#define PopArray(arena, count, type) (type *)PopSize_(arena, count*sizeof(type))
#define PopSize(arena, size) (void*)PopSize_(arena, size)
inline void* PopSize_(Arena& arena, memory_size size) {
    memory_size bytes_erased = size < arena.used ? size : arena.used;
    void* result = (void*)(arena.base + arena.used - bytes_erased);
    ZeroSize(bytes_erased, result);
    arena.used -= bytes_erased;
    return result;
}

inline Arena SuballocateMemoryArena(Arena& arena, memory_size size) {
    Arena result = {};
    result.base = (uint8*)PushSize(arena, size);
    result.size = size;
    return result;
}

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Strings                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

struct String {
    int64 length;
    const char* content;

    String() {
        length = 0;
        content = nullptr;
    }

    String(const char* String) {
        length = strlen(String);
        content = String;
    }

    String(const char* String, int L) {
        length = L;
        content = String;
    }

    char operator[](int index) {
        if (index < -length || index > length) {
            return '\0';
        }
        if (index < 0) {
            return content[length + index];
        }
        return content[index];
    };
};

char* CopyString(Arena& arena, String string) {
    char* result = PushArray(arena, string.length, char);
    strncpy(result, string.content, string.length);
    return result;
}

String PersistString(Arena& arena, String string) {
    int* length = PushArray(arena, 1, int);
    *length = string.length;
    char* content = CopyString(arena, string);
    return String(content, string.length);
}

String ReadString(void* memory) {
    int* length = (int*)memory;
    String result;
    result.length = *length;
    result.content = (char*)(length + 1);
    return result;
}

bool operator==(String s1, String s2) {
    bool result = s1.length == s2.length;
    if (result) {
        for (int i = 0; i < s1.length; i++) {
            if (s1[i] != s2[i]) {
                return false;
            }
        }
    }
    return result;
}

String Slice(String string, int start, int end) {
    Assert(end >= start);
    String result;
    result.length = end - start;
    result.content = string.content + start;
    return result;
}

String Format(Arena& arena, const char* format, int input_count, ...) {
    String result = {};
    int format_length = strlen(format);

    if (format_length == 0) {
        result.content = nullptr;
        return result;
    }

    char* pointer = (char*)(arena.base + arena.used);
    result.content = pointer;

    va_list args;
    va_start(args, input_count);

    int dest_index = 0;
    for (int source_index = 0; source_index < format_length; source_index++) {
        if (format[source_index] == '{') {
            switch (format[++source_index]) {
                case 'i': {
                    int32 input = va_arg(args, int32);
                    if (input == 0) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    else if (input < 0) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '-';
                        result.length++;
                        input = -input;
                    }
                    int digit_count = 0;
                    char digits[16];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'I': {
                    int64 input = va_arg(args, int64);
                    if (input == 0) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    else if (input < 0) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '-';
                        result.length++;
                        input = -input;
                    }
                    int digit_count = 0;
                    char digits[32];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'u': {
                    uint32 input = va_arg(args, uint32);
                    if (input == 0) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    int digit_count = 0;
                    char digits[16];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'U': {
                    uint64 input = va_arg(args, uint64);
                    if (input == 0) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    int digit_count = 0;
                    char digits[32];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'x': {
                    uint64 input = va_arg(args, uint64);
                    PushArray(arena, 18, char);
                    pointer[dest_index++] = '0';
                    pointer[dest_index++] = 'x';
                    for (int i = 15; i >= 0; i--) {
                        uint64 r = input % 16;
                        if (r < 10) {
                            pointer[dest_index + i] = '0' + r;
                        }
                        else {
                            pointer[dest_index + i] = 'a' + (r - 10);
                        }
                        input >>= 4;
                    }
                    dest_index += 16;
                    result.length += 18;
                } break;

                case 'f': {
                    double input = va_arg(args, double);
                    if (input < 0) {
                        input = -input;
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '-';
                        result.length++;
                    }

                    uint64 integer_part = floor(input);
                    double decimal_part = input - integer_part;

                    if (integer_part == 0) {
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '0';
                        result.length++;
                    }

                    int digit_count = 0;
                    char digits[32];
                    while (integer_part > 0) {
                        digits[digit_count++] = '0' + (integer_part % 10);
                        integer_part /= 10;
                    }
                    
                    PushArray(arena, digit_count + 1, char);
                    for (int i = 0; i < digit_count; i++) {
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                    }
                    pointer[dest_index++] = '.';
                    result.length += digit_count + 1;

                    int decimal_count = format[++source_index] - '0';
                    for (int i = 0; i < decimal_count; i++) {
                        decimal_part *= 10.0;
                        int digit = decimal_part;
                        PushArray(arena, 1, char);
                        pointer[dest_index++] = '0' + digit;
                        result.length++;
                        decimal_part -= digit;
                    }
                } break;

                case 's': {
                    String input = va_arg(args, String);
                    result.length += input.length;
                    PushArray(arena, input.length, char);
                    for (int i = 0; i < input.length; i++) {
                        pointer[dest_index++] = input[i];
                    }
                } break;

                default: {
                    Assert(false, "Invalid format flag.");
                }
            }

            Assert(format[source_index + 1] == '}');
            source_index++;
        }
        else {
            PushArray(arena, 1, char);
            pointer[dest_index++] = format[source_index];
            result.length++;
        }
    }
    PushArray(arena, 1, char);
    va_end(args);
    return result;
}

/*
    Split a String given a separator. Beware that the `Input` parameter will be modified.
*/
String Split(String& input, char separator) {
    String result;
    result.length = 0;
    result.content = input.content;
    for (int i = 0; i < input.length; i++) {
        if (input[i] == separator) {
            return result;
        }
        else {
            input.content++;
            input.length--;
            result.length++;
        }
    }
    return result;
}

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Logging                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

enum class Log_Mode {
    File,
    Terminal
};

enum class Log_Level {
    Info,
    Test,
    Warn,
    Error,
    Fatal
};

Log_Mode LOG_MODE = Log_Mode::Terminal;

void Log(Log_Level Level, String Content);

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| Data structures                                                                                                                          |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

/*
    Memory for elements must be previously allocated.
*/
template <typename T> class Stack {
private:
    T* top;
    T* base;
public:
    uint64 n;
    uint64 capacity;

    Stack(uint64 max_size = 32) {
        n = 0;
        T* memory = (T*)calloc(32, sizeof(T));
        top = memory;
        base = memory;
        capacity = max_size;
    }

    ~Stack() {
        free(top);
        n = 0;
        capacity = 0;
    }

    void Push(T element) {
        if (n < capacity) {
            *top++ = element;
            n++;
        }
        else Assert(false, "Stack is full.");
    }

    T Pop() {
        if (n > 0) {
            top--;
            T result = *top;
            *top = {};
            n--;
            return result;
        }
        return {};
    }

    void Clear() {
        ZeroSize(n * sizeof(T), top - n);
        top -= n;
        n = 0;
    }

    const T& operator[](uint32 i) const {
        Assert(0 <= i && i < n);
        return base[i];
    }
};

struct Link {
    Link* previous;
    Link* next;
    void* data;
};

void Attach(struct Link* link1, struct Link* link2) {
    Assert(link1 || link2, "Two empty links tried to be linked.");
    if (link1) {
        link1->next = link2;
    }
    if (link2) {
        link2->previous = link1;
    }
}

void Delete(struct Link* this_link) {
    Attach(this_link->previous, this_link->next);
    this_link->previous = nullptr;
    this_link->next = nullptr;
}

/*
    Doubly-linked list. All links must have been allocated somewhere previously.
*/
struct Linked_List {
    struct Link* first;
    struct Link* last;

    void PushBack(struct Link* element) {
        if (!first || !last) {
            first = element;
        }
        else {
            Attach(last, element);
        }
        last = element;
    }

    void PushFront(struct Link* element) {
        if (!first || !last) {
            last = element;
        }
        else {
            Attach(element, first);
        }
        first = element;
    }

    void MakeCircular() {
        Attach(last, first);
    }

    void Break(struct Link* link) {
        if (first == link) first = link->next;
        if (last == link)  last  = link->previous;
        Delete(link);
    }

    bool IsEmpty() {
        return !first && !last;
    }
};

inline Linked_List Concatenate(Linked_List list1, Linked_List list2) {
    Linked_List result = {};
    result.first = list1.first;
    Attach(list1.last, list2.first);
    result.last = list2.last;
    return result;
}

uint64 GetLength(Linked_List list) {
	struct Link* link = list.first;
	uint64 result = 0;
    do {
		link = link->next;
        result++;
    } while (link && link != list.first);
	return result;
}

/*
    Fixed length array.
*/
template <typename T> class Array {
private:
    T* data;
public:
    uint32 length;

    Array() {
        length = 0;
        data = nullptr;
    }

    Array(Arena& arena, uint32 N) {
        length = N;
        data = PushArray(arena, N, T);
    }

    ~Array() {
        length = 0;
        data = nullptr;
    }

    T& operator[](int64 index) const {
        if (-index > length || index >= length) {
            Log(Log_Level::Error, "Array index out of bounds.");
        }
        else if (index >= 0) {
            return data[index];
        }
        else if (index < 0) {
            return data[length - index];
        }
    }
};

/*
    Fixed length lists that track available slots.
*/
template <typename T> class Free_List {
private:
    T* slots;
    uint32* free_slots;
public:
    bool* occupied;
    uint32 size = 0;
    uint32 count = 0;
    uint32 free_count = 0;

    Free_List(Arena& arena, uint32 n) {
        size = n;
        slots = PushArray(arena, n, T);
        free_slots = PushArray(arena, n, uint32);
        occupied = PushArray(arena, n, bool);
    }

    uint32 Insert(T element) {
        Assert(count < size);
        
        uint32 result;
        if (free_count > 0) {
            result = free_slots[free_count--];
        }
        else {
            result = count++;
        }

        slots[result] = element;
        occupied[result] = true;
        return result;
    }

    T& operator[](uint32 index) {
        return slots[index];
    }

    T operator[](uint32 index) const {
        return slots[index];
    }

    void Remove(uint32 index) {
        if (occupied[index]) {
            free_slots[free_count++] = index;
            occupied[index] = false;
            count--;
        }
    }
};

// Naive implementation of exponential array (see https://www.youtube.com/watch?v=i-h95QIGchY)

const int MAX_XARRAY_CHUNKS = 32;

struct XArray_Meta {
    uint8 shift;          // Exponent for size of first chunk
    uint8 chunk_count;
    uint32 element_size;
};

struct XArray_Header {
    uint64 n;
    uint8* chunks[];
};

static inline void* XArrayGet(XArray_Header* header, XArray_Meta meta, uint64 i) {
    uint32 chunk_index = 0;
    uint64 chunk_size = 1 << meta.shift;
    uint64 element = i;

    uint64 shift_count = i >> meta.shift;
    if (shift_count > 0) {
        chunk_index = MSB64(shift_count);
        element -= chunk_size << chunk_index;
        chunk_index++;
    }

    return header->chunks[chunk_index] + element * meta.element_size; 
}

template <typename T> class XArray {
private:
    XArray_Meta meta;
    XArray_Header* header;

    void NewChunk(memory_size size) {
        if (meta.chunk_count >= MAX_XARRAY_CHUNKS) {
            Assert(false, "Xarray chunk index overflow.");
        }
        header->chunks[meta.chunk_count++] = (uint8*)malloc(size);
    }

public:
    XArray() {
        meta = { 4, 0, sizeof(T) };
        header = (XArray_Header*)calloc(MAX_XARRAY_CHUNKS + 1, sizeof(void*));
        NewChunk(meta.element_size * (1 << meta.shift));
    }

    ~XArray() {
        for (int i = 0; i < meta.chunk_count; i++) {
            free(header->chunks[i]);
        }

        free(header);
    }

    const T& operator[] (uint64 i) const {
        Assert(0 <= i && i < header->n, "Index out of range.");
        return *(T*)XArrayGet(header, meta, i);
    }

    T* Insert(const T& element = {}) {
        uint64 total_size = meta.element_size * (1 << (meta.shift + meta.chunk_count - 1));
        uint64 new_index = header->n++;
        if (header->n * sizeof(T) >= total_size) {
            NewChunk(total_size);
        }

        T* pointer = (T*)XArrayGet(header, meta, new_index);
        *pointer = element;
        return pointer;
    }

    uint64 Size() {
        return header->n;
    }

    void Clear() {
        header->n = 0;
    }
};

template <typename S, typename T> class Hash_Table {
private:
    memory_size mask;
    memory_size used;
    struct Entry {
        uint64 hash;
        S key;
        T value;
    }* entries;
    bool* occupied;

    uint64 Hash(S key) {
        return XXH64(&key, sizeof(S), 0);
    }

    void Resize() {
        memory_size size = mask + 1;
        memory_size new_size = size << 1;
        memory_size new_mask = new_size - 1;
        void* new_memory = calloc(new_size, sizeof(bool) + sizeof(Entry));
        Entry* new_entries = (Entry*)new_memory;
        bool* new_occupied = (bool*)(new_entries + new_size);

        for (memory_size i = 0; i < size; i++) {
            if (occupied[i]) {
                Entry* entry = entries + i;
                memory_size new_index = entry->hash & new_mask;
    
                while (new_occupied[new_index]) {
                    new_index = (new_index + 1) & new_mask;
                }
                
                Entry* new_entry = new_entries + new_index;
                *new_entry = *entry;
                new_occupied[new_index] = true;
            }
        }

        free(entries);
        entries = new_entries;
        occupied = new_occupied;
        mask = new_mask;
    }

    memory_size Find(uint64 hash, S key, bool& found) {
        memory_size index = hash & mask;

        memory_size test_index = index;
        while (occupied[test_index]) {
            Entry* entry = entries + test_index;
            if (entry->hash == hash && entry->key == key) {
                found = true;
                return test_index;
            }
            test_index = (test_index + 1) & mask;
            if (test_index == index) break;
        }
        found = false;
        return test_index;
    }

public:
    Hash_Table(uint32 size_exponent = 6) {
        used = 0;
        memory_size size = (uint64)1 << size_exponent;
        mask = size - 1;
        void* memory = calloc(size, sizeof(bool) + sizeof(Entry));
        entries = (Entry*)memory;
        occupied = (bool*)(entries + size);
    }

    ~Hash_Table() {
        free(entries);
        used = 0;
        mask = 0;
        entries = nullptr;
        occupied = nullptr;
    }

    void Insert(S key, T value) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        Entry* entry = entries + index;
        if (!found) {
            used += 1;
            // Resize if 50% full
            if (2 * used >= mask + 1) {
                Resize();
                index = Find(hash, key, found);
                entry = entries + index;
            }
            occupied[index] = true;
            entry->hash = hash;
            entry->key = key;
        }
        entry->value = value;
    }

    void Delete(S key) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (!found) return;

        used -= 1;
        memory_size next_index = index;
        while (true) {
            next_index = (next_index + 1) & mask;
            if (!occupied[next_index]) break;
            Entry* next_entry = entries + next_index;
            memory_size ideal_index = next_entry->hash & mask;
            bool stays = (index <= next_index) ?
                (index < ideal_index && ideal_index <= next_index) : 
                (index < ideal_index || ideal_index <= next_index);
            if (stays) continue;
            entries[index] = entries[next_index];
            index = next_index;
        }
        occupied[index] = false;
    }

    bool Get(S key, T** value) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (found) *value = &entries[index].value;
        return found;
    }
};

template <typename T> class Hash_Table<String, T> {
private:
    memory_size mask;
    memory_size used;
    struct Entry {
        uint64 hash;
        String key;
        T value;
    }* entries;
    bool* occupied;

    uint64 Hash(String key) {
        return XXH64(key.content, key.length, 0);
    }

    void Resize() {
        memory_size size = mask + 1;
        memory_size new_size = size << 1;
        memory_size new_mask = new_size - 1;
        void* new_memory = calloc(new_size, sizeof(bool) + sizeof(Entry));
        Entry* new_entries = (Entry*)new_memory;
        bool* new_occupied = (bool*)(new_entries + new_size);

        for (memory_size i = 0; i < size; i++) {
            if (occupied[i]) {
                Entry* entry = entries + i;
                memory_size new_index = entry->hash & new_mask;
    
                while (new_occupied[new_index]) {
                    new_index = (new_index + 1) & new_mask;
                }
                
                Entry* new_entry = new_entries + new_index;
                *new_entry = *entry;
                new_occupied[new_index] = true;
            }
        }

        free(entries);
        entries = new_entries;
        occupied = new_occupied;
        mask = new_mask;
    }

    memory_size Find(uint64 hash, String key, bool& found) {
        memory_size index = hash & mask;

        memory_size test_index = index;
        while (occupied[test_index]) {
            Entry* entry = entries + test_index;
            if (entry->hash == hash && entry->key == key) {
                found = true;
                return test_index;
            }
            test_index = (test_index + 1) & mask;
            if (test_index == index) break;
        }
        found = false;
        return test_index;
    }

public:
    Hash_Table(uint32 size_exponent = 6) {
        used = 0;
        memory_size size = (uint64)1 << size_exponent;
        mask = size - 1;
        void* memory = calloc(size, sizeof(bool) + sizeof(Entry));
        entries = (Entry*)memory;
        occupied = (bool*)(entries + size);
    }

    ~Hash_Table() {
        free(entries);
        used = 0;
        mask = 0;
        entries = nullptr;
        occupied = nullptr;
    }

    T* Insert(String key) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        Entry* entry = entries + index;
        if (!found) {
            used += 1;
            // Resize if 50% full
            if (2 * used >= mask + 1) {
                Resize();
                index = Find(hash, key, found);
                entry = entries + index;
            }
            occupied[index] = true;
            entry->hash = hash;
            entry->key = key;
        }
        return &entry->value;
    }

    void Delete(String key) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (!found) return;

        used -= 1;
        memory_size next_index = index;
        while (true) {
            next_index = (next_index + 1) & mask;
            if (!occupied[next_index]) break;
            Entry* next_entry = entries + next_index;
            memory_size ideal_index = next_entry->hash & mask;
            bool stays = (index <= next_index) ?
                (index < ideal_index && ideal_index <= next_index) : 
                (index < ideal_index || ideal_index <= next_index);
            if (stays) continue;
            entries[index] = entries[next_index];
            index = next_index;
        }
        occupied[index] = false;
    }

    bool Get(String key, T** value) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (found) *value = &entries[index].value;
        return found;
    }
};

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| File IO                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

typedef int64 Timestamp;

#if _WIN32
#define PATH_SEPARATOR "\\"
#elif __linux__
#define PATH_SEPARATOR "/"
#endif

struct File_Info {
    String path;
    Timestamp timestamp; 
    memory_size size;
};

struct File_Chunk_Info {
    File_Info info;
    memory_size size;
    memory_size offset;
};

String GetFileName(String path) {
    String result;
    
    const char* pChar = path.content + path.length - 1;
    for (; pChar > path.content; pChar--) {
        if (*pChar == '\\' || *pChar == '/') {
            pChar++;
            break;
        }
        result.length++;
    }

    result.content = pChar;
    return result;
}

String GetFileExtension(String path) {
    String result;

    const char* pChar = path.content + path.length - 1;
    for (; pChar > path.content; pChar--) {
        if (*pChar == '.') {
            pChar++;
            break;
        }
        result.length++;
    }

    result.content = pChar;

    return result;
}

String GetFileNameWithoutExtension(String path) {
    String filename = GetFileName(path);
    String extension = GetFileExtension(path);
    filename.length -= filename.length + 1;
    return filename;
}

struct Record_And_Playback {
    void* record_file;
    int record_index;
    void* playback_file;
    int playback_index;
    void* game_memory_block;
    uint64 total_size;
};

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| Monitor info                                                                                                                             |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

const uint8 MAX_SUPPORTED_MONITORS = 8;

struct Monitor_Info {
    uint8 id;
    char device_name[128];
    char display_name[128];
    struct {
        float left;
        float top;
        float width;
        float height;
    } work_area;
    struct {
        float left;
        float top;
        float width;
        float height;
    } monitor_rect;
    bool is_primary;
};

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| Multi-threading                                                                                                                           |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

struct Process_Info {
    int pid;
    void* handle;
    void* thread_handle;
    bool running;
};

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

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| OS Platform                                                                                                                              |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

enum System_OS {
    Windows,
    Linux,
};

#define PLATFORM_ALLOCATE_MEMORY(name) void* name(memory_size size)
typedef PLATFORM_ALLOCATE_MEMORY(Platform_Allocate_Memory);

#define PLATFORM_FREE_MEMORY(name) void name(void* memory)
typedef PLATFORM_FREE_MEMORY(Platform_Free_Memory);

#define PLATFORM_FILE_EXISTS(name) bool name(String path)
typedef PLATFORM_FILE_EXISTS(Platform_File_Exists);

#define PLATFORM_READ_FILE_CHUNK(name) File_Chunk_Info name(String path, memory_size offset, memory_size chunk_size, void* memory)
typedef PLATFORM_READ_FILE_CHUNK(Platform_Read_File_Chunk);

#define PLATFORM_WRITE_FILE_CHUNK(name) bool name(String path, memory_size offset, memory_size chunk_size, void* memory)
typedef PLATFORM_WRITE_FILE_CHUNK(Platform_Write_File_Chunk);

#define PLATFORM_APPEND_TO_FILE(name) bool name(String path, uint64 size, void* memory)
typedef PLATFORM_APPEND_TO_FILE(Platform_Append_To_File);

#define PLATFORM_COPY_FILE(name) bool name(String source, String destination)
typedef PLATFORM_COPY_FILE(Platform_Copy_File);

#define PLATFORM_DELETE_FILE(name) bool name(String path)
typedef PLATFORM_DELETE_FILE(Platform_Delete_File);

#define PLATFORM_GET_FILE_INFO(name) File_Info name(String path)
typedef PLATFORM_GET_FILE_INFO(Platform_Get_File_Info);

/*
    This function should be accompanied by a QueryPerformanceFrequency function that sets the
    .performance_counter_frequency member of the Platform struct.
*/
#define PLATFORM_GET_WALL_CLOCK(name) uint64 name()
typedef PLATFORM_GET_WALL_CLOCK(Platform_Get_Wall_Clock);

#define PLATFORM_RUN_COMMAND(name) Process_Info name(char* command)
typedef PLATFORM_RUN_COMMAND(Platform_Run_Command);

#define PLATFORM_WAIT_FOR_PROCESS(name) int32 name(Process_Info* process, uint32 timeout)
typedef PLATFORM_WAIT_FOR_PROCESS(Platform_Wait_For_Process);

struct Platform_API {
    Platform_Allocate_Memory*     AllocateMemory;
    Platform_Free_Memory*         FreeMemory;
    Platform_File_Exists*         FileExists;
    Platform_Get_File_Info*       GetFileInfo;
    Platform_Read_File_Chunk*     ReadFileChunk;
    Platform_Write_File_Chunk*    WriteFileChunk;
    Platform_Append_To_File*      AppendToFile;
    Platform_Copy_File*           FileCopy;
    Platform_Delete_File*         FileDelete;
    Platform_Get_Wall_Clock*      GetWallClock;
    Platform_Run_Command*         RunCommand;
    Platform_Wait_For_Process*    WaitForProcess;
    uint64                        performance_counter_frequency;

    void* ReadEntireFile(String path, File_Info* file_info = nullptr) {
        File_Info info = GetFileInfo(path);
        if (file_info) {
            *file_info = info;
        }

        if (info.size > 0) {
            void* result = AllocateMemory(info.size);

            File_Chunk_Info chunk_info = ReadFileChunk(path, 0, info.size, result);
            if (chunk_info.size > 0) {
                return result;
            }
            else {
                FreeMemory(result);
            }
        }
        return nullptr;
    }

    bool WriteEntireFile(String path, memory_size size, void* memory) {
        return WriteFileChunk(path, 0, size, memory);
    }
};

#ifdef _WIN32
    #include "Win32PlatformLayer.h"
#elif __linux__
    #include "LinuxPlatformLayer.h"
#else
    UNKNOWN_OPERATING_SYSTEM
#endif

void Raise(const char* error_message) {
    Assert(false, error_message);
}

uint64 SeedRNG(Arena& arena) {
    uint64 seed = 0;

#ifdef _DEBUG
    time_t seconds = time(nullptr);
    tm* local_time = localtime(&seconds);
    seed = local_time->tm_mday + 123456789;
#else
    seed = Platform.GetWallClock();
#endif
    for (int i = 0; i < 8; i++) {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;
    }

    String seed_text = Format(arena, "RNG seed: {U}.", 1, seed);
    Log(Log_Level::Info, seed_text);
    return seed;
}

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Timing                                                                                                                          |
// +---------------------------------------------------------------------------------------------------------------------------------+

struct Time_Record {
    uint64 cycle_count;
    
    String file_name;
    String function_name;
    
    int line_number;
    int hit_count;
};

const int MAX_TIME_RECORDS = 128;
Time_Record *time_records = nullptr;

struct Timed_Block {
    Time_Record* record;
    uint64 start_cycles;
    uint32 aux;

    Timed_Block(int counter, const char* file_name, int line_number, const char* function_name) {
        record = time_records + counter;
        record->file_name = file_name;
        record->function_name = function_name;
        record->line_number = line_number;
        record->hit_count++;
        start_cycles = Platform.GetWallClock();
    }

    ~Timed_Block() {
        record->cycle_count += Platform.GetWallClock() - start_cycles;
    }
};

#define TIMED_BLOCK__(line) Timed_Block timed_block_##line(__COUNTER__, __FILE__, __LINE__, __FUNCTION__)
#define TIMED_BLOCK_(line) TIMED_BLOCK__(line);
#define TIMED_BLOCK TIMED_BLOCK_(__LINE__)

#endif