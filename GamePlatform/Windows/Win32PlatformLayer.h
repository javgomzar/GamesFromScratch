#include "GamePlatform.h"
#include "Resource.h"

System_OS SystemOS = Windows;

#define RENDERER_INITIALIZE void InitializeRenderer(Arena* arena, Render_Group* group, HWND window, HINSTANCE instance, HDC device_context)
#define RENDERER_RENDER void Render(Render_Group* group, matrix4 view, game_input* input, HWND Window, double Time)

void Log(Log_Level level, String string) {
    // Level
    char level_string[16];
    switch (level) {
        case Log_Level::Info:  { strncpy_s(level_string, "[INFO]  ", 8); } break;
        case Log_Level::Warn:  { strncpy_s(level_string, "[WARN]  ", 8); } break;
        case Log_Level::Error: { strncpy_s(level_string, "[ERROR] ", 8); } break;
        case Log_Level::Test:  { strncpy_s(level_string, "[TEST]  ", 8); } break;
    }
    level_string[8] = 0;

    // Timestamp
    time_t t = time(NULL);
    struct tm tm;
    localtime_s(&tm, &t);
    char date[21];
    sprintf_s(date, "%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Logging
    switch (LOG_MODE) {
        case Log_Mode::File:
        {
            HANDLE file_handle = CreateFileA("log.log", FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
            if (file_handle != INVALID_HANDLE_VALUE) {
                DWORD bytes_written = 0;
                WriteFile(file_handle, date, 20, &bytes_written, 0);
                WriteFile(file_handle, level_string, 8, &bytes_written, 0);
                WriteFile(file_handle, string.content, string.length, &bytes_written, 0);
            }
            else {
                Assert(false);
            }

            CloseHandle(file_handle);
        } break;

        case Log_Mode::Terminal:
        {
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(console, date, 20, NULL, NULL);
            switch (level) {
                case Log_Level::Info:  { SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE); } break;
                case Log_Level::Warn:  { SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN); } break;
                case Log_Level::Error: { SetConsoleTextAttribute(console, FOREGROUND_RED); } break;
                case Log_Level::Test:  { SetConsoleTextAttribute(console, FOREGROUND_GREEN); } break;
            }
            WriteConsoleA(console, level_string, 8, NULL, NULL);
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(console, string.content, string.length, NULL, NULL);
            WriteConsoleA(console, "\n", 1, NULL, NULL);
        } break;
    }
}

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory) {
    void* result = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!result) {
        DWORD ErrorCode = GetLastError();
        char text_buffer[128];
        sprintf_s(text_buffer, "Couldn't allocate %I64u bytes. Error %d.", size, ErrorCode);
        Log(Log_Level::Error, text_buffer);
    }
    return result;
}

PLATFORM_FREE_MEMORY(Win32FreeMemory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

PLATFORM_FILE_EXISTS(Win32FileExists) {
    static char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);
    return GetFileAttributesA(path_buffer) != INVALID_FILE_ATTRIBUTES;
}

PLATFORM_GET_FILE_INFO(Win32GetFileInfo) {
    File_Info result = {};
    result.path = path;

    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);

    WIN32_FIND_DATAA FindData = {};
    HANDLE FileHandle = FindFirstFileA(path_buffer, &FindData);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        *(FILETIME*)&result.timestamp = FindData.ftLastWriteTime;
        ULARGE_INTEGER Size = {FindData.nFileSizeLow, FindData.nFileSizeHigh};
        result.size = Size.QuadPart;
        FindClose(FileHandle);
    }
    else {
        DWORD ErrorCode = GetLastError();
        char ErrorBuffer[MAX_PATH + 32];
        if (ErrorCode == ERROR_PATH_NOT_FOUND) {
            sprintf_s(ErrorBuffer, "Path %s not found.", path_buffer);
        }
        else if (ErrorCode == ERROR_FILE_NOT_FOUND) {
            sprintf_s(ErrorBuffer, "File %s not found.", path_buffer);
        }
        Log(Log_Level::Error, ErrorBuffer);
    }

    return result;
}

