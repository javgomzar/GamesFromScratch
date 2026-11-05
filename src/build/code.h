#include "GameLibrary.h"

static const char const source_dll_name[] = "bin\\GameLibrary.dll";
static const char const temp_dll_name[] = "bin\\GameLibraryTemp.dll";
static const char const meta_file[] = "bin\\meta.exe";

struct Game_Code {
    Build_Configuration build_config;
    Process_Info library_compilation;
    Process_Info metaprogramming_compilation;
    Process_Info metaprogramming_execution;
    int64 dll_last_write_time;
    HMODULE game_code_dll;
    Game_Update* update;
    bool is_valid;

    void Initialize(Arena& arena) {
        build_config = ReadBuildConfiguration(arena, "build/build.conf");
    }

    void Load() {
        
    }
};

// Dynamic library loading
void GameUpdateStub(GAME_UPDATE_INPUTS) {}

void LoadGameCode(Game_Code* result, LPCSTR source_dll_name, LPCSTR temp_dll_name) {
    result->update = GameUpdateStub;

    char error_text[256];
    DWORD last_error = 0;

    bool copy_result = Win32FileCopy(source_dll_name, temp_dll_name);
    if (!copy_result) {
        last_error = GetLastError();
        if (last_error == ERROR_SHARING_VIOLATION) {
            int retries = 0;
            do {
                Log(Log_Level::Warn, "Retrying game code loading after sharing violation.");
                Sleep(100);
                copy_result = Win32FileCopy(source_dll_name, temp_dll_name);
                retries++;
                if (retries > 100) {
                    Log(Log_Level::Error, "Max number of retries reached.");
                    break;
                }
            } while (!copy_result);
        }
        else {
            sprintf_s(error_text, "Error copying file %s into %s. Error code %d.", source_dll_name, temp_dll_name, last_error);
            Log(Log_Level::Error, error_text);
            return;
        }
    }

    result->game_code_dll = LoadLibraryA(temp_dll_name);
    if (result->game_code_dll) {
        result->update = (Game_Update*)GetProcAddress(result->game_code_dll, "GameUpdate");
        result->is_valid = (result->update);

        result->dll_last_write_time = Win32GetFileInfo(source_dll_name).timestamp;
    }
    else {
        last_error = GetLastError();
        sprintf_s(error_text, "Error loading game code. Error code %d.", last_error);
        Log(Log_Level::Error, error_text); // If error is 126 (dependency error while loading DLL) try using Process Monitor.
    }
}

void UnloadGameCode(Game_Code* code) {
    if (code->game_code_dll) {
        bool UnloadResult = FreeLibrary(code->game_code_dll);
    }

    code->is_valid = false;
    code->update = GameUpdateStub;
}