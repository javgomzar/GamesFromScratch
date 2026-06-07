#include "GamePlatform.h"
#include "Resource.h"

system_os SystemOS = Windows;

#define RENDERER_INITIALIZE void InitializeRenderer(render_group* Group, HWND Window, HINSTANCE Instance, HDC DeviceContext)
#define RENDERER_RENDER void Render(render_group* Group, matrix4 View, game_input* Input, HWND Window, double Time)

void Log(log_level Level, string String) {
    // Level
    char LevelString[16];
    switch (Level) {
        case log_level::Info:  { strncpy_s(LevelString, "[INFO]  ", 8); } break;
        case log_level::Warn:  { strncpy_s(LevelString, "[WARN]  ", 8); } break;
        case log_level::Error: { strncpy_s(LevelString, "[ERROR] ", 8); } break;
        case log_level::Test:  { strncpy_s(LevelString, "[TEST]  ", 8); } break;
    }
    LevelString[8] = 0;

    // Timestamp
    time_t t = time(NULL);
    struct tm tm;
    localtime_s(&tm, &t);
    char Date[21];
    sprintf_s(Date, "%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Logging
    switch (LOG_MODE) {
        case File_Log_Mode:
        {
            HANDLE FileHandle = CreateFileA("log.log", FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
            if (FileHandle != INVALID_HANDLE_VALUE) {
                DWORD BytesWritten = 0;
                WriteFile(FileHandle, Date, 20, &BytesWritten, 0);
                WriteFile(FileHandle, LevelString, 8, &BytesWritten, 0);
                WriteFile(FileHandle, String.Content, String.Length, &BytesWritten, 0);
            }
            else {
                Assert(false);
            }

            CloseHandle(FileHandle);
        } break;

        case Terminal_Log_Mode:
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(hConsole, Date, 20, NULL, NULL);
            switch (Level) {
                case log_level::Info:  { SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE); } break;
                case log_level::Warn:  { SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN); } break;
                case log_level::Error: { SetConsoleTextAttribute(hConsole, FOREGROUND_RED); } break;
                case log_level::Test:  { SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN); } break;
            }
            WriteConsoleA(hConsole, LevelString, 8, NULL, NULL);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(hConsole, String.Content, String.Length, NULL, NULL);
            WriteConsoleA(hConsole, "\n", 1, NULL, NULL);
        } break;
    }
}

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory) {
    void* Result = VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!Result) {
        DWORD ErrorCode = GetLastError();
        char TextBuffer[128];
        sprintf_s(TextBuffer, "Couldn't allocate %I64u bytes. Error %d.", Size, ErrorCode);
        Log(log_level::Error, TextBuffer);
    }
    return Result;
}

