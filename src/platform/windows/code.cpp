#include "pch.h"
#include "code.h"


String source_dll_name = "bin\\game_library.dll";
String temp_dll_name = "bin\\game_library_temp.dll";

void GameUpdateStub(Memory& memory) {};

static HMODULE game_code_dll;

bool Game_Code::NeedsUpdate() {
    int64 timestamp = Platform::GetFileInfo(source_dll_name).timestamp;
    return timestamp > dll_last_write_time;
}

bool Game_Code::Load(Arena& transient) {
    update = GameUpdateStub;

    char error_text[256];
    DWORD last_error = 0;

    bool copy_result = Platform::FileCopy(source_dll_name, temp_dll_name);
    if (!copy_result) {
        last_error = GetLastError();
        if (last_error == ERROR_SHARING_VIOLATION) {
            int retries = 0;
            do {
                Log::Warn("Retrying game code loading after sharing violation.");
                Sleep(100);
                copy_result = Platform::FileCopy(source_dll_name, temp_dll_name);
                ++retries;
                if (retries > 100) {
                    Log::Error("Max number of retries reached.");
                    break;
                }
            } while (!copy_result);
        }
        else {
            String error_message = String::Format(transient,
                "Error copying file {s} into {s}. Error code {i}.",
                3,
                source_dll_name,
                temp_dll_name,
                last_error
            );
            Log::Error(error_message);
            return false;
        }
    }

    game_code_dll = LoadLibraryA(temp_dll_name.content);
    if (game_code_dll) {
        update = (Game_Update)GetProcAddress(game_code_dll, "GameUpdate");
        is_valid = (update);

        dll_last_write_time = Platform::GetFileInfo(source_dll_name).timestamp;
        if (is_valid) {
            Log::Info("Game code correctly loaded.");
        }
        return is_valid;
    }
    else {
        last_error = GetLastError();
        String error_message = String::Format(transient, "Error loading game code. Error code {u}.", 1, last_error);
        Log::Error(error_message); // If error is 126 (dependency error while loading DLL) try using Process Monitor.
    }
    return false;
}

void Game_Code::Unload() {
    if (game_code_dll) {
        bool result = FreeLibrary(game_code_dll);
    }

    is_valid = false;
    update = GameUpdateStub;
}
