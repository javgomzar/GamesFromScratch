#pragma once

#include <build.h>
#include "library.h"


using Game_Update = void(*)(Memory&);

struct Game_Code {
    int64 dll_last_write_time = 0;
    Game_Update update = nullptr;
    bool is_valid = false;

    bool NeedsUpdate();
    void Load(Arena& transient);
    void Unload();
};
