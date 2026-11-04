#include "GameBuild.h"

constexpr int ARENA_SIZE = Kilobytes(8);
static char TEXT_BUFFER[2*ARENA_SIZE];

int main(int argc, char* argv[]) {
#if _WIN32
    LARGE_INTEGER perf_count_frequency_result;
    QueryPerformanceFrequency(&perf_count_frequency_result);
    Platform.performance_counter_frequency = perf_count_frequency_result.QuadPart;
#endif

    Arena permanent = MemoryArena(ARENA_SIZE, TEXT_BUFFER);
    Arena transient = MemoryArena(ARENA_SIZE, TEXT_BUFFER + ARENA_SIZE);

    Build_Configuration config = ReadBuildConfiguration(permanent, argv[1]);

    // Hot reload
    if (argc == 3 && argv[2]) {
        if (strcmp(argv[2], "-hot") != 0) {
            String error_text = Format(transient, "Incorrect argument '{s}', try '-hot'.", 1, argv[2]);
            Log(Log_Level::Error, error_text.content);
            return 2;
        }
        
        MetaProgram(transient, &config);
        
        uint64 library_compilation_start = Platform.GetWallClock();
        Process_Info library_compilation = CompileGameLibraryHot(transient, config);
        int32 wait_result = Platform.WaitForProcess(&library_compilation, -1);
        uint64 library_compilation_end = Platform.GetWallClock();
        LogCompilationResult(transient, "Game library", wait_result, library_compilation_start, library_compilation_end);
    }
    // Normal compilation
    else if (argc == 2) {
        // Precompiled headers
        Process_Info pch_process = {};
        if (config.pch) {
            bool precompile_headers = true;
            const char* pch_source_path = "GameBuild" PATH_SEPARATOR "pch.h";
            if (Platform.FileExists(pch_source_path) && Platform.FileExists(config.pch_output_path.content)) {
                File_Info precompiled_headers_output = Platform.GetFileInfo(config.pch_output_path.content);
                File_Info precompiled_headers_source = Platform.GetFileInfo(pch_source_path);
                precompile_headers = precompiled_headers_source.timestamp > precompiled_headers_output.timestamp;
            }
            if (precompile_headers) {
                Log(Log_Level::Info, "Compiling pre-compiled headers.");

                char* command = (char*)(transient.base + transient.used);
                switch (config.compiler) {
                    case MSVC: {
                        Format(transient,
                            "{s} /std:c++20 /nologo {s} GameBuild/pch.cpp /c {s} "
                            "/Yc\"pch.h\" /Fp\"{s}\" /Fo\"bin\\{s}.obj\" /Fd\"bin\\{s}.pdb\"",
                            6,
                            config.compiler_path, 
                            config.include,
                            config.compiler_flags,
                            config.pch_output_path,
                            config.pch_output_file,
                            config.pch_output_file
                        );
                    } break;

                    case clang: {
                        Format(transient,
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

                uint64 start = Platform.GetWallClock();
                pch_process = Platform.RunCommand(command);
                uint32 exit_code = Platform.WaitForProcess(&pch_process, -1);
                uint64 end = Platform.GetWallClock();

                LogCompilationResult(transient, "Precompiled headers", exit_code, start, end);
                Clear(transient);
            }
        }

        // Metaprogramming
        if (config.preprocess) {
            MetaProgram(transient, &config);
        }

        // Platform layer
        uint64 platform_start = Platform.GetWallClock();
        Process_Info platform_process = CompilePlatformLayer(transient, &config);
        uint32 platform_exit_code = Platform.WaitForProcess(&platform_process, -1);
        uint64 platform_end = Platform.GetWallClock();
        const char* os_layer_name = SystemOS == Windows ? "Windows platform layer" : "Linux platform layer";
        LogCompilationResult(transient, os_layer_name, platform_exit_code, platform_start, platform_end);
        Clear(transient);

        // Game library
        uint64 library_start = Platform.GetWallClock();
        Process_Info library_process = CompileGameLibrary(transient, &config);
        int32 library_exit_code = Platform.WaitForProcess(&library_process, -1);
        uint64 library_end = Platform.GetWallClock();
        LogCompilationResult(transient, "Game library", library_exit_code, library_start, library_end);
    }
    else {
        Log(Log_Level::Error, "No build configuration file provided. Usage: Build <build.conf file path> [-hot]");
    }

    return 0;
}