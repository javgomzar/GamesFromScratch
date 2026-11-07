#include "build.h"


constexpr int ARENA_SIZE = Kilobytes(8);
static char TEXT_BUFFER[2*ARENA_SIZE];

int main(int argc, char* argv[]) {
#if _WIN32
    LARGE_INTEGER perf_count_frequency_result;
    QueryPerformanceFrequency(&perf_count_frequency_result);
    Platform::performance_counter_frequency = perf_count_frequency_result.QuadPart;
#endif

    Build::permanent = Arena(ARENA_SIZE, TEXT_BUFFER);
    Build::transient = Arena(ARENA_SIZE, TEXT_BUFFER + ARENA_SIZE);

    Build::ReadConfig(argv[1]);

    // Hot reload
    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            String error_text = String::Format(Build::transient, "Incorrect argument '{s}', try '-hot'.", 1, argv[2]);
            Log::Error(error_text);
            return 2;
        }
        
        Build::MetaProgram();
        Build::HotReload();
    }
    // Normal compilation
    else if (argc == 2) {
        // Precompiled headers
        Process_Info pch_process = {};
        if (config.pch) {
            bool precompile_headers = true;
            if (Platform::FileExists(config.pch_header_path) && Platform::FileExists(config.pch_output_path.content)) {
                File_Info precompiled_headers_output = Platform::GetFileInfo(config.pch_output_path.content);
                File_Info precompiled_headers_source = Platform::GetFileInfo(config.pch_header_path);
                precompile_headers = precompiled_headers_source.timestamp > precompiled_headers_output.timestamp;
            }
            if (precompile_headers) {
                Log::Info("Compiling pre-compiled headers.");

                char* command = (char*)(transient.base + transient.used);
                switch (config.compiler) {
                    case MSVC: {
                        String::Format(transient,
                            "{s} /std:c++20 /nologo {s} {s} /c {s} "
                            "/Yc\"pch.h\" /Fp\"{s}\" /Fo\"build\\obj\\{s}.obj\" /Fd\"bin\\{s}.pdb\"",
                            7,
                            config.compiler_path, 
                            config.include,
                            config.pch_source_path,
                            config.compiler_flags,
                            config.pch_output_path,
                            config.pch_output_file,
                            config.pch_output_file
                        );
                    } break;

                    case clang: {
                        String::Format(transient,
                            "{s} -x c++-header -std=c++20 -fPIC {s} {s} GameBuild/pch.h -o {s}",
                            4,
                            config.compiler_path, 
                            config.compiler_flags,
                            config.include, 
                            config.pch_output_path
                        );
                    } break;

                    default: Raise("Invalid compiler. Currently only MSVC is supported.");
                }

                uint64 start = Platform::GetWallClock();
                pch_process = Platform::RunCommand(command);
                uint32 exit_code = Platform::WaitForProcess(&pch_process, -1);
                uint64 end = Platform::GetWallClock();

                LogCompilationResult(transient, "Precompiled headers", exit_code, start, end);
                transient.Clear();
            }
        }

        // Metaprogramming
        if (config.preprocess) {
            MetaProgram(transient, config);
        }

        // Platform layer
        uint64 platform_start = Platform::GetWallClock();
        Process_Info platform_process = CompilePlatformLayer(transient, &config);
        uint32 platform_exit_code = Platform::WaitForProcess(&platform_process, -1);
        uint64 platform_end = Platform::GetWallClock();
        const char* os_layer_name = Platform::OS == Platform::Windows ? "Windows platform layer" : "Linux platform layer";
        LogCompilationResult(transient, os_layer_name, platform_exit_code, platform_start, platform_end);
        transient.Clear();

        // Game library
        uint64 library_start = Platform::GetWallClock();
        Process_Info library_process = CompileGameLibrary(transient, &config);
        int32 library_exit_code = Platform::WaitForProcess(&library_process, -1);
        uint64 library_end = Platform::GetWallClock();
        LogCompilationResult(transient, "Game library", library_exit_code, library_start, library_end);
    }
    else {
        Log::Error("No build configuration file provided. Usage: Build <build.conf file path> [-hot]");
    }

    return 0;
}