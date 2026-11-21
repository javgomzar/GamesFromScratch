#include "build.h"


memory_size Memory::permanent_size = Kilobytes(8);
memory_size Memory::transient_size = Kilobytes(8);

int main(int argc, char* argv[]) {
    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            String error_text = String::Format(Memory::transient, "Incorrect argument '{s}', try '-hot'.", 1, argv[2]);
            Log::Error(error_text);
            return 2;
        }
    }
    else {
        assert(argc == 2);
    }

    Build::ReadConfig(argv[1]);

    bool success = false;

    // Pre-compiled headers
    String pch_output = String::Format(Memory::transient, "{s}pch.pch", 1, Build::config.obj_dir);
    if (Build::config.pch && NeedsUpdate(Build::config.pch_header_path, pch_output)) {
        success = Build::PrecompileHeaders();
        if (!success) {
            return 2;
        }
    }

    // Metaprogramming
    if (Build::config.preprocess) {
        success = Build::MetaProgram();
        if (!success) {
            return 2;
        }
    }

    success = Build::Compile();
    if (!success) {
        return 2;
    }

    // Hot reload
    if (argc == 3) {
        success = Build::HotReload();
        if (!success) {
            return 2;
        }
    }
    // Normal compilation
    else if (argc == 2) {
        success = Build::LinkPlatform();
        if (!success) {
            return 2;
        }

        success = Build::LinkGameLibrary();
        if (!success) {
            return 2;
        }
    }
    else {
        Log::Error("No build configuration file provided. Usage: Build <Build::conf file path> [-hot]");
        return 2;
    }

    return 0;
}
