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

typedef size_t memory_index;

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value)*1024)

#define INTROSPECT

#define ENUM_START(Name) enum Name {
#define ENUM_END(Name) Name##_count }
#define ENUM(Name, ...) ENUM_START(Name) __VA_ARGS__, ENUM_END(Name);

#define FLAGS(...)

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
	unsigned long Result = 0;
#ifdef _WIN32
	_BitScanReverse(&Result, X);
#else
    Result = 31 - __builtin_clz(X);
#endif
	return (uint32)Result;
}

inline uint8 MSB64(uint64 X) {
	unsigned long Result = 0;
#ifdef _WIN32
	_BitScanReverse64(&Result, X);
#else
    Result = 31 - __builtin_clzll(X);
#endif
	return (uint32)Result;
}

inline void Assert(bool assertion, const char* Message = "") {
#ifdef _DEBUG
    if (!assertion)
        throw Message;
#endif
}

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Logging                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

enum log_mode {
    File_Log_Mode,
    Terminal_Log_Mode
};

enum log_level {
    Info,
    Warn,
    Error
};

log_mode LOG_MODE = Terminal_Log_Mode;

void Log(log_level Level, const char* Content);

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Memory arenas                                                                                                                   |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

struct memory_arena {
    memory_index Size;
    memory_index Used;
    uint8* Base;
};

inline void ZeroSize(memory_index Size, void* Memory) {
    if (Size > 0) {
        memset(Memory, 0, Size);
    }
}

inline memory_arena MemoryArena(memory_index Size, void* Base) {
    memory_arena Result;
    Result.Size = Size;
    Result.Base = (uint8*)Base;
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

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| Data structures                                                                                                                          |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

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
    Assert(Link1 != NULL || Link2 != NULL, "Two empty links tried to be linked.");
    if (Link1 != NULL) {
        Link1->Next = Link2;
    }
    if (Link2 != NULL) {
        Link2->Previous = Link1;
    }
}

void Delete(struct link* ThisLink) {
    Attach(ThisLink->Previous, ThisLink->Next);
    ThisLink->Previous = NULL;
    ThisLink->Next = NULL;
}

/*
    Doubly-linked list. All links must have been allocated somewhere previously.
*/
struct linked_list {
    struct link* First;
    struct link* Last;

    void PushBack(struct link* Element) {
        if (First == NULL || Last == NULL) {
            First = Element;
        }
        else {
            Attach(Last, Element);
        }
        Last = Element;
    }

