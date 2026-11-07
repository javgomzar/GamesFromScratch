#include "platform.h"


Platform::System_OS Platform::OS = Windows;

void Log::LogLevel(Log::Level level, String message) {
    char level_string[16];
    switch (level) {
        case Log::Level::Test:  { strncpy_s(level_string, "[TEST]  ", 8); } break;
        case Log::Level::Info:  { strncpy_s(level_string, "[INFO]  ", 8); } break;
        case Log::Level::Warn:  { strncpy_s(level_string, "[WARN]  ", 8); } break;
        case Log::Level::Error: { strncpy_s(level_string, "[ERROR] ", 8); } break;
        case Log::Level::Fatal: { strncpy_s(level_string, "[FATAL] ", 8); } break;
    }
    level_string[8] = 0;

    time_t t = time(NULL);
    struct tm tm;
    localtime_s(&tm, &t);
    char date[21];
    sprintf_s(date, "%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    switch (mode) {
        case Mode::File:
        {
            HANDLE file_handle = CreateFileA("log.log", FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
            if (file_handle != INVALID_HANDLE_VALUE) {
                DWORD bytes_written = 0;
                WriteFile(file_handle, date, 20, &bytes_written, 0);
                WriteFile(file_handle, level_string, 8, &bytes_written, 0);
                WriteFile(file_handle, message.content, message.length, &bytes_written, 0);
            }
            else {
                Raise("Invalid file handle.");
            }

            CloseHandle(file_handle);
        } break;

        case Mode::Terminal:
        {
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(console, date, 20, NULL, NULL);
            switch (level) {
                case Level::Test:  { SetConsoleTextAttribute(console, FOREGROUND_GREEN); } break;
                case Level::Info:  { SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE); } break;
                case Level::Warn:  { SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN); } break;
                case Level::Error: { SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY); } break;
                case Level::Fatal: { SetConsoleTextAttribute(console, FOREGROUND_RED); }
            }
            WriteConsoleA(console, level_string, 8, NULL, NULL);
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            WriteConsoleA(console, message.content, message.length, NULL, NULL);
            WriteConsoleA(console, "\n", 1, NULL, NULL);
        } break;
    }

    if (level == Level::Fatal) {
        exit(1);
    }
}

void* Platform::AllocateMemory(memory_size size) {
    void* result = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!result) {
        DWORD error_code = GetLastError();
        char text_buffer[128];
        sprintf_s(text_buffer, "Couldn't allocate %I64u bytes. Error %d.", size, error_code);
        Log::Error(text_buffer);
    }
    return result;
}

void Platform::FreeMemory(void* memory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

bool Platform::FileExists(String path) {
    static char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);
    return GetFileAttributesA(path_buffer) != INVALID_FILE_ATTRIBUTES;
}

File_Info Platform::GetFileInfo(String path) {
    File_Info result = {};
    result.path = path;

    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);

    WIN32_FIND_DATAA find_data = {};
    HANDLE file_handle = FindFirstFileA(path_buffer, &find_data);
    if (file_handle != INVALID_HANDLE_VALUE) {
        *(FILETIME*)&result.timestamp = find_data.ftLastWriteTime;
        ULARGE_INTEGER size = {find_data.nFileSizeLow, find_data.nFileSizeHigh};
        result.size = size.QuadPart;
        FindClose(file_handle);
    }
    else {
        DWORD error_code = GetLastError();
        char error_buffer[MAX_PATH + 32];
        if (error_code == ERROR_PATH_NOT_FOUND) {
            sprintf_s(error_buffer, "Path %s not found.", path_buffer);
        }
        else if (error_code == ERROR_FILE_NOT_FOUND) {
            sprintf_s(error_buffer, "File %s not found.", path_buffer);
        }
        Log::Error(error_buffer);
    }

    return result;
}

File_Chunk_Info Platform::ReadFileChunk(String path, memory_size offset, memory_size chunk_size, void* memory) {
    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);

    File_Chunk_Info result = {};
    result.size = 0;
    result.offset = offset;
    result.info = GetFileInfo(path);

    char error_buffer[256];
    HANDLE file_handle = CreateFileA(path_buffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER large_offset;
        large_offset.QuadPart = offset;
        if (!SetFilePointerEx(file_handle, large_offset, NULL, FILE_BEGIN)) {
            sprintf_s(error_buffer, "Couldn't set file pointer to offset %I64u at file %s.", offset, path_buffer);
            Log::Error(error_buffer);
            return result;
        }
        
        DWORD bytes_read;
        bool success = ReadFile(file_handle, memory, chunk_size, &bytes_read, NULL);
        if (!success || bytes_read != chunk_size) {
            sprintf_s(error_buffer, "Couldn't read chunk from file %s.", path_buffer);
            Log::Error(error_buffer);
        }
        result.size = bytes_read;
        CloseHandle(file_handle);
        return result;
    }

    DWORD error = GetLastError();
    if (error == ERROR_PATH_NOT_FOUND) {
        sprintf_s(error_buffer, "Path %s not found.", path_buffer);
    }
    else if (error == ERROR_SHARING_VIOLATION) {
        sprintf_s(error_buffer, "File %s sharing violation.", path_buffer);
    }
    else {
        sprintf_s(error_buffer, "Couldn't read file %s. Error %d.", path_buffer, error);
    }
    Log::Error(error_buffer);

    return result;
}

