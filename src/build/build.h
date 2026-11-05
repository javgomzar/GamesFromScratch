#ifndef GAME_BUILD_H
#define GAME_BUILD_H

#include "GamePlatform.h"
#include "Tokenizer.h"
#include "parse.h"


enum Build_Mode {
    Release,
    Debug,
};

inline Build_Mode GetBuildMode(Token token) {
    if (token == "DEBUG") return Debug;
    else                  return Release;
}

enum Compiler {
    MSVC,
    clang
};

inline Compiler GetCompiler(Token token) {
    if (token == "MSVC") return MSVC;
    if (token == "clang") return clang;
    
    Raise("Invalid compiler. Currently only MSVC and clang are supported.");
    return MSVC;
}

enum class Renderer {
    OpenGL,
    DirectX,
    Vulkan,
};

inline Renderer GetRenderer(Token token) {
    if      (token == "Vulkan")  return Renderer::Vulkan;
    else if (token == "DirectX") return Renderer::DirectX;
    else if (token != "OpenGL")  Raise("Invalid renderer. Currently only OpenGL, DirectX or Vulkan are supported.");
    return Renderer::OpenGL;
}

inline String GetRendererString(Renderer renderer) {
    switch(renderer) {
        case Renderer::OpenGL:  return "OPENGL";
        case Renderer::DirectX: return "DIRECTX";
        case Renderer::Vulkan:  return "VULKAN";
    }
    return "";
}

inline const char* GetRendererLibs(Renderer renderer) {
    if (SystemOS == Windows) {
        switch(renderer) {
            case Renderer::OpenGL:  return "glew32.lib";
            case Renderer::DirectX: return "D3d11.lib d3dcompiler.lib";
            case Renderer::Vulkan:  return "vulkan-1.lib shaderc_combined.lib";
        }
    }
    else if (SystemOS == Linux) {
        return "-lglfw";
    }

    Raise("Invalid OS");
    return "";
}

inline const char* GetCompilerFlags(Compiler compiler, Build_Mode mode) {
    switch (compiler) {
        case MSVC: {
            switch(mode) {
                case Release: return "/O2";
                case Debug:   return "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
            }
        } break;

        case clang: {
            switch(mode) {
                case Release: return "-O2";
                case Debug:   return "-g -D_DEBUG -fsanitize=address -fsanitize=undefined";
            }
        } break;

        default: Raise("Invalid compiler. Only MSVC and clang are supported.");
    }

    return "";
}

struct Build_Configuration {
    Build_Mode mode;
    Compiler compiler;
    Renderer renderer;
    String compiler_path;
    String include;
    String lib;
    String linked_libs;
    String renderer_libs;
    String compiler_flags;
    String pch_source_dir;
    String pch_header_path;
    String pch_source_path;
    String pch_output_file;
    String pch_output_path;
    String metaprogramming_code_path;
    String metaprogramming_output_file;
    String metaprogramming_input_files;
    int metaprogramming_input_file_count;
    bool preprocess;
    bool pch;
};

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

#endif