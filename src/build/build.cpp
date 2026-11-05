#include "build.h"


constexpr int ARENA_SIZE = Kilobytes(8);
static char TEXT_BUFFER[2*ARENA_SIZE];

/* Compiles the meta-program (if needed) and runs it.*/
void MetaProgram(Arena& arena, Build_Configuration* config) {
    File_Info metaprogramming_source = Platform.GetFileInfo(config->metaprogramming_code_path.content);
    File_Info metaprogramming_binary;

    bool metaprogramming_binary_exists = Platform.FileExists(config->metaprogramming_output_file.content);        
    if (metaprogramming_binary_exists) {
        metaprogramming_binary = Platform.GetFileInfo(config->metaprogramming_output_file.content);
    }
    
    int32 wait_result = 0;
    uint64 metaprogramming_compilation_start = 0;
    uint64 metaprogramming_compilation_end = 0;
    Process_Info metaprogramming_compilation = {};
    if (!metaprogramming_binary_exists || metaprogramming_source.timestamp > metaprogramming_binary.timestamp) {
        metaprogramming_compilation_start = Platform.GetWallClock();
        metaprogramming_compilation = CompileMetaprogramming(arena, config);
        wait_result = Platform.WaitForProcess(&metaprogramming_compilation, -1);
        metaprogramming_compilation_end = Platform.GetWallClock();
        LogCompilationResult(arena, "Metaprogramming", wait_result, metaprogramming_compilation_start, metaprogramming_compilation_end);
    }

    char* meta_command = PushArray(arena, 64, char);
    strncpy(meta_command, config->metaprogramming_output_file.content, 64);
    uint64 metaprogramming_execution_start = Platform.GetWallClock();
    Process_Info metaprogramming_execution = Platform.RunCommand(meta_command);
    wait_result = Platform.WaitForProcess(&metaprogramming_execution, -1);
    if (wait_result >= 0) {
        uint64 end = Platform.GetWallClock();
        float time = GetSecondsElapsed(metaprogramming_execution_start, end);
        Log_Level level = wait_result > 0 ? Log_Level::Error : Log_Level::Info;
        String log_text;
        if (wait_result == 0) {
            log_text = Format(arena, "Metaprogramming executed in {f2} milliseconds.", 1, 1000.0f * time);
        }
        else {
            log_text = Format(arena, "Metaprogramming execution failed with code '{i}'", 1, wait_result);
        }
        Log(level, log_text.content);
    }
}

Process_Info CompilePlatformLayer(Arena& arena, Build_Configuration* config) {
    char* command = (char*)(arena.base + arena.used);
    switch (config->compiler) {
        case MSVC: {
            String command_string = Format(arena,
                "{s} /std:c++20 /nologo "
                "src\\GamePlatform\\Windows\\Win32PlatformLayer.cpp build\\obj\\{s}.obj " 
                "/D GAME_RENDER_API_{s} {s} "
                "/Fe\"bin\\RunGame.exe\" "
                "/Fo\"build\\obj\\Win32PlatformLayer.obj\" "
                "/Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" {s} "
                "/link {s} {s} {s} "
                "src\\GamePlatform\\Windows\\Win32PlatformLayer.res /MACHINE:X64",
                10,
                config->compiler_path,
                config->pch_output_file,
                GetRendererString(config->renderer),
                config->compiler_flags,
                config->pch_output_file,
                config->pch_output_file,
                config->include,
                config->lib,
                config->linked_libs,
                config->renderer_libs
            );
        } break;

        case clang: {
            Format(arena,
                "{s} -std=c++20 -fPIC {s} {s} -include-pch bin/{s}.gch "
                "LinuxPlatformLayer/LinuxPlatformLayer.cpp {s} -o bin/RunGame && chmod +x bin/RunGame",
                5,
                config->compiler_path,
                config->compiler_flags,
                config->include,
                config->pch_output_file,
                config->renderer_libs
            );
        };
    }

    return Platform.RunCommand(command);
}

Process_Info CompileGameLibrary(Arena& arena, Build_Configuration* config) {
    char* Command = (char*)(arena.base + arena.used);
    switch(config->compiler) {
        case MSVC: {
            Format(arena,
                "{s} /std:c++20 /nologo /D GAMELIBRARY_EXPORTS "
                "src\\GameLibrary\\GameLibrary.cpp {s} {s} /Fo\"build\\obj\\GameLibrary.obj\" "
                "/Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" "
                "/link {s} build\\obj\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
                "/PDB:\"bin\\GameLibrary.pdb\" "
                "/ILK:\"bin\\GameLibrary.ilk\" /OUT:\"bin\\GameLibrary.dll\"",
                7,
                config->compiler_path, 
                config->compiler_flags, 
                config->include, 
                config->pch_output_file, 
                config->pch_output_file, 
                config->lib, 
                config->pch_output_file
            );
        } break;

        case clang: {
            Format(arena,
                "{s} -std=c++20 -shared -fPIC {s} {s} -include-pch bin/{s}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
                4,
                config->compiler_path,
                config->compiler_flags,
                config->include,
                config->pch_output_file
            );
        } break;
    }

    return Platform.RunCommand(Command);
}

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
            if (Platform.FileExists(config.pch_header_path) && Platform.FileExists(config.pch_output_path.content)) {
                File_Info precompiled_headers_output = Platform.GetFileInfo(config.pch_output_path.content);
                File_Info precompiled_headers_source = Platform.GetFileInfo(config.pch_header_path);
                precompile_headers = precompiled_headers_source.timestamp > precompiled_headers_output.timestamp;
            }
            if (precompile_headers) {
                Log(Log_Level::Info, "Compiling pre-compiled headers.");

                char* command = (char*)(transient.base + transient.used);
                switch (config.compiler) {
                    case MSVC: {
                        Format(transient,
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