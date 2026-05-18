#ifndef GAME_BUILD_H
#define GAME_BUILD_H

#include "GamePlatform.h"
#include "Tokenizer.h"


enum build_mode {
    Release,
    Debug,
};

inline build_mode GetBuildMode(token Token) {
    if (Token == "DEBUG") return Debug;
    else                  return Release;
}

enum compiler {
    MSVC,
    clang
};

inline compiler GetCompiler(token Token) {
    if (Token == "MSVC") return MSVC;
    if (Token == "clang") return clang;
    
    Raise("Invalid compiler. Currently only MSVC and clang are supported.");
    return MSVC;
}

enum renderer {
    Renderer_OpenGL,
    Renderer_DirectX,
    Renderer_Vulkan,
};

renderer GetRenderer(token Token) {
    if      (Token == "Vulkan")  return Renderer_Vulkan;
    else if (Token == "DirectX") return Renderer_DirectX;
    else if (Token != "OpenGL")  Raise("Invalid renderer. Currently only OpenGL, DirectX or Vulkan are supported.");
    return Renderer_OpenGL;
}

string GetRendererString(renderer Renderer) {
    switch(Renderer) {
        case Renderer_OpenGL:  return "OPENGL";
        case Renderer_DirectX: return "DIRECTX";
        case Renderer_Vulkan:  return "VULKAN";
    }
    return "";
}

const char* GetRendererLibs(renderer Renderer) {
    if (SystemOS == Windows) {
        switch(Renderer) {
            case Renderer_OpenGL:  return "slang.lib glew32.lib";
            case Renderer_DirectX: return "slang.lib D3d11.lib d3dcompiler.lib";
            case Renderer_Vulkan:  return "slang.lib vulkan-1.lib shaderc_combined.lib";
        }
    }
    else if (SystemOS == Linux) {
        return "-lglfw";
    }

    Raise("Invalid OS");
    return "";
}

const char* GetCompilerFlags(compiler Compiler, build_mode Mode) {
    switch (Compiler) {
        case MSVC: {
            switch(Mode) {
                case Release: return "/O2";
                case Debug:   return "/D _DEBUG /EHsc /MDd /Zi /Od /fsanitize=address";
            }
        } break;

        case clang: {
            switch(Mode) {
                case Release: return "-O2";
                case Debug:   return "-g -D_DEBUG -fsanitize=address -fsanitize=undefined";
            }
        } break;

        default: Raise("Invalid compiler. Only MSVC and clang are supported.");
    }

    return "";
}

struct build_configuration {
    build_mode Mode;
    compiler Compiler;
    renderer Renderer;
    string CompilerPath;
    string Include;
    string Lib;
    string LinkedLibs;
    string RendererLibs;
    string CompilerFlags;
    string PCHOutputFile;
    string PCHOutputPath;
    string MetaprogrammingCodePath;
    string MetaprogrammingOutputFile;
    bool Preprocess;
    bool PCH;
};

