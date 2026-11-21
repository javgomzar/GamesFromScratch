#include "pch.h"
#include "build.h"


Build::Config Build::config;

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
                        Build::config.mode = Build::Config::Mode::Debug;
                        Build::config.obj_dir = "build" PATH_SEPARATOR "debug/";
                    }
                    else {
                        Build::config.mode = Build::Config::Mode::Release;
                        Build::config.obj_dir = "build" PATH_SEPARATOR "release/";
                    }
                }
    
                else if (token == "COMPILER") {
                    RequireToken(tokenizer, Token_Equal);
                    Token compiler_token = RequireToken(tokenizer, Token_Identifier);
                    if (compiler_token == "MSVC")       Build::config.compiler = Build::Config::Compiler::MSVC;
                    else if (compiler_token == "clang") Build::config.compiler = Build::Config::Compiler::clang;
                    else Log::Fatal("Invalid compiler. Currently only MSVC and clang are supported.");
                }
    
                else if (token == "COMPILER_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.compiler_path = ParsePath(tokenizer);
                }

                else if (token == "LINKER_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.linker_path = ParsePath(tokenizer);
                }

                else if (token == "PRECOMPILE_HEADERS") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.pch = ParseBool(tokenizer);
                }

                else if (token == "PRECOMPILED_HEADERS_PATH") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.pch_source_dir = ParsePath(tokenizer);
                }
    
                else if (token == "INCLUDE") {
                    RequireToken(tokenizer, Token_Equal);
                    String_Builder builder;
                    String flag;
                    if (Build::config.compiler == Build::Config::Compiler::MSVC) {
                        flag = "/I";
                    }
                    else {
                        flag = "-I";
                    }
                    do {
                        String path = ParsePath(tokenizer);
                        builder.Append(flag);
                        builder.Append("\"");
                        builder.Append(path);
                        builder.Append("\"");
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        RequireToken(tokenizer, Token_Semicolon);
                        builder.Append(" ");
                    } while (true);
                    Build::config.include = builder.Build(Memory::permanent);
                }
    
                else if (token == "LIB") {
                    RequireToken(tokenizer, Token_Equal);
                    String_Builder builder;
                    do {
                        builder.Append("/LIBPATH:\"");
                        String path = ParsePath(tokenizer);
                        builder.Append(path);
                        builder.Append("\"");
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        builder.Append(" ");
                        RequireToken(tokenizer, Token_Semicolon);
                    } while (true);
                    Build::config.lib = builder.Build(Memory::permanent);
                }

                // Linked libraries
                else if (token == "LINK") {
                    RequireToken(tokenizer, Token_Equal);
                    String_Builder builder;
                    do {
                        String library = ParsePath(tokenizer);
                        builder.Append(library);
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        RequireToken(tokenizer, Token_Semicolon);
                        builder.Append(" ");
                    } while(true);
                    Build::config.linked_libs = builder.Build(Memory::permanent);
                }
    
                // Renderer
                else if (token == "RENDERER") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.renderer_source = ParsePath(tokenizer);
                    String file_name = GetFileNameWithoutExtension(Build::config.renderer_source);
                    Build::config.renderer_obj = String::Format(Memory::permanent, "{s}{s}.obj", 2, Build::config.obj_dir, file_name);
                }

                // Source files
                else if (token == "SOURCE") {
                    RequireToken(tokenizer, Token_Equal);
                    tokenizer.AdvanceWhitespaceAndComments();
                    String_Builder source_files;
                    String_Builder obj_files;

                    do {
                        String source_file = ParsePath(tokenizer);
                        String file_name = GetFileNameWithoutExtension(source_file);
                        String obj_file = String::Format(Memory::transient, "{s}{s}.obj", 2, Build::config.obj_dir, file_name);

                        bool update = NeedsUpdate(source_file, obj_file);
                        if (update) {
                            source_files.Append(source_file);
                        }
                        obj_files.Append(obj_file);

                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                        if (update) {
                            source_files.Append(" ");
                        }
                        obj_files.Append(" ");
                    } while(token.type == Token_Semicolon);

                    Build::config.source_files = source_files.Build(Memory::permanent);
                    Build::config.obj_files = obj_files.Build(Memory::permanent);

                    Memory::transient.Clear();
                }

                // Metaprogramming source file
                else if (token == "META") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.preprocess = true;
                    Build::config.metaprogramming_main = ParsePath(tokenizer);
                }

                else if (token == "META_SOURCE") {
                    RequireToken(tokenizer, Token_Equal);
                    tokenizer.AdvanceWhitespaceAndComments();
                    String_Builder meta_source_files;

                    do {
                        String source_file = ParsePath(tokenizer);
                        meta_source_files.Append(source_file);
                        if (tokenizer.at[0] != ';') {
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                        meta_source_files.Append(" ");
                    } while(token.type == Token_Semicolon);

                    Build::config.metaprogramming_source = meta_source_files.Build(Memory::permanent);
                }

                // Metaprogramming input files
                else if (token == "META_FILES") {
                    RequireToken(tokenizer, Token_Equal);
                    tokenizer.AdvanceWhitespaceAndComments();
                    Build::config.metaprogramming_input_files.content = tokenizer.at;
                    Build::config.metaprogramming_input_files.length = 0;
                    Build::config.metaprogramming_input_file_count = 0;
                    do {
                        String path = ParsePath(tokenizer);
                        Build::config.metaprogramming_input_file_count += 1;
                        if (tokenizer.at[0] != ';') {
                            const char* end = tokenizer.at;
                            Build::config.metaprogramming_input_files.length = end - Build::config.metaprogramming_input_files.content;
                            break;
                        }
                        token = RequireToken(tokenizer, Token_Semicolon);
                    } while(true);
                }

                // Platform layer main
                else if (token == "PLATFORM") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.platform_main = ParsePath(tokenizer);
                    String platform_main_file_name = GetFileNameWithoutExtension(Build::config.platform_main);
                    Build::config.platform_obj = String::Format(Memory::permanent, "{s}{s}.obj", 2, Build::config.obj_dir, platform_main_file_name);
                }

                // Game library
                else if (token == "GAME") {
                    RequireToken(tokenizer, Token_Equal);
                    Build::config.game_library_main = ParsePath(tokenizer);
                    String game_library_main_file_name = GetFileNameWithoutExtension(Build::config.game_library_main);
                    Build::config.game_library_obj = String::Format(Memory::permanent, "{s}{s}.obj", 2, Build::config.obj_dir, game_library_main_file_name);
                }
            }

            token = tokenizer.GetToken();
        }

        switch (Build::config.compiler) {
            case Build::Config::Compiler::MSVC: {
                switch(Build::config.mode) {
                    case Build::Config::Mode::Release: Build::config.compiler_flags = "/O2";
                    case Build::Config::Mode::Debug:   Build::config.compiler_flags = "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
                }
            } break;

            case Build::Config::Compiler::clang: {
                switch(Build::config.mode) {
                    case Build::Config::Mode::Release: Build::config.compiler_flags = "-O2";
                    case Build::Config::Mode::Debug:   Build::config.compiler_flags = "-g -D_DEBUG -fsanitize=address -fsanitize=undefined";
                }
            } break;

            default: Log::Fatal("Invalid compiler. Only MSVC and clang are supported.");
        }
        
        if (Platform::OS == Platform::Windows) {
            Build::config.pch_header_path = String::Format(Memory::permanent, "{s}" PATH_SEPARATOR "pch.h", 1, Build::config.pch_source_dir);
            Build::config.pch_source_path = String::Format(Memory::permanent, "{s}" PATH_SEPARATOR "pch.cpp", 1, Build::config.pch_source_dir);
        }
        else {
            // TODO
        }
    }
    else {
        String error = String::Format(Memory::permanent, "Build configuration file {s} not found.", 1, config_file_path);
        Log::Fatal(error);
    }
}