void* Platform::ReadEntireFile(String path, File_Info* file_info = nullptr) {
    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);

    char error_buffer[256];
    HANDLE file_handle = CreateFileA(path_buffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER size;
        if (file_info) {
            *file_info = GetFileInfo(path);
            size.QuadPart = file_info->size;
        }
        else {
            bool ok = GetFileSizeEx(file_handle, &size);
            if (!ok) {
                sprintf_s(error_buffer, "Failed to get size of file %s.", path_buffer);
                Log::Error(error_buffer);
                return nullptr;
            }
        }

        if (size.QuadPart == 0) {
            sprintf_s(error_buffer, "Skipped reading of empty file %s.", path_buffer);
            Log::Warn(error_buffer);
            return nullptr;
        }

        void* result = AllocateMemory(size.QuadPart);
        DWORD bytes_read;
        bool ok = ReadFile(file_handle, result, size.QuadPart, &bytes_read, NULL);
        if (!ok || bytes_read != size.QuadPart) {
            sprintf_s(error_buffer, "Failed to read file %s.", path_buffer);
            Log::Error(error_buffer);
            FreeMemory(result);
            return nullptr;
        }

        CloseHandle(file_handle);
        return result;
    }
    
    DWORD error = GetLastError();
    if (error == ERROR_PATH_NOT_FOUND) {
        sprintf_s(error_buffer, "Path %s not found.", path_buffer);
    }
    else if (error == ERROR_SHARING_VIOLATION) {
        sprintf_s(error_buffer, "File %s sharing violation.", path_buffer);
    }
    else {
        sprintf_s(error_buffer, "Couldn't read file %s. Error %d.", path_buffer, error);
    }
    Log::Error(error_buffer);

    return nullptr;
}

bool Platform::WriteFileChunk(String path, memory_size offset, memory_size chunk_size, void* memory) {
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
                if (result) {
                    Log::Info(text_buffer);
                }
                else {
                    Log::Error(text_buffer);
                }
            }
        }
        else {
            sprintf_s(text_buffer, "Couldn't set file pointer to %I64u at file %s.", offset, path_buffer);
            Log::Error(text_buffer);
        }
        CloseHandle(FileHandle);
    }

    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_PATH_NOT_FOUND) {
            sprintf_s(text_buffer, "Path %s not found.", path_buffer);
        }
        else {
            sprintf_s(text_buffer, "Couldn't write to file %s. Error %d.", path_buffer, error);
        }
        Log::Error(text_buffer);
    }

    return result;
}

bool Platform::WriteEntireFile(String path, memory_size size, void* memory) {
    return WriteFileChunk(path, 0, size, memory);
}

bool Platform::AppendToFile(String path, memory_size size, void* memory) {
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
                if (result) {
                    Log::Info(text_buffer);
                }
                else {
                    Log::Error(text_buffer);
                }
            }
        }
        else {
            sprintf_s(text_buffer, "Couldn't set file pointer to end at file %s.", path_buffer);
            Log::Error(text_buffer);
        }
        CloseHandle(file_handle);
    }
    
    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_PATH_NOT_FOUND) {
            sprintf_s(text_buffer, "Path %s not found.", path_buffer);
        }
        else {
            sprintf_s(text_buffer, "Couldn't append to file %s. Error %d.", path_buffer, error);
        }
        Log::Error(text_buffer);
    }

    return result;
}

bool Platform::FileCopy(String source, String destination) {
    char source_buffer[256];
    strncpy_s(source_buffer, source.content, source.length);
    char destination_buffer[256];
    strncpy_s(destination_buffer, destination.content, destination.length);
    bool result = CopyFileA(source_buffer, destination_buffer, FALSE);
    return result;
}

bool Platform::FileDelete(String path) {
    char path_buffer[MAX_PATH];
    strncpy_s(path_buffer, path.content, path.length);
    bool result = DeleteFileA(path_buffer);
    return result;
}

uint64 Platform::GetWallClock() {
    uint64 result = 0;
    QueryPerformanceCounter((LARGE_INTEGER*)&result);
    return result;
}

Process_Info Platform::RunCommand(char* command) {
    STARTUPINFOA start_info = {};
    start_info.cb = sizeof(start_info);

    PROCESS_INFORMATION process_info = {};

    bool success = CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &start_info, &process_info);
    if (!success) {
        DWORD err = GetLastError();
        char error_buffer[2048];
        sprintf_s(error_buffer, "Error '%d' when trying to run command:\n    %s", err, command);
        Log::Error(error_buffer);
    }

    Process_Info result = {};
    result.handle = process_info.hProcess;
    result.thread_handle = process_info.hThread;
    result.running = success;

    return result;
}

int32 Platform::WaitForProcess(Process_Info& process, uint32 timeout) {
    DWORD wait_result = WaitForSingleObject(process.handle, timeout);
    int32 result = -1;
    if (wait_result == WAIT_FAILED) {
        char error_buffer[128] = {};
        DWORD error_code = GetLastError();
        sprintf_s(error_buffer, "Error while waiting for a process to end. Error code '%d'.", error_code);
        Log::Error(error_buffer);
        return result;
    }
    else if (wait_result == WAIT_OBJECT_0) {
        process.running = false;
        DWORD exit_code = 0;
        GetExitCodeProcess(process.handle, &exit_code);
        result = exit_code;
        CloseHandle(process.handle);
        CloseHandle(process.thread_handle);
    }
    return result;
}

float Platform::GetSecondsElapsed(uint64 start, uint64 end) {
    uint64 timeelapsed = end - start;
    return timeelapsed / (float)Platform::performance_counter_frequency;
}
