#ifndef GAME_BUILD_H
#define GAME_BUILD_H

#include "GamePlatform.h"
#include "Tokenizer.h"


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

Renderer GetRenderer(Token token) {
    if      (token == "Vulkan")  return Renderer::Vulkan;
    else if (token == "DirectX") return Renderer::DirectX;
    else if (token != "OpenGL")  Raise("Invalid renderer. Currently only OpenGL, DirectX or Vulkan are supported.");
    return Renderer::OpenGL;
}

String GetRendererString(Renderer renderer) {
    switch(renderer) {
        case Renderer::OpenGL:  return "OPENGL";
        case Renderer::DirectX: return "DIRECTX";
        case Renderer::Vulkan:  return "VULKAN";
    }
    return "";
}

const char* GetRendererLibs(Renderer renderer) {
    if (SystemOS == Windows) {
        switch(renderer) {
            case Renderer::OpenGL:  return "slang.lib glew32.lib";
            case Renderer::DirectX: return "slang.lib D3d11.lib d3dcompiler.lib";
            case Renderer::Vulkan:  return "slang.lib vulkan-1.lib shaderc_combined.lib";
        }
    }
    else if (SystemOS == Linux) {
        return "-lglfw";
    }

    Raise("Invalid OS");
    return "";
}

const char* GetCompilerFlags(Compiler compiler, Build_Mode mode) {
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
    renderer renderer;
    String compiler_path;
    String include;
    String lib;
    String linked_libs;
    String renderer_libs;
    String compiler_flags;
    String pch_output_file;
    String pch_output_path;
    String metaprogramming_code_path;
    String metaprogramming_output_file;
    String metaprogramming_input_files;
    int metaprogramming_input_file_count;
    bool preprocess;
    bool pch;
};