bool RunAndTime(char* command, String text) {
    uint64 start = Platform::GetWallClock();
    Process_Info process = Platform::RunCommand(command);
    int32 exit_code = Platform::WaitForProcess(process, -1);
    uint64 end = Platform::GetWallClock();
    bool success = exit_code == 0;
    if (success) {
        String message = String::Format(Memory::transient,
            "{s} done in {f2} milliseconds.", 2, text, 1000.0f * Platform::GetSecondsElapsed(start, end));
        Log::Info(message);
    }
    else {
        String message = String::Format(Memory::transient, "{s} failed, exit code '{i}'.", 2, text, exit_code);
        Log::Error(message);
    }
    return success;
}

bool Build::PrecompileHeaders() {
    Log::Info("Precompiling headers.");
    char* command = (char*)(Memory::permanent.base + Memory::permanent.used);
    switch (Build::config.compiler) {
        case Build::Config::Compiler::MSVC: {
            String::Format(Memory::permanent,
                "{s} /std:c++20 /nologo {s} {s} /c {s} "
                "/Yc\"pch.h\" /Fp\"{s}pch.pch\" /Fo\"{s}pch.obj\" /Fd\"{s}pch.pdb\"",
                7,
                Build::config.compiler_path,
                Build::config.include,
                Build::config.pch_source_path,
                Build::config.compiler_flags,
                Build::config.obj_dir,
                Build::config.obj_dir,
                Build::config.obj_dir
            );
        } break;

        default: Log::Fatal("Invalid compiler. Currently only MSVC and clang are supported.");
    }

    return RunAndTime(command, "Headers precompilation");
}

