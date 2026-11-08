#include "platform.h"


Log::Mode Log::mode = Mode::Terminal;

void Log::Info(String message)  { LogLevel(Level::Info, message); }
void Log::Test(String message)  { LogLevel(Level::Test, message); }
void Log::Warn(String message)  { LogLevel(Level::Warn, message); }
void Log::Error(String message) { LogLevel(Level::Error, message); }
void Log::Fatal(String message) { LogLevel(Level::Fatal, message); }

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
    filename.length -= extension.length + 1;
    return filename;
}

uint64 SeedRNG(Arena& arena) {
    uint64 seed = 0;

#ifdef _DEBUG
    time_t seconds = time(nullptr);
    tm* local_time = localtime(&seconds);
    seed = local_time->tm_mday + 123456789;
#else
    seed = Platform::GetWallClock();
#endif
    for (int i = 0; i < 8; i++) {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;
    }

    String seed_text = String::Format(arena, "RNG seed: {U}.", 1, seed);
    Log::Info(seed_text);
    return seed;
}

Timed_Block::Timed_Block(int counter, const char* file_name, int line_number, const char* function_name) {
    // record = time_records + counter;
    record->file_name = file_name;
    record->function_name = function_name;
    record->line_number = line_number;
    record->hit_count++;
    start_cycles = Platform::GetWallClock();
}

Timed_Block::~Timed_Block() {
    record->cycle_count += Platform::GetWallClock() - start_cycles;
}
