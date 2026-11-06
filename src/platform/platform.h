#pragma once

#include "common.h"

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

void Log(Log_Level Level, String Content);

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

uint64 SeedRNG(Arena& arena);

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