PLATFORM_FREE_MEMORY(Win32FreeMemory) {
    if (Memory) {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

PLATFORM_FILE_EXISTS(Win32FileExists) {
    static char PathBuffer[MAX_PATH];
    strncpy_s(PathBuffer, Path.Content, Path.Length);
    return GetFileAttributesA(PathBuffer) != INVALID_FILE_ATTRIBUTES;
}

PLATFORM_GET_FILE_INFO(Win32GetFileInfo) {
    file_info Result = {};
    Result.Path = Path;

    char PathBuffer[MAX_PATH];
    strncpy_s(PathBuffer, Path.Content, Path.Length);

    WIN32_FIND_DATAA FindData = {};
    HANDLE FileHandle = FindFirstFileA(PathBuffer, &FindData);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        *(FILETIME*)&Result.Timestamp = FindData.ftLastWriteTime;
        ULARGE_INTEGER Size = {FindData.nFileSizeLow, FindData.nFileSizeHigh};
        Result.Size = Size.QuadPart;
        FindClose(FileHandle);
    }
    else {
        DWORD ErrorCode = GetLastError();
        char ErrorBuffer[64];
        if (ErrorCode == ERROR_PATH_NOT_FOUND) {
            sprintf_s(ErrorBuffer, "Path %s not found.", PathBuffer);
        }
        else if (ErrorCode == ERROR_FILE_NOT_FOUND) {
            sprintf_s(ErrorBuffer, "File %s not found.", PathBuffer);
        }
        Log(log_level::Error, ErrorBuffer);
    }

    return Result;
}

PLATFORM_READ_FILE_CHUNK(Win32ReadFileChunk) {
    char PathBuffer[MAX_PATH];
    strncpy_s(PathBuffer, Path.Content, Path.Length);

    file_chunk_info Result = {};
    Result.Size = 0;
    Result.Offset = Offset;
    Result.Info = Win32GetFileInfo(Path);

    char TextBuffer[256];
    HANDLE FileHandle = CreateFileA(PathBuffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER LargeOffset;
        LargeOffset.QuadPart = Offset;
        if (!SetFilePointerEx(FileHandle, LargeOffset, NULL, FILE_BEGIN)) {
            sprintf_s(TextBuffer, "Couldn't set file pointer to offset %I64u at file %s.", Offset, Path);
            Log(log_level::Error, TextBuffer);
            return Result;
        }
        
        DWORD BytesRead;
        if (ReadFile(FileHandle, Memory, ChunkSize, &BytesRead, NULL) && BytesRead == ChunkSize) {
#if _DEBUG
            sprintf_s(TextBuffer, "%d bytes read from file %s.", BytesRead, Path.Content);
            Log(log_level::Info, TextBuffer);
#endif
        }
        else {
            sprintf_s(TextBuffer, "Couldn't read chunk from file %s.", Path.Content);
            Log(log_level::Error, TextBuffer);
        }
        Result.Size = BytesRead;
        CloseHandle(FileHandle);
        return Result;
    }

    DWORD WinError = GetLastError();
    if (WinError == ERROR_PATH_NOT_FOUND) {
        sprintf_s(TextBuffer, "Path %s not found.", Path.Content);
    }
    else if (WinError == ERROR_SHARING_VIOLATION) {
        sprintf_s(TextBuffer, "File %s sharing violation.", Path.Content);
    }
    else {
        sprintf_s(TextBuffer, "Couldn't read file %s. Error %d.", Path.Content, WinError);
    }
    Log(log_level::Error, TextBuffer);

    return Result;
}

PLATFORM_WRITE_FILE_CHUNK(Win32WriteFileChunk) {
    bool Result = false;
    char PathBuffer[MAX_PATH];
    strncpy_s(PathBuffer, Path.Content, Path.Length);
    char TextBuffer[256];
    HANDLE FileHandle = CreateFileA(PathBuffer, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER LargeOffset;
        LargeOffset.QuadPart = Offset;
        if (SetFilePointerEx(FileHandle, LargeOffset, NULL, FILE_BEGIN)) {
            DWORD BytesWritten;
            if (WriteFile(FileHandle, Memory, ChunkSize, &BytesWritten, NULL)) {
                sprintf_s(TextBuffer, "%d bytes written to file %s.", BytesWritten, PathBuffer);
                Result = BytesWritten == ChunkSize;
                Log(Result ? log_level::Info : log_level::Error, TextBuffer);
            }
        }
        else {
            sprintf_s(TextBuffer, "Couldn't set file pointer to %I64u at file %s.", Offset, PathBuffer);
            Log(log_level::Error, TextBuffer);
        }
        CloseHandle(FileHandle);
    }

    if (!Result) {
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            sprintf_s(TextBuffer, "Path %s not found.", PathBuffer);
        }
        else {
            sprintf_s(TextBuffer, "Couldn't write to file %s. Error %d.", PathBuffer, WinError);
        }
        Log(log_level::Error, TextBuffer);
    }

    return Result;
}

PLATFORM_APPEND_TO_FILE(Win32AppendToFile) {
    char TextBuffer[256];
    bool Result = false;

    char PathBuffer[MAX_PATH];
    strncpy_s(PathBuffer, Path.Content, Path.Length);
    HANDLE FileHandle = CreateFileA(PathBuffer, FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        if (SetFilePointerEx(FileHandle, { 0 }, NULL, FILE_END)) {
            DWORD BytesWritten;
            if (WriteFile(FileHandle, Memory, Size, &BytesWritten, 0)) {
                sprintf_s(TextBuffer, "%d bytes appended to file %s.", BytesWritten, PathBuffer);
                Result = BytesWritten == Size;
                Log(Result ? log_level::Info : log_level::Error, TextBuffer);
            }
        }
        else {
            sprintf_s(TextBuffer, "Couldn't set file pointer to end at file %s.", PathBuffer);
            Log(log_level::Error, TextBuffer);
        }
        CloseHandle(FileHandle);
    }
    
    if (!Result) {
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            sprintf_s(TextBuffer, "Path %s not found.", PathBuffer);
        }
        else {
            sprintf_s(TextBuffer, "Couldn't append to file %s. Error %d.", PathBuffer, WinError);
        }
        Log(log_level::Error, TextBuffer);
    }

    return Result;
}

