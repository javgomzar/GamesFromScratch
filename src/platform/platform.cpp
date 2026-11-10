#include "pch.h"
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

bool NeedsUpdate(String source_path, String destination_path) {
    if (Platform::FileExists(source_path)) {
        File_Info source = Platform::GetFileInfo(source_path);
        if (Platform::FileExists(destination_path)) {
            File_Info destination = Platform::GetFileInfo(destination_path);
            return destination.timestamp < source.timestamp;
        }
        return true;
    }
    char temp_buffer[512];
    Arena temp_arena = Arena(512, temp_buffer);
    String message = String::Format(temp_arena, "File {s} doesn't exist.", 1, source_path);
    Log::Error(message);
    return false;
}

const uint8 MAX_SUPPORTED_MONITORS = 8;

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
