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
#define ENUM_END(Name) Name##_count }
#define ENUM(Name, ...) ENUM_START(Name) __VA_ARGS__, ENUM_END(Name);

#define FLAGS(...)

#define ArrayCount(arr) (sizeof((arr)) / sizeof((arr)[0]))

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

inline uint8 MSB32(uint32 X) {
	unsigned long result = 0;
#ifdef _WIN32
	_BitScanReverse(&result, X);
#else
    result = 31 - __builtin_clz(X);
#endif
	return (uint32)result;
}

inline uint8 MSB64(uint64 X) {
	unsigned long result = 0;
#ifdef _WIN32
	_BitScanReverse64(&result, X);
#else
    result = 31 - __builtin_clzll(X);
#endif
	return (uint32)result;
}

inline void Assert(bool assertion, const char* Message = "") {
#ifdef _DEBUG
    if (!assertion)
        throw Message;
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

inline void ZeroSize(memory_size Size, void* Memory) {
    if (Size > 0) {
        memset(Memory, 0, Size);
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

inline void Free(Arena* arena) {
    free(arena->base);
    arena->base = nullptr;
    arena->size = 0;
    arena->used = 0;
}

inline void Clear(Arena* arena) {
    ZeroSize(arena->used, arena->base);
    arena->used = 0;
}

#define PushStruct(arena, type) (type *)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type) (type *)PushSize_(arena, count*sizeof(type))
#define PushSize(arena, size) (void*)PushSize_(arena, size)
inline void* PushSize_(Arena* arena, memory_size size) {
    Assert(arena->size >= arena->used + size);
    void* result = arena->base + arena->used;
    arena->used += size;
    return result;
}

#define PopStruct(arena, type) (type *)PopSize_(arena, sizeof(type))
#define PopArray(arena, count, type) (type *)PopSize_(arena, count*sizeof(type))
#define PopSize(arena, size) (void*)PopSize_(arena, size)
inline void* PopSize_(Arena* arena, memory_size size) {
    memory_size bytes_erased = size < arena->used ? size : arena->used;
    void* result = (void*)(arena->base + arena->used - bytes_erased);
    ZeroSize(bytes_erased, result);
    arena->used -= bytes_erased;
    return result;
}

inline Arena SuballocateMemoryArena(Arena* arena, memory_size size) {
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
    int length;
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

char* CopyString(Arena* arena, String String) {
    char* result = PushArray(arena, String.length, char);
    strncpy(result, String.content, String.length);
    return result;
}

String PersistString(Arena* arena, String string) {
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

String Format(Arena* arena, const char* format, int input_count, ...) {
    String result = {};
    result.length = 0;
    int format_length = strlen(format);

    if (format_length == 0) {
        result.content = nullptr;
        return result;
    }

    char* pointer = (char*)(arena->base + arena->used);
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
+------------------------------------------------------------------------------------------------------------------------------------------+
| Data structures                                                                                                                          |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

/*
    Memory for elements must be previously allocated.
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
            T result = *Top;
            *Top = {};
            n--;
            return result;
        }
        return {};
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

void Attach(struct link* Link1, struct link* Link2) {
    Assert(Link1 || Link2, "Two empty links tried to be linked.");
    if (Link1) {
        Link1->Next = Link2;
    }
    if (Link2) {
        Link2->Previous = Link1;
    }
}

void Delete(struct link* ThisLink) {
    Attach(ThisLink->Previous, ThisLink->Next);
    ThisLink->Previous = nullptr;
    ThisLink->Next = nullptr;
}

/*
    Doubly-linked list. All links must have been allocated somewhere previously.
*/
struct linked_list {
    struct link* First;
    struct link* Last;

    void PushBack(struct link* Element) {
        if (!First || !Last) {
            First = Element;
        }
        else {
            Attach(Last, Element);
        }
        Last = Element;
    }

    void PushFront(struct link* Element) {
        if (!First || !Last) {
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

    void Break(struct link* Link) {
        if (First == Link) First = Link->Next;
        if (Last == Link)  Last  = Link->Previous;
        Delete(Link);
    }

    bool IsEmpty() {
        return !First && !Last;
    }
};

inline linked_list Concatenate(linked_list L1, linked_list L2) {
    linked_list result = {};
    result.First = L1.First;
    Attach(L1.Last, L2.First);
    result.Last = L2.Last;
    return result;
}

uint64 GetLength(linked_list List) {
	struct link* Link = List.First;
	uint64 result = 0;
    do {
		Link = Link->Next;
        result++;
    } while (Link && Link != List.First);
	return result;
}

/*
    Fixed length array.
*/
template <typename T> class array {
private:
    T* Data;
public:
    uint32 Count;
    uint32 Size;

    array() {
        Data = nullptr;
        Count = 0;
        Size = 0;
    }

    array(Arena* Arena, uint32 N) {
        Size = N;
        Count = 0;
        Data = PushArray(Arena, N, T);
    }

    ~array() {
        Data = nullptr;
        Size = 0;
    }

    bool IsFull() {
        return Count == Size;
    }

    T* Insert(T Value) {
        T* result = nullptr;
        if (Count < Size) {
            result = Data + Count;
            Data[Count++] = Value;
        }
        return result;
    }

    T& operator[](int64 Index) {
        if (-Index > Count || Index >= Count) {
            throw "Array index out of bounds.";
        }
        else if (Index >= 0) {
            return Data[Index];
        }
        else if (Index < 0) {
            return Data[Count - Index];
        }
        Assert(false, "Invalid code path.");
    }

    const T& operator[](int64 Index) const {
        if (-Index > Count || Index >= Count) {
            throw "Array index out of bounds.";
        }
        else if (Index >= 0) {
            return Data[Index];
        }
        else if (Index < 0) {
            return Data[Count - Index];
        }
    }
};

/*
    Fixed length lists that track available slots.
*/
template <typename T> class free_list {
private:
    T* Slots;
    uint32* FreeSlots;
public:
    bool* IsOccupied;
    uint32 Size = 0;
    uint32 Count = 0;
    uint32 FreeCount = 0;

    free_list(Arena* Arena, uint32 N) {
        Size = N;
        Slots = PushArray(Arena, N, T);
        FreeSlots = PushArray(Arena, N, uint32);
        IsOccupied = PushArray(Arena, N, bool);
    }

    uint32 Insert(T Element) {
        Assert(Count < Size);
        
        uint32 result;
        if (FreeCount > 0) {
            result = FreeSlots[FreeCount--];
        }
        else {
            result = Count++;
        }

        Slots[result] = Element;
        IsOccupied[result] = true;
        return result;
    }

    T& operator[](uint32 Index) {
        return Slots[Index];
    }

    T operator[](uint32 Index) const {
        return Slots[Index];
    }

    void Remove(uint32 Index) {
        if (IsOccupied[Index]) {
            FreeSlots[FreeCount++] = Index;
            IsOccupied[Index] = false;
            Count--;
        }
    }
};

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

    void NewChunk(memory_size Size) {
        if (Meta.nChunks >= MAX_XARRAY_CHUNKS) {
            Assert(false, "Xarray chunk index overflow.");
        }
        Header->Chunks[Meta.nChunks++] = (uint8*)malloc(Size);
    }

public:
    xarray() {
        Meta = { 4, 0, sizeof(T) };
        Header = (xarray_header*)calloc(MAX_XARRAY_CHUNKS + 1, sizeof(void*));
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
        if (Header->n * sizeof(T) >= TotalSize) {
            NewChunk(TotalSize);
        }

        T* pointer = (T*)xar_get(Header, Meta, NewIndex);
        *pointer = Element;
        return pointer;
    }

    uint64 Size() {
        return Header->n;
    }

    void Clear() {
        Header->n = 0;
    }
};

template <typename S, typename T> class hash_table {
private:
    uint32 Size;
    uint32 Used;
    struct entry {
        uint64 Hash;
        S Key;
        T Value;
    }* Entries;
    bool* Deleted;

    uint64 Hash(S Key) {
        S LocalCopy = Key;
        return XXH64(&LocalCopy, sizeof(S), 0);
    }

    bool Resize() {
        uint32 NewSize = 2*Size;
        void* NewContent = calloc(NewSize, sizeof(entry));
        if (NewContent) {
            entry* NewEntries = (entry*)NewContent;
            for (int i = 0; i < Size; i++) {
                entry* Entry = Entries + i;
                if (Entry->Key.Length > 0) {
                    uint32 NewIndex = Entry->Hash % NewSize;
                    entry* NewEntry = NewEntries + NewIndex;
                    while (NewEntry->Key.Length > 0) {
                        NewIndex = (NewIndex + 1) % NewSize;
                        NewEntry = NewEntries + NewIndex;
                    }
                    *NewEntry = *Entry;
                }
            }
            free(Entries);
            Entries = NewEntries;
            Size = NewSize;
            bool* NewDeleted = (bool*)calloc(NewSize, sizeof(bool));
            if (NewDeleted) {
                free(Deleted);
                Deleted = NewDeleted;
            }
            else {
                Assert(false);
            }
        }
        else {
            Assert(false);
        }
        return NewContent != nullptr;
    }

    uint32 Search(uint64 H, S Key, bool CanBeDeleted = false) {
        uint32 Index = H % Size;
        entry* Entry = Entries + Index;
        if (Entry->Key == nullptr) {
            return Index;
        }
        while (Entry->Key) {
            if (
                CanBeDeleted && Deleted[Index] || 
                !Deleted[Index] && H == Entry->Hash && Entry->Key == Key
            ) {
                return Index;
            }
            Index = (Index + 1) % Size;
            Entry = Entries + Index;
        }
        return Index;
    }

public:
    hash_table(uint32 SizeExponent = 6) {
        Used = 0;
        Size = 1 << SizeExponent;
        Entries = (entry*)calloc(Size, sizeof(entry));
        Deleted = (bool*)calloc(Size, sizeof(bool));
    }

    ~hash_table() {
        free(Entries);
    }

    void Insert(S Key, T Value) {
        uint64 H = Hash(Key);
        uint32 Index = Search(H, Key, true);
        entry* Entry = Entries + Index;
        if (Entry->Key == nullptr) {
            Used += 1;
            if (Used >= 0.75f * Size) {
                Resize();
                Entry = Entries + Search(H, Key);
            }
        }
        else if (Deleted[Index]) {
            Deleted[Index] = false;
        }
        Entry->Hash = H;
        Entry->Key = Key;
        Entry->Value = Value;
    }

    void Delete(S Key) {
        uint64 H = Hash(Key);
        uint32 Index = Search(H, Key);
        entry* Entry = Entries + Index;
        if (Entry->Key && !Deleted[Index]) {
            Deleted[Index] = true;
        }
    }

    T Get(S Key) {
        uint64 H = Hash(Key);
        uint32 Index = Search(H, Key);
        entry* Entry = Entries + Index;
        return Entry->Value;
    }
};

template <typename T> class hash_table<String, T> {
private:
    uint32 Size;
    uint32 Used;
    struct entry {
        uint64 Hash;
        String Key;
        T Value;
    }* Entries;
    bool* Deleted;

    uint64 Hash(String Key) {
        return XXH64(Key.content, Key.length, 0);
    }

    bool Resize() {
        uint32 NewSize = 2*Size;
        void* NewContent = calloc(NewSize, sizeof(entry));
        if (NewContent) {
            entry* NewEntries = (entry*)NewContent;
            for (int i = 0; i < Size; i++) {
                entry* Entry = Entries + i;
                if (Entry->Key.Length > 0) {
                    uint32 NewIndex = Entry->Hash % NewSize;
                    entry* NewEntry = NewEntries + NewIndex;
                    while (NewEntry->Key.Length > 0) {
                        NewIndex = (NewIndex + 1) % NewSize;
                        NewEntry = NewEntries + NewIndex;
                    }
                    *NewEntry = *Entry;
                }
            }
            free(Entries);
            Entries = NewEntries;
            Size = NewSize;
            bool* NewDeleted = (bool*)calloc(NewSize, sizeof(bool));
            if (NewDeleted) {
                free(Deleted);
                Deleted = NewDeleted;
            }
            else {
                Assert(false);
            }
        }
        else {
            Assert(false);
        }
        return NewContent != nullptr;
    }

    uint32 Search(uint64 H, const char* Key, bool CanBeDeleted = false) {
        uint32 Index = H % Size;
        entry* Entry = Entries + Index;
        if (Entry->Key == nullptr) {
            return Index;
        }
        while (Entry->Key) {
            if (
                CanBeDeleted && Deleted[Index] || 
                !Deleted[Index] && H == Entry->Hash && strcmp(Entry->Key, Key) == 0
            ) {
                return Index;
            }
            Index = (Index + 1) % Size;
            Entry = Entries + Index;
        }
        return Index;
    }

public:
    hash_table(uint32 SizeExponent = 6) {
        Used = 0;
        Size = 1 << SizeExponent;
        Entries = (entry*)calloc(Size, sizeof(entry));
        Deleted = (bool*)calloc(Size, sizeof(bool));
    }

    ~hash_table() {
        free(Entries);
    }

    void Insert(const char* Key, T Value) {
        uint64 H = Hash(Key);
        uint32 Index = Search(H, Key, true);
        entry* Entry = Entries + Index;
        if (Entry->Key == nullptr) {
            Used += 1;
            if (Used >= 0.75f * Size) {
                Resize();
                Entry = Entries + Search(H, Key);
            }
        }
        else if (Deleted[Index]) {
            Deleted[Index] = false;
        }
        Entry->Hash = H;
        Entry->Key = Key;
        Entry->Value = Value;
    }

    void Delete(const char* Key) {
        uint64 H = Hash(Key);
        uint32 Index = Search(H, Key);
        entry* Entry = Entries + Index;
        if (Entry->Key && !Deleted[Index]) {
            Deleted[Index] = true;
        }
    }

    T Get(const char* Key) {
        uint64 H = Hash(Key);
        uint32 Index = Search(H, Key);
        entry* Entry = Entries + Index;
        Assert(Entry->Key);
        return Entry->Value;
    }
};

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Logging                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

enum log_mode {
    File_Log_Mode,
    Terminal_Log_Mode
};

enum class log_level {
    Info,
    Test,
    Warn,
    Error
};

log_mode LOG_MODE = Terminal_Log_Mode;

void Log(log_level Level, String Content);

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

// Record and playback
struct record_and_playback {
    void* RecordFile;
    int RecordIndex;
    void* PlaybackFile;
    int PlaybackIndex;
    void* GameMemoryBlock;
    uint64 TotalSize;
};

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| Monitor info                                                                                                                             |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

const uint8 MAX_SUPPORTED_MONITORS = 8;

struct monitor_info {
    uint8 ID;
    char DeviceName[128];
    char DisplayName[128];
    struct {
        float Left;
        float Top;
        float Width;
        float Height;
    } WorkArea;
    struct {
        float Left;
        float Top;
        float Width;
        float Height;
    } MonitorRect;
    bool IsPrimary;
};

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| Multithreading                                                                                                                           |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

struct process_info {
    int PID;
    void* Handle;
    void* ThreadHandle;
    bool Running;
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
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_FREE_MEMORY(name) void name(void* memory)
typedef PLATFORM_FREE_MEMORY(platform_free_memory);

#define PLATFORM_FILE_EXISTS(name) bool name(String path)
typedef PLATFORM_FILE_EXISTS(platform_file_exists);

#define PLATFORM_READ_FILE_CHUNK(name) File_Chunk_Info name(String path, memory_size offset, memory_size chunk_size, void* memory)
typedef PLATFORM_READ_FILE_CHUNK(platform_read_file_chunk);

#define PLATFORM_WRITE_FILE_CHUNK(name) bool name(String path, memory_size offset, memory_size chunk_size, void* memory)
typedef PLATFORM_WRITE_FILE_CHUNK(platform_write_file_chunk);

#define PLATFORM_APPEND_TO_FILE(name) bool name(String path, uint64 size, void* memory)
typedef PLATFORM_APPEND_TO_FILE(platform_append_to_file);

#define PLATFORM_COPY_FILE(name) bool name(String source, String destination)
typedef PLATFORM_COPY_FILE(platform_copy_file);

#define PLATFORM_DELETE_FILE(name) bool name(String path)
typedef PLATFORM_DELETE_FILE(platform_delete_file);

#define PLATFORM_GET_FILE_INFO(name) File_Info name(String path)
typedef PLATFORM_GET_FILE_INFO(platform_get_file_info);

/*
    This function should be accompanied by a QueryPerformanceFrequency function that sets the
    .PerformanceCounterFrequency member of the Platform struct.
*/
#define PLATFORM_GET_WALL_CLOCK(name) uint64 name()
typedef PLATFORM_GET_WALL_CLOCK(platform_get_wall_clock);

#define PLATFORM_RUN_COMMAND(name) process_info name(char* Command)
typedef PLATFORM_RUN_COMMAND(platform_run_command);

#define PLATFORM_WAIT_FOR_PROCESS(name) int32 name(process_info* Process, uint32 Timeout)
typedef PLATFORM_WAIT_FOR_PROCESS(platform_wait_for_process);

struct platform_api {
    platform_allocate_memory*     AllocateMemory;
    platform_free_memory*         FreeMemory;
    platform_file_exists*         FileExists;
    platform_get_file_info*       GetFileInfo;
    platform_read_file_chunk*     ReadFileChunk;
    platform_write_file_chunk*    WriteFileChunk;
    platform_append_to_file*      AppendToFile;
    platform_copy_file*           FileCopy;
    platform_delete_file*         FileDelete;
    platform_get_wall_clock*      GetWallClock;
    platform_run_command*         RunCommand;
    platform_wait_for_process*    WaitForProcess;
    uint64                        PerformanceCounterFrequency;

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

    bool WriteEntireFile(String Path, memory_size Size, void* Memory) {
        return WriteFileChunk(Path, 0, Size, Memory);
    }
};

#ifdef _WIN32
    #include "Win32PlatformLayer.h"
#elif __linux__
    #include "LinuxPlatformLayer.h"
#else
    UNKNOWN_OPERATING_SYSTEM
#endif

void Raise(const char* ErrorMessage) {
    Assert(false, ErrorMessage);
}

uint64 SeedRNG(Arena* arena) {
    uint64 seed = 0;

#ifdef _DEBUG
    time_t seconds = time(nullptr);
    tm* time_info = localtime(&seconds);
    seed = time_info->tm_mday + 123456789;
#else
    Seed = Platform.GetWallClock();
#endif
    for (int i = 0; i < 8; i++) {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;
    }

    String seed_text = Format(arena, "RNG seed: {U}.", 1, seed);
    Log(log_level::Info, seed_text.content);
    return seed;
}

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Timing                                                                                                                          |
// +---------------------------------------------------------------------------------------------------------------------------------+

struct time_record {
    uint64 CycleCount;
    
    String FileName;
    String FunctionName;
    
    int LineNumber;
    int HitCount;
};

const int MAX_TIME_RECORDS = 128;
time_record *TimeRecords = nullptr;

struct timed_block {
    time_record* Record;
    uint64 StartCycles;
    uint32 Aux;

    timed_block(int Counter, const char* FileName, int LineNumber, const char* FunctionName) {
        Record = TimeRecords + Counter;
        Record->FileName = FileName;
        Record->FunctionName = FunctionName;
        Record->LineNumber = LineNumber;
        Record->HitCount++;
        StartCycles = Platform.GetWallClock();
    }

    ~timed_block() {
        Record->CycleCount += Platform.GetWallClock() - StartCycles;
    }
};

#define TIMED_BLOCK__(Line) timed_block TimedBlock_##Line(__COUNTER__, __FILE__, __LINE__, __FUNCTION__)
#define TIMED_BLOCK_(Line) TIMED_BLOCK__(Line);
#define TIMED_BLOCK TIMED_BLOCK_(__LINE__)

#endif