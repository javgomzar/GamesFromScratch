#pragma once

#include "resource.h"
#include "framework.h"
#include "stdio.h"
#include "XInput.h"
#include "xaudio2.h"
#include "glew.h"
#include "..\GameLibrary\GameLibrary.h"


// Platform services for the game
PLATFORM_FREE_FILE_MEMORY(PlatformFreeFileMemory) {
    if (Memory) {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

PLATFORM_READ_ENTIRE_FILE(PlatformReadEntireFile) {
    read_file_result Result = {};
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
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
                    PlatformFreeFileMemory(Result.Content);
                    Result.Content = 0;
                }
            }
        }

        CloseHandle(FileHandle);
    }
    else {
        DWORD LastError = GetLastError();
        char ErrorText[256];
        sprintf_s(ErrorText, "Error while opening file %s. Error code %d.\n", Filename, LastError);
        Log(Error, ErrorText);
    }
    return Result;
};

PLATFORM_WRITE_ENTIRE_FILE(PlatformWriteEntireFile) {
    bool Result = false;
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
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
        Assert(false);
    }
    return Result;
}

PLATFORM_APPEND_TO_FILE(PlatformAppendToFile) {
    bool Result = false;
    HANDLE FileHandle = CreateFileA(Filename, FILE_APPEND_DATA, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
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