build_configuration ReadBuildConfiguration(memory_arena* Arena, string ConfigurationFilePath) {
    build_configuration Config;
    file_info ConfigFileInfo;
    void* ConfigFile = Platform.ReadEntireFile(ConfigurationFilePath.Content, &ConfigFileInfo);
    if (ConfigFile) {
        tokenizer Tokenizer = InitTokenizer(ConfigFile, ConfigFileInfo.Size);
        token Token = GetToken(Tokenizer);
    
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
                    Config.CompilerPath.Content = Tokenizer.At;
                    Config.CompilerPath.Length = ParsePath(Tokenizer.At);
                    for (int i = 0; i < Config.CompilerPath.Length; i++) {
                        Advance(Tokenizer);
                    }
                }

                // Precompiled headers
                else if (Token == "PRECOMPILE_HEADERS") {
                    RequireToken(Tokenizer, Token_Equal);
                    Config.PCH = ParseBool(Tokenizer);
                }
    
                // Include
                else if (Token == "INCLUDE") {
                    RequireToken(Tokenizer, Token_Equal);

                    Config.Include.Length = 0;
                    Config.Include.Content = (const char*)(Arena->Base + Arena->Used);

                    int Index = 0;
                    do {
                        AdvanceUntilNextLine(Tokenizer);
                        char* Pointer = PushArray(Arena, 3, char);
                        Config.Include.Length += 3;
                        if (Config.Compiler == MSVC) {
                            *Pointer++ = '/';
                        }
                        else if (Config.Compiler == clang) {
                            *Pointer++ = '-';
                        }
                        *Pointer++ = 'I';
                        *Pointer++ = '\"';
                        int PathLength = ParsePath(Tokenizer.At);
                        PushArray(Arena, PathLength + 1, char);
                        Config.Include.Length += PathLength + 1;
                        for (int i = 0; i < PathLength; i++) {
                            *Pointer++ = Tokenizer.At[0];
                            Advance(Tokenizer);
                        }
                        *Pointer++ = '\"';
                        if (Tokenizer.At[0] != ';') {
                            PushArray(Arena, 1, char);
                            *Pointer++ = '\0';
                            break;
                        }
                        PushArray(Arena, 1, char);
                        Config.Include.Length += 1;
                        *Pointer++ = ' ';
                    } while (true);
                }
    
                // Lib
                else if (Token == "LIB") {
                    RequireToken(Tokenizer, Token_Equal);

                    Config.Lib.Length = 0;
                    Config.Lib.Content = (char*)(Arena->Base + Arena->Used);

                    do {
                        AdvanceUntilNextLine(Tokenizer);
                        char* Pointer = PushArray(Arena, 10, char);
                        strncpy(Pointer, "/LIBPATH:\"", 10);
                        Config.Lib.Length += 10;
                        int PathLength = ParsePath(Tokenizer.At);
                        Pointer = PushArray(Arena, PathLength + 1, char);
                        Config.Lib.Length += PathLength + 1;
                        for (int i = 0; i < PathLength; i++) {
                            *Pointer++ = Tokenizer.At[0];
                            Advance(Tokenizer);
                        }
                        *Pointer++ = '\"';
                        if (Tokenizer.At[0] != ';') {
                            PushArray(Arena, 1, char);
                            *Pointer++ = '\0';
                            break;
                        }
                        PushArray(Arena, 1, char);
                        Config.Lib.Length += 1;
                        *Pointer++ = ' ';
                    } while (true);
                }

                // Linked libraries
                else if (Token == "LINK") {
                    RequireToken(Tokenizer, Token_Equal);
                    Config.LinkedLibs.Length = 0;
                    Config.LinkedLibs.Content = (char*)(Arena->Base + Arena->Used);
                    do {
                        token Library = RequireToken(Tokenizer, Token_Identifier);
                        RequireToken(Tokenizer, Token_Dot);
                        RequireToken(Tokenizer, "lib");
                        char* LinkedLibraries = PushArray(Arena, Library.Length + 4, char);
                        strncpy(LinkedLibraries, Library.Text, Library.Length + 4);
                        Config.LinkedLibs.Length += Library.Length + 4;
                        if (Tokenizer.At[0] != ';') {
                            break;
                        }
                        RequireToken(Tokenizer, Token_Semicolon);
                        char* Space = PushArray(Arena, 1, char);
                        *Space = ' ';
                        Config.LinkedLibs.Length += 1;
                    } while(true);
                }
    
                // Metaprogramming source file
                else if (Token == "META") {
                    Config.Preprocess = true;
                    RequireToken(Tokenizer, Token_Equal);
                    int MetaprogrammingFilePathLength = ParsePath(Tokenizer.At);
                    char* Pointer = PushArray(Arena, MetaprogrammingFilePathLength + 1, char);
                    strncpy(Pointer, Tokenizer.At, MetaprogrammingFilePathLength);
                    Config.MetaprogrammingCodePath.Length = MetaprogrammingFilePathLength;
                    Config.MetaprogrammingCodePath.Content = Pointer;
                    AdvanceUntilNextLine(Tokenizer);
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
            Config.PCHOutputPath = Format(Arena, "bin" PATH_SEPARATOR "{s}.pch", 1, Config.PCHOutputFile);
            Config.MetaprogrammingOutputFile = "bin" PATH_SEPARATOR "Meta.exe";
        }
        else {
            Config.PCHOutputPath = Format(Arena, "bin" PATH_SEPARATOR "{s}.gch", 1, Config.PCHOutputFile);
            Config.MetaprogrammingOutputFile = "bin" PATH_SEPARATOR "Meta";
        }
    }
    else {
        string Error = Format(Arena, "Build configuration file {s} not found.", 1, ConfigurationFilePath);
        Raise(Error.Content);
    }

    return Config;
}

