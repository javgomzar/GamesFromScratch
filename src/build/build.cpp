#include "build.h"


Build::Build(memory_size arena_size, String config_file_path) : permanent(arena_size), transient(arena_size) {
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
                        config.mode = Mode::Debug;
                        config.obj_dir = "build" PATH_SEPARATOR "debug" PATH_SEPARATOR;
                    }
                    else {
                        config.mode = Mode::Release;
                        config.obj_dir = "build" PATH_SEPARATOR "release" PATH_SEPARATOR;
                    }
                }
    
                else if (token == "COMPILER") {
                    RequireToken(tokenizer, Token_Equal);
                    Token compiler_token = RequireToken(tokenizer, Token_Identifier);
                    if (compiler_token == "MSVC")       config.compiler = Compiler::MSVC;
                    else if (compiler_token == "clang") config.compiler = Compiler::clang;
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
                    config.pch_source_dir.content = tokenizer.at;
                    config.pch_source_dir.length = ParsePath(tokenizer.at);
                    tokenizer.Advance(config.pch_source_dir.length);
                }
    
                else if (token == "INCLUDE") {
                    RequireToken(tokenizer, Token_Equal);

                    config.include.length = 0;
                    config.include.content = (const char*)(permanent.base + permanent.used);

                    do {
                        tokenizer.AdvanceUntilNextLine();
                        char* pointer = permanent.PushArray<char>(3);
                        config.include.length += 3;
                        if (config.compiler == Compiler::MSVC) {
                            *pointer++ = '/';
                        }
                        else if (config.compiler == Compiler::clang) {
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
                    if      (renderer_token == "Vulkan")  config.renderer = Renderer::Vulkan;
                    else if (renderer_token == "DirectX") config.renderer = Renderer::DirectX;
                    else if (renderer_token == "OpenGL")  config.renderer = Renderer::OpenGL;
                    else Log::Fatal("Invalid renderer. Currently only OpenGL, DirectX or Vulkan are supported.");

                    if (Platform::OS == Platform::Windows) {
                        switch(config.renderer) {
                            case Renderer::OpenGL:  config.renderer_libs = "glew32.lib";
                            case Renderer::DirectX: config.renderer_libs = "D3d11.lib d3dcompiler.lib";
                            case Renderer::Vulkan:  config.renderer_libs = "vulkan-1.lib shaderc_combined.lib";
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
                    char* start = (char*)(permanent.base + permanent.used);
                    memory_size length = 0;
                    do {
                        tokenizer.AdvanceWhitespaceAndComments();
                        int path_length = ParsePath(tokenizer.at);
                        String source_file = String(tokenizer.at, path_length);

                        bool add = true;
                        if (Platform::FileExists(path)) {
                            String file_name = GetFileNameWithoutExtension(path);
                            String obj_file = String::Format(transient, "{s}{s}.obj", 2, config.obj_dir, file_name);
                            if (Platform::FileExists(obj_file)) {
                                File_Info obj_info = Platform::GetFileInfo(obj_file);
                                File_Info source_info = Platform::GetFileInfo(path);
                                add = obj_info.timestamp < source_info.timestamp;
                            }
                            
                            if (add) {
                                String::Copy(permanent, obj_file);
                                length += obj_file.length;
                            }
                        }
                        else {
                            String error_message = String::Format(transient, "Failed to find source file {s}.", 1, path);
                            Log::Warn(error_message);
                        }

                        tokenizer.Advance(path_length);
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                        if (add) {
                            char* space = permanent.Push<char>();
                            *space = ' ';
                        }
                    } while(token.type == Token_Semicolon);

                    config.obj_files = String(start, length);

                    transient.Clear();
                }

                // Metaprogramming source file
                else if (token == "META") {
                    RequireToken(tokenizer, Token_Equal);
                    config.preprocess = true;
                    int metaprogramming_file_path_length = ParsePath(tokenizer.at);
                    char* pointer = permanent.PushArray<char>(metaprogramming_file_path_length + 1);
                    strncpy(pointer, tokenizer.at, metaprogramming_file_path_length);
                    config.metaprogramming_code_path.length = metaprogramming_file_path_length;
                    config.metaprogramming_code_path.content = pointer;
                    metaprogramming.output_mode = Build_Process::Output_Mode::EXE;
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
                    platform.output_mode = Build_Process::Output_Mode::EXE;
                    tokenizer.Advance(path_length);
                }

                // Game library
                else if (token == "GAME") {
                    RequireToken(tokenizer, Token_Equal);
                    int path_length = ParsePath(tokenizer.at);
                    game_library.main = String(tokenizer.at, path_length);
                    game_library.output_mode = Build_Process::Output_Mode::DLL;
                    tokenizer.Advance(path_length);
                }
            }
    
            token = tokenizer.GetToken();
        }

        switch (config.compiler) {
            case Compiler::MSVC: {
                switch(config.mode) {
                    case Mode::Release: config.compiler_flags = "/O2";
                    case Mode::Debug:   config.compiler_flags = "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
                }
            } break;

            case Compiler::clang: {
                switch(config.mode) {
                    case Mode::Release: config.compiler_flags = "-O2";
                    case Mode::Debug:   config.compiler_flags = "-g -D_DEBUG -fsanitize=address -fsanitize=undefined";
                }
            } break;

            default: Log::Fatal("Invalid compiler. Only MSVC and clang are supported.");
        }
        
        config.pch_output_file = config.mode == Mode::Debug ? "debug_pch" : "pch";
        if (Platform::OS == Platform::Windows) {
            config.pch_output_path = String::Format(permanent, "bin" PATH_SEPARATOR "{s}.pch", 1, config.pch_output_file);
            config.metaprogramming_output_file = "bin" PATH_SEPARATOR "Meta.exe";
            config.pch_header_path = String::Format(permanent, "{s}" PATH_SEPARATOR "pch.h", 1, config.pch_source_dir);
            config.pch_source_path = String::Format(permanent, "{s}" PATH_SEPARATOR "pch.cpp", 1, config.pch_source_dir);
        }
        else {
            config.pch_output_path = String::Format(permanent, "bin" PATH_SEPARATOR "{s}.gch", 1, config.pch_output_file);
            config.metaprogramming_output_file = "bin" PATH_SEPARATOR "Meta";
        }
    }
    else {
        String error = String::Format(permanent, "Build configuration file {s} not found.", 1, config_file_path);
        Log::Fatal(error);
    }
}

