#include "GamePlatform.h"
#include "pch.h"

#define RENDERER_INITIALIZE void InitializeRenderer(render_group* Group, HWND Window, HINSTANCE Instance, HDC DeviceContext)
#define RENDERER_RENDER void Render(render_group* Group, camera* Camera, game_input* Input, HWND Window, double Time)

system_os SystemOS = Windows;

static char Environment[8192] = {};
int FillEnvironmentBuffer(char* Buffer) {
    char* EnvironmentPen = Buffer;
    char* EnvironmentStringsPointer = GetEnvironmentStrings();
    int EnvironmentLength = 0;

    while (EnvironmentStringsPointer[0] != '\0' || EnvironmentStringsPointer[1] != '\0') {
        *EnvironmentPen++ = *EnvironmentStringsPointer++;
        EnvironmentLength += 1;
    }
    return EnvironmentLength;
}
static int EnvironmentLength = FillEnvironmentBuffer(Environment);

void Log(log_level Level, const char* Content) {
    // Level
    char LevelString[9];
    switch (Level) {
        case Info:  { strcpy_s(LevelString, "[INFO]  "); } break;
        case Warn:  { strcpy_s(LevelString, "[WARN]  "); } break;
        case Error: { strcpy_s(LevelString, "[ERROR] "); } break;
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

        case Terminal_Log_Mode:
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(hConsole, Date, 20, NULL, NULL);
            switch (Level) {
                case Info:  { SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE); } break;
                case Warn:  { SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN); } break;
                case Error: { SetConsoleTextAttribute(hConsole, FOREGROUND_RED); } break;
            }
            WriteConsoleA(hConsole, LevelString, 8, NULL, NULL);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(hConsole, Content, strlen(Content), NULL, NULL);
            WriteConsoleA(hConsole, "\n", 1, NULL, NULL);
        } break;
    }
}

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory) {
    void* Result = VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!Result) {
        DWORD ErrorCode = GetLastError();
        char ErrorText[128];
        sprintf_s(ErrorText, "Couldn't allocate %I64u bytes. Error %d.", Size, ErrorCode);
        Log(Error, ErrorText);
    }
    return Result;
}

PLATFORM_FREE_MEMORY(Win32FreeMemory) {
    if (Memory) {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

PLATFORM_FILE_EXISTS(Win32FileExists) {
    return GetFileAttributesA(Path) != INVALID_FILE_ATTRIBUTES;
}

PLATFORM_GET_FILE_INFO(Win32GetFileInfo) {
    file_info Result = {};
    Result.Path = Path;

    WIN32_FIND_DATAA FindData = {};
    HANDLE FileHandle = FindFirstFileA(Path, &FindData);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        *(FILETIME*)&Result.Timestamp = FindData.ftLastWriteTime;
        ULARGE_INTEGER Size = {FindData.nFileSizeLow, FindData.nFileSizeHigh};
        Result.Size = Size.QuadPart;
        FindClose(FileHandle);
    }

    return Result;
}

PLATFORM_READ_FILE_CHUNK(Win32ReadFileChunk) {
    char ErrorText[256];

    file_chunk_info Result = {};
    Result.ChunkSize = ChunkSize;
    Result.Offset = Offset;
    Result.Info = Win32GetFileInfo(Path);

    HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER LargeOffset;
        LargeOffset.QuadPart = Offset;
        if (!SetFilePointerEx(FileHandle, LargeOffset, NULL, FILE_BEGIN)) {
            sprintf_s(ErrorText, "Couldn't set file pointer to offset %I64u at file %s", Offset, Path);
            Log(Error, ErrorText);
            return Result;
        }
        
        DWORD BytesRead;
        if (ReadFile(FileHandle, Memory, ChunkSize, &BytesRead, NULL) && BytesRead == ChunkSize) {
            sprintf_s(ErrorText, "%d bytes read from file %s", BytesRead, Path);
            Log(Info, ErrorText);
        }
        else {
            sprintf_s(ErrorText, "Couldn't read chunk from file %s", Path);
            Log(Error, ErrorText);
        }
        CloseHandle(FileHandle);
        return Result;
    }

    DWORD WinError = GetLastError();
    if (WinError == ERROR_PATH_NOT_FOUND) {
        sprintf_s(ErrorText, "Path %s not found.", Path);
    }
    else {
        sprintf_s(ErrorText, "Couldn't read file %s. Error %d.", Path, WinError);
    }
    Log(Error, ErrorText);

    return Result;
}