void LogCompilationResult(memory_arena* Arena, string Name, int32 ExitCode, uint64 Start, uint64 End) {
    log_level Level = ExitCode == 0 ? log_level::Info : log_level::Error;
    string LogString;
    if (ExitCode == 0) {
        LogString = Format(Arena, "{s} code compiled in {f2} milliseconds.", 2, Name, 1000.0f * GetSecondsElapsed(Start, End));
    }
    else { 
        LogString = Format(Arena, "{s} code compilation failed, exit code '{i}'.", 2, Name, ExitCode);
    }
    Log(Level, LogString.Content);
}

process_info CompileMetaprogramming(memory_arena* Arena, build_configuration* Config) {
    Log(log_level::Info, "Compiling metaprogramming code.");
    char* Command = (char*)(Arena->Base + Arena->Used);
    switch(Config->Compiler) {
        case MSVC: {
            Format(Arena,
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
            Format(Arena,
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
void MetaProgram(memory_arena* Arena, build_configuration* Config) {
    file_info MetaprogrammingSource = Platform.GetFileInfo(Config->MetaprogrammingCodePath.Content);
    file_info MetaprogrammingBinary;

    bool MetaprogrammingBinaryExists = Platform.FileExists(Config->MetaprogrammingOutputFile.Content);        
    if (MetaprogrammingBinaryExists) {
        MetaprogrammingBinary = Platform.GetFileInfo(Config->MetaprogrammingOutputFile.Content);
    }
    
    int32 WaitResult = 0;
    uint64 MetaprogrammingCompilationStart = 0;
    uint64 MetaprogrammingCompilationEnd = 0;
    process_info MetaprogrammingCompilation = {};
    if (!MetaprogrammingBinaryExists || MetaprogrammingSource.Timestamp > MetaprogrammingBinary.Timestamp) {
        MetaprogrammingCompilationStart = Platform.GetWallClock();
        MetaprogrammingCompilation = CompileMetaprogramming(Arena, Config);
        WaitResult = Platform.WaitForProcess(&MetaprogrammingCompilation, -1);
        MetaprogrammingCompilationEnd = Platform.GetWallClock();
        LogCompilationResult(Arena, "Metaprogramming", WaitResult, MetaprogrammingCompilationStart, MetaprogrammingCompilationEnd);
    }

    char* MetaCommand = PushArray(Arena, 64, char);
    strncpy(MetaCommand, Config->MetaprogrammingOutputFile.Content, 64);
    uint64 MetaprogrammingExecutionStart = Platform.GetWallClock();
    process_info MetaprogrammingExecution = Platform.RunCommand(MetaCommand);
    WaitResult = Platform.WaitForProcess(&MetaprogrammingExecution, -1);
    if (WaitResult >= 0) {
        uint64 End = Platform.GetWallClock();
        float Time = GetSecondsElapsed(MetaprogrammingExecutionStart, End);
        log_level Level = WaitResult > 0 ? log_level::Error : log_level::Info;
        string LogText;
        if (WaitResult == 0) {
            LogText = Format(Arena, "Metaprogramming executed in {f2} milliseconds.", 1, 1000.0f * Time);
        }
        else {
            LogText = Format(Arena, "Metaprogramming execution failed with code '{i}'", 1, WaitResult);
        }
        Log(Level, LogText.Content);
    }
}

process_info CompilePlatformLayer(memory_arena* Arena, build_configuration* Config) {
    char* Command = (char*)(Arena->Base + Arena->Used);
    switch (Config->Compiler) {
        case MSVC: {
            
            string CommandString = Format(Arena,
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
            Format(Arena,
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

process_info CompileGameLibrary(memory_arena* Arena, build_configuration* Config) {
    char* Command = (char*)(Arena->Base + Arena->Used);
    switch(Config->Compiler) {
        case MSVC: {
            Format(Arena,
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
            Format(Arena,
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

process_info CompileGameLibraryHot(memory_arena* Arena, build_configuration* Config) {
#if _WIN32
    int nHotReloads = 0;
    string PDBFile = Format(Arena, "bin\\GameLibrary{i}.pdb", 1, nHotReloads);
    bool Exists;
    do {
        Exists = Platform.FileExists(PDBFile.Content);
        if (Exists) {
            nHotReloads += 1;
            PopArray(Arena, PDBFile.Length + 1, char);
            PDBFile = Format(Arena, "bin\\GameLibrary{i}.pdb", 1, nHotReloads);
        }
    } while(Exists);
    char* Command = (char*)(Arena->Base + Arena->Used);
    Format(Arena,
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
    char* Command = (char*)(Arena->Base + Arena->Used);
    Format(Arena,
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