    void PushFront(struct link* Element) {
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

    void Break(struct link* Link) {
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
	struct link* Link = List.First;
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

template <typename T> class free_list {
private:
    T* Slots;
    uint32* FreeSlots;
public:
    bool* IsOccupied;
    uint32 Size = 0;
    uint32 Count = 0;
    uint32 FreeCount = 0;

    free_list(memory_arena* Arena, uint32 N) {
        Size = N;
        Slots = PushArray(Arena, N, T);
        FreeSlots = PushArray(Arena, N, uint32);
        IsOccupied = PushArray(Arena, N, bool);
    }

    uint32 Insert(T Element) {
        Assert(Count < Size);
        
        uint32 Result;
        if (FreeCount > 0) {
            Result = FreeSlots[FreeCount--];
        }
        else {
            Result = Count++;
        }

        Slots[Result] = Element;
        IsOccupied[Result] = true;
        return Result;
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

    void NewChunk(memory_index Size) {
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

template <typename T> class hash_table {
private:
    uint32 Size;
    uint32 Used;
    struct entry {
        uint64 Hash;
        const char* Key;
        T Value;
    }* Entries;
    bool* Deleted;

    uint64 Hash(const char* Key) {
        return XXH64(Key, strlen(Key), 0);
    }

    bool Resize() {
        uint32 NewSize = 2*Size;
        void* NewContent = calloc(NewSize, sizeof(entry));
        if (NewContent) {
            entry* NewEntries = (entry*)NewContent;
            for (int i = 0; i < Size; i++) {
                entry* Entry = Entries + i;
                if (Entry->Key) {
                    uint32 NewIndex = Entry->Hash % NewSize;
                    entry* NewEntry = NewEntries + NewIndex;
                    while (NewEntry->Key) {
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

// Strings

struct string {
    int Length;
    const char* Content;

    string() {
        Length = 0;
        Content = nullptr;
    }

    string(const char* String) {
        Length = strlen(String);
        Content = String;
    }

    char operator[](int Index) {
        return Content[Index];
    };
};

char* PushString(memory_arena* Arena, string String) {
    char* Result = PushArray(Arena, String.Length, char);
    strncpy(Result, String.Content, String.Length);
    return Result;
}

bool operator==(string S1, string S2) {
    bool Result = S1.Length == S2.Length;
    if (Result) {
        for (int i = 0; i < S1.Length; i++) {
            if (S1[i] != S2[i]) {
                return false;
            }
        }
    }
    return Result;
}

string Slice(string S, int Start, int End) {
    Assert(End >= Start);
    string Result;
    Result.Length = End - Start;
    Result.Content = S.Content + Start;
    return Result;
}

string Format(memory_arena* Arena, const char* Format, int nInputs, ...) {
    string Result = {};
    Result.Length = 0;
    int FormatLength = strlen(Format);

    if (FormatLength == 0) {
        Result.Content = nullptr;
        return Result;
    }

    char* Pointer = (char*)(Arena->Base + Arena->Used);
    Result.Content = Pointer;

    va_list Args;
    va_start(Args, nInputs);

    int DestIndex = 0;
    for (int SourceIndex = 0; SourceIndex < FormatLength; SourceIndex++) {
        if (Format[SourceIndex] == '{') {
            switch (Format[++SourceIndex]) {
                case 'i': {
                    int32 Input = va_arg(Args, int32);
                    if (Input == 0) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '0';
                        Result.Length++;
                        break;
                    }
                    else if (Input < 0) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '-';
                        Result.Length++;
                        Input = -Input;
                    }
                    int nDigits = 0;
                    char Digits[16];
                    while (Input > 0) {
                        Digits[nDigits++] = '0' + (Input % 10);
                        Input /= 10;
                    }
                    for (int i = 0; i < nDigits; i++) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = Digits[nDigits - (i + 1)];
                        Result.Length++;
                    }
                } break;

                case 'I': {
                    int64 Input = va_arg(Args, int64);
                    if (Input == 0) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '0';
                        Result.Length++;
                        break;
                    }
                    else if (Input < 0) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '-';
                        Result.Length++;
                        Input = -Input;
                    }
                    int nDigits = 0;
                    char Digits[32];
                    while (Input > 0) {
                        Digits[nDigits++] = '0' + (Input % 10);
                        Input /= 10;
                    }
                    for (int i = 0; i < nDigits; i++) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = Digits[nDigits - (i + 1)];
                        Result.Length++;
                    }
                } break;

                case 'u': {
                    uint32 Input = va_arg(Args, uint32);
                    if (Input == 0) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '0';
                        Result.Length++;
                        break;
                    }
                    int nDigits = 0;
                    char Digits[16];
                    while (Input > 0) {
                        Digits[nDigits++] = '0' + (Input % 10);
                        Input /= 10;
                    }
                    for (int i = 0; i < nDigits; i++) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = Digits[nDigits - (i + 1)];
                        Result.Length++;
                    }
                } break;

                case 'U': {
                    uint64 Input = va_arg(Args, uint64);
                    if (Input == 0) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '0';
                        Result.Length++;
                        break;
                    }
                    int nDigits = 0;
                    char Digits[32];
                    while (Input > 0) {
                        Digits[nDigits++] = '0' + (Input % 10);
                        Input /= 10;
                    }
                    for (int i = 0; i < nDigits; i++) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = Digits[nDigits - (i + 1)];
                        Result.Length++;
                    }
                } break;

                case 'x': {
                    uint64 Input = va_arg(Args, uint64);
                    PushArray(Arena, 18, char);
                    Pointer[DestIndex++] = '0';
                    Pointer[DestIndex++] = 'x';
                    for (int i = 15; i >= 0; i--) {
                        uint64 r = Input % 16;
                        if (r < 10) {
                            Pointer[DestIndex + i] = '0' + r;
                        }
                        else {
                            Pointer[DestIndex + i] = 'a' + (r - 10);
                        }
                        Input >>= 4;
                    }
                    DestIndex += 16;
                    Result.Length += 18;
                } break;

                case 'f': {
                    double Input = va_arg(Args, double);
                    if (Input < 0) {
                        Input = -Input;
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '-';
                        Result.Length++;
                    }

                    uint64 IntegerPart = floor(Input);
                    double DecimalPart = Input - IntegerPart;

                    if (IntegerPart == 0) {
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '0';
                        Result.Length++;
                    }

                    int nDigits = 0;
                    char Digits[32];
                    while (IntegerPart > 0) {
                        Digits[nDigits++] = '0' + (IntegerPart % 10);
                        IntegerPart /= 10;
                    }
                    
                    PushArray(Arena, nDigits + 1, char);
                    for (int i = 0; i < nDigits; i++) {
                        Pointer[DestIndex++] = Digits[nDigits - (i + 1)];
                    }
                    Pointer[DestIndex++] = '.';
                    Result.Length += nDigits + 1;

                    int nDecimals = Format[++SourceIndex] - '0';
                    for (int i = 0; i < nDecimals; i++) {
                        DecimalPart *= 10.0;
                        int Digit = DecimalPart;
                        PushArray(Arena, 1, char);
                        Pointer[DestIndex++] = '0' + Digit;
                        Result.Length++;
                        DecimalPart -= Digit;
                    }
                } break;

                case 's': {
                    string Input = va_arg(Args, string);
                    Result.Length += Input.Length;
                    PushArray(Arena, Input.Length, char);
                    for (int i = 0; i < Input.Length; i++) {
                        Pointer[DestIndex++] = Input[i];
                    }
                } break;

                default: {
                    Assert(false, "Invalid format flag.");
                }
            }

            Assert(Format[SourceIndex + 1] == '}');
            SourceIndex++;
        }
        else {
            PushArray(Arena, 1, char);
            Pointer[DestIndex++] = Format[SourceIndex];
            Result.Length++;
        }
    }
    PushArray(Arena, 1, char);
    va_end(Args);
    return Result;
}

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| File IO                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

#define MAX_PATH_LENGTH 256

#if _WIN32
#define PATH_SEPARATOR "\\"
#elif __linux__
#define PATH_SEPARATOR "/"
#endif

struct file_info {
    char Path[MAX_PATH_LENGTH];
    int64 Timestamp; 
    memory_index Size;
};

struct file_chunk_info {
    file_info Info;
    memory_index Size;
    memory_index Offset;
};

const char* GetFileName(const char* Path) {
    uint64 L = strlen(Path);
    const char* pChar = nullptr;
    for (pChar = Path + L - 1; pChar > Path; pChar--) {
        if (*pChar == '\\' || *pChar == '/') {
            pChar++;
            break;
        }
    }

    return pChar;
}

const char* GetFileExtension(const char* Path) {
    uint64 L = strlen(Path);
    const char* pChar = nullptr;
    for (pChar = Path + L - 1; pChar > Path; pChar--) {
        if (*pChar == '.') {
            pChar++;
            break;
        }
    }

    return pChar;
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

enum system_os {
    Windows,
    Linux,
};

#define PLATFORM_ALLOCATE_MEMORY(name) void* name(memory_index Size)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_FREE_MEMORY(name) void name(void* Memory)
typedef PLATFORM_FREE_MEMORY(platform_free_memory);

#define PLATFORM_FILE_EXISTS(name) bool name(const char* Path)
typedef PLATFORM_FILE_EXISTS(platform_file_exists);

#define PLATFORM_READ_FILE_CHUNK(name) file_chunk_info name(const char* Path, memory_index Offset, memory_index ChunkSize, void* Memory)
typedef PLATFORM_READ_FILE_CHUNK(platform_read_file_chunk);

#define PLATFORM_WRITE_FILE_CHUNK(name) bool name(const char* Path, memory_index Offset, memory_index ChunkSize, void* Memory)
typedef PLATFORM_WRITE_FILE_CHUNK(platform_write_file_chunk);

#define PLATFORM_APPEND_TO_FILE(name) bool name(const char* Path, uint64 Size, void* Memory)
typedef PLATFORM_APPEND_TO_FILE(platform_append_to_file);

#define PLATFORM_COPY_FILE(name) bool name(const char* Source, const char* Destination)
typedef PLATFORM_COPY_FILE(platform_copy_file);

#define PLATFORM_DELETE_FILE(name) bool name(const char* Path)
typedef PLATFORM_DELETE_FILE(platform_delete_file);

#define PLATFORM_GET_FILE_INFO(name) file_info name(const char* Path)
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

    void* ReadEntireFile(const char* Path, file_info* FileInfo = nullptr) {
        file_info Info = GetFileInfo(Path);
        if (FileInfo) {
            *FileInfo = Info;
        }

        if (Info.Size > 0) {
            void* Result = AllocateMemory(Info.Size);

            file_chunk_info ChunkInfo = ReadFileChunk(Path, 0, Info.Size, Result);
            if (ChunkInfo.Size > 0) {
                return Result;
            }
            else {
                FreeMemory(Result);
            }
        }
        return nullptr;
    }

    bool WriteEntireFile(const char* Path, memory_index Size, void* Memory) {
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
    Log(Error, ErrorMessage);
    Assert(false);
}

uint64 SeedRNG(memory_arena* Arena) {
    uint64 Seed = 0;

#ifdef _DEBUG
    time_t Seconds = time(NULL);
    tm* TimeInfo = localtime(&Seconds);
    Seed = TimeInfo->tm_mday + 123456789;
#else
    Seed = Platform.GetWallClock();
#endif
    for (int i = 0; i < 8; i++) {
        Seed ^= Seed << 13;
        Seed ^= Seed >> 7;
        Seed ^= Seed << 17;
    }

    string SeedText = Format(Arena, "RNG seed: {U}.", 1, Seed);
    Log(Info, SeedText.Content);
    return Seed;
}

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Timing                                                                                                                          |
// +---------------------------------------------------------------------------------------------------------------------------------+

struct time_record {
    uint64 CycleCount;
    
    string FileName;
    string FunctionName;
    
    int LineNumber;
    int HitCount;
};

const int MAX_TIME_RECORDS = 128;
time_record *TimeRecords = NULL;

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