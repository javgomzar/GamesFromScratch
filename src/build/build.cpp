#include "build.h"


constexpr int ARENA_SIZE = Kilobytes(8);
static char TEXT_BUFFER[2*ARENA_SIZE];

Build_Configuration ReadBuildConfiguration(Arena& permanent, String config_file_path) {
    Build_Configuration config;
    File_Info config_file_info;
    void* config_file = Platform.ReadEntireFile(config_file_path.content, &config_file_info);
    if (config_file) {
        Tokenizer tokenizer = InitTokenizer(config_file, config_file_info.size);
        Token token = GetToken(tokenizer);
    
        while(token.type != Token_End) {
            // Comments
            if (token.type == Token_Pound) {
                AdvanceUntilNextLine(tokenizer);
            }
    
            else if (token.type == Token_Identifier) {
                if (token == "MODE") {
                    RequireToken(tokenizer, Token_Equal);
                    Token mode_token = RequireToken(tokenizer, Token_Identifier);
                    config.mode = GetBuildMode(mode_token);
                }
    
                else if (token == "COMPILER") {
                    RequireToken(tokenizer, Token_Equal);
                    config.compiler = GetCompiler(RequireToken(tokenizer, Token_Identifier));
                }
    
                else if (token == "COMPILER_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    config.compiler_path.content = tokenizer.at;
                    config.compiler_path.length = ParsePath(tokenizer.at);
                    Advance(tokenizer, config.compiler_path.length);
                }

                else if (token == "PRECOMPILE_HEADERS") {
                    RequireToken(tokenizer, Token_Equal);
                    config.pch = ParseBool(tokenizer);
                }

                else if (token == "PRECOMPILED_HEADERS_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    config.pch_source_dir.content = tokenizer.at;
                    config.pch_source_dir.length = ParsePath(tokenizer.at);
                    Advance(tokenizer, config.pch_source_dir.length);
                }
    
                else if (token == "INCLUDE") {
                    RequireToken(tokenizer, Token_Equal);

                    config.include.length = 0;
                    config.include.content = (const char*)(permanent.base + permanent.used);

                    do {
                        AdvanceUntilNextLine(tokenizer);
                        char* pointer = PushArray(permanent, 3, char);
                        config.include.length += 3;
                        if (config.compiler == MSVC) {
                            *pointer++ = '/';
                        }
                        else if (config.compiler == clang) {
                            *pointer++ = '-';
                        }
                        *pointer++ = 'I';
                        *pointer++ = '\"';
                        int path_length = ParsePath(tokenizer.at);
                        PushArray(permanent, path_length + 1, char);
                        config.include.length += path_length + 1;
                        strncpy(pointer, tokenizer.at, path_length);
                        Advance(tokenizer, path_length);
                        pointer += path_length;
                        *pointer++ = '\"';
                        if (tokenizer.at[0] != ';') {
                            PushArray(permanent, 1, char);
                            *pointer++ = '\0';
                            break;
                        }
                        PushArray(permanent, 1, char);
                        config.include.length += 1;
                        *pointer++ = ' ';
                    } while (true);
                }
    
                else if (token == "LIB") {
                    RequireToken(tokenizer, Token_Equal);

                    config.lib.length = 0;
                    config.lib.content = (char*)(permanent.base + permanent.used);

                    do {
                        AdvanceUntilNextLine(tokenizer);
                        char* pointer = PushArray(permanent, 10, char);
                        strncpy(pointer, "/LIBPATH:\"", 10);
                        config.lib.length += 10;
                        int path_length = ParsePath(tokenizer.at);
                        pointer = PushArray(permanent, path_length + 1, char);
                        config.lib.length += path_length + 1;
                        strncpy(pointer, tokenizer.at, path_length);
                        Advance(tokenizer, path_length);
                        pointer += path_length;
                        *pointer++ = '\"';
                        if (tokenizer.at[0] != ';') {
                            PushArray(permanent, 1, char);
                            *pointer++ = '\0';
                            break;
                        }
                        PushArray(permanent, 1, char);
                        config.lib.length += 1;
                        *pointer++ = ' ';
                    } while (true);
                }

                // Linked libraries
                else if (token == "LINK") {
                    RequireToken(tokenizer, Token_Equal);
                    config.linked_libs.length = 0;
                    config.linked_libs.content = (char*)(permanent.base + permanent.used);
                    do {
                        Token library = RequireToken(tokenizer, Token_Identifier);
                        RequireToken(tokenizer, Token_Dot);
                        RequireToken(tokenizer, "lib");
                        char* linked_libraries = PushArray(permanent, library.length + 4, char);
                        strncpy(linked_libraries, library.text, library.length + 4);
                        config.linked_libs.length += library.length + 4;
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        RequireToken(tokenizer, Token_Semicolon);
                        char* Space = PushArray(permanent, 1, char);
                        *Space = ' ';
                        config.linked_libs.length += 1;
                    } while(true);
                }
    
                // Metaprogramming source file
                else if (token == "META") {
                    RequireToken(tokenizer, Token_Equal);
                    config.preprocess = true;
                    int metaprogramming_file_path_length = ParsePath(tokenizer.at);
                    char* pointer = PushArray(permanent, metaprogramming_file_path_length + 1, char);
                    strncpy(pointer, tokenizer.at, metaprogramming_file_path_length);
                    config.metaprogramming_code_path.length = metaprogramming_file_path_length;
                    config.metaprogramming_code_path.content = pointer;
                    AdvanceUntilNextLine(tokenizer);
                }

                // Metaprogramming input files
                else if (token == "META_FILES") {
                    RequireToken(tokenizer, Token_Equal);
                    AdvanceWhitespaceAndComments(tokenizer);
                    config.metaprogramming_input_files.content = tokenizer.at;
                    config.metaprogramming_input_files.length = 0;
                    config.metaprogramming_input_file_count = 0;
                    do {
                        AdvanceWhitespaceAndComments(tokenizer);
                        int PathLength = ParsePath(tokenizer.at);
                        Advance(tokenizer, PathLength);
                        config.metaprogramming_input_file_count += 1;
                        const char* End = tokenizer.at;
                        if (tokenizer.at[0] != ';') {
                            config.metaprogramming_input_files.length = End - config.metaprogramming_input_files.content;
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                    } while(token.type == Token_Semicolon);
                }
    
                // Renderer
                else if (token == "RENDERER") {
                    RequireToken(tokenizer, Token_Equal);
                    Token renderer_token = RequireToken(tokenizer, Token_Identifier);
                    config.renderer = GetRenderer(renderer_token);
                    config.renderer_libs = GetRendererLibs(config.renderer);
                }
            }
    
            token = GetToken(tokenizer);
        }

        config.compiler_flags = GetCompilerFlags(config.compiler, config.mode);
        config.pch_output_file = config.mode == Debug ? "debug_pch" : "pch";
        if (SystemOS == Windows) {
            config.pch_output_path = Format(permanent, "bin" PATH_SEPARATOR "{s}.pch", 1, config.pch_output_file);
            config.metaprogramming_output_file = "bin" PATH_SEPARATOR "Meta.exe";
            config.pch_header_path = Format(permanent, "{s}" PATH_SEPARATOR "pch.h", 1, config.pch_source_dir);
            config.pch_source_path = Format(permanent, "{s}" PATH_SEPARATOR "pch.cpp", 1, config.pch_source_dir);
        }
        else {
            config.pch_output_path = Format(permanent, "bin" PATH_SEPARATOR "{s}.gch", 1, config.pch_output_file);
            config.metaprogramming_output_file = "bin" PATH_SEPARATOR "Meta";
        }
    }
    else {
        String error = Format(permanent, "Build configuration file {s} not found.", 1, config_file_path);
        Raise(error.content);
    }

    return config;
}

void LogCompilationResult(Arena& arena, String Name, int32 ExitCode, uint64 Start, uint64 End) {
    Log_Level Level = ExitCode == 0 ? Log_Level::Info : Log_Level::Error;
    String LogString;
    if (ExitCode == 0) {
        LogString = Format(arena, "{s} code compiled in {f2} milliseconds.", 2, Name, 1000.0f * GetSecondsElapsed(Start, End));
    }
    else { 
        LogString = Format(arena, "{s} code compilation failed, exit code '{i}'.", 2, Name, ExitCode);
    }
    Log(Level, LogString.content);
}

Process_Info CompileMetaprogramming(Arena& arena, Build_Configuration* config) {
    Log(Log_Level::Info, "Compiling metaprogramming code.");
    char* Command = (char*)(arena.base + arena.used);
    switch(config->compiler) {
        case MSVC: {
            Format(arena,
                "{s} /std:c++20 /nologo {s} /Fo\"build\\obj\\meta.obj\" /Fd\"bin\\meta.pdb\" {s} {s} "
                "/link {s} /OUT:\"bin\\meta.exe\" /PDB:\"bin\\meta.pdb\"", 
                5,
                config->compiler_path, 
                config->include, 
                config->metaprogramming_code_path, 
                config->compiler_flags, 
                config->lib
            );
        } break;

        case clang: {
            Format(arena,
                "{s} -std=c++20 {s} {s} -o bin/Meta", 
                4,
                config->compiler_path, 
                config->include, 
                config->metaprogramming_code_path, 
                config->compiler_flags
            );
        } break;

        default: Raise("Invalid compiler. Only MSVC and clang are supported for now.");
    }
    return Platform.RunCommand(Command);
}

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

Process_Info CompileGameLibraryHot(Arena& arena, Build_Configuration& config) {
#if _WIN32
    int hot_reload_count = 0;
    String pdb_file = Format(arena, "bin\\GameLibrary{i}.pdb", 1, hot_reload_count);
    bool exists;
    do {
        exists = Platform.FileExists(pdb_file.content);
        if (exists) {
            hot_reload_count += 1;
            PopArray(arena, pdb_file.length + 1, char);
            pdb_file = Format(arena, "bin\\GameLibrary{i}.pdb", 1, hot_reload_count);
        }
    } while(exists);
    char* command = (char*)(arena.base + arena.used);
    Format(arena,
        "{s} /std:c++20 /nologo /D GAMELIBRARY_EXPORTS src\\GameLibrary\\GameLibrary.cpp {s} {s} "
        "/Fo\"build\\obj\\GameLibrary.obj\" /Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" "
        "/link {s} build\\obj\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
        "/PDB:\"{s}\" /ILK:\"bin\\GameLibrary.ilk\" "
        "/OUT:\"bin\\GameLibrary.dll\"",
        8,
        config.compiler_path, 
        config.compiler_flags, 
        config.include, 
        config.pch_output_file, 
        config.pch_output_file, 
        config.lib,
        config.pch_output_file, 
        pdb_file
    );
#else
    char* Command = (char*)(arena->base + arena->used);
    Format(arena,
        "{s} -std=c++20 -shared -fPIC {s} {s} -include-pch bin/{s}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
        4,
        config->compiler_path,
        config->compiler_flags,
        config->include,
        config->pch_output
    );
#endif
    return Platform.RunCommand(command);
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