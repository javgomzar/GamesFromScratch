#pragma once

#include "pch.h"

#include "GamePlatform.h"
#include "Win32Debug.h"

// Platform services for the game
PLATFORM_FREE_FILE_MEMORY(PlatformFreeFileMemory) {
    if (Memory) {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

PLATFORM_READ_ENTIRE_FILE(PlatformReadEntireFile) {
    read_file_result Result = {};
    HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        strcpy_s(Result.Path, Path);

        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize)) {
            Result.Content = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Content) {
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Content, FileSize.QuadPart, &BytesRead, 0)) {
                    Result.ContentSize = FileSize.QuadPart;
                }
                else {
                    PlatformFreeFileMemory(Result.Content);
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

PLATFORM_WRITE_ENTIRE_FILE(PlatformWriteEntireFile) {
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

PLATFORM_APPEND_TO_FILE(PlatformAppendToFile) {
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

// Record and playback
struct record_and_playback {
    HANDLE RecordFile;
    int RecordIndex;
    HANDLE PlaybackFile;
    int PlaybackIndex;
    void* GameMemoryBlock;
    uint64 TotalSize;
};