Build_Configuration ReadBuildConfiguration(Arena* arena, String config_file_path) {
    Build_Configuration config;
    File_Info config_file_info;
    void* config_file = Platform.ReadEntireFile(config_file_path.content, &config_file_info);
    if (config_file) {
        Tokenizer tokenizer = InitTokenizer(config_file, config_file_info.Size);
        Token token = GetToken(Tokenizer);
    
        while(Token.Type != Token_End) {
            // Comments
            if (Token.Type == Token_Pound) {
                AdvanceUntilNextLine(Tokenizer);
            }
    
            else if (Token.Type == Token_Identifier) {
                // Mode
                if (Token == "MODE") {
                    RequireToken(Tokenizer, Token_Equal);
                    token ModeToken = RequireToken(Tokenizer, Token_Identifier);
                    Config.Mode = GetBuildMode(ModeToken);
                }
    
                // Compiler
                else if (Token == "COMPILER") {
                    RequireToken(Tokenizer, Token_Equal);
                    Config.Compiler = GetCompiler(RequireToken(Tokenizer, Token_Identifier));
                }
    
                // Compiler path
                else if (Token == "COMPILER_PATH") {
                    RequireToken(Tokenizer, Token_Equal);
                    Config.CompilerPath.content = Tokenizer.At;
                    Config.CompilerPath.length = ParsePath(Tokenizer.At);
                    Advance(Tokenizer, Config.CompilerPath.length);
                }

                // Precompiled headers
                else if (Token == "PRECOMPILE_HEADERS") {
                    RequireToken(Tokenizer, Token_Equal);
                    Config.PCH = ParseBool(Tokenizer);
                }
    
                // Include
                else if (Token == "INCLUDE") {
                    RequireToken(Tokenizer, Token_Equal);

                    Config.Include.length = 0;
                    Config.Include.content = (const char*)(arena->base + arena->used);

                    int Index = 0;
                    do {
                        AdvanceUntilNextLine(Tokenizer);
                        char* Pointer = PushArray(arena, 3, char);
                        Config.Include.length += 3;
                        if (Config.Compiler == MSVC) {
                            *Pointer++ = '/';
                        }
                        else if (Config.Compiler == clang) {
                            *Pointer++ = '-';
                        }
                        *Pointer++ = 'I';
                        *Pointer++ = '\"';
                        int PathLength = ParsePath(Tokenizer.At);
                        PushArray(arena, PathLength + 1, char);
                        Config.Include.length += PathLength + 1;
                        strncpy(Pointer, Tokenizer.At, PathLength);
                        Advance(Tokenizer, PathLength);
                        Pointer += PathLength;
                        *Pointer++ = '\"';
                        if (Tokenizer.At[0] != ';') {
                            PushArray(arena, 1, char);
                            *Pointer++ = '\0';
                            break;
                        }
                        PushArray(arena, 1, char);
                        Config.Include.length += 1;
                        *Pointer++ = ' ';
                    } while (true);
                }
    
                // Lib
                else if (Token == "LIB") {
                    RequireToken(Tokenizer, Token_Equal);

                    Config.Lib.length = 0;
                    Config.Lib.content = (char*)(arena->base + arena->used);

                    do {
                        AdvanceUntilNextLine(Tokenizer);
                        char* Pointer = PushArray(arena, 10, char);
                        strncpy(Pointer, "/LIBPATH:\"", 10);
                        Config.Lib.length += 10;
                        int PathLength = ParsePath(Tokenizer.At);
                        Pointer = PushArray(arena, PathLength + 1, char);
                        Config.Lib.length += PathLength + 1;
                        strncpy(Pointer, Tokenizer.At, PathLength);
                        Advance(Tokenizer, PathLength);
                        Pointer += PathLength;
                        *Pointer++ = '\"';
                        if (Tokenizer.At[0] != ';') {
                            PushArray(arena, 1, char);
                            *Pointer++ = '\0';
                            break;
                        }
                        PushArray(arena, 1, char);
                        Config.Lib.length += 1;
                        *Pointer++ = ' ';
                    } while (true);
                }

                // Linked libraries
                else if (Token == "LINK") {
                    RequireToken(Tokenizer, Token_Equal);
                    Config.LinkedLibs.length = 0;
                    Config.LinkedLibs.content = (char*)(arena->base + arena->used);
                    do {
                        token Library = RequireToken(Tokenizer, Token_Identifier);
                        RequireToken(Tokenizer, Token_Dot);
                        RequireToken(Tokenizer, "lib");
                        char* LinkedLibraries = PushArray(arena, Library.length + 4, char);
                        strncpy(LinkedLibraries, Library.Text, Library.length + 4);
                        Config.LinkedLibs.length += Library.length + 4;
                        if (Tokenizer.At[0] != ';') {
                            break;
                        }
                        RequireToken(Tokenizer, Token_Semicolon);
                        char* Space = PushArray(arena, 1, char);
                        *Space = ' ';
                        Config.LinkedLibs.length += 1;
                    } while(true);
                }
    
                // Metaprogramming source file
                else if (Token == "META") {
                    RequireToken(Tokenizer, Token_Equal);
                    Config.Preprocess = true;
                    int MetaprogrammingFilePathLength = ParsePath(Tokenizer.At);
                    char* Pointer = PushArray(arena, MetaprogrammingFilePathLength + 1, char);
                    strncpy(Pointer, Tokenizer.At, MetaprogrammingFilePathLength);
                    Config.MetaprogrammingCodePath.length = MetaprogrammingFilePathLength;
                    Config.MetaprogrammingCodePath.content = Pointer;
                    AdvanceUntilNextLine(Tokenizer);
                }

                // Metaprogramming input files
                else if (Token == "META_FILES") {
                    RequireToken(Tokenizer, Token_Equal);
                    AdvanceWhitespaceAndComments(Tokenizer);
                    Config.MetaprogrammingInputFiles.content = Tokenizer.At;
                    Config.MetaprogrammingInputFiles.length = 0;
                    Config.nMetaprogrammingInputFiles = 0;
                    do {
                        AdvanceWhitespaceAndComments(Tokenizer);
                        int PathLength = ParsePath(Tokenizer.At);
                        Advance(Tokenizer, PathLength);
                        Config.nMetaprogrammingInputFiles += 1;
                        const char* End = Tokenizer.At;
                        if (Tokenizer.At[0] != ';') {
                            Config.MetaprogrammingInputFiles.length = End - Config.MetaprogrammingInputFiles.content;
                            break;
                        }
                        Token = RequireToken(Tokenizer, Token_Semicolon);
                    } while(Token.Type == Token_Semicolon);
                }
    
                // Renderer
                else if (Token == "RENDERER") {
                    RequireToken(Tokenizer, Token_Equal);
                    token RendererToken = RequireToken(Tokenizer, Token_Identifier);
                    Config.Renderer = GetRenderer(RendererToken);
                    Config.RendererLibs = GetRendererLibs(Config.Renderer);
                }
            }
    
            Token = GetToken(Tokenizer);
        }

        Config.CompilerFlags = GetCompilerFlags(Config.Compiler, Config.Mode);
        Config.PCHOutputFile = Config.Mode == Debug ? "debug_pch" : "pch";
        if (SystemOS == Windows) {
            Config.PCHOutputPath = Format(arena, "bin" PATH_SEPARATOR "{s}.pch", 1, Config.PCHOutputFile);
            Config.MetaprogrammingOutputFile = "bin" PATH_SEPARATOR "Meta.exe";
        }
        else {
            Config.PCHOutputPath = Format(arena, "bin" PATH_SEPARATOR "{s}.gch", 1, Config.PCHOutputFile);
            Config.MetaprogrammingOutputFile = "bin" PATH_SEPARATOR "Meta";
        }
    }
    else {
        String Error = Format(arena, "Build configuration file {s} not found.", 1, ConfigurationFilePath);
        Raise(Error.content);
    }

    return Config;
}