PLATFORM_READ_FILE_CHUNK(Win32ReadFileChunk) {
    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);

    File_Chunk_Info result = {};
    result.size = 0;
    result.offset = offset;
    result.info = Win32GetFileInfo(path);

    char text_buffer[256];
    HANDLE file_handle = CreateFileA(path_buffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER large_offset;
        large_offset.QuadPart = offset;
        if (!SetFilePointerEx(file_handle, large_offset, NULL, FILE_BEGIN)) {
            sprintf_s(text_buffer, "Couldn't set file pointer to offset %I64u at file %s.", offset, path);
            Log(Log_Level::Error, text_buffer);
            return result;
        }
        
        DWORD bytes_read;
        if (ReadFile(file_handle, memory, chunk_size, &bytes_read, NULL) && bytes_read == chunk_size) {
#if _DEBUG
            sprintf_s(text_buffer, "%d bytes read from file %s.", bytes_read, path_buffer);
            Log(Log_Level::Info, text_buffer);
#endif
        }
        else {
            sprintf_s(text_buffer, "Couldn't read chunk from file %s.", path_buffer);
            Log(Log_Level::Error, text_buffer);
        }
        result.size = bytes_read;
        CloseHandle(file_handle);
        return result;
    }

    DWORD WinError = GetLastError();
    if (WinError == ERROR_PATH_NOT_FOUND) {
        sprintf_s(text_buffer, "Path %s not found.", path_buffer);
    }
    else if (WinError == ERROR_SHARING_VIOLATION) {
        sprintf_s(text_buffer, "File %s sharing violation.", path_buffer);
    }
    else {
        sprintf_s(text_buffer, "Couldn't read file %s. Error %d.", path_buffer, WinError);
    }
    Log(Log_Level::Error, text_buffer);

    return result;
}

PLATFORM_WRITE_FILE_CHUNK(Win32WriteFileChunk) {
    bool result = false;
    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);
    char text_buffer[256];
    HANDLE FileHandle = CreateFileA(path_buffer, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER large_offset;
        large_offset.QuadPart = offset;
        if (SetFilePointerEx(FileHandle, large_offset, NULL, FILE_BEGIN)) {
            DWORD BytesWritten;
            if (WriteFile(FileHandle, memory, chunk_size, &BytesWritten, NULL)) {
                sprintf_s(text_buffer, "%d bytes written to file %s.", BytesWritten, path_buffer);
                result = BytesWritten == chunk_size;
                Log(result ? Log_Level::Info : Log_Level::Error, text_buffer);
            }
        }
        else {
            sprintf_s(text_buffer, "Couldn't set file pointer to %I64u at file %s.", offset, path_buffer);
            Log(Log_Level::Error, text_buffer);
        }
        CloseHandle(FileHandle);
    }

    if (!result) {
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            sprintf_s(text_buffer, "Path %s not found.", path_buffer);
        }
        else {
            sprintf_s(text_buffer, "Couldn't write to file %s. Error %d.", path_buffer, WinError);
        }
        Log(Log_Level::Error, text_buffer);
    }

    return result;
}

PLATFORM_APPEND_TO_FILE(Win32AppendToFile) {
    char text_buffer[256];
    bool result = false;

    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);
    HANDLE file_handle = CreateFileA(path_buffer, FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
    if (file_handle != INVALID_HANDLE_VALUE) {
        if (SetFilePointerEx(file_handle, { 0 }, NULL, FILE_END)) {
            DWORD bytes_written;
            if (WriteFile(file_handle, memory, size, &bytes_written, 0)) {
                sprintf_s(text_buffer, "%d bytes appended to file %s.", bytes_written, path_buffer);
                result = bytes_written == size;
                Log(result ? Log_Level::Info : Log_Level::Error, text_buffer);
            }
        }
        else {
            sprintf_s(text_buffer, "Couldn't set file pointer to end at file %s.", path_buffer);
            Log(Log_Level::Error, text_buffer);
        }
        CloseHandle(file_handle);
    }
    
    if (!result) {
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            sprintf_s(text_buffer, "Path %s not found.", path_buffer);
        }
        else {
            sprintf_s(text_buffer, "Couldn't append to file %s. Error %d.", path_buffer, WinError);
        }
        Log(Log_Level::Error, text_buffer);
    }

    return result;
}

