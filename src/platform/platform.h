#pragma once

#include <common.h>
#include <time.h>

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Logging                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

class Log {
public:
    enum class Mode {
        File,
        Terminal
    };
    static Mode mode;
    enum class Level {
        Info,
        Test,
        Warn,
        Error,
        Fatal
    };
    static void Info(String message);
    static void Test(String message);
    static void Warn(String message);
    static void Error(String message);
    static void Fatal(String message);
private:
    static void LogLevel(Level level, String message);
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

struct Record_And_Playback {
    void* record_file;
    int record_index;
    void* playback_file;
    int playback_index;
    void* game_memory_block;
    uint64 total_size;
};

String GetFileName(String path);
String GetFileExtension(String path);
String GetFileNameWithoutExtension(String path);
bool NeedsUpdate(String source_path, String destination_path);

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| Monitor info                                                                                                                             |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

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

struct Platform {
    enum System_OS {
        Windows,
        Linux,
    };
    static System_OS OS;
    static void* AllocateMemory(memory_size size);
    static void FreeMemory(void* memory);
    static bool FileExists(String path);
    static File_Chunk_Info ReadFileChunk(String path, memory_size offset, memory_size chunk_size, void* memory);
    static void* ReadEntireFile(String path, File_Info* file_info);
    static bool WriteFileChunk(String path, memory_size offset, memory_size chunk_size, void* memory);
    static bool WriteEntireFile(String path, memory_size size, void* memory);
    static bool AppendToFile(String path, memory_size size, void* memory);
    static bool FileCopy(String source, String destination);
    static bool FileDelete(String path);
    static File_Info GetFileInfo(String path);
    static uint64 GetWallClock();
    static float GetSecondsElapsed(uint64 start, uint64 end);
    static Process_Info RunCommand(char* command);
    static int32 WaitForProcess(Process_Info& process, uint32 timeout);
    static uint64 performance_counter_frequency;
};

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Timing                                                                                                                          |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

struct Time_Record {
    uint64 cycle_count;
    
    String file_name;
    String function_name;
    
    int line_number;
    int hit_count;
};

struct Timed_Block {
    Time_Record* record;
    uint64 start_cycles;
    uint32 aux;

    Timed_Block(int counter, const char* file_name, int line_number, const char* function_name);
    ~Timed_Block();
};

#define TIMED_BLOCK__(line) Timed_Block timed_block_##line(__COUNTER__, __FILE__, __LINE__, __FUNCTION__)
#define TIMED_BLOCK_(line) TIMED_BLOCK__(line);
#define TIMED_BLOCK TIMED_BLOCK_(__LINE__)