PLATFORM_COPY_FILE(Win32FileCopy) {
    char SourceBuffer[256];
    strncpy_s(SourceBuffer, Source.Content, Source.Length);
    char DestinationBuffer[256];
    strncpy_s(DestinationBuffer, Destination.Content, Destination.Length);
    bool CopyResult = CopyFileA(SourceBuffer, DestinationBuffer, FALSE);
    return CopyResult;
}

PLATFORM_DELETE_FILE(Win32FileDelete) {
    char PathBuffer[MAX_PATH];
    strncpy_s(PathBuffer, Path.Content, Path.Length);
    bool DeleteResult = DeleteFileA(PathBuffer);
    return DeleteResult;
}

PLATFORM_GET_WALL_CLOCK(Win32GetWallClock) {
    uint64 Result = 0;
    QueryPerformanceCounter((LARGE_INTEGER*)&Result);
    return Result;
}

PLATFORM_RUN_COMMAND(Win32RunCommand) {
    STARTUPINFOA StartInfo = {};
    StartInfo.cb = sizeof(StartInfo);

    PROCESS_INFORMATION ProcessInfo = {};

    bool Success = CreateProcessA(NULL, Command, NULL, NULL, FALSE, 0, NULL, NULL, &StartInfo, &ProcessInfo);
    if (!Success) {
        DWORD Err = GetLastError();
        char ErrorBuffer[2048];
        sprintf_s(ErrorBuffer, "Error '%d' when trying to run command:\n    %s", Err, Command);
        Log(log_level::Error, ErrorBuffer);
    }

    process_info Process = {};
    Process.Handle = ProcessInfo.hProcess;
    Process.ThreadHandle = ProcessInfo.hThread;
    Process.Running = Success;

    return Process;
}

PLATFORM_WAIT_FOR_PROCESS(Win32WaitForProcess) {
    DWORD WaitResult = WaitForSingleObject(Process->Handle, Timeout);
    int32 Result = -1;
    if (WaitResult == WAIT_FAILED) {
        char ErrorBuffer[128] = {};
        DWORD ErrorCode = GetLastError();
        sprintf_s(ErrorBuffer, "Error while waiting for a process to end. Error code '%d'.", ErrorCode);
        Log(log_level::Error, ErrorBuffer);
        return Result;
    }
    else if (WaitResult == WAIT_OBJECT_0) {
        Process->Running = false;
        DWORD ExitCode = 0;
        GetExitCodeProcess(Process->Handle, &ExitCode);
        Result = ExitCode;
        CloseHandle(Process->Handle);
        CloseHandle(Process->ThreadHandle);
    }
    return Result;
}

platform_api Platform = {
    .AllocateMemory = Win32AllocateMemory,
    .FreeMemory = Win32FreeMemory,
    .FileExists = Win32FileExists,
    .GetFileInfo = Win32GetFileInfo,
    .ReadFileChunk = Win32ReadFileChunk,
    .WriteFileChunk = Win32WriteFileChunk,
    .AppendToFile = Win32AppendToFile,
    .FileCopy = Win32FileCopy,
    .FileDelete = Win32FileDelete,
    .GetWallClock = Win32GetWallClock,
    .RunCommand = Win32RunCommand,
    .WaitForProcess = Win32WaitForProcess,
};

void SaveBMP(const char* Path, int32 Width, int32 Height, uint32 Offset, uint32 HeaderSize, void* Header, void* Pixels) {
    HANDLE hFile = CreateFileA(Path, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        uint32 Size = Offset + 4 * Width * Height;
        HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, Size, NULL);
        if (!hMapping) {
            DWORD WinError = GetLastError();
            Log(log_level::Error, "Memory map for file returned invalid handle.");
            Assert(false);
        }

        uint8* Memory = (uint8*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, Size);
        memcpy(Memory, Header, HeaderSize);

        uint8* PixelDst = Memory + Offset;
        memcpy(PixelDst, Pixels, 4 * Width * Height);

        FlushViewOfFile(Memory, Size);
        UnmapViewOfFile(Memory);
        CloseHandle(hMapping);
        CloseHandle(hFile);
    }
    else {
        // Debug
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            Log(log_level::Error, "Path not found.");
        }
        Assert(false);
    }
}

inline float GetSecondsElapsed(uint64 Start, uint64 End) {
    uint64 TimeElapsed = End - Start;
    return TimeElapsed / (float)Platform.PerformanceCounterFrequency;
}