bool Build::MetaProgram() {
    bool success = false;
    if (NeedsUpdate(Build::config.metaprogramming_main, Build::config.metaprogramming_binary)) {
        Log::Info("Compiling metaprogramming code.");
        char* command = (char*)(Memory::permanent.base + Memory::permanent.used);
        String command_string = String::Format(Memory::permanent,
            "{s} /std:c++20 /nologo {s} /Fo\"{s}\" /Fd\"bin\\meta.pdb\" /MP {s} {s} {s} "
            "/link {s} {s} /OUT:\"bin\\meta.exe\" /PDB:\"bin\\meta.pdb\"", 
            8,
            Build::config.compiler_path,
            Build::config.include,
            Build::config.obj_dir,
            Build::config.metaprogramming_main,
            Build::config.metaprogramming_source,
            Build::config.compiler_flags,
            Build::config.lib,
            Build::config.linked_libs
        );

        success = RunAndTime(command, "Metaprogramming compilation");
        if (!success) {
            return false;
        }
    }

    Log::Info("Running metaprogramming.");
    char* command = (char*)(Memory::permanent.base + Memory::permanent.used);
    String command_string = String::Copy(Memory::permanent, Build::config.metaprogramming_binary);
    return RunAndTime(command, "Metaprogramming execution");
}

bool Build::Compile() {
    bool compile = false;
    String_Builder source_files_builder;

    if (Build::config.source_files.length > 0) {
        source_files_builder.Append(Build::config.source_files);
        source_files_builder.Append(" ");
        compile = true;
    }

    if (NeedsUpdate(Build::config.platform_main, Build::config.platform_obj)) {
        source_files_builder.Append(Build::config.platform_main);
        source_files_builder.Append(" ");
        compile = true;
    }

    if (NeedsUpdate(Build::config.game_library_main, Build::config.game_library_obj)) {
        source_files_builder.Append(Build::config.game_library_main);
        source_files_builder.Append(" ");
        compile = true;
    }

    if (NeedsUpdate(Build::config.renderer_source, Build::config.renderer_obj)) {
        source_files_builder.Append(Build::config.renderer_source);
        source_files_builder.Append(" ");
        compile = true;
    }

    if (compile) {
        String source_files = source_files_builder.Build(Memory::permanent);
        Log::Info("Compiling source files.");
        char* command = (char*)(Memory::permanent.base + Memory::permanent.used);
        String string = String::Format(Memory::permanent,
            "{s} /std:c++20 /nologo /c /MP {s} {s}/Fo\"{s}\" "
            "/Yu\"pch.h\" /Fp\"{s}pch.pch\" /Fd\"{s}pch.pdb\" {s}",
            7,
            Build::config.compiler_path,
            Build::config.compiler_flags,
            source_files,
            Build::config.obj_dir,
            Build::config.obj_dir,
            Build::config.obj_dir,
            Build::config.include
        );
        
       return RunAndTime(command, "Source files compilation");
    }

    return true;
}

