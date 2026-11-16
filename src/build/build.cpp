#include "pch.h"
#include "build.h"


Build::Build(memory_size arena_size) : 
    permanent(arena_size), transient(arena_size)
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

                else if (token == "LINKER_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    config.linker_path.content = tokenizer.at;
                    config.linker_path.length = ParsePath(tokenizer.at);
                    tokenizer.Advance(config.linker_path.length);
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
                    String_Builder source_files;
                    String_Builder obj_files;

                    do {
                        tokenizer.AdvanceWhitespaceAndComments();
                        int path_length = ParsePath(tokenizer.at);
                        String source_file = String(tokenizer.at, path_length);
                        String file_name = GetFileNameWithoutExtension(source_file);
                        String obj_file = String::Format(transient, "{s}{s}.obj", 2, config.obj_dir, file_name);

                        bool update = NeedsUpdate(source_file, obj_file);
                        if (update) {
                            source_files.Append(source_file);
                        }
                        obj_files.Append(obj_file);

                        tokenizer.Advance(path_length);
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                        if (update) {
                            source_files.Append(" ");
                        }
                        obj_files.Append(" ");
                    } while(token.type == Token_Semicolon);

                    config.source_files = source_files.Build(permanent);
                    config.obj_files = obj_files.Build(permanent);

                    transient.Clear();
                }

                // Metaprogramming source file
                else if (token == "META") {
                    RequireToken(tokenizer, Token_Equal);
                    config.preprocess = true;
                    int metaprogramming_file_path_length = ParsePath(tokenizer.at);
                    char* pointer = permanent.PushArray<char>(metaprogramming_file_path_length + 1);
                    strncpy(pointer, tokenizer.at, metaprogramming_file_path_length);
                    config.metaprogramming_main = String(pointer, metaprogramming_file_path_length);
                    tokenizer.AdvanceUntilNextLine();
                }

                else if (token == "META_SOURCE") {
                    RequireToken(tokenizer, Token_Equal);
                    tokenizer.AdvanceWhitespaceAndComments();
                    String_Builder meta_source_files;

                    do {
                        tokenizer.AdvanceWhitespaceAndComments();
                        int path_length = ParsePath(tokenizer.at);
                        String source_file = String(tokenizer.at, path_length);
                        meta_source_files.Append(source_file);

                        tokenizer.Advance(path_length);
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                        meta_source_files.Append(" ");
                    } while(token.type == Token_Semicolon);

                    config.metaprogramming_source = meta_source_files.Build(permanent);
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
                    config.platform_main = String(tokenizer.at, path_length);
                    tokenizer.Advance(path_length);

                    String platform_main_file_name = GetFileNameWithoutExtension(config.platform_main);
                    config.platform_obj = String::Format(permanent, "{s}{s}.obj", 2, config.obj_dir, platform_main_file_name);
                }

                // Game library
                else if (token == "GAME") {
                    RequireToken(tokenizer, Token_Equal);
                    int path_length = ParsePath(tokenizer.at);
                    config.game_library_main = String(tokenizer.at, path_length);
                    tokenizer.Advance(path_length);

                    String game_library_main_file_name = GetFileNameWithoutExtension(config.game_library_main);
                    config.game_library_obj = String::Format(permanent, "{s}{s}.obj", 2, config.obj_dir, game_library_main_file_name);
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
        
        if (Platform::OS == Platform::Windows) {
            config.pch_header_path = String::Format(permanent, "{s}" PATH_SEPARATOR "pch.h", 1, config.pch_source_dir);
            config.pch_source_path = String::Format(permanent, "{s}" PATH_SEPARATOR "pch.cpp", 1, config.pch_source_dir);
        }
        else {
            // TODO
        }
    }
    else {
        String error = String::Format(permanent, "Build configuration file {s} not found.", 1, config_file_path);
        Log::Fatal(error);
    }
}

bool RunAndTime(Arena& transient, char* command, String text) {
    uint64 start = Platform::GetWallClock();
    Process_Info process = Platform::RunCommand(command);
    int32 exit_code = Platform::WaitForProcess(process, -1);
    uint64 end = Platform::GetWallClock();
    bool success = exit_code == 0;
    if (success) {
        String message = String::Format(transient,
            "{s} done in {f2} milliseconds.", 2, text, 1000.0f * Platform::GetSecondsElapsed(start, end));
        Log::Info(message);
    }
    else {
        String message = String::Format(transient, "{s} failed, exit code '{i}'.", 2, text, exit_code);
        Log::Error(message);
    }
    return success;
}

bool Build::PrecompileHeaders() {
    Log::Info("Precompiling headers.");
    char* command = (char*)(permanent.base + permanent.used);
    switch (config.compiler) {
        case Build_Config::Compiler::MSVC: {
            String::Format(permanent,
                "{s} /std:c++20 /nologo {s} {s} /c {s} "
                "/Yc\"pch.h\" /Fp\"{s}pch.pch\" /Fo\"{s}pch.obj\" /Fd\"{s}pch.pdb\"",
                7,
                config.compiler_path,
                config.include,
                config.pch_source_path,
                config.compiler_flags,
                config.obj_dir,
                config.obj_dir,
                config.obj_dir
            );
        } break;

        default: Log::Fatal("Invalid compiler. Currently only MSVC and clang are supported.");
    }

    return RunAndTime(transient, command, "Headers precompilation");
}

