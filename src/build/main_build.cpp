#include "build.h"


int main(int argc, char* argv[]) {
    Build build = Build(Kilobytes(8));

    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            String error_text = String::Format(build.transient, "Incorrect argument '{s}', try '-hot'.", 1, argv[2]);
            Log::Error(error_text);
            return 2;
        }
    }
    else {
        assert(argc == 2);
    }

    build.ReadConfig(argv[1]);

    bool success = false;

    // Pre-compiled headers
    String pch_output = String::Format(build.transient, "{s}pch.pch", 1, build.config.obj_dir);
    if (build.config.pch && NeedsUpdate(build.config.pch_header_path, pch_output)) {
        success = build.PrecompileHeaders();
        if (!success) {
            return 2;
        }
    }

    // Metaprogramming
    if (build.config.preprocess) {
        success = build.MetaProgram();
        if (!success) {
            return 2;
        }
    }

    success = build.Compile();
    if (!success) {
        return 2;
    }

    // Hot reload
    if (argc == 3) {
        success = build.HotReload();
        if (!success) {
            return 2;
        }
    }
    // Normal compilation
    else if (argc == 2) {
        success = build.LinkPlatform();
        if (!success) {
            return 2;
        }

        success = build.LinkGameLibrary();
        if (!success) {
            return 2;
        }
    }
    else {
        Log::Error("No build configuration file provided. Usage: Build <build.conf file path> [-hot]");
        return 2;
    }

    return 0;
}
