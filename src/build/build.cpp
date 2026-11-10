#include "pch.h"
#include "build.h"


Build::Build(memory_size arena_size) : 
    permanent(arena_size), transient(arena_size),
    metaprogramming("Metaprogramming", String(), Build_Process::Output_Mode::EXE),
    platform("Platform layer", String(), Build_Process::Output_Mode::EXE),
    game_library("Game library", String(), Build_Process::Output_Mode::DLL)
{}

void Build::ReadConfig(String config_file_path) {
    File_Info config_file_info;
    void* config_file = Platform::ReadEntireFile(config_file_path.content, &config_file_info);
    if (config_file) {
        Tokenizer tokenizer = Tokenizer(config_file, config_file_info.size);
        Token token = tokenizer.GetToken();
    
        while(token.type != Token_End) {
            // Comments
            if (token.type == Token_Pound) {
                tokenizer.AdvanceUntilNextLine();
            }
    
            else if (token.type == Token_Identifier) {
                if (token == "MODE") {
                    RequireToken(tokenizer, Token_Equal);
                    Token mode_token = RequireToken(tokenizer, Token_Identifier);
                    if (mode_token == "DEBUG") {
                        config.mode = Build_Config::Mode::Debug;
                        config.obj_dir = "build" PATH_SEPARATOR "debug/";
                    }
                    else {
                        config.mode = Build_Config::Mode::Release;
                        config.obj_dir = "build" PATH_SEPARATOR "release/";
                    }
                }
    
                else if (token == "COMPILER") {
                    RequireToken(tokenizer, Token_Equal);
                    Token compiler_token = RequireToken(tokenizer, Token_Identifier);
                    if (compiler_token == "MSVC")       config.compiler = Build_Config::Compiler::MSVC;
                    else if (compiler_token == "clang") config.compiler = Build_Config::Compiler::clang;
                    else Log::Fatal("Invalid compiler. Currently only MSVC and clang are supported.");
                }
    
                else if (token == "COMPILER_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    config.compiler_path.content = tokenizer.at;
                    config.compiler_path.length = ParsePath(tokenizer.at);
                    tokenizer.Advance(config.compiler_path.length);
                }

                else if (token == "PRECOMPILE_HEADERS") {
                    RequireToken(tokenizer, Token_Equal);
                    config.pch = ParseBool(tokenizer);
                }

                else if (token == "PRECOMPILED_HEADERS_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    int path_length = ParsePath(tokenizer.at);
                    config.pch_source_dir = String(tokenizer.at, path_length);
                    tokenizer.Advance(path_length);
                }
    
                else if (token == "INCLUDE") {
                    RequireToken(tokenizer, Token_Equal);

                    config.include.length = 0;
                    config.include.content = (const char*)(permanent.base + permanent.used);

                    do {
                        tokenizer.AdvanceUntilNextLine();
                        char* pointer = permanent.PushArray<char>(3);
                        config.include.length += 3;
                        if (config.compiler == Build_Config::Compiler::MSVC) {
                            *pointer++ = '/';
                        }
                        else if (config.compiler == Build_Config::Compiler::clang) {
                            *pointer++ = '-';
                        }
                        *pointer++ = 'I';
                        *pointer++ = '\"';
                        int path_length = ParsePath(tokenizer.at);
                        permanent.PushArray<char>(path_length + 1);
                        config.include.length += path_length + 1;
                        strncpy(pointer, tokenizer.at, path_length);
                        tokenizer.Advance(path_length);
                        pointer += path_length;
                        *pointer++ = '\"';
                        if (tokenizer.at[0] != ';') {
                            permanent.Push<char>();
                            *pointer++ = '\0';
                            break;
                        }
                        permanent.Push<char>();
                        config.include.length += 1;
                        *pointer++ = ' ';
                    } while (true);
                }
    
                else if (token == "LIB") {
                    RequireToken(tokenizer, Token_Equal);

                    config.lib.length = 0;
                    config.lib.content = (char*)(permanent.base + permanent.used);

                    do {
                        tokenizer.AdvanceUntilNextLine();
                        char* pointer = permanent.PushArray<char>(10);
                        strncpy(pointer, "/LIBPATH:\"", 10);
                        config.lib.length += 10;
                        int path_length = ParsePath(tokenizer.at);
                        pointer = permanent.PushArray<char>(path_length + 1);
                        config.lib.length += path_length + 1;
                        strncpy(pointer, tokenizer.at, path_length);
                        tokenizer.Advance(path_length);
                        pointer += path_length;
                        *pointer++ = '\"';
                        if (tokenizer.at[0] != ';') {
                            permanent.Push<char>();
                            *pointer++ = '\0';
                            break;
                        }
                        permanent.Push<char>();
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
                        char* linked_libraries = permanent.PushArray<char>(library.length + 4);
                        strncpy(linked_libraries, library.text, library.length + 4);
                        config.linked_libs.length += library.length + 4;
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        RequireToken(tokenizer, Token_Semicolon);
                        char* space = permanent.Push<char>();
                        *space = ' ';
                        config.linked_libs.length += 1;
                    } while(true);
                }
    
                // Renderer
                else if (token == "RENDERER") {
                    RequireToken(tokenizer, Token_Equal);
                    Token renderer_token = RequireToken(tokenizer, Token_Identifier);
                    if      (renderer_token == "Vulkan")  config.renderer = Build_Config::Renderer::Vulkan;
                    else if (renderer_token == "DirectX") config.renderer = Build_Config::Renderer::DirectX;
                    else if (renderer_token == "OpenGL")  config.renderer = Build_Config::Renderer::OpenGL;
                    else Log::Fatal("Invalid renderer. Currently only OpenGL, DirectX or Vulkan are supported.");

                    if (Platform::OS == Platform::Windows) {
                        switch(config.renderer) {
                            case Build_Config::Renderer::OpenGL:  config.renderer_libs = "glew32.lib";
                            case Build_Config::Renderer::DirectX: config.renderer_libs = "D3d11.lib d3dcompiler.lib";
                            case Build_Config::Renderer::Vulkan:  config.renderer_libs = "vulkan-1.lib shaderc_combined.lib";
                        }
                    }
                    else if (Platform::OS == Platform::Linux) {
                        config.renderer_libs = "-lglfw";
                    }
                }

                // Source files
                else if (token == "SOURCE") {
                    RequireToken(tokenizer, Token_Equal);
                    tokenizer.AdvanceWhitespaceAndComments();
                    char* source_files_start = (char*)(permanent.base + permanent.used);
                    memory_size source_files_length = 0;

                    char* obj_files_start = (char*)(transient.base + transient.used);
                    memory_size obj_files_length = 0;

                    char temp_buffer[1024];
                    Arena temp_arena = Arena(1024, temp_buffer);
                    do {
                        tokenizer.AdvanceWhitespaceAndComments();
                        int path_length = ParsePath(tokenizer.at);
                        String source_file = String(tokenizer.at, path_length);
                        String file_name = GetFileNameWithoutExtension(source_file);
                        String obj_file = String::Format(temp_arena, "{s}{s}.obj", 2, config.obj_dir, file_name);

                        bool update = NeedsUpdate(source_file, obj_file);
                        if (update) {
                            String::Copy(permanent, source_file);
                            source_files_length += source_file.length;
                        }
                        String::Copy(transient, obj_file);
                        obj_files_length += obj_file.length;
                        temp_arena.Clear();

                        tokenizer.Advance(path_length);
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                        if (update) {
                            char* space = permanent.Push<char>();
                            *space = ' ';
                            ++source_files_length;
                        }
                        char* space = transient.Push<char>();
                        *space = ' ';
                        ++obj_files_length;
                    } while(token.type == Token_Semicolon);

                    config.source_files = String(source_files_start, source_files_length);
                    config.obj_files = String::Copy(permanent, String(obj_files_start, obj_files_length));

                    transient.Clear();
                }

                // Metaprogramming source file
                else if (token == "META") {
                    RequireToken(tokenizer, Token_Equal);
                    config.preprocess = true;
                    int metaprogramming_file_path_length = ParsePath(tokenizer.at);
                    char* pointer = permanent.PushArray<char>(metaprogramming_file_path_length + 1);
                    strncpy(pointer, tokenizer.at, metaprogramming_file_path_length);
                    metaprogramming.main = String(pointer, metaprogramming_file_path_length);
                    tokenizer.AdvanceUntilNextLine();
                }

                // Metaprogramming input files
                else if (token == "META_FILES") {
                    RequireToken(tokenizer, Token_Equal);
                    tokenizer.AdvanceWhitespaceAndComments();
                    config.metaprogramming_input_files.content = tokenizer.at;
                    config.metaprogramming_input_files.length = 0;
                    config.metaprogramming_input_file_count = 0;
                    do {
                        tokenizer.AdvanceWhitespaceAndComments();
                        int path_length = ParsePath(tokenizer.at);
                        tokenizer.Advance(path_length);
                        config.metaprogramming_input_file_count += 1;
                        const char* End = tokenizer.at;
                        if (tokenizer.at[0] != ';') {
                            config.metaprogramming_input_files.length = End - config.metaprogramming_input_files.content;
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                    } while(token.type == Token_Semicolon);
                }

                // Platform layer main
                else if (token == "PLATFORM") {
                    RequireToken(tokenizer, Token_Equal);
                    int path_length = ParsePath(tokenizer.at);
                    platform.main = String(tokenizer.at, path_length);
                    tokenizer.Advance(path_length);
                }

                // Game library
                else if (token == "GAME") {
                    RequireToken(tokenizer, Token_Equal);
                    int path_length = ParsePath(tokenizer.at);
                    game_library.main = String(tokenizer.at, path_length);
                    tokenizer.Advance(path_length);
                }
            }
    
            token = tokenizer.GetToken();
        }

        switch (config.compiler) {
            case Build_Config::Compiler::MSVC: {
                switch(config.mode) {
                    case Build_Config::Mode::Release: config.compiler_flags = "/O2";
                    case Build_Config::Mode::Debug:   config.compiler_flags = "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
                }
            } break;

            case Build_Config::Compiler::clang: {
                switch(config.mode) {
                    case Build_Config::Mode::Release: config.compiler_flags = "-O2";
                    case Build_Config::Mode::Debug:   config.compiler_flags = "-g -D_DEBUG -fsanitize=address -fsanitize=undefined";
                }
            } break;

            default: Log::Fatal("Invalid compiler. Only MSVC and clang are supported.");
        }
        
        config.pch_output_file = config.mode == Build_Config::Mode::Debug ? "debug_pch" : "pch";
        if (Platform::OS == Platform::Windows) {
            config.pch_output_path = String::Format(permanent, "bin" PATH_SEPARATOR "{s}.pch", 1, config.pch_output_file);
            config.pch_header_path = String::Format(permanent, "{s}" PATH_SEPARATOR "pch.h", 1, config.pch_source_dir);
            config.pch_source_path = String::Format(permanent, "{s}" PATH_SEPARATOR "pch.cpp", 1, config.pch_source_dir);
        }
        else {
            config.pch_output_path = String::Format(permanent, "bin" PATH_SEPARATOR "{s}.gch", 1, config.pch_output_file);
        }
    }
    else {
        String error = String::Format(permanent, "Build configuration file {s} not found.", 1, config_file_path);
        Log::Fatal(error);
    }
}

void LogResult(Arena& transient, int32 exit_code, String text, uint64 start, uint64 end) {
    if (exit_code == 0) {
        String message = String::Format(transient,
            "{s} done in {f2} milliseconds.", 2, text, 1000.0f * Platform::GetSecondsElapsed(start, end));
        Log::Info(message);
    }
    else {
        String message = String::Format(transient, "{s} failed, exit code '{i}'.", 2, text, exit_code);
        Log::Error(message);
    }
}

void Build_Process::Link(Arena& transient, Build_Config& config) {
    String start_message = String::Format(transient, "{s} compilation started.", 1, name);
    Log::Info(start_message);
    
    char* command = nullptr;
    switch (config.compiler) {
        case Build_Config::Compiler::MSVC: {
            switch (output_mode) {
                case Build_Process::Output_Mode::EXE: {

                } break;
            }

            command = (char*)(transient.base + transient.used);
            String command_string = String::Format(transient, "{s} /std:c++20 /nologo {s} "
                "{s} {s}/Fo\"{s}\"",
                5,
                config.compiler_path,
                config.include

            );
        } break;

        case Build_Config::Compiler::clang: {
            Log::Fatal("Not implemented.");
        } break;
    }

    uint64 start = Platform::GetWallClock();
    Process_Info process = Platform::RunCommand(command);
    uint32 exit_code = Platform::WaitForProcess(process, -1);
    uint64 end = Platform::GetWallClock();

    LogResult(transient, exit_code, name, start, end);

    transient.Clear();
}

void Build::Compile() {
    // Precompiled headers
    if (config.pch && NeedsUpdate(config.pch_header_path, config.pch_output_path)) {
        Log::Info("Precompiling headers.");
        char* command = (char*)(transient.base + transient.used);
        switch (config.compiler) {
            case Build_Config::Compiler::MSVC: {
                String::Format(transient,
                    "{s} /std:c++20 /nologo {s} {s} /c {s} "
                    "/Yc\"pch.h\" /Fp\"{s}\" /Fo\"{s}{s}.obj\" /Fd\"bin\\{s}.pdb\"",
                    8,
                    config.compiler_path, 
                    config.include,
                    config.pch_source_path,
                    config.compiler_flags,
                    config.pch_output_path,
                    config.obj_dir,
                    config.pch_output_file,
                    config.pch_output_file
                );
            } break;

            case Build_Config::Compiler::clang: {
                String::Format(transient,
                    "{s} -x c++-header -std=c++20 -fPIC {s} {s} GameBuild/pch.h -o {s}",
                    4,
                    config.compiler_path, 
                    config.compiler_flags,
                    config.include, 
                    config.pch_output_path
                );
            } break;

            default: Log::Fatal("Invalid compiler. Currently only MSVC and clang are supported.");
        }

        uint64 start = Platform::GetWallClock();
        Process_Info process = Platform::RunCommand(command);
        uint32 exit_code = Platform::WaitForProcess(process, -1);
        uint64 end = Platform::GetWallClock();

        LogResult(transient, exit_code, "Headers precompilation", start, end);
    }

    // Metaprogramming
    if (config.preprocess) {
        MetaProgram();
    }

    // Source files compilation
    String source_files = String((char*)(permanent.base + permanent.used), 0);

    if (config.source_files.length > 0) {
        String::Copy(permanent, config.source_files);
        char* space = permanent.Push<char>();
        *space = ' ';
        source_files.length += config.source_files.length + 1;
    }

    String platform_file_name = GetFileNameWithoutExtension(platform.main);
    String platform_obj = String::Format(transient, "{s}{s}.obj", 2, config.obj_dir, platform_file_name);
    platform.update = NeedsUpdate(platform.main, platform_obj);

    if (platform.update) {
        String::Copy(permanent, platform.main);
        char* space = permanent.Push<char>();
        *space = ' ';
        source_files.length += platform.main.length + 1;
    }

    String game_file_name = GetFileNameWithoutExtension(game_library.main);
    String game_obj = String::Format(transient, "{s}{s}.obj", 2, config.obj_dir, game_file_name);
    game_library.update = NeedsUpdate(game_library.main, game_obj);

    if (game_library.update) {
        String::Copy(permanent, game_library.main);
        char* space = permanent.Push<char>();
        *space = ' ';
        source_files.length += game_library.main.length + 1;
    }

    char* command = (char*)(transient.base + transient.used);
    String string = String::Format(transient,
        "{s} /std:c++20 /nologo /c /MP {s} {s}/Fo\"{s}\" "
        "/Yu\"pch.h\" /Fp\"bin\\{s}.pch\" /Fd\"bin\\{s}.pdb\" {s}",
        7,
        config.compiler_path,
        config.compiler_flags,
        source_files,
        config.obj_dir,
        config.pch_output_file,
        config.pch_output_file,
        config.include
    );
    
    uint64 start = Platform::GetWallClock();
    Process_Info process = Platform::RunCommand(command);
    uint32 exit_code = Platform::WaitForProcess(process, -1);
    uint64 end = Platform::GetWallClock();

    LogResult(transient, exit_code, "Source files compilation", start, end);
}

/* Compiles the meta-program (if needed) and runs it.*/
void Build::MetaProgram() {
    String meta_file_name = GetFileNameWithoutExtension(metaprogramming.main);
    String meta_binary = String::Format(transient, "bin\\{s}.exe", 1, meta_file_name);
    metaprogramming.update = NeedsUpdate(metaprogramming.main, meta_binary);

    if (metaprogramming.update) {
        Log::Info("Compiling metaprogramming code.");
        char* command = (char*)(transient.base + transient.used);
        String command_string = String::Format(transient,
            "{s} /std:c++20 /nologo {s} /Fo\"{s}{s}.obj\" /Fd\"bin\\{s}.pdb\" {s} {s} "
            "/link {s} {s} /OUT:\"{s}\" /PDB:\"bin\\{s}.pdb\"", 
            11,
            config.compiler_path,
            config.include,
            config.obj_dir,
            meta_file_name,
            meta_file_name,
            metaprogramming.main,
            config.compiler_flags,
            config.lib,
            config.obj_files,
            meta_binary,
            meta_file_name
        );

        uint64 start = Platform::GetWallClock();
        Process_Info process = Platform::RunCommand(command);
        uint32 exit_code = Platform::WaitForProcess(process, -1);
        uint64 end = Platform::GetWallClock();

        LogResult(transient, exit_code, "Metaprogramming compilation", start, end);
    }

    char* meta_command = (char*)(transient.base + transient.used);
    String meta_execution = String::Copy(transient, meta_binary);

    uint64 start = Platform::GetWallClock();
    Process_Info process = Platform::RunCommand(meta_command);
    uint32 exit_code = Platform::WaitForProcess(process, -1);
    uint64 end = Platform::GetWallClock();

    LogResult(transient, exit_code, "Metaprogramming execution", start, end);
    transient.Clear();
}

// void Build::CompilePlatformLayer(Arena& arena) {
//     char* command = (char*)(arena.base + arena.used);
//     switch (config.compiler) {
//         case config.MSVC: {
//             String command_string = String::Format(arena,
//                 "{s} /std:c++20 /nologo "
//                 "src\\GamePlatform\\Windows\\Win32PlatformLayer.cpp build\\obj\\{s}.obj " 
//                 "{s} "
//                 "/Fe\"bin\\RunGame.exe\" "
//                 "/Fo\"build\\obj\\Win32PlatformLayer.obj\" "
//                 "/Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" {s} "
//                 "/link {s} {s} {s} "
//                 "src\\GamePlatform\\Windows\\Win32PlatformLayer.res /MACHINE:X64",
//                 10,
//                 config->compiler_path,
//                 config->pch_output_file,
//                 config->compiler_flags,
//                 config->pch_output_file,
//                 config->pch_output_file,
//                 config->include,
//                 config->lib,
//                 config->linked_libs,
//                 config->renderer_libs
//             );
//         } break;

//         case clang: {
//             String::Format(arena,
//                 "{s} -std=c++20 -fPIC {s} {s} -include-pch bin/{s}.gch "
//                 "LinuxPlatformLayer/LinuxPlatformLayer.cpp {s} -o bin/RunGame && chmod +x bin/RunGame",
//                 5,
//                 config->compiler_path,
//                 config->compiler_flags,
//                 config->include,
//                 config->pch_output_file,
//                 config->renderer_libs
//             );
//         };
//     }

//     return Platform::RunCommand(command);
// }

// Process_Info CompileGameLibrary(Arena& arena) {
//     char* Command = (char*)(arena.base + arena.used);
//     switch(config->compiler) {
//         case MSVC: {
//             String::Format(arena,
//                 "{s} /std:c++20 /nologo /D GAMELIBRARY_EXPORTS "
//                 "src\\GameLibrary\\GameLibrary.cpp {s} {s} /Fo\"build\\obj\\GameLibrary.obj\" "
//                 "/Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" "
//                 "/link {s} build\\obj\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
//                 "/PDB:\"bin\\GameLibrary.pdb\" "
//                 "/ILK:\"bin\\GameLibrary.ilk\" /OUT:\"bin\\GameLibrary.dll\"",
//                 7,
//                 config->compiler_path, 
//                 config->compiler_flags, 
//                 config->include, 
//                 config->pch_output_file, 
//                 config->pch_output_file, 
//                 config->lib, 
//                 config->pch_output_file
//             );
//         } break;

//         case clang: {
//             String::Format(arena,
//                 "{s} -std=c++20 -shared -fPIC {s} {s} -include-pch bin/{s}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
//                 4,
//                 config->compiler_path,
//                 config->compiler_flags,
//                 config->include,
//                 config->pch_output_file
//             );
//         } break;
//     }

//     return Platform::RunCommand(Command);
// }

// void Build::HotReload() {
//     game_library.start = Platform::GetWallClock();
// #if _WIN32
//     int hot_reload_count = 0;
//     String pdb_file = String::Format(transient, "bin\\GameLibrary{i}.pdb", 1, hot_reload_count);
//     bool exists;
//     do {
//         exists = Platform::FileExists(pdb_file.content);
//         if (exists) {
//             hot_reload_count += 1;
//             transient.PopArray<char>(pdb_file.length + 1);
//             pdb_file = String::Format(transient, "bin\\GameLibrary{i}.pdb", 1, hot_reload_count);
//         }
//     } while(exists);

//     char* command = (char*)(transient.base + transient.used);
//     String::Format(transient,
//         "{s} /std:c++20 /nologo /D GAMELIBRARY_EXPORTS src\\GameLibrary\\GameLibrary.cpp {s} {s} "
//         "/Fo\"build\\obj\\GameLibrary.obj\" /Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" "
//         "/link {s} build\\obj\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
//         "/PDB:\"{s}\" /ILK:\"bin\\GameLibrary.ilk\" "
//         "/OUT:\"bin\\GameLibrary.dll\"",
//         8,
//         config.compiler_path, 
//         config.compiler_flags, 
//         config.include, 
//         config.pch_output_file, 
//         config.pch_output_file, 
//         config.lib,
//         config.pch_output_file, 
//         pdb_file
//     );
// #else
//     char* Command = (char*)(arena->base + arena->used);
//     Format(arena,
//         "{s} -std=c++20 -shared -fPIC {s} {s} -include-pch bin/{s}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
//         4,
//         config->compiler_path,
//         config->compiler_flags,
//         config->include,
//         config->pch_output
//     );
// #endif
//     game_library.info = Platform::RunCommand(command);
//     game_library.result = Platform::WaitForProcess(game_library.info, -1);
//     game_library.end = Platform::GetWallClock();
//     LogResult(game_library);
// }