bool Build::MetaProgram() {
    bool success = false;
    if (NeedsUpdate(config.metaprogramming_main, config.metaprogramming_binary)) {
        Log::Info("Compiling metaprogramming code.");
        char* command = (char*)(permanent.base + permanent.used);
        String command_string = String::Format(permanent,
            "{s} /std:c++20 /nologo {s} /Fo\"{s}\" /Fd\"bin\\meta.pdb\" /MP {s} {s} {s} "
            "/link {s} {s} /OUT:\"bin\\meta.exe\" /PDB:\"bin\\meta.pdb\"", 
            8,
            config.compiler_path,
            config.include,
            config.obj_dir,
            config.metaprogramming_main,
            config.metaprogramming_source,
            config.compiler_flags,
            config.lib,
            config.linked_libs
        );

        success = RunAndTime(transient, command, "Metaprogramming compilation");
        if (!success) {
            return false;
        }
    }

    char* command = (char*)(permanent.base + permanent.used);
    String command_string = String::Copy(permanent, config.metaprogramming_binary);
    return RunAndTime(transient, command, "Metaprogramming execution");
}

bool Build::Compile() {
    bool compile = false;
    String_Builder source_files_builder;

    if (config.source_files.length > 0) {
        source_files_builder.Append(config.source_files);
        source_files_builder.Append(" ");
        compile = true;
    }

    if (NeedsUpdate(config.platform_main, config.platform_obj)) {
        source_files_builder.Append(config.platform_main);
        source_files_builder.Append(" ");
        compile = true;
    }

    if (NeedsUpdate(config.game_library_main, config.game_library_obj)) {
        source_files_builder.Append(config.game_library_main);
        source_files_builder.Append(" ");
        compile = true;
    }

    if (compile) {
        String source_files = source_files_builder.Build(permanent);
        Log::Info("Compiling source files.");
        char* command = (char*)(permanent.base + permanent.used);
        String string = String::Format(permanent,
            "{s} /std:c++20 /nologo /c /MP {s} {s}/Fo\"{s}\" "
            "/Yu\"pch.h\" /Fp\"{s}pch.pch\" /Fd\"{s}pch.pdb\" {s}",
            7,
            config.compiler_path,
            config.compiler_flags,
            source_files,
            config.obj_dir,
            config.obj_dir,
            config.obj_dir,
            config.include
        );
        
       return RunAndTime(transient, command, "Source files compilation");
    }

    return true;
}

bool Build::LinkPlatform() {
    Log::Info("Linking platform layer");
    
    char* command = nullptr;
    switch (config.compiler) {
        case Build_Config::Compiler::MSVC: {
            command = (char*)(permanent.base + permanent.used);
            String linker_flags = config.mode == Build_Config::Mode::Debug ? "/DEBUG" : "";
            String command_string = String::Format(permanent,
                "{s} /NOLOGO {s}pch.obj {s} {s} {s} {s} {s} "
                "src\\platform\\windows\\Win32PlatformLayer.res /MACHINE:X64 /PDB:{s}run_game.pdb /OUT:run_game.exe",
                7,
                config.linker_path,
                config.obj_dir,
                config.obj_files,
                config.platform_obj,
                config.linked_libs,
                config.lib,
                linker_flags,
                config.obj_dir
            );

            return RunAndTime(transient, command, "Platform layer linking");
        } break;

        case Build_Config::Compiler::clang: {
            Log::Fatal("Not implemented.");
        } break;
    }
    return false;
}

bool Build::LinkGameLibrary() {
    return false;
}

// void Build::CompilePlatformLayer(Arena& arena) {
//     char* command = (char*)(arena.base + arena.used);
//     switch (config.compiler) {
//         case config.MSVC: {
//             
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

bool Build::HotReload() {
    int hot_reload_count = 0;
    String pdb_file = String::Format(transient, "bin\\game_library{i}.pdb", 1, hot_reload_count);
    bool exists;
    do {
        exists = Platform::FileExists(pdb_file.content);
        if (exists) {
            hot_reload_count += 1;
            transient.PopArray<char>(pdb_file.length + 1);
            pdb_file = String::Format(transient, "bin\\game_library{i}.pdb", 1, hot_reload_count);
        }
    } while(exists);

    char* command = (char*)(permanent.base + permanent.used);
    String::Format(permanent,
        "{s} /std:c++20 /nologo /D GAMELIBRARY_EXPORTS {s} {s} {s} "
        "/Fo\"build\\obj\\GameLibrary.obj\" /Fd\"{s}pch.pdb\" /Yu\"pch.h\" /Fp\"{s}pch.pch\" "
        "/link {s} build\\obj\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
        "/PDB:\"{s}\" /ILK:\"bin\\GameLibrary.ilk\" "
        "/OUT:\"bin\\GameLibrary.dll\"",
        8,
        config.compiler_path, 
        config.compiler_flags, 
        config.include, 
        config.obj_dir, 
        config.obj_dir, 
        config.lib,
        config.obj_dir, 
        pdb_file
    );

    return RunAndTime(transient, command, "Hot reloading");
}