void LogCompilationResult(arena* arena, String Name, int32 ExitCode, uint64 Start, uint64 End) {
    log_level Level = ExitCode == 0 ? log_level::Info : log_level::Error;
    String LogString;
    if (ExitCode == 0) {
        LogString = Format(arena, "{s} code compiled in {f2} milliseconds.", 2, Name, 1000.0f * GetSecondsElapsed(Start, End));
    }
    else { 
        LogString = Format(arena, "{s} code compilation failed, exit code '{i}'.", 2, Name, ExitCode);
    }
    Log(Level, LogString.content);
}

process_info CompileMetaprogramming(arena* arena, build_configuration* Config) {
    Log(log_level::Info, "Compiling metaprogramming code.");
    char* Command = (char*)(arena->base + arena->used);
    switch(Config->Compiler) {
        case MSVC: {
            Format(arena,
                "{s} /std:c++20 /nologo /W0 {s} /Fo\"bin\\Meta.obj\" /Fd\"bin\\Meta.pdb\" {s} {s} "
                "/link {s} /OUT:\"bin\\Meta.exe\" /PDB:\"bin\\Meta.pdb\"", 
                5,
                Config->CompilerPath, 
                Config->Include, 
                Config->MetaprogrammingCodePath, 
                Config->CompilerFlags, 
                Config->Lib
            );
        } break;

        case clang: {
            Format(arena,
                "{s} -std=c++20 {s} {s} -o bin/Meta", 
                4,
                Config->CompilerPath, 
                Config->Include, 
                Config->MetaprogrammingCodePath, 
                Config->CompilerFlags
            );
        } break;

        default: Raise("Invalid compiler. Only MSVC and clang are supported for now.");
    }
    return Platform.RunCommand(Command);
}

/* Compiles the meta-program (if needed) and runs it.*/
void MetaProgram(arena* arena, build_configuration* Config) {
    File_Info MetaprogrammingSource = Platform.GetFileInfo(Config->MetaprogrammingCodePath.content);
    File_Info MetaprogrammingBinary;

    bool MetaprogrammingBinaryExists = Platform.FileExists(Config->MetaprogrammingOutputFile.content);        
    if (MetaprogrammingBinaryExists) {
        MetaprogrammingBinary = Platform.GetFileInfo(Config->MetaprogrammingOutputFile.content);
    }
    
    int32 WaitResult = 0;
    uint64 MetaprogrammingCompilationStart = 0;
    uint64 MetaprogrammingCompilationEnd = 0;
    process_info MetaprogrammingCompilation = {};
    if (!MetaprogrammingBinaryExists || MetaprogrammingSource.Timestamp > MetaprogrammingBinary.Timestamp) {
        MetaprogrammingCompilationStart = Platform.GetWallClock();
        MetaprogrammingCompilation = CompileMetaprogramming(arena, Config);
        WaitResult = Platform.WaitForProcess(&MetaprogrammingCompilation, -1);
        MetaprogrammingCompilationEnd = Platform.GetWallClock();
        LogCompilationResult(arena, "Metaprogramming", WaitResult, MetaprogrammingCompilationStart, MetaprogrammingCompilationEnd);
    }

    char* MetaCommand = PushArray(arena, 64, char);
    strncpy(MetaCommand, Config->MetaprogrammingOutputFile.content, 64);
    uint64 MetaprogrammingExecutionStart = Platform.GetWallClock();
    process_info MetaprogrammingExecution = Platform.RunCommand(MetaCommand);
    WaitResult = Platform.WaitForProcess(&MetaprogrammingExecution, -1);
    if (WaitResult >= 0) {
        uint64 End = Platform.GetWallClock();
        float Time = GetSecondsElapsed(MetaprogrammingExecutionStart, End);
        log_level Level = WaitResult > 0 ? log_level::Error : log_level::Info;
        String LogText;
        if (WaitResult == 0) {
            LogText = Format(arena, "Metaprogramming executed in {f2} milliseconds.", 1, 1000.0f * Time);
        }
        else {
            LogText = Format(arena, "Metaprogramming execution failed with code '{i}'", 1, WaitResult);
        }
        Log(Level, LogText.content);
    }
}

