#include "GamePlatform.h"

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

PLATFORM_FREE_FILE_MEMORY(Win32FreeFileMemory) {
    if (Memory) {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

PLATFORM_READ_ENTIRE_FILE(Win32ReadEntireFile) {
    read_file_result Result = {};
    Result.Path = Path;
    HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize)) {
            Result.Content = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Content) {
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Content, FileSize.QuadPart, &BytesRead, 0)) {
                    Result.ContentSize = FileSize.QuadPart;
                }
                else {
                    Win32FreeFileMemory(Result.Content);
                    Result.Content = 0;
                }
            }
        }

        WIN32_FIND_DATAA Data;
        HANDLE hFind = FindFirstFileA(Path, &Data);
        *(FILETIME*)&Result.Timestamp = Data.ftLastWriteTime;

        CloseHandle(FileHandle);
    }
    else {
        DWORD LastError = GetLastError();
        char ErrorText[256];
        sprintf_s(ErrorText, "Error while opening file %s. Error code %d.", Path, LastError);
        Log(Error, ErrorText);
    }
    return Result;
};

PLATFORM_WRITE_ENTIRE_FILE(Win32WriteEntireFile) {
    bool Result = false;
    HANDLE FileHandle = CreateFileA(Path, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0)) {
            Result = true;
        }
        CloseHandle(FileHandle);
    }
    else {
        // Debug
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            Log(Error, "Path not found.");
        }
        Assert(false);
    }
    return Result;
}

PLATFORM_APPEND_TO_FILE(Win32AppendToFile) {
    bool Result = false;
    HANDLE FileHandle = CreateFileA(Path, FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        if (SetFilePointerEx(FileHandle, { 0 }, NULL, FILE_END)) {
            DWORD BytesWritten;
            if (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0)) {
                Result = true;
            }
        }
        else {
            Assert(false);
        }

        CloseHandle(FileHandle);
    }
    else {
        // Debug
        Assert(false);
    }
    return Result;
}

PLATFORM_GET_LAST_WRITE_TIME(Win32GetLastWriteTime) {
    int64 Result = 0;

    WIN32_FIND_DATAA FindData = {};
    HANDLE FileHandle = FindFirstFileA(Path, &FindData);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        *(FILETIME*)&Result = FindData.ftLastWriteTime;
        FindClose(FileHandle);
    }

    return Result;
}

PLATFORM_GET_WALL_CLOCK(Win32GetWallClock) {
    uint64 Result = 0;
    QueryPerformanceCounter((LARGE_INTEGER*)&Result);
    return Result;
}

const platform_api Platform = {
    .ReadEntireFile   = Win32ReadEntireFile,
    .WriteEntireFile  = Win32WriteEntireFile,
    .FreeFileMemory   = Win32FreeFileMemory,
    .AppendToFile     = Win32AppendToFile,
    .GetLastWriteTime = Win32GetLastWriteTime,
    .GetWallClock     = Win32GetWallClock,
};