void Build::Compile() {
    char* command = (char*)(transient.base + transient.used);
    String string = String::Format(transient, "{s} /std:c++20 /nologo /c /MP "
        "{s} /Fo\"{s}\" "
        "/Yu\"pch.h\" /Fp\"bin\\{s}.pch\" {s}",
        4,
        config.compiler_path,
        config.compiler_flags,
        config.obj_dir,
        config.pch_output_file,
        config.include
    );

    Process_Info process = Platform::RunCommand(command);
    int32 wait_result = Platform::WaitForProcess(process, -1);

    if (wait_result > 0) {
        Log::Error("Error");
    }

}

// /* Compiles the meta-program (if needed) and runs it.*/
// void Build::MetaProgram() {
//     File_Info metaprogramming_source = Platform::GetFileInfo(config.metaprogramming_code_path);
//     File_Info metaprogramming_binary;

//     bool metaprogramming_binary_exists = Platform::FileExists(config.metaprogramming_output_file);        
//     if (metaprogramming_binary_exists) {
//         metaprogramming_binary = Platform::GetFileInfo(config.metaprogramming_output_file);
//     }
    
//     if (!metaprogramming_binary_exists || metaprogramming_source.timestamp > metaprogramming_binary.timestamp) {
//         Build::Compile(metaprogramming);
//     }

//     char* meta_command = transient.PushArray<char>(64);
//     strncpy(meta_command, config.metaprogramming_output_file.content, 64);
//     uint64 metaprogramming_execution_start = Platform::GetWallClock();
//     Process_Info metaprogramming_execution = Platform::RunCommand(meta_command);
//     int32 wait_result = Platform::WaitForProcess(metaprogramming_execution, -1);
//     if (wait_result >= 0) {
//         uint64 end = Platform::GetWallClock();
//         float time = Platform::GetSecondsElapsed(metaprogramming_execution_start, end);
//         if (wait_result == 0) {
//             String log_text = String::Format(transient, "Metaprogramming executed in {f2} milliseconds.", 1, 1000.0f * time);
//             Log::Info(log_text);
//         }
//         else {
//             String log_text = String::Format(transient, "Metaprogramming execution failed with code '{i}'", 1, wait_result);
//             Log::Error(log_text);
//         }
//     }

//     transient.Clear();
// }

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

// Process_Info CompileMetaprogramming(Arena& arena, Build_Configuration* config) {
//     Log::Info("Compiling metaprogramming code.");
//     char* Command = (char*)(arena.base + arena.used);
//     switch(config->compiler) {
//         case MSVC: {
//             String::Format(arena,
//                 "{s} /std:c++20 /nologo {s} /Fo\"build\\obj\\meta.obj\" /Fd\"bin\\meta.pdb\" {s} {s} "
//                 "/link {s} /OUT:\"bin\\meta.exe\" /PDB:\"bin\\meta.pdb\"", 
//                 5,
//                 config->compiler_path, 
//                 config->include, 
//                 config->metaprogramming_code_path, 
//                 config->compiler_flags, 
//                 config->lib
//             );
//         } break;

//         case clang: {
//             String::Format(arena,
//                 "{s} -std=c++20 {s} {s} -o bin/Meta", 
//                 4,
//                 config->compiler_path, 
//                 config->include, 
//                 config->metaprogramming_code_path, 
//                 config->compiler_flags
//             );
//         } break;

//         default: Raise("Invalid compiler. Only MSVC and clang are supported for now.");
//     }
//     return Platform::RunCommand(Command);
// }

// void Build::LogResult(Build_Process& process) {
//     if (process.result == 0) {
//         String message = String::Format(Build::transient, 
//             "{s} code compiled in {f2} milliseconds.", 2, process.name, 1000.0f * Platform::GetSecondsElapsed(process.start, process.end));
//         Log::Info(message);
//     }
//     else {
//         String message = String::Format(Build::transient, "{s} code compilation failed, exit code '{i}'.", 2, process.name, process.result);
//         Log::Error(message);
//     }
// }

// void Build::Compile(Build_Process& process) {
//     char* command = (char*)(transient.base + transient.used);
//     String::Format(transient, 
//         "{s} /std:c++20 /nologo {s} {s} /Fo\"{s}\" /Fd\"bin\\{s}.pdb\" {s} "
//         "/link {s}",
//         0,
//         config.compiler_path,

//     );

//     process.start = Platform::GetWallClock();
//     process.info = Platform::RunCommand(command);
//     process.result = Platform::WaitForProcess(process.info, -1);
//     process.end = Platform::GetWallClock();

//     LogResult(process);
//     transient.Clear();
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
