#include "platform.h"
#include "Resource.h"


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
