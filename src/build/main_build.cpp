#include "build.h"


int main(int argc, char* argv[]) {
    Build build = Build(Kilobytes(8));
    build.ReadConfig(argv[1]);

    // Hot reload
    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            String error_text = String::Format(build.transient, "Incorrect argument '{s}', try '-hot'.", 1, argv[2]);
            Log::Error(error_text);
            return 2;
        }
        
        // build.MetaProgram();
        // build.HotReload();
    }
    // Normal compilation
    else if (argc == 2) {
        
        build.Compile();
        build.transient.Clear();

        // // Metaprogramming
        // if (config.preprocess) {
        //     MetaProgram(transient, config);
        // }

        // // Platform layer
        // uint64 platform_start = Platform::GetWallClock();
        // Process_Info platform_process = CompilePlatformLayer(transient, &config);
        // uint32 platform_exit_code = Platform::WaitForProcess(&platform_process, -1);
        // uint64 platform_end = Platform::GetWallClock();
        // const char* os_layer_name = Platform::OS == Platform::Windows ? "Windows platform layer" : "Linux platform layer";
        // LogCompilationResult(transient, os_layer_name, platform_exit_code, platform_start, platform_end);
        // transient.Clear();

        // // Game library
        // uint64 library_start = Platform::GetWallClock();
        // Process_Info library_process = CompileGameLibrary(transient, &config);
        // int32 library_exit_code = Platform::WaitForProcess(&library_process, -1);
        // uint64 library_end = Platform::GetWallClock();
        // LogCompilationResult(transient, "Game library", library_exit_code, library_start, library_end);
    }
    else {
        Log::Error("No build configuration file provided. Usage: Build <build.conf file path> [-hot]");
    }

    return 0;
}
