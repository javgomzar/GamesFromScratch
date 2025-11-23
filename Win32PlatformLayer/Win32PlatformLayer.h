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
        case File:
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

        case Terminal:
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

PLATFORM_FILE_EXISTS(Win32FileExists) {
    return GetFileAttributesA(Path) != INVALID_FILE_ATTRIBUTES;
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
        Result.ContentSize = 0;
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

PLATFORM_COPY_FILE(Win32CopyFile) {
    bool CopyResult = CopyFileA(Source, Destination, FALSE);
    return CopyResult;
}

PLATFORM_DELETE_FILE(Win32DeleteFile) {
    bool CopyResult = DeleteFileA(Path);
    return CopyResult;
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
    .FileExists       = Win32FileExists,
    .ReadEntireFile   = Win32ReadEntireFile,
    .WriteEntireFile  = Win32WriteEntireFile,
    .FreeFileMemory   = Win32FreeFileMemory,
    .AppendToFile     = Win32AppendToFile,
    .Copy             = Win32CopyFile,
    .Delete           = Win32DeleteFile,
    .GetLastWriteTime = Win32GetLastWriteTime,
    .GetWallClock     = Win32GetWallClock,
    .RunCommand       = Win32RunCommand,
    .WaitForProcess   = Win32WaitForProcess,
};

inline float GetSecondsElapsed(uint64 Start, uint64 End) {
    uint64 TimeElapsed = End - Start;
    return TimeElapsed / (float)Platform.PerformanceCounterFrequency;
}