PLATFORM_COPY_FILE(Win32FileCopy) {
    char SourceBuffer[256];
    strncpy_s(SourceBuffer, source.content, source.length);
    char DestinationBuffer[256];
    strncpy_s(DestinationBuffer, destination.content, destination.length);
    bool Copyresult = CopyFileA(SourceBuffer, DestinationBuffer, FALSE);
    return Copyresult;
}

PLATFORM_DELETE_FILE(Win32FileDelete) {
    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);
    bool Deleteresult = DeleteFileA(path_buffer);
    return Deleteresult;
}

PLATFORM_GET_WALL_CLOCK(Win32GetWallClock) {
    uint64 result = 0;
    QueryPerformanceCounter((LARGE_INTEGER*)&result);
    return result;
}

PLATFORM_RUN_COMMAND(Win32RunCommand) {
    STARTUPINFOA start_info = {};
    start_info.cb = sizeof(start_info);

    PROCESS_INFORMATION process_info = {};

    bool success = CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &start_info, &process_info);
    if (!success) {
        DWORD err = GetLastError();
        char error_buffer[2048];
        sprintf_s(error_buffer, "Error '%d' when trying to run command:\n    %s", err, command);
        Log(Log_Level::Error, error_buffer);
    }

    Process_Info result = {};
    result.handle = process_info.hProcess;
    result.thread_handle = process_info.hThread;
    result.running = success;

    return result;
}

PLATFORM_WAIT_FOR_PROCESS(Win32WaitForProcess) {
    DWORD wait_result = WaitForSingleObject(process->handle, timeout);
    int32 result = -1;
    if (wait_result == WAIT_FAILED) {
        char error_buffer[128] = {};
        DWORD error_code = GetLastError();
        sprintf_s(error_buffer, "Error while waiting for a process to end. Error code '%d'.", error_code);
        Log(Log_Level::Error, error_buffer);
        return result;
    }
    else if (wait_result == WAIT_OBJECT_0) {
        process->running = false;
        DWORD exit_code = 0;
        GetExitCodeProcess(process->handle, &exit_code);
        result = exit_code;
        CloseHandle(process->handle);
        CloseHandle(process->thread_handle);
    }
    return result;
}

Platform_API Platform = {
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

void SaveBMP(const char* path, int32 width, int32 height, uint32 offset, uint32 header_size, void* header, void* pixels) {
    HANDLE file_handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
    if (file_handle != INVALID_HANDLE_VALUE) {
        uint32 size = offset + 4 * width * height;
        HANDLE mapping_handle = CreateFileMappingA(file_handle, NULL, PAGE_READWRITE, 0, size, NULL);
        if (!mapping_handle) {
            DWORD win_error = GetLastError();
            Log(Log_Level::Error, "Memory map for file returned invalid handle.");
            Assert(false);
        }

        uint8* memory = (uint8*)MapViewOfFile(mapping_handle, FILE_MAP_WRITE, 0, 0, size);
        memcpy(memory, header, header_size);

        uint8* pixeldst = memory + offset;
        memcpy(pixeldst, pixels, 4 * width * height);

        FlushViewOfFile(memory, size);
        UnmapViewOfFile(memory);
        CloseHandle(mapping_handle);
        CloseHandle(file_handle);
    }
    else {
        // Debug
        DWORD win_error = GetLastError();
        if (win_error == ERROR_PATH_NOT_FOUND) {
            Log(Log_Level::Error, "Path not found.");
        }
        Assert(false);
    }
}

inline float GetSecondsElapsed(uint64 start, uint64 end) {
    uint64 timeelapsed = end - start;
    return timeelapsed / (float)Platform.performance_counter_frequency;
}
