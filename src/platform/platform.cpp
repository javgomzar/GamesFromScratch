#include "platform.h"

String GetFileName(String path) {
    String result;
    
    const char* pChar = path.content + path.length - 1;
    for (; pChar > path.content; pChar--) {
        if (*pChar == '\\' || *pChar == '/') {
            pChar++;
            break;
        }
        result.length++;
    }

    result.content = pChar;
    return result;
}

String GetFileExtension(String path) {
    String result;

    const char* pChar = path.content + path.length - 1;
    for (; pChar > path.content; pChar--) {
        if (*pChar == '.') {
            pChar++;
            break;
        }
        result.length++;
    }

    result.content = pChar;

    return result;
}

String GetFileNameWithoutExtension(String path) {
    String filename = GetFileName(path);
    String extension = GetFileExtension(path);
    filename.length -= filename.length + 1;
    return filename;
}

uint64 SeedRNG(Arena& arena) {
    uint64 seed = 0;

#ifdef _DEBUG
    time_t seconds = time(nullptr);
    tm* local_time = localtime(&seconds);
    seed = local_time->tm_mday + 123456789;
#else
    seed = Platform.GetWallClock();
#endif
    for (int i = 0; i < 8; i++) {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;
    }

    String seed_text = Format(arena, "RNG seed: {U}.", 1, seed);
    Log(Log_Level::Info, seed_text);
    return seed;
}