bool Build::LinkPlatform() {
    Log::Info("Linking platform layer.");
    
    char* command = nullptr;
    switch (Build::config.compiler) {
        case Build::Config::Compiler::MSVC: {
            command = (char*)(Memory::permanent.base + Memory::permanent.used);
            String linker_flags = Build::config.mode == Build::Config::Mode::Debug ? "/DEBUG" : "";
            String command_string = String::Format(Memory::permanent,
                "{s} /NOLOGO {s}pch.obj {s} {s} {s} {s} {s} {s} "
                "src\\platform\\windows\\Win32PlatformLayer.res /MACHINE:X64 /PDB:{s}run_game.pdb /OUT:bin\\run_game.exe",
                8,
                Build::config.linker_path,
                Build::config.obj_dir,
                Build::config.obj_files,
                Build::config.renderer_obj,
                Build::config.platform_obj,
                Build::config.linked_libs,
                Build::config.lib,
                linker_flags,
                Build::config.obj_dir
            );

            return RunAndTime(command, "Platform layer linking");
        } break;

        case Build::Config::Compiler::clang: {
            Log::Fatal("Not implemented.");
        } break;
    }
    return false;
}

bool Build::LinkGameLibrary() {
    Log::Info("Linking game library.");
    
    char* command = nullptr;
    switch (Build::config.compiler) {
        case Build::Config::Compiler::MSVC: {
            command = (char*)(Memory::permanent.base + Memory::permanent.used);
            String linker_flags = Build::config.mode == Build::Config::Mode::Debug ? "/DEBUG" : "";
            String command_string = String::Format(Memory::permanent,
                "{s} /NOLOGO {s}pch.obj {s} {s} {s} {s} {s} "
                "/MACHINE:X64 /DLL /IMPLIB:bin\\game_library.lib /PDB:{s}game_library.pdb /OUT:bin\\game_library.dll",
                7,
                Build::config.linker_path,
                Build::config.obj_dir,
                Build::config.obj_files,
                Build::config.game_library_obj,
                Build::config.linked_libs,
                Build::config.lib,
                linker_flags,
                Build::config.obj_dir
            );

            return RunAndTime(command, "Game library linking");
        } break;

        case Build::Config::Compiler::clang: {
            Log::Fatal("Not implemented.");
        } break;
    }
    return false;
}

// void Build::CompilePlatformLayer(Arena& arena) {
//     char* command = (char*)(arena.base + arena.used);
//     switch (Build::config.compiler) {
//         case Build::config.MSVC: {
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
    String pdb_file = String::Format(Memory::transient, "bin\\game_library{i}.pdb", 1, hot_reload_count);
    bool exists;
    do {
        exists = Platform::FileExists(pdb_file.content);
        if (exists) {
            hot_reload_count += 1;
            Memory::transient.PopArray<char>(pdb_file.length + 1);
            pdb_file = String::Format(Memory::transient, "bin\\game_library{i}.pdb", 1, hot_reload_count);
        }
    } while(exists);

    char* command = (char*)(Memory::permanent.base + Memory::permanent.used);
    String::Format(Memory::permanent,
        "{s} /std:c++20 /nologo /D GAMELIBRARY_EXPORTS {s} {s} {s} "
        "/Fo\"build\\obj\\GameLibrary.obj\" /Fd\"{s}pch.pdb\" /Yu\"pch.h\" /Fp\"{s}pch.pch\" "
        "/link {s} build\\obj\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
        "/PDB:\"{s}\" /ILK:\"bin\\GameLibrary.ilk\" "
        "/OUT:\"bin\\GameLibrary.dll\"",
        8,
        Build::config.compiler_path, 
        Build::config.compiler_flags, 
        Build::config.include, 
        Build::config.obj_dir, 
        Build::config.obj_dir, 
        Build::config.lib,
        Build::config.obj_dir, 
        pdb_file
    );

    return RunAndTime(command, "Hot reloading");
}