PLATFORM_WRITE_FILE_CHUNK(Win32WriteFileChunk) {
    char ErrorText[256];
    bool Result = false;

    HANDLE FileHandle = CreateFileA(Path, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER LargeOffset;
        LargeOffset.QuadPart = Offset;
        if (SetFilePointerEx(FileHandle, LargeOffset, NULL, FILE_BEGIN)) {
            DWORD BytesWritten;
            if (WriteFile(FileHandle, Memory, ChunkSize, &BytesWritten, NULL)) {
                sprintf_s(ErrorText, "%d bytes written to file %s", BytesWritten, Path);
                Result = BytesWritten == ChunkSize;
                Log(Result ? Info : Error, ErrorText);
            }
        }
        else {
            sprintf_s(ErrorText, "Couldn't set file pointer to %I64u at file %s", Offset, Path);
            Log(Error, ErrorText);
        }
        CloseHandle(FileHandle);
    }

    if (!Result) {
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            sprintf_s(ErrorText, "Path %s not found.", Path);
        }
        else {
            sprintf_s(ErrorText, "Couldn't write to file %s. Error %d.", Path, WinError);
        }
        Log(Error, ErrorText);
    }

    return Result;
}

PLATFORM_APPEND_TO_FILE(Win32AppendToFile) {
    char ErrorText[256];
    bool Result = false;

    HANDLE FileHandle = CreateFileA(Path, FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        if (SetFilePointerEx(FileHandle, { 0 }, NULL, FILE_END)) {
            DWORD BytesWritten;
            if (WriteFile(FileHandle, Memory, Size, &BytesWritten, 0)) {
                sprintf_s(ErrorText, "%d bytes written to file %s", BytesWritten, Path);
                Result = BytesWritten == Size;
                Log(Result ? Info : Error, ErrorText);
            }
        }
        else {
            sprintf_s(ErrorText, "Couldn't set file pointer to end at file %s", Path);
            Log(Error, ErrorText);
        }
        CloseHandle(FileHandle);
    }
    
    if (!Result) {
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            sprintf_s(ErrorText, "Path %s not found.", Path);
        }
        else {
            sprintf_s(ErrorText, "Couldn't append to file %s. Error %d.", Path, WinError);
        }
        Log(Error, ErrorText);
    }

    return Result;
}

PLATFORM_COPY_FILE(Win32FileCopy) {
    bool CopyResult = CopyFileA(Source, Destination, FALSE);
    return CopyResult;
}

PLATFORM_DELETE_FILE(Win32FileDelete) {
    bool CopyResult = DeleteFileA(Path);
    return CopyResult;
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

    bool Success = CreateProcessA(NULL, Command, NULL, NULL, FALSE, 0, Environment, NULL, &StartInfo, &ProcessInfo);
    if (!Success) {
        DWORD Err = GetLastError();
        char ErrorBuffer[2048];
        sprintf_s(ErrorBuffer, "Error '%d' when trying to run command:\n    %s", Err, Command);
        Log(Error, ErrorBuffer);
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
        Log(Error, ErrorBuffer);
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
            Log(Error, "Memory map for file returned invalid handle.");
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
            Log(Error, "Path not found.");
        }
        Assert(false);
    }
}

inline float GetSecondsElapsed(uint64 Start, uint64 End) {
    uint64 TimeElapsed = End - Start;
    return TimeElapsed / (float)Platform.PerformanceCounterFrequency;
}