process_info CompilePlatformLayer(arena* arena, build_configuration* Config) {
    char* Command = (char*)(arena->base + arena->used);
    switch (Config->Compiler) {
        case MSVC: {
            
            String CommandString = Format(arena,
                "{s} /std:c++20 /nologo /W0 "
                "GamePlatform\\Windows\\Win32PlatformLayer.cpp bin\\{s}.obj " 
                "/D GAME_RENDER_API_{s} {s} "
                "/Fe\"bin\\RunGame.exe\" "
                "/Fo\"bin\\Win32PlatformLayer.obj\" "
                "/Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" {s} "
                "/link {s} {s} {s} "
                "GamePlatform\\Windows\\Win32PlatformLayer.res /MACHINE:X64",
                10,
                Config->CompilerPath, 
                Config->PCHOutputFile, 
                GetRendererString(Config->Renderer), 
                Config->CompilerFlags, 
                Config->PCHOutputFile, 
                Config->PCHOutputFile, 
                Config->Include, 
                Config->Lib,
                Config->LinkedLibs,
                Config->RendererLibs
            );
        } break;

        case clang: {
            Format(arena,
                "{s} -std=c++20 -fPIC {s} {s} -include-pch bin/{s}.gch "
                "LinuxPlatformLayer/LinuxPlatformLayer.cpp {s} -o bin/RunGame && chmod +x bin/RunGame",
                5,
                Config->CompilerPath,
                Config->CompilerFlags,
                Config->Include,
                Config->PCHOutputFile,
                Config->RendererLibs
            );
        };
    }

    return Platform.RunCommand(Command);
}

process_info CompileGameLibrary(arena* arena, build_configuration* Config) {
    char* Command = (char*)(arena->base + arena->used);
    switch(Config->Compiler) {
        case MSVC: {
            Format(arena,
                "{s} /std:c++20 /W0 /nologo /D GAMELIBRARY_EXPORTS "
                "GameLibrary\\GameLibrary.cpp {s} {s} /Fo\"bin\\GameLibrary.obj\" "
                "/Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" "
                "/link {s} bin\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
                "/PDB:\"bin\\GameLibrary.pdb\" "
                "/ILK:\"bin\\GameLibrary.ilk\" /OUT:\"bin\\GameLibrary.dll\"",
                7,
                Config->CompilerPath, 
                Config->CompilerFlags, 
                Config->Include, 
                Config->PCHOutputFile, 
                Config->PCHOutputFile, 
                Config->Lib, 
                Config->PCHOutputFile
            );
        } break;

        case clang: {
            Format(arena,
                "{s} -std=c++20 -shared -fPIC {s} {s} -include-pch bin/{s}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
                4,
                Config->CompilerPath,
                Config->CompilerFlags,
                Config->Include,
                Config->PCHOutputFile
            );
        } break;
    }

    return Platform.RunCommand(Command);
}

process_info CompileGameLibraryHot(arena* arena, build_configuration* Config) {
#if _WIN32
    int nHotReloads = 0;
    String PDBFile = Format(arena, "bin\\GameLibrary{i}.pdb", 1, nHotReloads);
    bool Exists;
    do {
        Exists = Platform.FileExists(PDBFile.content);
        if (Exists) {
            nHotReloads += 1;
            PopArray(arena, PDBFile.length + 1, char);
            PDBFile = Format(arena, "bin\\GameLibrary{i}.pdb", 1, nHotReloads);
        }
    } while(Exists);
    char* Command = (char*)(arena->base + arena->used);
    Format(arena,
        "{s} /std:c++20 /W0 /nologo /D GAMELIBRARY_EXPORTS GameLibrary\\GameLibrary.cpp {s} {s} "
        "/Fo\"bin\\GameLibrary.obj\" /Fd\"bin\\{s}.pdb\" /Yu\"pch.h\" /Fp\"bin\\{s}.pch\" "
        "/link {s} bin\\{s}.obj /DLL /IMPLIB:\"bin\\GameLibrary.lib\" "
        "/PDB:\"{s}\" /ILK:\"bin\\GameLibrary.ilk\" "
        "/OUT:\"bin\\GameLibrary.dll\"",
        8,
        Config->CompilerPath, 
        Config->CompilerFlags, 
        Config->Include, 
        Config->PCHOutputFile, 
        Config->PCHOutputFile, 
        Config->Lib, 
        Config->PCHOutputFile, 
        PDBFile
    );
#else
    char* Command = (char*)(arena->base + arena->used);
    Format(arena,
        "{s} -std=c++20 -shared -fPIC {s} {s} -include-pch bin/{s}.gch GameLibrary/GameLibrary.cpp -o bin/GameLibrary.so",
        4,
        Config->CompilerPath,
        Config->CompilerFlags,
        Config->Include,
        Config->PCHOutput
    );
#endif
    return Platform.RunCommand(Command);
}

#endif