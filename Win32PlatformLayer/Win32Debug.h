#if !defined(WIN32_DEBUG_H)

#include <windows.h>
#include <stdio.h>

#include "GamePlatform.h"

/*
    TODO:
        - Make debug records thread safe
        - Plot performance counters
*/

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Logging                                                                                                                         |
// +---------------------------------------------------------------------------------------------------------------------------------+

enum log_mode {
    File,
    Terminal
};

enum log_level {
    Info,
    Warn,
    Error
};

struct logger {
    log_mode Mode;
};

const log_mode LOG_MODE = Terminal;

void Log(log_level Level, const char* Content) {
    // Level
    char LevelString[9];
    int LevelStringLength = 0;
    switch (Level) {
        case Info:
        {
            strcpy_s(LevelString, "[INFO] ");
            LevelStringLength = 7;
        } break;
        case Warn:
        {
            strcpy_s(LevelString, "[WARN] ");
            LevelStringLength = 7;
        } break;
        case Error:
        {
            strcpy_s(LevelString, "[ERROR] ");
            LevelStringLength = 8;
        } break;
    }
    LevelString[LevelStringLength] = 0;

    // Timestamp
    time_t t = time(NULL);
    struct tm tm;
    localtime_s(&tm, &t);
    char Date[21];
    sprintf_s(Date, "%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Logging
    switch (LOG_MODE) {
        case File:
        {
            HANDLE FileHandle = CreateFileA("log.log", FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
            if (FileHandle != INVALID_HANDLE_VALUE) {
                DWORD BytesWritten = 0;
                WriteFile(FileHandle, Date, 20, &BytesWritten, 0);
                WriteFile(FileHandle, LevelString, LevelStringLength, &BytesWritten, 0);
                int i = 0;
                while (*(Content + i) != 0) {
                    i++;
                }
                WriteFile(FileHandle, Content, i, &BytesWritten, 0);
            }
            else {
                Assert(false);
            }

            CloseHandle(FileHandle);
        } break;
        case Terminal:
        {
            OutputDebugStringA(Date);
            OutputDebugStringA(LevelString);
            OutputDebugStringA(Content);
            OutputDebugStringA("\n");
        } break;
    }
}

void Raise(const char* ErrorMessage) {
    Log(Error, ErrorMessage);
    Assert(false);
}

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Timing                                                                                                                          |
// +---------------------------------------------------------------------------------------------------------------------------------+

#define TIMED_BLOCK__(FunctionName) timed_block TimedBlock_##FunctionName(__COUNTER__, __FILE__, __LINE__, __FUNCTION__)
#define TIMED_BLOCK_(Line) TIMED_BLOCK__(Line);
#define TIMED_BLOCK TIMED_BLOCK_(__LINE__)

struct debug_record {
    uint64 CycleCount;
    
    char* FileName;
    char* FunctionName;
    
    int LineNumber;
    int HitCount;
};

debug_record DebugRecordArray[];

struct timed_block {
    debug_record* Record;
    uint64 StartCycles;

    timed_block(int Counter, char* FileName, int LineNumber, char* FunctionName) {
        Record = DebugRecordArray + Counter;
        Record->FileName = FileName;
        Record->FunctionName = FunctionName;
        Record->LineNumber = LineNumber;
        Record->HitCount++; 
        StartCycles = __rdtsc();
    }

    ~timed_block() {
        Record->CycleCount += __rdtsc() - StartCycles;
    }
};


#define WIN32_DEBUG_H
#endif